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
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"
#include "controller.h"
#include "interpreter.h"
#include "usb.h"
#include "LoRaWAN.h"
#include "gpsCom.h"
#include "sensors.h"

/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId usbToHostTaskHandle;
osThreadId usbFromHostTaskHandle;
osThreadId controllerTaskHandle;
osThreadId interpreterTaskHandle;
osThreadId loraTaskHandle;
osThreadId gpscomTaskHandle;
osThreadId sensorsTaskHandle;
osMessageQId usbToHostQueueHandle;
osMessageQId usbFromHostQueueHandle;
osMessageQId loraInQueueHandle;
osMessageQId loraOutQueueHandle;
osMessageQId loraMacQueueHandle;
osMessageQId gpscomInQueueHandle;
osMessageQId gpscomOutQueueHandle;
osMessageQId sensorsOutQueueHandle;
osMessageQId sensorsInQueueHandle;
osMessageQId interOutQueueHandle;
osTimerId controllerSendTimerHandle;
osTimerId gpscomtRXTimerHandle;
osMutexId gpscomCtxMutexHandle;
osMutexId trackMeApplUpDataMutexHandle;
osMutexId trackMeApplDnDataMutexHandle;
osSemaphoreId usbToHostBinarySemHandle;

/* USER CODE BEGIN Variables */
extern uint8_t usbFromHostISRBuf[64];
extern uint32_t usbFromHostISRBufLen;
EventGroupHandle_t usbToHostEventGroupHandle;
EventGroupHandle_t adcEventGroupHandle;
EventGroupHandle_t extiEventGroupHandle;
EventGroupHandle_t spiEventGroupHandle;
EventGroupHandle_t loraEventGroupHandle;
EventGroupHandle_t controllerEventGroupHandle;
EventGroupHandle_t gpscomEventGroupHandle;
EventGroupHandle_t sensorsEventGroupHandle;

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartUsbToHostTask(void const * argument);
void StartUsbFromHostTask(void const * argument);
void StartControllerTask(void const * argument);
void StartInterpreterTask(void const * argument);
void StartLoraTask(void const * argument);
void StartGpscomTask(void const * argument);
void StartSensorsTask(void const * argument);
void controllerSendTimerCallback(void const * argument);
void gpscomtRXTimerCallback(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
}

