/*
 * interpreter.c
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <sys/_stdint.h>
#include <cmsis_os.h>
#include "main.h"
#include "usb.h"

#include "interpreter.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId usbFromHostQueueHandle;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void prvDoInterprete(const uint8_t *buf, uint32_t len);

/* Global functions ----------------------------------------------------------*/
void interpreterInterpreterTaskInit(void)
{
  uint8_t inChr = 0;

  /* Clear queue */
  while (xQueueReceive(usbFromHostQueueHandle, &inChr, 0) == pdPASS) {
  }
}

void interpreterInterpreterTaskLoop(void)
{
  static uint8_t inBuf[64] = { 0 };
  static uint32_t inBufPos = 0;
  static uint8_t prevCr = 0;
  uint8_t chr;
  BaseType_t xStatus;

  /* Wait for input from the USB CDC host */
  xStatus = xQueueReceive(usbFromHostQueueHandle, &chr, portMAX_DELAY);
  if (pdPASS == xStatus) {
    /* CR/LF handling */
    if (chr == 0x0d) {
      prevCr = 1;

    } else if (chr == 0x0a) {
      if (!prevCr) {
        chr = 0x0d;
      }
      prevCr = 0;
    } else {
      prevCr = 0;
    }

    inBuf[inBufPos++] = chr;

    /* Process line */
    if (chr == 0x0d || (inBufPos >= (sizeof(inBuf) - 1))) {
      inBuf[inBufPos] = 0;

      /* Interpreter */
      if (inBufPos < (sizeof(inBuf) - 1)) {
        prvDoInterprete(inBuf, inBufPos);
      }

      /* Prepare for next command */
      memset(inBuf, 0, sizeof(inBuf));
      inBufPos = 0;
    }
  }
}


const uint8_t helpMsg[] =
    "\r\n"
    "\r\n"
    "List of commands\r\n"
    "----------------\r\n"
    "\r\n"
    "help\t\tPrint this list of commands.\r\n"
    "restart\t\tRestart this device.\r\n"
    "\r\n";
void printHelp(void)
{
  usbToHost(helpMsg, strlen((char*) helpMsg));
}


/* Private functions ---------------------------------------------------------*/
void prvDoInterprete(const uint8_t *buf, uint32_t len)
{
  const char *cb = (const char*) buf;

  if (!strncmp("help", cb, len)) {
    printHelp();

  } else if(!strncmp("restart", cb, len)) {
    doRestart();
  }
}
