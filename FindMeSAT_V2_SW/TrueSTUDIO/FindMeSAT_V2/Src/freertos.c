/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include <stddef.h>
#include <sys/_stdint.h>
#include <stdio.h>
#include <usbd_cdc_if.h>
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"
#include "usb.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId usbToHostTaskHandle;
osThreadId usbFromHostTaskHandle;
osMessageQId usbToHostQueueHandle;

/* USER CODE BEGIN Variables */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef  GPIO_InitStruct;
volatile uint32_t g_timer_us = 0;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartUsbToHostTask(void const * argument);
void StartUsbFromHostTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
	// none needed
}

__weak unsigned long getRunTimeCounterValue(void)
{
	uint64_t timer_us = HAL_GetTick() & 0x003fffffUL;  // avoid overflows
	timer_us *= 1000UL;
	timer_us += TIM2->CNT;
	g_timer_us = timer_us;
	return timer_us;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
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
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of usbToHostTask */
  osThreadDef(usbToHostTask, StartUsbToHostTask, osPriorityAboveNormal, 0, 128);
  usbToHostTaskHandle = osThreadCreate(osThread(usbToHostTask), NULL);

  /* definition and creation of usbFromHostTask */
  osThreadDef(usbFromHostTask, StartUsbFromHostTask, osPriorityAboveNormal, 0, 128);
  usbFromHostTaskHandle = osThreadCreate(osThread(usbFromHostTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of usbToHostQueue */
  osMessageQDef(usbToHostQueue, 64, uint8_t);
  usbToHostQueueHandle = osMessageCreate(osMessageQ(usbToHostQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN StartDefaultTask */
  uint32_t i = 0;
  unsigned char buf[32] = { 0 };
  strcpy((char*) buf, "+\r\n");

  /* -1- Enable GPIO Clock (to be able to program the configuration registers) */
  LED1_GPIO_CLK_ENABLE();  // Green
  LED2_GPIO_CLK_ENABLE();  // Blue
  LED3_GPIO_CLK_ENABLE();  // Red

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

  /* Infinite loop */
  for(;;)
  {
    //HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
    snprintf((char*) (buf + 1), sizeof(buf), " %010ld\r\n", getRunTimeCounterValue());

#if 0
    /* LED toggle */
	HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
	vTaskDelay(100 / portTICK_PERIOD_MS);
#elif 0
    i++;

	/* LED counter */
	HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN,
				   (i & 1) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN,
				   (i & 2) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN,
				   (i & 4) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
	vTaskDelay(25 / portTICK_PERIOD_MS);
#elif 0
	if (!i) {
		   i = 10000000UL;  // 10E+06  - @80MHz=0.76s, @16MHz=3,76s
		   HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
	}
	--i;
#else
	UNUSED(i);

	usbToHost(buf, strlen((char*) buf));
#endif
  }
  /* USER CODE END StartDefaultTask */
}

/* StartUsbToHostTask function */
void StartUsbToHostTask(void const * argument)
{
	/* USER CODE BEGIN StartUsbToHostTask */
	unsigned char buf[32];
	uint8_t bufCtr = 0;

	/* Clear queue */
	while (uxQueueMessagesWaiting(usbToHostQueueHandle) == pdPASS) {
	}

	/* DEBUG */
	HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);						// Blue off
	HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);						// Red off

	/* Infinite loop */
	for(;;)
	{
		BaseType_t	xStatus;
		uint8_t		inChr = 0;

	    /* Take next character from the queue*/
		HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);					// Green off
		xStatus = xQueueReceive(usbToHostQueueHandle, &inChr, 100 / portTICK_PERIOD_MS);
		HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);						// Green on

		if (pdPASS == xStatus) {
			if (inChr && (bufCtr < (sizeof(buf) - 1))) {
				buf[bufCtr++] = inChr;

			} else {
				uint8_t cnt;

				/* Do not send empty buffer */
				if (!bufCtr) {
					continue;
				}

				buf[bufCtr] = 0;
				for (cnt = 50; cnt; cnt--) {
					uint8_t ucRetVal = CDC_Transmit_FS(buf, bufCtr);
					if (USBD_BUSY != ucRetVal) {
						/* Data accepted for transmission */
						bufCtr = 0;
						HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);	// Red off
						break;

					} else {
						/* USB EP busy - try again */
						HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);		// Red on

						/* Delay for next USB packet to come and go */
						HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);		// Blue on
						vTaskDelay(1 / portTICK_PERIOD_MS);
						HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);	// Blue off
					}
				}

				if (!cnt) {
					/* USB EP still busy - drop whole buffer content */
					bufCtr = 0;
					HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);			// Red on
				}
			}
		}
	}
	/* USER CODE END StartUsbToHostTask */
}

/* StartUsbFromHostTask function */
void StartUsbFromHostTask(void const * argument)
{
  /* USER CODE BEGIN StartUsbFromHostTask */
  /* Infinite loop */
  for(;;)
  {
	  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  /* USER CODE END StartUsbFromHostTask */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