__weak unsigned long getRunTimeCounterValue(void)
{
  return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
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
}
/* USER CODE END 2 */

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

  /* Create the mutex(es) */
  /* definition and creation of gpscomCtxMutex */
  osMutexDef(gpscomCtxMutex);
  gpscomCtxMutexHandle = osMutexCreate(osMutex(gpscomCtxMutex));

  /* definition and creation of trackMeApplUpDataMutex */
  osMutexDef(trackMeApplUpDataMutex);
  trackMeApplUpDataMutexHandle = osMutexCreate(osMutex(trackMeApplUpDataMutex));

  /* definition and creation of trackMeApplDnDataMutex */
  osMutexDef(trackMeApplDnDataMutex);
  trackMeApplDnDataMutexHandle = osMutexCreate(osMutex(trackMeApplDnDataMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of usbToHostBinarySem */
  osSemaphoreDef(usbToHostBinarySem);
  usbToHostBinarySemHandle = osSemaphoreCreate(osSemaphore(usbToHostBinarySem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  usbToHostEventGroupHandle = xEventGroupCreate();
  adcEventGroupHandle = xEventGroupCreate();
  extiEventGroupHandle = xEventGroupCreate();
  spiEventGroupHandle = xEventGroupCreate();
  loraEventGroupHandle = xEventGroupCreate();
  controllerEventGroupHandle = xEventGroupCreate();
  gpscomEventGroupHandle = xEventGroupCreate();
  sensorsEventGroupHandle = xEventGroupCreate();
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of controllerSendTimer */
  osTimerDef(controllerSendTimer, controllerSendTimerCallback);
  controllerSendTimerHandle = osTimerCreate(osTimer(controllerSendTimer), osTimerPeriodic, NULL);

  /* definition and creation of gpscomtRXTimer */
  osTimerDef(gpscomtRXTimer, gpscomtRXTimerCallback);
  gpscomtRXTimerHandle = osTimerCreate(osTimer(gpscomtRXTimer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  vTimerSetTimerID(controllerSendTimerHandle, "controllerSendTmr");
  vTimerSetTimerID(gpscomtRXTimerHandle, "gpscomRXTmr");

  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of usbToHostTask */
  osThreadDef(usbToHostTask, StartUsbToHostTask, osPriorityAboveNormal, 0, 128);
  usbToHostTaskHandle = osThreadCreate(osThread(usbToHostTask), NULL);

  /* definition and creation of usbFromHostTask */
  osThreadDef(usbFromHostTask, StartUsbFromHostTask, osPriorityAboveNormal, 0, 128);
  usbFromHostTaskHandle = osThreadCreate(osThread(usbFromHostTask), NULL);

  /* definition and creation of controllerTask */
  osThreadDef(controllerTask, StartControllerTask, osPriorityNormal, 0, 1024);
  controllerTaskHandle = osThreadCreate(osThread(controllerTask), NULL);

  /* definition and creation of interpreterTask */
  osThreadDef(interpreterTask, StartInterpreterTask, osPriorityNormal, 0, 128);
  interpreterTaskHandle = osThreadCreate(osThread(interpreterTask), NULL);

  /* definition and creation of loraTask */
  osThreadDef(loraTask, StartLoraTask, osPriorityHigh, 0, 1024);
  loraTaskHandle = osThreadCreate(osThread(loraTask), NULL);

  /* definition and creation of gpscomTask */
  osThreadDef(gpscomTask, StartGpscomTask, osPriorityRealtime, 0, 1024);
  gpscomTaskHandle = osThreadCreate(osThread(gpscomTask), NULL);

  /* definition and creation of sensorsTask */
  osThreadDef(sensorsTask, StartSensorsTask, osPriorityAboveNormal, 0, 128);
  sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of usbToHostQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(usbToHostQueue, 512, uint8_t);
  usbToHostQueueHandle = osMessageCreate(osMessageQ(usbToHostQueue), NULL);

  /* definition and creation of usbFromHostQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(usbFromHostQueue, 32, uint8_t);
  usbFromHostQueueHandle = osMessageCreate(osMessageQ(usbFromHostQueue), NULL);

  /* definition and creation of loraInQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(loraInQueue, 8, uint8_t);
  loraInQueueHandle = osMessageCreate(osMessageQ(loraInQueue), NULL);

  /* definition and creation of loraOutQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(loraOutQueue, 8, uint8_t);
  loraOutQueueHandle = osMessageCreate(osMessageQ(loraOutQueue), NULL);

  /* definition and creation of loraMacQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(loraMacQueue, 32, uint8_t);
  loraMacQueueHandle = osMessageCreate(osMessageQ(loraMacQueue), NULL);

  /* definition and creation of gpscomInQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(gpscomInQueue, 64, uint8_t);
  gpscomInQueueHandle = osMessageCreate(osMessageQ(gpscomInQueue), NULL);

  /* definition and creation of gpscomOutQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(gpscomOutQueue, 8, uint8_t);
  gpscomOutQueueHandle = osMessageCreate(osMessageQ(gpscomOutQueue), NULL);

  /* definition and creation of sensorsOutQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(sensorsOutQueue, 32, uint8_t);
  sensorsOutQueueHandle = osMessageCreate(osMessageQ(sensorsOutQueue), NULL);

  /* definition and creation of sensorsInQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(sensorsInQueue, 8, uint8_t);
  sensorsInQueueHandle = osMessageCreate(osMessageQ(sensorsInQueue), NULL);

  /* definition and creation of interOutQueue */
/* what about the sizeof here??? cd native code */
  osMessageQDef(interOutQueue, 64, uint8_t);
  interOutQueueHandle = osMessageCreate(osMessageQ(interOutQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  vQueueAddToRegistry(usbToHostQueueHandle,       "USBtoHostQ");
  vQueueAddToRegistry(usbToHostBinarySemHandle,   "USBtoHostBSem");
  vQueueAddToRegistry(usbFromHostQueueHandle,     "USBfromHostQ");
  vQueueAddToRegistry(loraInQueueHandle,          "loraInQ");
  vQueueAddToRegistry(loraOutQueueHandle,         "loraOutQ");
  vQueueAddToRegistry(loraMacQueueHandle,         "loraMacQ");
  vQueueAddToRegistry(gpscomInQueueHandle,        "gpscomInQ");
  vQueueAddToRegistry(gpscomOutQueueHandle,       "gpscomOutQ");
  vQueueAddToRegistry(sensorsInQueueHandle,       "sensorsInQ");
  vQueueAddToRegistry(sensorsOutQueueHandle,      "sensorsOutQ");
  vQueueAddToRegistry(interOutQueueHandle,        "interOutQ");
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN StartDefaultTask */
  mainDefaultTaskInit();

  /* Infinite loop */
  for (;;) {
    mainDefaultTaskLoop();
  }
  /* USER CODE END StartDefaultTask */
}

/* StartUsbToHostTask function */
void StartUsbToHostTask(void const * argument)
{
  /* USER CODE BEGIN StartUsbToHostTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  usbUsbToHostTaskInit();

  /* Infinite loop */
  for (;;) {
    usbUsbToHostTaskLoop();
  }
  /* USER CODE END StartUsbToHostTask */
}

/* StartUsbFromHostTask function */
void StartUsbFromHostTask(void const * argument)
{
  /* USER CODE BEGIN StartUsbFromHostTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  usbUsbFromHostTaskInit();

  /* Infinite loop */
  for (;;) {
    usbUsbFromHostTaskLoop();
  }
  /* USER CODE END StartUsbFromHostTask */
}

/* StartControllerTask function */
void StartControllerTask(void const * argument)
{
  /* USER CODE BEGIN StartControllerTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  controllerControllerTaskInit();

  /* Infinite loop */
  for(;;)
  {
    controllerControllerTaskLoop();
  }
  /* USER CODE END StartControllerTask */
}

/* StartInterpreterTask function */
void StartInterpreterTask(void const * argument)
{
  /* USER CODE BEGIN StartInterpreterTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  interpreterInterpreterTaskInit();

  /* Infinite loop */
  for(;;)
  {
    interpreterInterpreterTaskLoop();
  }
  /* USER CODE END StartInterpreterTask */
}

/* StartLoraTask function */
void StartLoraTask(void const * argument)
{
  /* USER CODE BEGIN StartLoraTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  loRaWANLoraTaskInit();

  /* Infinite loop */
  for(;;)
  {
    loRaWANLoraTaskLoop();
  }
  /* USER CODE END StartLoraTask */
}

/* StartGpscomTask function */
void StartGpscomTask(void const * argument)
{
  /* USER CODE BEGIN StartGpscomTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  gpscomGpscomTaskInit();

  /* Infinite loop */
  for(;;)
  {
    gpscomGpscomTaskLoop();
  }
  /* USER CODE END StartGpscomTask */
}

/* StartSensorsTask function */
void StartSensorsTask(void const * argument)
{
  /* USER CODE BEGIN StartSensorsTask */

  /* Give DefaultTask time to prepare the device */
  osDelay(100);

  sensorsSensorsTaskInit();

  /* Infinite loop */
  for(;;)
  {
    sensorsSensorsTaskLoop();
  }
  /* USER CODE END StartSensorsTask */
}

/* controllerSendTimerCallback function */
void controllerSendTimerCallback(void const * argument)
{
  /* USER CODE BEGIN controllerSendTimerCallback */
  controllerSendTimerCallbackImpl((TimerHandle_t) argument);
  /* USER CODE END controllerSendTimerCallback */
}

/* gpscomtRXTimerCallback function */
void gpscomtRXTimerCallback(void const * argument)
{
  /* USER CODE BEGIN gpscomtRXTimerCallback */
  gpscomtRXTimerCallbackImpl((TimerHandle_t) argument);
  /* USER CODE END gpscomtRXTimerCallback */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
