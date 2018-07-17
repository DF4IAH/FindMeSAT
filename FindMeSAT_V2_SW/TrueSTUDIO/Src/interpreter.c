/*
 * interpreter.c
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

/* Includes ------------------------------------------------------------------*/
#include <sys/_stdint.h>
#include <cmsis_os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"
#include "main.h"
#include "usb.h"
#include "controller.h"

#include "interpreter.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId         usbFromHostQueueHandle;
extern osMessageQId         interOutQueueHandle;
extern osSemaphoreId        usbToHostBinarySemHandle;
extern EventGroupHandle_t   usbToHostEventGroupHandle;
extern EventGroupHandle_t   controllerEventGroupHandle;
extern uint32_t             g_monMsk;
extern char                 usbClrScrBuf[4];


/* Private variables ---------------------------------------------------------*/
const uint16_t              Interpreter_MaxWaitMs             = 100;


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static void prvUnknownCommand(void)
{
  usbLog("\r\n?? unknown command - please try 'help' ??\r\n\r\n");
}

static void prvPushToInterOutQueue(const uint8_t* cmdAry, uint8_t cmdLen)
{
  for (uint8_t idx = 0; idx < cmdLen; ++idx) {
    xQueueSendToBack(interOutQueueHandle, cmdAry + idx, 1);
  }
  xEventGroupSetBits(controllerEventGroupHandle, Controller_EGW__INTER_QUEUE_OUT);
}

static void prvSendNMEA(const char* nmeaStr)
{
  const uint8_t nmeaStrLen = strlen(nmeaStr);
  uint8_t nmeaAry[64] = { (nmeaStrLen + 1),  InterOutQueueCmds__NmeaSend};

  memcpy((nmeaAry + 2), nmeaStr, nmeaStrLen);

  /* Send message to the GPS device */
  prvPushToInterOutQueue(nmeaAry, nmeaStrLen + 2);
}

static void prvDoInterprete(const uint8_t *buf, uint32_t len)
{
  const char *cb = (const char*) buf;

  if (!strncmp("adr ", cb, 4) && (4 < len)) {
    const long    val         = strtol(cb + 4, NULL, 10);
    const uint8_t adrSet      = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__ADRset, adrSet };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("c", cb, 1) && (1 == len)) {
    interpreterClearScreen();

  } else if (!strncmp("conf ", cb, 5) && (5 < len)) {
    const long    val         = strtol(cb + 5, NULL, 10);
    const uint8_t confPackets = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__ConfirmedPackets, confPackets };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("dr ", cb, 3) && (3 < len)) {
    const long    val         = strtol(cb + 3, NULL, 10);
    const uint8_t drSet = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__DRset, drSet };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("help", cb, 4) && (4 == len)) {
    interpreterPrintHelp();

  } else if (!strncmp("push", cb, 4) && (4 == len)) {
    /* Set flag for sending and upload data */
    const uint8_t pushAry[2]  = { 1, InterOutQueueCmds__DoSendDataUp };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("mon ", cb, 4) && (4 < len)) {
    const long    val         = strtol(cb + 4, NULL, 10);
    g_monMsk                  = (uint32_t) val;

  } else if (!strncmp("pwrred ", cb, 7) && (7 < len)) {
    const long    val         = strtol(cb + 7, NULL, 10);
    const uint8_t pwrRed      = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__PwrRedDb, pwrRed };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("reqcheck", cb, 8) && (8 == len)) {
    /* LoRaWAN link check message */
    const uint8_t pushAry[2]  = { 1, InterOutQueueCmds__LinkCheckReq };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("reqtime", cb, 7) && (7 == len)) {
    /* LoRaWAN device time message */
    const uint8_t pushAry[2]  = { 1, InterOutQueueCmds__DeviceTimeReq };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("restart", cb, 7) && (7 == len)) {
    SystemResetbyARMcore();

  } else if (!strncmp("timer ", cb, 6) && (6 < len)) {
    const long    val         = strtol(cb + 6, NULL, 10);
    const uint8_t repeatTimer = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__Timer, repeatTimer };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("$", cb, 1) && (1 < len)) {
    prvSendNMEA(cb);

  } else {
    prvUnknownCommand();
  }
}


/* Global functions ----------------------------------------------------------*/

const uint8_t               interpreterHelpMsg01[]            = "\r\n";
const uint8_t               interpreterHelpMsg02[]            = "\t===============================\r\n";
const uint8_t               interpreterHelpMsg03[]            = "\tHELP - listing of the commands:\r\n";
const uint8_t               interpreterHelpMsg11[]            = "\t\tCommand\t\tRemarks\r\n";
const uint8_t               interpreterHelpMsg12[]            = "\t\t-------\t\t-------\r\n";

const uint8_t               interpreterHelpMsg21[]            = "\t\tadr <n>\t\tADR (adaptive data rate):\r\n";
const uint8_t               interpreterHelpMsg22[]            = "\t\t\t\t\t0 = off.\r\n";
const uint8_t               interpreterHelpMsg23[]            = "\t\t\t\t\t1 = on.\r\n\r\n";

const uint8_t               interpreterHelpMsg24[]            = "\t\tc\t\tClear screen.\r\n\r\n";

