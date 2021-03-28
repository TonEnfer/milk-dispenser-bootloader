/*
 * Loader_Func.c
 *
 *  Created on: 28 мар. 2021 г.
 *      Author: tonik
 */

#include "stm32h7xx.h"

#define VECT_TAB_OFFSET  0x00000000UL
#define DATA_IN_D2_SRAM

__attribute__((section(".loader_text"))) void Loader_SystemInit() {
#if defined (DATA_IN_D2_SRAM)
	__IO uint32_t tmpreg;
#endif /* DATA_IN_D2_SRAM */

	/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << (10 * 2)) | (3UL << (11 * 2))); /* set CP10 and CP11 Full Access */
#endif
	/* Reset the RCC clock configuration to the default reset state ------------*/

	/* Increasing the CPU frequency */
	if (FLASH_LATENCY_DEFAULT > (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
		/* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
		MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY,
				(uint32_t)(FLASH_LATENCY_DEFAULT));
	}

	/* Set HSION bit */
	RCC->CR |= RCC_CR_HSION;

	/* Reset CFGR register */
	RCC->CFGR = 0x00000000;

	/* Reset HSEON, HSECSSON, CSION, HSI48ON, CSIKERON, PLL1ON, PLL2ON and PLL3ON bits */
	RCC->CR &= 0xEAF6ED7FU;

	/* Decreasing the number of wait states because of lower CPU frequency */
	if (FLASH_LATENCY_DEFAULT < (READ_BIT((FLASH->ACR), FLASH_ACR_LATENCY))) {
		/* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
		MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY,
				(uint32_t)(FLASH_LATENCY_DEFAULT));
	}

#if defined(D3_SRAM_BASE)
	/* Reset D1CFGR register */
	RCC->D1CFGR = 0x00000000;

	/* Reset D2CFGR register */
	RCC->D2CFGR = 0x00000000;

	/* Reset D3CFGR register */
	RCC->D3CFGR = 0x00000000;
#else
	/* Reset CDCFGR1 register */
	RCC->CDCFGR1 = 0x00000000;

	/* Reset CDCFGR2 register */
	RCC->CDCFGR2 = 0x00000000;

	/* Reset SRDCFGR register */
	RCC->SRDCFGR = 0x00000000;
#endif
	/* Reset PLLCKSELR register */
	RCC->PLLCKSELR = 0x02020200;

	/* Reset PLLCFGR register */
	RCC->PLLCFGR = 0x01FF0000;
	/* Reset PLL1DIVR register */
	RCC->PLL1DIVR = 0x01010280;
	/* Reset PLL1FRACR register */
	RCC->PLL1FRACR = 0x00000000;

	/* Reset PLL2DIVR register */
	RCC->PLL2DIVR = 0x01010280;

	/* Reset PLL2FRACR register */

	RCC->PLL2FRACR = 0x00000000;
	/* Reset PLL3DIVR register */
	RCC->PLL3DIVR = 0x01010280;

	/* Reset PLL3FRACR register */
	RCC->PLL3FRACR = 0x00000000;

	/* Reset HSEBYP bit */
	RCC->CR &= 0xFFFBFFFFU;

	/* Disable all interrupts */
	RCC->CIER = 0x00000000;

#if (STM32H7_DEV_ID == 0x450UL)
	/* dual core CM7 or single core line */
	if ((DBGMCU->IDCODE & 0xFFFF0000U) < 0x20000000U) {
		/* if stm32h7 revY*/
		/* Change  the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
		*((__IO uint32_t*) 0x51008108) = 0x000000001U;
	}
#endif

#if defined (DATA_IN_D2_SRAM)
	/* in case of initialized data in D2 SRAM (AHB SRAM) , enable the D2 SRAM clock (AHB SRAM clock) */
#if defined(RCC_AHB2ENR_D2SRAM3EN)
	RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN
			| RCC_AHB2ENR_D2SRAM3EN);
#elif defined(RCC_AHB2ENR_D2SRAM2EN)
  RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN);
#else
  RCC->AHB2ENR |= (RCC_AHB2ENR_AHBSRAM1EN | RCC_AHB2ENR_AHBSRAM2EN);
#endif /* RCC_AHB2ENR_D2SRAM3EN */

	tmpreg = RCC->AHB2ENR;
	(void) tmpreg;
#endif /* DATA_IN_D2_SRAM */

#if defined(DUAL_CORE) && defined(CORE_CM4)
	/* Configure the Vector Table location add offset address for cortex-M4 ------------------*/
#ifdef VECT_TAB_SRAM
	SCB->VTOR = D2_AXISRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
	SCB->VTOR = FLASH_BANK2_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif /* VECT_TAB_SRAM */

#else

	/*
	 * Disable the FMC bank1 (enabled after reset).
	 * This, prevents CPU speculation access on this bank which blocks the use of FMC during
	 * 24us. During this time the others FMC master (such as LTDC) cannot use it!
	 */
	FMC_Bank1_R->BTCR[0] = 0x000030D2;

	/* Configure the Vector Table location add offset address for cortex-M7 ------------------*/
#ifdef VECT_TAB_SRAM
  SCB->VTOR = D1_AXISRAM_BASE  | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal AXI-RAM */
#else
	SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif

#endif /*DUAL_CORE && CORE_CM4*/
}

__attribute__((section(".loader_text"))) void Loader_CopyCodeToRam(uint32_t *s, uint32_t* e, uint32_t *r) {

	for (; s != e; s++) {
		*r = *s;
		r++;
	}
}
