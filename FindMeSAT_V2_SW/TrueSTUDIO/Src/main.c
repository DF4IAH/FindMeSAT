
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stddef.h>
#include <sys/_stdint.h>
#include <stdio.h>
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"
#include "controller.h"


#ifndef portAIRCR_REG
#define portAIRCR_REG                       ( * ( ( volatile uint32_t * ) 0xE000ED0C ) )
#define SCB_AIRCR_SYSRESETREQ               0x05FA0002
#endif

#ifndef SCB_AIRCR_SYSRESETREQ_Msk
#define SCB_AIRCR_SYSRESETREQ_Pos           2U
#define SCB_AIRCR_SYSRESETREQ_Msk           (1UL << SCB_AIRCR_SYSRESETREQ_Pos)
#endif


/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef    htim3;
extern TIM_HandleTypeDef    htim4;
extern TIM_HandleTypeDef    htim5;

extern EventGroupHandle_t   controllerEventGroupHandle;


static GPIO_InitTypeDef     GPIO_InitStruct;

#if 0
volatile ENABLE_MASK_t      g_enableMsk                       = ENABLE_MASK__LORA_BARE      |                               ENABLE_MASK__GPS_DATA | ENABLE_MASK__GPS_1PPS;
#elif 1
volatile ENABLE_MASK_t      g_enableMsk                       =                               ENABLE_MASK__LORAWAN_DEVICE | ENABLE_MASK__GPS_DATA | ENABLE_MASK__GPS_1PPS;
#else
volatile ENABLE_MASK_t      g_enableMsk                       = ENABLE_MASK__LORA_BARE      | ENABLE_MASK__LORAWAN_DEVICE | ENABLE_MASK__GPS_DATA | ENABLE_MASK__GPS_1PPS;
#endif
volatile MON_MASK_t         g_monMsk                          = MON_MASK__LORA;

volatile uint64_t	          g_timer_us                        = 0ULL;
volatile uint64_t	          g_timerStart_us                   = 0ULL;
volatile uint64_t           g_realTime_Boot                   = 0ULL;

volatile uint32_t           g_unx_s                           = 0UL;                            // Current    time - since UN*X epoch in  s
volatile uint32_t           g_unx_s_next                      = 0UL;
volatile uint32_t           g_pps_us                          = 0UL;                            // 1PPS event time - 0 .. 999999 us

volatile uint32_t           g_TIM5_CCR2                       = 0UL;
volatile uint32_t           g_TIM5_ofs                        = 0UL;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_16MHz(void);
void SMPS_Init(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void mainDefaultTaskInit(void)
{
  /* -1- Enable GPIO Clock (to be able to program the configuration registers) */
  LED1_GPIO_CLK_ENABLE()
  ;  // Green
  LED2_GPIO_CLK_ENABLE()
  ;  // Blue
  LED3_GPIO_CLK_ENABLE()
  ;  // Red

  /* -2- Configure IO in output push-pull mode to drive external LEDs */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Pin = LED1_PIN;
  HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LED2_PIN;
  HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LED3_PIN;
  HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);
}

