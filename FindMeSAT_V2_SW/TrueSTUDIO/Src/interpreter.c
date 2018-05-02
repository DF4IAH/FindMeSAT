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
void Reset_Handler(void);
static void prvDoInterprete(const uint8_t *buf, uint32_t len);
static void prvUnknownCommand(void);

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

    /* Process valid data */
    if (chr) {
      /* Echoing back to USB CDC IN when enabled */
      EventBits_t eb = xEventGroupWaitBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__ECHO_ON, 0, 0, 1);
      if (eb & USB_TO_HOST_EG__BUF_EMPTY) {
        usbToHost(&chr, 1);
        if (chr == 0x0d) {
          /* Add a LF for the terminal */
          osSemaphoreWait(usbToHostBinarySemHandle, 0);
          usbToHostWait((uint8_t*) "\n", 1);
          osSemaphoreRelease(usbToHostBinarySemHandle);
        }
      }

      /* Concatenate for the interpreter buffer */
      if ((chr != 0x0a) && (chr != 0x0d)) {
        inBuf[inBufPos++] = chr;
      }
    }

    /* Process line */
    if ((chr == 0x0d) || (inBufPos >= (sizeof(inBuf) - 1))) {
      inBuf[inBufPos] = 0;

      /* Interpreter */
      if (inBufPos && (inBufPos < (sizeof(inBuf) - 1))) {
        prvDoInterprete(inBuf, inBufPos);
      }
      interpreterShowCursor();

      /* Prepare for next command */
      memset(inBuf, 0, sizeof(inBuf));
      inBufPos = 0;
    }
  }
}


const uint8_t interpreterHelpMsg01[] =
    "\r\n";
const uint8_t interpreterHelpMsg02[] =
    "\t====================================\r\n";
const uint8_t interpreterHelpMsg03[] =
    "\t*  HELP:  Listing of the commands  *\r\n";
const uint8_t interpreterHelpMsg11[] =
    "\tCommand       +\tRemarks\r\n";
const uint8_t interpreterHelpMsg12[] =
    "\t--------------+\t---------------------------------------------\r\n";
const uint8_t interpreterHelpMsg21[] =
    "\thelp\t\tPrint this list of commands.\r\n";
const uint8_t interpreterHelpMsg22[] =
    "\trestart\t\tRestart this device.\r\n";
void interpreterPrintHelp(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));
  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg02, strlen((char*) interpreterHelpMsg02));

  usbToHostWait(interpreterHelpMsg03, strlen((char*) interpreterHelpMsg03));

  usbToHostWait(interpreterHelpMsg02, strlen((char*) interpreterHelpMsg02));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));
  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg11, strlen((char*) interpreterHelpMsg11));

  usbToHostWait(interpreterHelpMsg12, strlen((char*) interpreterHelpMsg12));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg21, strlen((char*) interpreterHelpMsg21));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg22, strlen((char*) interpreterHelpMsg22));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg12, strlen((char*) interpreterHelpMsg12));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));
  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  osSemaphoreRelease(usbToHostBinarySemHandle);
}

const uint8_t interpreterShowCursor01[] =
    "> ";
void interpreterShowCursor(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait(interpreterShowCursor01, strlen((char*) interpreterShowCursor01));
  osSemaphoreRelease(usbToHostBinarySemHandle);
}


/* Private functions ---------------------------------------------------------*/
void prvDoInterprete(const uint8_t *buf, uint32_t len)
{
  const char *cb = (const char*) buf;

  if (!strncmp("help", cb, 4)) {
    interpreterPrintHelp();

  } else if(!strncmp("restart", cb, 7)) {
    Reset_Handler();

  } else {
    prvUnknownCommand();
  }
}


const uint8_t interpreterUnknownCommand01[] =
    "\r\n?? unknown command - please try 'help' ??\r\n\r\n";
void prvUnknownCommand(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait(interpreterUnknownCommand01, strlen((char*) interpreterUnknownCommand01));
  osSemaphoreRelease(usbToHostBinarySemHandle);
}
