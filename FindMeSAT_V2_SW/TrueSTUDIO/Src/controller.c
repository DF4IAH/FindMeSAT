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

const char controllerPackages0x00[] = "LQFP64";
const char controllerPackages0x02[] = "LQFP100";
const char controllerPackages0x03[] = "UFBGA132";
const char controllerPackages0x04[] = "LQFP144, WLCSP81 or WLCSP72";
const char controllerPackages0x10[] = "UFBGA169";
const char controllerPackages0x11[] = "WLCSP100";
const char controllerPackages0xXX[] = "(reserved)";
void prvControllerPrintUID(void)
{
  char lotBuf[8];
  char buf[140] = { 0 };
  const char *packagePtr = NULL;

  uint32_t uidPosX    = (*((uint32_t*)  UID_BASE     )      ) & 0X0000ffffUL;
  uint32_t uidPosY    = (*((uint32_t*)  UID_BASE     ) >> 16) & 0X0000ffffUL;
  uint32_t uidWaf     = (*((uint32_t*) (UID_BASE + 4))      ) & 0X000000ffUL;
  char* uidLot        = ((char*)       (UID_BASE + 5));
  memcpy(lotBuf, uidLot, 7);
  lotBuf[7] = 0;

  uint32_t package    = (*((uint32_t*)  PACKAGE_BASE))        & 0X0000001fUL;
  switch(package) {
  case 0x00:
    packagePtr = controllerPackages0x00;
    break;

  case 0x02:
    packagePtr = controllerPackages0x02;
    break;

  case 0x03:
    packagePtr = controllerPackages0x03;
    break;

  case 0x04:
    packagePtr = controllerPackages0x04;
    break;

  case 0x10:
    packagePtr = controllerPackages0x10;
    break;

  case 0x11:
    packagePtr = controllerPackages0x11;
    break;

  default:
    packagePtr = controllerPackages0xXX;
  }

  uint16_t flashSize = (uint16_t) ((*((uint32_t*) FLASHSIZE_BASE)) & 0x0000ffffUL);

  sprintf(buf,
      "\r\n"
      "\tMCU Info:\r\n"
      "\t=========\r\n"
      "\r\n"
      "\t\tLot-ID:\t\t%s\r\n"
      "\t\tWafer:\t\t%lu\r\n"
      "\t\tPos. X/Y:\t%2lu/%2lu\r\n"
      "\t\tPackage(s):\t%s\r\n"
      "\t\tFlash size:\t%4u kB\r\n\r\n\r\n",
      lotBuf, uidWaf, uidPosX, uidPosY, packagePtr, flashSize);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) buf, strlen(buf));
  osSemaphoreRelease(usbToHostBinarySemHandle);
}