void mainDefaultTaskLoop(void)
{
#if 0
  unsigned char buf[32] = { 0 };
  strcpy((char*) buf, "+\r\n");

  snprintf((char*) (buf + 1), sizeof(buf), " %010ld\r\n", getRunTimeCounterValue());
  usbToHost(buf, strlen((char*) buf));
#endif

  osDelay(60 * 1000);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* Check if ARM core is already in reset state */
  if (!(RCC->CSR & 0xff000000UL)) {
    /* Disable SMPS */
    HAL_GPIO_WritePin(PORT_SMPS, PIN_SMPS_SWITCH_ENABLE, GPIO_PIN_RESET);
    __asm volatile( "ISB" );
    HAL_GPIO_WritePin(PORT_SMPS, PIN_SMPS_ENABLE, GPIO_PIN_RESET);

    /* ARM software reset to be done */
    SystemResetbyARMcore();
  }
  __HAL_RCC_CLEAR_RESET_FLAGS();

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* For frequencies > 24 MHz switch to SCALE1 */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  MX_TIM5_Init();
  MX_SPI1_Init();
  MX_UART5_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Enable 1PPS time capture on TIM5/Channel2 */
  HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);

  /* Enable external SMPS for Vdd12 */
  SMPS_Init();

  /* Speed down to 16 MHz */
  SystemClock_16MHz();

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /* Something got wrong! */
  Error_Handler();

  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability 
    */
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_HSI;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_HSI;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 2;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV8;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV8;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /**Enable MSI Auto calibration 
    */
  HAL_RCCEx_EnableMSIPLLMode();

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */
void SystemClock_16MHz(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability
    */
  HAL_PWR_EnableBkUpAccess();

  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_HSI;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /**Enable MSI Auto calibration
    */
  HAL_RCCEx_EnableMSIPLLMode();

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

void SMPS_Init(void)
{
  uint32_t SMPS_status_local = 0;

  /* Enable clock to the destinations needed */
  PWR_AND_CLK_SMPS();

  /* Reduce to max. speed (24 MHz) of Range2 */
  SystemClock_16MHz();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /* Prepare port pins */
  SMPS_status_local = BSP_SMPS_Init(0);
  if (SMPS_status_local != SMPS_OK) {
    Error_Handler();
  }

  /* Enable SMPS switcher and check for Power Good after 10ms */
  SMPS_status_local = BSP_SMPS_Enable(10, 1);
  if (SMPS_status_local != SMPS_OK) {
    Error_Handler();
  }

  /* Close switch between SMPS switcher and Vdd12 - (PG being checked again)*/
  SMPS_status_local = BSP_SMPS_Supply_Enable(0, 1);
  if (SMPS_status_local != SMPS_OK) {
    Error_Handler();
  }

  /* Go up to target speed */
  SystemClock_Config();

  /* Keep the internal regulator below of the SMPS voltage */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
}


/* Used by the run-time stats */
void configureTimerForRunTimeStats(void)
{
  getRunTimeCounterValue();

  /* Interrupt disabled block */
  {
    taskDISABLE_INTERRUPTS();
    g_timerStart_us = g_timer_us;
    taskENABLE_INTERRUPTS();
  }
}

/* Used by the run-time stats */
unsigned long getRunTimeCounterValue(void)
{
  uint64_t l_timerStart_us;
  uint64_t timer_us = HAL_GetTick() & 0x003fffffUL;  // avoid overflows
  timer_us *= 1000UL;
  timer_us += TIM2->CNT;

  /* Interrupt disabled block */
  {
    taskDISABLE_INTERRUPTS();
    g_timer_us      = timer_us;
    l_timerStart_us = g_timerStart_us;
    taskENABLE_INTERRUPTS();
  }

  return (unsigned long) (timer_us - l_timerStart_us);
}


/* Adjusts the global timers TIM4 / TIM3 / TIM5 with UNIX time (�s) */
int32_t setRealTime(uint64_t unixTime_us)
{
  uint16_t  l_TIM4_CNT;
  uint16_t  l_TIM3_CNT;
  uint32_t  l_TIM5_CNT;
  uint64_t  l_realTime_Outdated;
  uint64_t  l_realTime_Now;
  int64_t   l_diff_last;

  /* Interrupt disabled block */
  {
    taskDISABLE_INTERRUPTS();
    l_TIM5_CNT = TIM5->CNT;
    l_TIM3_CNT = TIM3->CNT;
    l_TIM4_CNT = TIM4->CNT;
    taskENABLE_INTERRUPTS();
  }

  /* Calculate the timestamp of the outdated timer */
  l_realTime_Outdated  = (uint64_t)l_TIM4_CNT << 48U;
  l_realTime_Outdated |= (uint64_t)l_TIM3_CNT << 32U;
  l_realTime_Outdated += (uint64_t) (l_TIM5_CNT / (HSI_VALUE / 1e7));

  /* Calculate the last offset */
  l_diff_last = l_realTime_Outdated - unixTime_us;

  /* Calculate the new boot timestamp */
  l_realTime_Now  = l_realTime_Outdated;
  l_realTime_Now -= l_diff_last;

  /* Interrupt disabled block */
  {
    taskDISABLE_INTERRUPTS();
    TIM5->CNT = l_TIM5_CNT;
    TIM3->CNT = l_TIM3_CNT;
    TIM4->CNT = l_TIM4_CNT;
    taskENABLE_INTERRUPTS();
  }

  return (int32_t) l_diff_last;
}

/* Returns the UNIX time in �s */
uint64_t getRealTime(void)
{
  uint16_t  l_TIM4_CNT;
  uint16_t  l_TIM3_CNT;
  uint32_t  l_TIM5_CNT;
  uint64_t  l_realTime_Now;

  /* Interrupt disabled block */
  {
    taskDISABLE_INTERRUPTS();
    l_TIM5_CNT = TIM5->CNT;
    l_TIM3_CNT = TIM3->CNT;
    l_TIM4_CNT = TIM4->CNT;
    taskENABLE_INTERRUPTS();
  }

  /* Calculate the timestamp of now */
  l_realTime_Now  = (uint64_t)l_TIM4_CNT << 48U;
  l_realTime_Now |= (uint64_t)l_TIM3_CNT << 32U;
  l_realTime_Now += (uint64_t) (l_TIM5_CNT / (HSI_VALUE / 1e7));

  return l_realTime_Now;
}


void SystemResetbyARMcore(void)
{
  /* Switch off SMPS first */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  HAL_GPIO_WritePin(PORT_SMPS, PIN_SMPS_SWITCH_ENABLE, GPIO_PIN_RESET);
  __asm volatile( "ISB" );
  HAL_GPIO_WritePin(PORT_SMPS, PIN_SMPS_ENABLE, GPIO_PIN_RESET);

  /* Set SW reset bit */
  SCB->AIRCR = 0x05FA0000UL | SCB_AIRCR_SYSRESETREQ_Msk;
}

void vAssertCalled(const char *pcFile, uint32_t ulLine)
{
  /* Inside this function, pcFile holds the name of the source file that contains
  the line that detected the error, and ulLine holds the line number in the source
  file. The pcFile and ulLine values can be printed out, or otherwise recorded,
  before the following infinite loop is entered. */
  //RecordErrorInformationHere( pcFile, ulLine );
  /* Disable interrupts so the tick interrupt stops executing, then sit in a loop
  so execution does not move past the line that failed the assertion. */
  taskDISABLE_INTERRUPTS();
  for(;;)
  {
      __asm volatile( "nop" );
  }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   vAssertCalled(__FILE__, __LINE__);
}

void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  vAssertCalled(__FILE__, __LINE__);
}