const uint8_t               interpreterHelpMsg25[]            = "\t\tconf <n>\tConfirmation of packets:\r\n";
const uint8_t               interpreterHelpMsg26[]            = "\t\t\t\t\t0 = unconfirmed packets.\r\n";
const uint8_t               interpreterHelpMsg27[]            = "\t\t\t\t\t1 = confirmed packets.\r\n\r\n";

const uint8_t               interpreterHelpMsg28[]            = "\t\tdr <n>\t\tDataRate 0..5\r\n\r\n";

const uint8_t               interpreterHelpMsg29[]            = "\t\thelp\t\tPrint this list of commands.\r\n\r\n";

const uint8_t               interpreterHelpMsg30[]            = "\t\tmon <n>\t\tMonitor bitmask:\r\n";
const uint8_t               interpreterHelpMsg31[]            = "\t\t\t\t\t0x01 = LoRa transport.\r\n";
const uint8_t               interpreterHelpMsg32[]            = "\t\t\t\t\t0x08 = GPS time sync.\r\n\r\n";

const uint8_t               interpreterHelpMsg33[]            = "\t\tpush\t\tPush current readings up to LoRa TTN server.\r\n\r\n";

const uint8_t               interpreterHelpMsg34[]            = "\t\tpwrred <n>\tPower reduction 0..20 dB\r\n\r\n";

const uint8_t               interpreterHelpMsg35[]            = "\t\treqcheck\tRequest LoRaWAN link check.\r\n";
const uint8_t               interpreterHelpMsg36[]            = "\t\treqtime\t\tRequest LoRaWAN UTC time.\r\n\r\n";

const uint8_t               interpreterHelpMsg37[]            = "\t\trestart\t\tRestart this device.\r\n\r\n";

const uint8_t               interpreterHelpMsg38[]            = "\t\ttimer <n>\tTimer pushing sensor data up:\r\n";
const uint8_t               interpreterHelpMsg39[]            = "\t\t\t\t\t0  = timer off.\r\n";
const uint8_t               interpreterHelpMsg40[]            = "\t\t\t\t\t>0 = repeat time in seconds.\r\n\r\n";

const uint8_t               interpreterHelpMsg41[]            = "\t\t$...\t\tNMEA command to the GPS device, checksum optional.\r\n\r\n";


void interpreterPrintHelp(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));
  usbToHostWait(interpreterHelpMsg03, strlen((char*) interpreterHelpMsg03));
  usbToHostWait(interpreterHelpMsg02, strlen((char*) interpreterHelpMsg02));
  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg11, strlen((char*) interpreterHelpMsg11));
  usbToHostWait(interpreterHelpMsg12, strlen((char*) interpreterHelpMsg12));
  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  usbToHostWait(interpreterHelpMsg21, strlen((char*) interpreterHelpMsg21));
  usbToHostWait(interpreterHelpMsg22, strlen((char*) interpreterHelpMsg22));
  usbToHostWait(interpreterHelpMsg23, strlen((char*) interpreterHelpMsg23));
  usbToHostWait(interpreterHelpMsg24, strlen((char*) interpreterHelpMsg24));
  usbToHostWait(interpreterHelpMsg25, strlen((char*) interpreterHelpMsg25));
  usbToHostWait(interpreterHelpMsg26, strlen((char*) interpreterHelpMsg26));
  usbToHostWait(interpreterHelpMsg27, strlen((char*) interpreterHelpMsg27));
  usbToHostWait(interpreterHelpMsg28, strlen((char*) interpreterHelpMsg28));
  usbToHostWait(interpreterHelpMsg29, strlen((char*) interpreterHelpMsg29));
  usbToHostWait(interpreterHelpMsg30, strlen((char*) interpreterHelpMsg30));
  usbToHostWait(interpreterHelpMsg31, strlen((char*) interpreterHelpMsg31));
  usbToHostWait(interpreterHelpMsg32, strlen((char*) interpreterHelpMsg32));
  usbToHostWait(interpreterHelpMsg33, strlen((char*) interpreterHelpMsg33));
  usbToHostWait(interpreterHelpMsg34, strlen((char*) interpreterHelpMsg34));
  usbToHostWait(interpreterHelpMsg35, strlen((char*) interpreterHelpMsg35));
  usbToHostWait(interpreterHelpMsg36, strlen((char*) interpreterHelpMsg36));
  usbToHostWait(interpreterHelpMsg37, strlen((char*) interpreterHelpMsg37));
  usbToHostWait(interpreterHelpMsg38, strlen((char*) interpreterHelpMsg38));
  usbToHostWait(interpreterHelpMsg39, strlen((char*) interpreterHelpMsg39));
  usbToHostWait(interpreterHelpMsg40, strlen((char*) interpreterHelpMsg40));
  usbToHostWait(interpreterHelpMsg41, strlen((char*) interpreterHelpMsg41));

  usbToHostWait(interpreterHelpMsg01, strlen((char*) interpreterHelpMsg01));

  osSemaphoreRelease(usbToHostBinarySemHandle);
}

void interpreterShowCursor(void)
{
  usbLog("> ");
}

void interpreterClearScreen(void)
{
  usbLog(usbClrScrBuf);
}


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

  do {
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
            usbLog("\n");
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
  } while (!xQueueIsQueueEmptyFromISR(usbFromHostQueueHandle));
}
