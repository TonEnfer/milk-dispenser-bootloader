/**
  ******************************************************************************
  * @file      startup_stm32h743xx.s
  * @author    MCD Application Team
  * @brief     STM32H743xx Devices vector table for GCC based toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m7
  .fpu softvfp
  .thumb

.global  _g_pfnVectors
.global  loader_Default_Handler

.word _estack

.word _sapp
.word _eapp
.word _isr_table

.word _sisr;
.word _eisr;
.word _aisr;

.word _stxt;
.word _etxt;
.word _atxt;

.word _srodat
.word _erodat
.word _arodat

.word _sextab
.word _eextab
.word _aextab

.word _sarm
.word _earm
.word _aarm

.word _spreinitArray
.word _epreinitArray
.word _apreinitArray

.word _sinitArray
.word _einitArray
.word _ainitArray

.word _sfiniArray
.word _efiniArray
.word _afiniArray

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

    .section  .loader_text.loader_Reset_Handler
  .weak  loader_Reset_Handler

  .type  loader_Reset_Handler, %function
loader_Reset_Handler:
  ldr   sp, =_estack      /* set stack pointer */

/* Call the clock system initialization function.*/
  bl  Loader_SystemInit

  ldr r0, =_sisr
  ldr r1, =_eisr
  ldr r2, =_aisr

  bl Loader_CopyCodeToRam

  ldr r0, =_stxt
  ldr r1, =_etxt
  ldr r2, =_atxt

  bl Loader_CopyCodeToRam

  ldr r0, =_srodat
  ldr r1, =_erodat
  ldr r2, =_arodat

  bl Loader_CopyCodeToRam

  ldr r0, =_sextab
  ldr r1, =_eextab
  ldr r2, =_aextab

  bl Loader_CopyCodeToRam

  ldr r0, =_sarm
  ldr r1, =_earm
  ldr r2, =_aarm

  bl Loader_CopyCodeToRam

  ldr r0, =_spreinitArray
  ldr r1, =_epreinitArray
  ldr r2, =_apreinitArray

  bl Loader_CopyCodeToRam

  ldr r0, =_sinitArray
  ldr r1, =_einitArray
  ldr r2, =_ainitArray

  bl Loader_CopyCodeToRam

  ldr r0, =_sfiniArray
  ldr r1, =_efiniArray
  ldr r2, =_afiniArray

  bl Loader_CopyCodeToRam

  ldr r3, =_aisr
  ldr r3, [r3, #4]
  bx r3  @ indirect register sibling call
.size  loader_Reset_Handler, .-loader_Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None
 * @retval None
*/
    .section  .loader_text.loader_Default_Handler,"ax",%progbits
loader_Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  loader_Default_Handler, .-loader_Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
*******************************************************************************/
   .section  .loader_isr_vector,"a",%progbits
  .type  _g_pfnVectors, %object
  .size  _g_pfnVectors, .-_g_pfnVectors


_g_pfnVectors:
  .word  _estack
  .word  loader_Reset_Handler

  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler

  /* External Interrupts */
  .word     0                   /* Window WatchDog              */
  .word     0                /* PVD/AVD through EXTI Line detection */
  .word     0             /* Tamper and TimeStamps through the EXTI line */
  .word     0               /* RTC Wakeup through the EXTI line */
  .word     0                  /* FLASH                        */
  .word     0                    /* RCC                          */
  .word     0                  /* EXTI Line0                   */
  .word     0                  /* EXTI Line1                   */
  .word     0                  /* EXTI Line2                   */
  .word     0                  /* EXTI Line3                   */
  .word     0                  /* EXTI Line4                   */
  .word     0           /* DMA1 Stream 0                */
  .word     0           /* DMA1 Stream 1                */
  .word     0           /* DMA1 Stream 2                */
  .word     0           /* DMA1 Stream 3                */
  .word     0           /* DMA1 Stream 4                */
  .word     0           /* DMA1 Stream 5                */
  .word     0           /* DMA1 Stream 6                */
  .word     0                    /* ADC1, ADC2 and ADC3s         */
  .word     0             /* FDCAN1 interrupt line 0      */
  .word     0             /* FDCAN2 interrupt line 0      */
  .word     0             /* FDCAN1 interrupt line 1      */
  .word     0             /* FDCAN2 interrupt line 1      */
  .word     0                /* External Line[9:5]s          */
  .word     0               /* TIM1 Break interrupt         */
  .word     0                /* TIM1 Update interrupt        */
  .word     0           /* TIM1 Trigger and Commutation interrupt */
  .word     0                /* TIM1 Capture Compare         */
  .word     0                   /* TIM2                         */
  .word     0                   /* TIM3                         */
  .word     0                   /* TIM4                         */
  .word     0                /* I2C1 Event                   */
  .word     0                /* I2C1 Error                   */
  .word     0                /* I2C2 Event                   */
  .word     0                /* I2C2 Error                   */
  .word     0                   /* SPI1                         */
  .word     0                   /* SPI2                         */
  .word     0                 /* USART1                       */
  .word     0                 /* USART2                       */
  .word     0                 /* USART3                       */
  .word     0              /* External Line[15:10]s        */
  .word     0              /* RTC Alarm (A and B) through EXTI Line */
  .word     0                                 /* Reserved                     */
  .word     0         /* TIM8 Break and TIM12         */
  .word     0          /* TIM8 Update and TIM13        */
  .word     0     /* TIM8 Trigger and Commutation and TIM14 */
  .word     0                /* TIM8 Capture Compare         */
  .word     0           /* DMA1 Stream7                 */
  .word     0                    /* FMC                          */
  .word     0                 /* SDMMC1                       */
  .word     0                   /* TIM5                         */
  .word     0                   /* SPI3                         */
  .word     0                  /* UART4                        */
  .word     0                  /* UART5                        */
  .word     0               /* TIM6 and DAC1&2 underrun errors */
  .word     0                   /* TIM7                         */
  .word     0           /* DMA2 Stream 0                */
  .word     0           /* DMA2 Stream 1                */
  .word     0           /* DMA2 Stream 2                */
  .word     0           /* DMA2 Stream 3                */
  .word     0           /* DMA2 Stream 4                */
  .word     0                    /* Ethernet                     */
  .word     0               /* Ethernet Wakeup through EXTI line */
  .word     0              /* FDCAN calibration unit interrupt*/
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0                                 /* Reserved                     */
  .word     0           /* DMA2 Stream 5                */
  .word     0           /* DMA2 Stream 6                */
  .word     0           /* DMA2 Stream 7                */
  .word     0                 /* USART6                       */
  .word     0                /* I2C3 event                   */
  .word     0                /* I2C3 error                   */
  .word     0         /* USB OTG HS End Point 1 Out   */
  .word     0          /* USB OTG HS End Point 1 In    */
  .word     0            /* USB OTG HS Wakeup through EXTI */
  .word     0                 /* USB OTG HS                   */
  .word     0                   /* DCMI                         */
  .word     0                                 /* Reserved                     */
  .word     0                    /* Rng                          */
  .word     0                    /* FPU                          */
  .word     0                  /* UART7                        */
  .word     0                  /* UART8                        */
  .word     0                   /* SPI4                         */
  .word     0                   /* SPI5                         */
  .word     0                   /* SPI6                         */
  .word     0                   /* SAI1                         */
  .word     0                   /* LTDC                         */
  .word     0                /* LTDC error                   */
  .word     0                  /* DMA2D                        */
  .word     0                   /* SAI2                         */
  .word     0                /* QUADSPI                      */
  .word     0                 /* LPTIM1                       */
  .word     0                    /* HDMI_CEC                     */
  .word     0                /* I2C4 Event                   */
  .word     0                /* I2C4 Error                   */
  .word     0               /* SPDIF_RX                     */
  .word     0         /* USB OTG FS End Point 1 Out   */
  .word     0          /* USB OTG FS End Point 1 In    */
  .word     0            /* USB OTG FS Wakeup through EXTI */
  .word     0                 /* USB OTG FS                   */
  .word     0            /* DMAMUX1 Overrun interrupt    */
  .word     0          /* HRTIM Master Timer global Interrupt */
  .word     0            /* HRTIM Timer A global Interrupt */
  .word     0            /* HRTIM Timer B global Interrupt */
  .word     0            /* HRTIM Timer C global Interrupt */
  .word     0            /* HRTIM Timer D global Interrupt */
  .word     0            /* HRTIM Timer E global Interrupt */
  .word     0             /* HRTIM Fault global Interrupt   */
  .word     0            /* DFSDM Filter0 Interrupt        */
  .word     0            /* DFSDM Filter1 Interrupt        */
  .word     0            /* DFSDM Filter2 Interrupt        */
  .word     0            /* DFSDM Filter3 Interrupt        */
  .word     0                   /* SAI3 global Interrupt          */
  .word     0                 /* Serial Wire Interface 1 global interrupt */
  .word     0                  /* TIM15 global Interrupt      */
  .word     0                  /* TIM16 global Interrupt      */
  .word     0                  /* TIM17 global Interrupt      */
  .word     0             /* MDIOS Wakeup  Interrupt     */
  .word     0                  /* MDIOS global Interrupt      */
  .word     0                   /* JPEG global Interrupt       */
  .word     0                   /* MDMA global Interrupt       */
  .word     0                                 /* Reserved                    */
  .word     0                 /* SDMMC2 global Interrupt     */
  .word     0                  /* HSEM1 global Interrupt      */
  .word     0                                 /* Reserved                    */
  .word     0                   /* ADC3 global Interrupt       */
  .word     0            /* DMAMUX Overrun interrupt    */
  .word     0          /* BDMA Channel 0 global Interrupt */
  .word     0          /* BDMA Channel 1 global Interrupt */
  .word     0          /* BDMA Channel 2 global Interrupt */
  .word     0          /* BDMA Channel 3 global Interrupt */
  .word     0          /* BDMA Channel 4 global Interrupt */
  .word     0          /* BDMA Channel 5 global Interrupt */
  .word     0          /* BDMA Channel 6 global Interrupt */
  .word     0          /* BDMA Channel 7 global Interrupt */
  .word     0                  /* COMP1 global Interrupt     */
  .word     0                 /* LP TIM2 global interrupt   */
  .word     0                 /* LP TIM3 global interrupt   */
  .word     0                 /* LP TIM4 global interrupt   */
  .word     0                 /* LP TIM5 global interrupt   */
  .word     0                /* LP UART1 interrupt         */
  .word     0                                 /* Reserved                   */
  .word     0                    /* Clock Recovery Global Interrupt */
  .word     0                    /* ECC diagnostic Global Interrupt */
  .word     0                   /* SAI4 global interrupt      */
  .word     0                                 /* Reserved                   */
  .word     0                                 /* Reserved                   */
  .word     0             /* Interrupt for all 6 wake-up pins */

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the loader_Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

	.thumb_set loader_Reset_Handler, loader_Reset_Handler

   .weak      NMI_Handler
   .thumb_set NMI_Handler,loader_Default_Handler

   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,loader_Default_Handler

   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,loader_Default_Handler

   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,loader_Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,loader_Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,loader_Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,loader_Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,loader_Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,loader_Default_Handler

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

