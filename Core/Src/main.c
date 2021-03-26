/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "ltdc.h"
#include "quadspi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <usbd_core.h>
#include "apploader.h"
#include "config.h"
#include "gt911.h"
#include "int_flash.h"
#include "log.h"
#include "pump.h"
#include "pump_power.h"
#include "sst26_flash.h"
#include "tft.h"
#include "terminal.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAGIC_CONST (uint32_t)0xA5B6C7E8
#define BackupRAM_BASE 0x38800000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void printHex(const char *prefix, size_t length, uint8_t *buffer) {
	printf(prefix);
	for (size_t i = 0; i < length; i++) {
		if (i > 0) {
			if (i % 8 == 0) {
				printf("\n");
			} else {
				printf(" ");
			}
		}
		printf("0x%02X", buffer[i]);
	}
	printf("\n");
}

void enableBackupRAM() {
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_BKPRAM_CLK_ENABLE();
	volatile uint32_t i = 0xffff;
	while (i != 0)
		i--;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

//	SCB->CACR |= 1<<2;
	enableBackupRAM();
	__IO uint32_t *load_app_flag = (__IO uint32_t*)(BackupRAM_BASE);

	if (*load_app_flag == MAGIC_CONST) {
		*load_app_flag = (uint32_t) 0;
		AppLoader_load_application();
	}

	HAL_StatusTypeDef status = HAL_ERROR;

	/* USER CODE END 1 */

	/* MPU Configuration--------------------------------------------------------*/
	MPU_Config();

	/* Enable I-Cache---------------------------------------------------------*/
	SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	SCB_EnableDCache();

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_LTDC_Init();
	MX_TIM3_Init();
	MX_FMC_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_QUADSPI_Init();
	MX_USB_DEVICE_Init();
	MX_FATFS_Init();
	/* USER CODE BEGIN 2 */
	USBD_Stop(&hUsbDeviceFS);

	struct tTftFramebuffer framebuffer = TFT_init_framebuffer(&hltdc);
	terminal_init(&framebuffer);

	TFT_Set_brightness(256);

	HAL_GPIO_WritePin(DISP_EN_GPIO_Port, DISP_EN_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(DISP_EN_GPIO_Port, DISP_EN_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	log_debug("Debug log");
	log_info("Info log");
	log_warn("Warn log");
	log_error("Error log");

	SST26_config.hqspi = &hqspi;
	SST26_config.timeout = 1000;

	status = SST26_init();
	if (status != HAL_OK) {
		log_error("Cannot initialize SST26 chip (external flash): 0x%02X",
				status);
		Error_Handler();
	}

	gt911_irq = false;
	if (GT911_Init() != HAL_OK) {
		printf("Touch init error. Reset the device\n");
		while (1)
			;
	}
	HAL_Delay(100);
	gt911_irq = true;

	FATFS FatFs;
	FRESULT FatResult;

	FatResult = f_mount(&FatFs, "", 1);
	if (FatResult != FR_OK) {
		log_error("f_mount error: 0x%02X", FatResult);
		Error_Handler();
	}
	struct tBootloaderConfig *config = BtldrConfig_Get();
	log_info("Touch the screen within %d seconds to activate flash drive",
			config->wait_for_flash_activation_confirmation_ms / 1000);
	uint32_t timeout_at = HAL_GetTick()
			+ config->wait_for_flash_activation_confirmation_ms;
	uint32_t printAt = HAL_GetTick() + 1000;
	while (HAL_GetTick() < timeout_at) {
		if (gt911.TouchCount > 0) {
			log_info("Activating flash drive in %d seconds",
					config->activate_flash_drive_timeout_ms / 1000);
			log_info("Reboot device when finished");
			HAL_Delay(config->activate_flash_drive_timeout_ms);
			f_mount(NULL, "", 1);
			USBD_Start(&hUsbDeviceFS);
			while (1)
				;
		}
		if (HAL_GetTick() > printAt) {
			printAt = HAL_GetTick() + 1000;
			log_info("%ld  ", (timeout_at - HAL_GetTick()) / 1000);
		}
	}
	log_info(
			"Bootloader was not activated. Running main application in %d seconds",
			config->activate_main_program_timeout_ms / 1000);
	HAL_Delay(config->activate_main_program_timeout_ms);

	log_info("This is run #%llu", BtldrConfig_incrementRuns());

	log_info("listing files in root");
	DIR dir;
	FatResult = f_opendir(&dir, "/");
	if (FatResult != FR_OK) {
		log_error("f_opendir error: 0x%02X", FatResult);
		Error_Handler();
	}

	FILINFO fileinfo;
	for (;;) {
		FatResult = f_readdir(&dir, &fileinfo);
		if (FatResult != FR_OK) {
			log_error("f_readdir error: 0x%02X", FatResult);
			Error_Handler();
		}
		if (strlen(fileinfo.fname) == 0) {
			break;
		}

		log_info("%c------ %-10lu %s", ((fileinfo.fattrib & AM_DIR) ? 'd':' '),
				fileinfo.fsize, fileinfo.fname);

	}

	f_closedir(&dir);

	log_info("Checking firmware");
	enum APL_RES aplres = AppLoader_check_firmware();
	switch (aplres) {
	case APPLOADER_OK:
	default:
		break;
	case APPLOADER_NO_FIRMWARE:
		log_error("Application firmware not found: APP.BIN");
		break;
	case APPLOADER_NO_CRC_FILE:
		log_error("Application checksum not found: APP.CRC");
		break;
	case APPLOADER_CRC_MISMATCH:
		log_error("Application checksum mismatch: APP.BIN <-> APP.CRC");
		break;
	}

	if (APPLOADER_OK == aplres) {
		log_info("New firmware check ok: code 0x%02X", aplres);
		log_info("Updating firmware...");
		if (!AppLoader_update_firmware()) {
			log_error("Firmware update failed");
			Error_Handler();
		}
		log_info("Verifying...");
		if (!AppLoader_verify_firmware()) {
			log_error("Firmware verification failed!");
			Error_Handler();
		}
	}

	// Deinit what we can
	f_mount(NULL, "", 1);

	log_info("Loading application...");
	*load_app_flag = MAGIC_CONST;
	HAL_Delay(1000);
	NVIC_SystemReset();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		HAL_Delay(100000);
		printf("[%8ld] Hello, world!\n", HAL_GetTick());
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}
	/** Macro to configure the PLL clock source
	 */
	__HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48
			| RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 2;
	RCC_OscInitStruct.PLL.PLLN = 64;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == TOUCH_IRQ_Pin) {
		if (gt911_irq) {
			if (GT911_Scan(10) == HAL_ERROR) {
				GT911_Init();
				return;
			}
			GT911_CopyShadow();
		}
	}
}

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {
	MPU_Region_InitTypeDef MPU_InitStruct = { 0 };

	/* Disables the MPU */
	HAL_MPU_Disable();
	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.BaseAddress = 0x60000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
	MPU_InitStruct.SubRegionDisable = 0x0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	MPU_InitStruct.BaseAddress = 0x30000000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/** Initializes and configures the Region and the memory to be protected
	 */
	MPU_InitStruct.Number = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress = 0x38800000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4KB;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	/* Enables the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	log_error("Error handler");
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	log_error("Wrong parameters value: file %s on line %d\r\n", file, line);
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
