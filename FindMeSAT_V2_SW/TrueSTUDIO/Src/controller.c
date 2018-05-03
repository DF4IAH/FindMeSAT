/*
 * controller.c
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */


/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usb.h"
#include "interpreter.h"

#include "controller.h"


/* Private variables ---------------------------------------------------------*/
extern osSemaphoreId      usbToHostBinarySemHandle;
extern EventGroupHandle_t usbToHostEventGroupHandle;

/* Private function prototypes -----------------------------------------------*/
static void prvControllerInitBeforeGreet(void);
static void prvControllerInitAfterGreet(void);
static void prvControllerUsbGreet(void);


/* Global functions ----------------------------------------------------------*/
void controllerControllerTaskInit(void)
{
  uint32_t prevWakeTime = 0;

  /* Inits to be done before the USB CDC messaging is ready */
  prvControllerInitBeforeGreet();

  /* Delay until the USB CDC init phase is over */
  osDelayUntil(&prevWakeTime, 2900);

  /* Greetings to the USB CDC */
  prvControllerUsbGreet();

  prvControllerInitAfterGreet();
}

void controllerControllerTaskLoop(void)
{
  osDelay(100);
}


/* Private functions ---------------------------------------------------------*/
const uint8_t controllerGreetMsg01[] = "\r\n";
const uint8_t controllerGreetMsg02[] = "+=======================================================+\r\n";
const uint8_t controllerGreetMsg03[] = "*                                                       *\r\n";
const uint8_t controllerGreetMsg04[] = "*  FindMeSAT V2 - by DF4IAH - ARM powered by STM32L496  *\r\n";
const uint8_t controllerGreetMsg11[] = "\tFindMeSAT_V2 version: %08ld\r\n";
void prvControllerUsbGreet(void)
{
  uint8_t clrScrBuf[2] = { 0x0c, 0 };
  uint8_t verBuf[48];

  sprintf((char*) verBuf, (char*) controllerGreetMsg11, FINDMESAT_VERSION);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(clrScrBuf, 1);
  usbToHostWait(controllerGreetMsg01, strlen((char*) controllerGreetMsg01));

  usbToHostWait(controllerGreetMsg02, strlen((char*) controllerGreetMsg02));

  usbToHostWait(controllerGreetMsg03, strlen((char*) controllerGreetMsg03));

  usbToHostWait(controllerGreetMsg04, strlen((char*) controllerGreetMsg04));

  usbToHostWait(controllerGreetMsg03, strlen((char*) controllerGreetMsg03));

  usbToHostWait(controllerGreetMsg02, strlen((char*) controllerGreetMsg02));

  usbToHostWait(controllerGreetMsg01, strlen((char*) controllerGreetMsg01));

  usbToHostWait(verBuf, strlen((char*) verBuf));

  usbToHostWait(controllerGreetMsg01, strlen((char*) controllerGreetMsg01));
  usbToHostWait(controllerGreetMsg01, strlen((char*) controllerGreetMsg01));

  osSemaphoreRelease(usbToHostBinarySemHandle);

  interpreterPrintHelp();
  interpreterShowCursor();
}

void prvControllerInitBeforeGreet(void)
{
  /* USB typing echo */
  xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__ECHO_ON);   // TODO: should be from Config-FLASH page
}

void prvControllerInitAfterGreet(void)
{

}
