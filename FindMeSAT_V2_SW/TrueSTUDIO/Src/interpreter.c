/*
 * interpreter.c
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "FreeRTOS.h"
#include <sys/_stdint.h>
#include <cmsis_os.h>
#include "main.h"
#include "usb.h"

#include "interpreter.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId       usbFromHostQueueHandle;
extern osSemaphoreId      usbToHostBinarySemHandle;
extern EventGroupHandle_t usbToHostEventGroupHandle;

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
  static uint8_t inBuf[64] = { 0 };  // [8] works, [16] not !
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

    /* Echoing back to USB CDC IN */
    if (1) {
      usbToHost(&chr, 1);
      if (chr == 0x0d) {
        /* Add a LF for the terminal */
        usbToHost((uint8_t*) "\n", 1);
      }
    }

    /* Concatenate for the interpreter buffer */
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


const uint8_t helpMsg01[] =
    "\r\n";
const uint8_t helpMsg02[] =
    "\t====================================\r\n";
const uint8_t helpMsg03[] =
    "\t*  HELP:  Listing of the commands  *\r\n";
const uint8_t helpMsg11[] =
    "\tCommand       +\tRemarks\r\n";
const uint8_t helpMsg12[] =
    "\t--------------+\t---------------------------------------------\r\n";
const uint8_t helpMsg21[] =
    "\thelp\t\tPrint this list of commands.\r\n";
const uint8_t helpMsg22[] =
    "\trestart\t\tRestart this device.\r\n";
void printHelp(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));
  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  usbToHostWait(helpMsg02, strlen((char*) helpMsg02));

  usbToHostWait(helpMsg03, strlen((char*) helpMsg03));

  usbToHostWait(helpMsg02, strlen((char*) helpMsg02));

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));
  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  usbToHostWait(helpMsg11, strlen((char*) helpMsg11));

  usbToHostWait(helpMsg12, strlen((char*) helpMsg12));

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  usbToHostWait(helpMsg21, strlen((char*) helpMsg21));

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  usbToHostWait(helpMsg22, strlen((char*) helpMsg22));

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  usbToHostWait(helpMsg12, strlen((char*) helpMsg12));

  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));
  usbToHostWait(helpMsg01, strlen((char*) helpMsg01));

  osSemaphoreRelease(usbToHostBinarySemHandle);
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
