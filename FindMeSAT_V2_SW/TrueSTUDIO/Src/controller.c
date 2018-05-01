/*
 * controller.c
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */


/* Includes ------------------------------------------------------------------*/
#include "controller.h"

#include <string.h>
#include "cmsis_os.h"
#include "usb.h"


/* Forward declarations ------------------------------------------------------*/
static void prvControllerInitBeforeGreet(void);
static void prvControllerInitAfterGreet(void);
static void prvControllerUsbGreet(void);


/* Global functions ----------------------------------------------------------*/
void controllerInit(void)
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


/* Private functions ---------------------------------------------------------*/
const uint8_t controllerGreetMsg[] =
    "\r\n"
    "======================================\r\n"
    "* FindMeSAT V2 - USB Logging started *\r\n"
    "======================================\r\n"
    "\r\n";
void prvControllerUsbGreet(void)
{
  uint8_t clrScrbuf[2] = { 0x0c, 0 };

  usbToHost(clrScrbuf, 1);
  usbToHost(controllerGreetMsg, strlen((char*) controllerGreetMsg));
}

void prvControllerInitBeforeGreet(void)
{

}

void prvControllerInitAfterGreet(void)
{

}
