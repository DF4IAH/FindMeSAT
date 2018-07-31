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

extern ENABLE_MASK_t        g_enableMsk;
extern MON_MASK_t           g_monMsk;

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

static void prvSendLoRaBare(const char* sendMsg)
{
  const uint8_t sendMsgLen = strlen(sendMsg);
  uint8_t sendAry[128] = { (sendMsgLen + 2),  InterOutQueueCmds__LoRaBareSend};

  if (sendMsgLen && (sendMsgLen < (sizeof(sendAry) - 2))) {
    memcpy((sendAry + 2), sendMsg, sendMsgLen + 1);

    /* Send message with LoRa bare */
    prvPushToInterOutQueue(sendAry, sendMsgLen + 3);
  }
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
    const long    adrEnable   = strtol(cb + 4, NULL, 10);
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__ADRset, (adrEnable ?  1U : 0U) };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("c", cb, 1) && (1 == len)) {
    interpreterClearScreen();

  } else if (!strncmp("conf ", cb, 5) && (5 < len)) {
    const long    confEnable  = strtol(cb + 5, NULL, 10);
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__ConfirmedPackets, (confEnable ?  1U : 0U) };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("dr ", cb, 3) && (3 < len)) {
    const long    val         = strtol(cb + 3, NULL, 10);
    const uint8_t drSet = (uint8_t) val;
    const uint8_t pushAry[3]  = { 2, InterOutQueueCmds__DRset, drSet };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("f ", cb, 2) && (2 < len)) {
    /* LoRaBare frequency setting [f]=Hz */
    const long    f            = strtol(cb + 2, NULL, 10);
    const uint8_t frequencyHz0 = (uint8_t) (f       ) & 0xffUL;
    const uint8_t frequencyHz1 = (uint8_t) (f >>  8U) & 0xffUL;
    const uint8_t frequencyHz2 = (uint8_t) (f >> 16U) & 0xffUL;
    const uint8_t frequencyHz3 = (uint8_t) (f >> 24U) & 0xffUL;
    const uint8_t pushAry[6]   = { 5, InterOutQueueCmds__LoRaBareFrequency, frequencyHz0, frequencyHz1, frequencyHz2, frequencyHz3 };
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
    /* LoRaWAN power reduction setting [pwrred]=dB */
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

  } else if (!strncmp("rx ", cb, 3) && (3 < len)) {
    /* LoRaBare frequency setting [f]=Hz */
    const long    rxEnable     = strtol(cb + 3, NULL, 10);
    const uint8_t pushAry[3]   = { 2, InterOutQueueCmds__LoRaBareRxEnable, (rxEnable ?  1U : 0U) };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("timer ", cb, 6) && (6 < len)) {
    /* LoRaWAN timer setting [timer]=s */
    const long    val         = strtol(cb + 6, NULL, 10);
    const uint8_t repeatTimer0 = (uint8_t) (val     ) & 0xffUL;
    const uint8_t repeatTimer1 = (uint8_t) (val >> 8) & 0xffUL;
    const uint8_t pushAry[4]  = { 3, InterOutQueueCmds__Timer, repeatTimer0, repeatTimer1 };
    prvPushToInterOutQueue(pushAry, sizeof(pushAry));

  } else if (!strncmp("\"", cb, 1) && (1 < len)) {
    prvSendLoRaBare(cb + 1);

  } else if (!strncmp("$", cb, 1) && (1 < len)) {
    prvSendNMEA(cb);

  } else {
    prvUnknownCommand();
  }
}


/* Global functions ----------------------------------------------------------*/

const uint8_t               interpreterHelpMsg001[]            = "\r\n";
const uint8_t               interpreterHelpMsg002[]            = "\tHELP - list of commands:\r\n";
const uint8_t               interpreterHelpMsg003[]            = "\t========================\r\n";
//const uint8_t             interpreterHelpMsg011[]            = "\t\tCommand\t\tRemarks\r\n";
//const uint8_t             interpreterHelpMsg012[]            = "\t\t-------\t\t-------\r\n";

