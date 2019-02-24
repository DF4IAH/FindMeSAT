/* USER CODE BEGIN Header */
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
  * Copyright (c) 2019 STMicroelectronics International N.V. 
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
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

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

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

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
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
  osThreadDef(interpreterTask, StartInterpreterTask, osPriorityNormal, 0, 512);
  interpreterTaskHandle = osThreadCreate(osThread(interpreterTask), NULL);

  /* definition and creation of loraTask */
  osThreadDef(loraTask, StartLoraTask, osPriorityHigh, 0, 2048);
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
  osMessageQDef(usbToHostQueue, 512, uint8_t);
  usbToHostQueueHandle = osMessageCreate(osMessageQ(usbToHostQueue), NULL);

  /* definition and creation of usbFromHostQueue */
  osMessageQDef(usbFromHostQueue, 32, uint8_t);
  usbFromHostQueueHandle = osMessageCreate(osMessageQ(usbFromHostQueue), NULL);

  /* definition and creation of loraInQueue */
  osMessageQDef(loraInQueue, 256, uint8_t);
  loraInQueueHandle = osMessageCreate(osMessageQ(loraInQueue), NULL);

  /* definition and creation of loraOutQueue */
  osMessageQDef(loraOutQueue, 8, uint8_t);
  loraOutQueueHandle = osMessageCreate(osMessageQ(loraOutQueue), NULL);

  /* definition and creation of loraMacQueue */
  osMessageQDef(loraMacQueue, 32, uint8_t);
  loraMacQueueHandle = osMessageCreate(osMessageQ(loraMacQueue), NULL);

  /* definition and creation of gpscomInQueue */
  osMessageQDef(gpscomInQueue, 64, uint8_t);
  gpscomInQueueHandle = osMessageCreate(osMessageQ(gpscomInQueue), NULL);

  /* definition and creation of gpscomOutQueue */
  osMessageQDef(gpscomOutQueue, 8, uint8_t);
  gpscomOutQueueHandle = osMessageCreate(osMessageQ(gpscomOutQueue), NULL);

  /* definition and creation of sensorsOutQueue */
  osMessageQDef(sensorsOutQueue, 32, uint8_t);
  sensorsOutQueueHandle = osMessageCreate(osMessageQ(sensorsOutQueue), NULL);

  /* definition and creation of sensorsInQueue */
  osMessageQDef(sensorsInQueue, 8, uint8_t);
  sensorsInQueueHandle = osMessageCreate(osMessageQ(sensorsInQueue), NULL);

  /* definition and creation of interOutQueue */
  osMessageQDef(interOutQueue, 256, uint8_t);
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

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
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

/* USER CODE BEGIN Header_StartUsbToHostTask */
/**
* @brief Function implementing the usbToHostTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUsbToHostTask */
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

/* USER CODE BEGIN Header_StartUsbFromHostTask */
/**
* @brief Function implementing the usbFromHostTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUsbFromHostTask */
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

/* USER CODE BEGIN Header_StartControllerTask */
/**
* @brief Function implementing the controllerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartControllerTask */
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

/* USER CODE BEGIN Header_StartInterpreterTask */
/**
* @brief Function implementing the interpreterTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInterpreterTask */
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

/* USER CODE BEGIN Header_StartLoraTask */
/**
* @brief Function implementing the loraTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoraTask */
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

/* USER CODE BEGIN Header_StartGpscomTask */
/**
* @brief Function implementing the gpscomTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGpscomTask */
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

/* USER CODE BEGIN Header_StartSensorsTask */
/**
* @brief Function implementing the sensorsTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSensorsTask */
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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