//#define LED_IDLE_DEBUG
void  vApplicationIdleHook(void)
{
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
  to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
  task. It is essential that code added to this hook function never attempts
  to block in any way (for example, call xQueueReceive() with a block time
  specified, or call vTaskDelay()). If the application makes use of the
  vTaskDelete() API function (as this demo application does) then it is also
  important that vApplicationIdleHook() is permitted to return to its calling
  function, because it is the responsibility of the idle task to clean up
  memory allocated by the kernel to any task that has since been deleted. */
  /* TODO:
   * 1) Reduce 80 MHz  to  2 MHz
   * 2)  go to LPRun  (SMPS 2 High (-->  MR range 1) --> MR range 2 --> LPR
   * 3)  Go to LPSleep
   *
   * WAKEUP
   * 1)  In LPRun go to 80 MHz (LPR --> MR range 2 (--> MR range 1) --> SMPS 2 High)
   * 2)  Increase 2 MHz to 80 MHz
   */
#ifdef LED_IDLE_DEBUG
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);                                    // Blue on
#endif

  /* Enter sleep mode */
  __asm volatile( "WFI" );

#ifdef LED_IDLE_DEBUG
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);                                  // Blue off
#endif
  /* Increase clock frequency to 80 MHz */
  // TODO: TBD
}

void mainCalc_Float2Int(float in, uint32_t* out_i, uint16_t* out_p1000)
{
  if (in < 0) {
    in = -in;
  }

  *out_i      = (uint32_t) in;
  *out_p1000  = (uint16_t) (((uint32_t) (in * 1000.f)) % 1000);
}


/**
  * @brief  Input Capture callback in non-blocking mode
  * @param  htim TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance  == htim5.Instance           ) &&
      (htim->Channel   == HAL_TIM_ACTIVE_CHANNEL_2)) {

    /* Fetch and Stash - BEGIN */
    taskDISABLE_INTERRUPTS();

    const uint32_t l_TIM5_CNT   = htim->Instance->CNT;
    htim->Instance->CNT         = (uint32_t) ((l_TIM5_CNT + g_TIM5_ofs) % Tim5_reloadValue);
    g_TIM5_ofs                  = 0UL;

    const uint32_t l_TIM5_CCR2  = htim->Instance->CCR2;
    g_TIM5_CCR2                 = l_TIM5_CCR2;

    g_unx_s                     = g_unx_s_next;
#ifdef IS_N_X_1MHZ
    g_pps_us                    = (uint32_t) (l_TIM5_CCR2 / (Tim5_reloadValue / 1000000UL));
#else
    g_pps_us                    = (uint32_t) (l_TIM5_CCR2 / (Tim5_reloadValue / 1e6));
#endif

    taskENABLE_INTERRUPTS();
    /* Fetch and Stash - END */

    /* Show 1PPS by reversing the red LED */
    HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);                                               // Red toggling

    /* Message to controller */
    {
      BaseType_t  higherPriorityTaskWOken = 0UL;

      if (controllerEventGroupHandle) {
        xEventGroupSetBitsFromISR(controllerEventGroupHandle, Controller_EGW__TIM_PPS, &higherPriorityTaskWOken);
      }
    }
  }
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  if (htim->Instance == TIM5) {
    /* Seconds counter */
    ++g_unx_s;
  }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  vAssertCalled(file, line);
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  vAssertCalled((char *) file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