const uint8_t               interpreterHelpMsg111[]            =     "\t\t> Main commands\r\n";
const uint8_t               interpreterHelpMsg112[]            =     "\t\t--------------------------------------------------------------------------\r\n";
const uint8_t               interpreterHelpMsg121[]            = "\t\tc\t\tClear screen.\r\n";
const uint8_t               interpreterHelpMsg131[]            = "\t\thelp\t\tPrint this list of commands.\r\n";
const uint8_t               interpreterHelpMsg141[]            = "\t\tmon <n>\t\tMonitor bitmask:\r\n";
const uint8_t               interpreterHelpMsg142[]            = "\t\t\t\t\t0x01 = LoRa transport.\r\n";
const uint8_t               interpreterHelpMsg143[]            = "\t\t\t\t\t0x04 = NMEA messages.\r\n";
const uint8_t               interpreterHelpMsg144[]            = "\t\t\t\t\t0x08 = GPS time sync.\r\n";
const uint8_t               interpreterHelpMsg151[]            = "\t\trestart\t\tRestart this device.\r\n\r\n";

const uint8_t               interpreterHelpMsg211[]            = "\r\n\t\t> Common LoRa settings\r\n";
//const uint8_t             interpreterHelpMsgXXX[]            =     "\t\t--------------------------------------------------------------------------\r\n";
const uint8_t               interpreterHelpMsg221[]            = "\t\tdr <n>\t\tDataRate 0..5\r\n";
const uint8_t               interpreterHelpMsg231[]            = "\t\tpwrred <n>\tPower reduction of 0..20 dB\r\n\r\n";

const uint8_t               interpreterHelpMsg311[]            = "\r\n\t\t> LoRa bare mode specific settings\r\n";
//const uint8_t             interpreterHelpMsgXXX[]            =     "\t\t--------------------------------------------------------------------------\r\n";
const uint8_t               interpreterHelpMsg321[]            = "\t\tf <n>\t\tFrequency (TX/RX) in Hz.\r\n";
const uint8_t               interpreterHelpMsg331[]            = "\t\trx <n>\t\tReceiver activation:\r\n";
const uint8_t               interpreterHelpMsg332[]            = "\t\t\t\t\t0 = off.\r\n";
const uint8_t               interpreterHelpMsg333[]            = "\t\t\t\t\t1 = on.\r\n";
const uint8_t               interpreterHelpMsg341[]            = "\t\t\"...\t\tTransmit message via LoRa bare mode.\r\n\r\n";

const uint8_t               interpreterHelpMsg411[]            = "\r\n\t\t> LoRaWAN mode specific settings\r\n";
//const uint8_t             interpreterHelpMsgXXX[]            =     "\t\t--------------------------------------------------------------------------\r\n";
const uint8_t               interpreterHelpMsg421[]            = "\t\tadr <n>\t\tADR (adaptive data rate):\r\n";
const uint8_t               interpreterHelpMsg422[]            = "\t\t\t\t\t0 = off.\r\n";
const uint8_t               interpreterHelpMsg423[]            = "\t\t\t\t\t1 = on.\r\n";
const uint8_t               interpreterHelpMsg431[]            = "\t\tconf <n>\tConfirmation of packets:\r\n";
const uint8_t               interpreterHelpMsg432[]            = "\t\t\t\t\t0 = unconfirmed packets.\r\n";
const uint8_t               interpreterHelpMsg433[]            = "\t\t\t\t\t1 = confirmed packets.\r\n";
const uint8_t               interpreterHelpMsg441[]            = "\t\tpush\t\tPush current readings up to LoRaWAN application server.\r\n";
const uint8_t               interpreterHelpMsg451[]            = "\t\treqcheck\tRequest LoRaWAN link check.\r\n";
const uint8_t               interpreterHelpMsg461[]            = "\t\treqtime\t\tRequest LoRaWAN UTC time.\r\n";
const uint8_t               interpreterHelpMsg471[]            = "\t\ttimer <n>\tTimer pushing sensor data up:\r\n";
const uint8_t               interpreterHelpMsg472[]            = "\t\t\t\t\t 0 = timer off.\r\n";
const uint8_t               interpreterHelpMsg473[]            = "\t\t\t\t\t>0 = repeat time in seconds.  (1..4 --> 5 sec)\r\n\r\n";

const uint8_t               interpreterHelpMsg511[]            = "\r\n\t\t> GPS commands\r\n";
//const uint8_t             interpreterHelpMsgXXX[]            =     "\t\t--------------------------------------------------------------------------\r\n";
const uint8_t               interpreterHelpMsg521[]            = "\t\t$...\t\tNMEA command to the GPS device, checksum optional.\r\n\r\n";


