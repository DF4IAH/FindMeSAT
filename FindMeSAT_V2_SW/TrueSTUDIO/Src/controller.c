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
#include "stm32l496xx.h"
#include "cmsis_os.h"
#include "usb.h"
#include "interpreter.h"

#include "controller.h"

//#pragma GCC diagnostic ignored "-Wformat"


/* Private variables ---------------------------------------------------------*/
extern osSemaphoreId      usbToHostBinarySemHandle;
extern EventGroupHandle_t usbToHostEventGroupHandle;

/* Private function prototypes -----------------------------------------------*/
static void prvControllerInitBeforeGreet(void);
static void prvControllerInitAfterGreet(void);
static void prvControllerUsbGreet(void);
static void prvControllerPrintUID(void);


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
const char controllerGreetMsg01[] = "\r\n";
const char controllerGreetMsg02[] = "+=======================================================+\r\n";
const char controllerGreetMsg03[] = "*                                                       *\r\n";
const char controllerGreetMsg04[] = "*  FindMeSAT V2 - by DF4IAH - ARM powered by STM32L496  *\r\n";
const char controllerGreetMsg11[] = "\tFindMeSAT_V2 version: %08u\r\n";
void prvControllerUsbGreet(void)
{
  uint8_t clrScrBuf[2] = { 0x0c, 0 };
  char verBuf[48];

  sprintf(verBuf, controllerGreetMsg11, FINDMESAT_VERSION);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(clrScrBuf, 1);
  usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));

  usbToHostWait((uint8_t*) controllerGreetMsg02, strlen(controllerGreetMsg02));

  usbToHostWait((uint8_t*) controllerGreetMsg03, strlen(controllerGreetMsg03));

  usbToHostWait((uint8_t*) controllerGreetMsg04, strlen(controllerGreetMsg04));

  usbToHostWait((uint8_t*) controllerGreetMsg03, strlen(controllerGreetMsg03));

  usbToHostWait((uint8_t*) controllerGreetMsg02, strlen(controllerGreetMsg02));

  usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));

  usbToHostWait((uint8_t*) verBuf, strlen(verBuf));

  usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));
  usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));

  osSemaphoreRelease(usbToHostBinarySemHandle);

  interpreterPrintHelp();
}

void prvControllerInitBeforeGreet(void)
{
  /* USB typing echo */
  xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__ECHO_ON);   // TODO: should be from Config-FLASH page

}

void prvControllerInitAfterGreet(void)
{
  /* Print UID of the ARM-4M */
  prvControllerPrintUID();

  /* At the last position show the cursor */
  interpreterShowCursor();
}

void prvControllerPrintUID(void)
{
  char lotBuf[8];
  char buf[80] = { 0 };

  uint32_t uidPosX    = (*((uint32_t*)  UID_BASE     )      ) & 0X0000ffffUL;
  uint32_t uidPosY    = (*((uint32_t*)  UID_BASE     ) >> 16) & 0X0000ffffUL;
  uint32_t uidWaf     = (*((uint32_t*) (UID_BASE + 4))      ) & 0X000000ffUL;
  char* uidLot        = ((char*)       (UID_BASE + 5));
  memcpy(lotBuf, uidLot, 7);
  lotBuf[7] = 0;

  sprintf(buf, "\tLot-Id:\t\t%s\r\n\tWafer:\t\t%lu\r\n\tPos. X/Y:\t%2lu/%2lu\r\n\r\n", lotBuf, uidWaf, uidPosX, uidPosY);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) buf, strlen(buf));
  osSemaphoreRelease(usbToHostBinarySemHandle);
}