void interpreterPrintHelp(void)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);

  usbToHostWait(interpreterHelpMsg001, strlen((char*) interpreterHelpMsg001));
  usbToHostWait(interpreterHelpMsg002, strlen((char*) interpreterHelpMsg002));
  usbToHostWait(interpreterHelpMsg003, strlen((char*) interpreterHelpMsg003));
  usbToHostWait(interpreterHelpMsg001, strlen((char*) interpreterHelpMsg001));

//usbToHostWait(interpreterHelpMsg011, strlen((char*) interpreterHelpMsg011));
//usbToHostWait(interpreterHelpMsg012, strlen((char*) interpreterHelpMsg012));
//usbToHostWait(interpreterHelpMsg001, strlen((char*) interpreterHelpMsg001));

  usbToHostWait(interpreterHelpMsg111, strlen((char*) interpreterHelpMsg111));
  usbToHostWait(interpreterHelpMsg112, strlen((char*) interpreterHelpMsg112));
  usbToHostWait(interpreterHelpMsg121, strlen((char*) interpreterHelpMsg121));
  usbToHostWait(interpreterHelpMsg131, strlen((char*) interpreterHelpMsg131));
  usbToHostWait(interpreterHelpMsg141, strlen((char*) interpreterHelpMsg141));
  usbToHostWait(interpreterHelpMsg142, strlen((char*) interpreterHelpMsg142));
  usbToHostWait(interpreterHelpMsg143, strlen((char*) interpreterHelpMsg143));
  usbToHostWait(interpreterHelpMsg144, strlen((char*) interpreterHelpMsg144));
  usbToHostWait(interpreterHelpMsg151, strlen((char*) interpreterHelpMsg151));

  usbToHostWait(interpreterHelpMsg211, strlen((char*) interpreterHelpMsg211));
  usbToHostWait(interpreterHelpMsg112, strlen((char*) interpreterHelpMsg112));
  usbToHostWait(interpreterHelpMsg221, strlen((char*) interpreterHelpMsg221));
  usbToHostWait(interpreterHelpMsg231, strlen((char*) interpreterHelpMsg231));

  usbToHostWait(interpreterHelpMsg311, strlen((char*) interpreterHelpMsg311));
  usbToHostWait(interpreterHelpMsg112, strlen((char*) interpreterHelpMsg112));
  usbToHostWait(interpreterHelpMsg321, strlen((char*) interpreterHelpMsg321));
  usbToHostWait(interpreterHelpMsg331, strlen((char*) interpreterHelpMsg331));
  usbToHostWait(interpreterHelpMsg332, strlen((char*) interpreterHelpMsg332));
  usbToHostWait(interpreterHelpMsg333, strlen((char*) interpreterHelpMsg333));
  usbToHostWait(interpreterHelpMsg341, strlen((char*) interpreterHelpMsg341));

  usbToHostWait(interpreterHelpMsg411, strlen((char*) interpreterHelpMsg411));
  usbToHostWait(interpreterHelpMsg112, strlen((char*) interpreterHelpMsg112));
  usbToHostWait(interpreterHelpMsg421, strlen((char*) interpreterHelpMsg421));
  usbToHostWait(interpreterHelpMsg422, strlen((char*) interpreterHelpMsg422));
  usbToHostWait(interpreterHelpMsg423, strlen((char*) interpreterHelpMsg423));
  usbToHostWait(interpreterHelpMsg431, strlen((char*) interpreterHelpMsg431));
  usbToHostWait(interpreterHelpMsg432, strlen((char*) interpreterHelpMsg432));
  usbToHostWait(interpreterHelpMsg433, strlen((char*) interpreterHelpMsg433));
  usbToHostWait(interpreterHelpMsg441, strlen((char*) interpreterHelpMsg441));
  usbToHostWait(interpreterHelpMsg451, strlen((char*) interpreterHelpMsg451));
  usbToHostWait(interpreterHelpMsg461, strlen((char*) interpreterHelpMsg461));
  usbToHostWait(interpreterHelpMsg471, strlen((char*) interpreterHelpMsg471));
  usbToHostWait(interpreterHelpMsg472, strlen((char*) interpreterHelpMsg472));
  usbToHostWait(interpreterHelpMsg473, strlen((char*) interpreterHelpMsg473));

  usbToHostWait(interpreterHelpMsg511, strlen((char*) interpreterHelpMsg511));
  usbToHostWait(interpreterHelpMsg112, strlen((char*) interpreterHelpMsg112));
  usbToHostWait(interpreterHelpMsg521, strlen((char*) interpreterHelpMsg521));

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
