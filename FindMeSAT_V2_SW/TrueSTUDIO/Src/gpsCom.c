/*
 * gpsCom.c
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#include <sys/_stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "main.h"
#include "usb.h"

#include "gpsCom.h"


/* Variables -----------------------------------------------------------------*/
extern osThreadId           gpsComTaskHandle;
extern osMessageQId         gpscomInQueueHandle;
extern osMessageQId         gpscomOutQueueHandle;
extern osTimerId            gpscomtRXTimerHandle;
extern EventGroupHandle_t   gpscomEventGroupHandle;
extern UART_HandleTypeDef*  usartHuart3;


GpscomGpsCtx_t              gpscomGpsCtx                      = { };


static uint8_t              gpscomGpsTxDmaBuf[128]            = { 0 };

static uint16_t             gpscomGpsRxDmaBufLast             = 0;
static uint16_t             gpscomGpsRxDmaBufIdx              = 0;
static uint8_t              gpscomGpsRxDmaBuf[2048]           = { 0 };

static uint8_t              gpscomGpsRxLineBufIdx             = 0;
static uint8_t              gpscomGpsRxLineBuf[128]           = { 0 };


/* Global functions ----------------------------------------------------------*/

static float prvGpscomCalc_NmeaLonLat_float(float dddmm, uint8_t indicator)
{
  float latLon_deg  = floor(dddmm / 100.);
  float latLon_min  = dddmm - (latLon_deg * 100.);

  latLon_deg += latLon_min / 60.;

  /* Sign reversal */
  if ((indicator == 'W') || (indicator == 'S')) {
    latLon_deg = -latLon_deg;
  }

  return latLon_deg;
}

static uint8_t prvGpscomNmeaChecksum(char* outBuf, const uint8_t* inBuf, uint16_t len)
{
  char l_crcPad[3] = { 0 };

  if ((len > 10) && inBuf) {
    const uint16_t starIdx = len - 5;

    if (*(inBuf + starIdx) == '*') {
      uint8_t sum = 0;

      for (uint16_t idx = 1; idx < starIdx; idx++) {
        sum ^= *(inBuf + idx);
      }
      sprintf(l_crcPad, "%02X", sum);

      if (outBuf) {
        memcpy(outBuf, l_crcPad, 2);
      }
      return ((l_crcPad[0] == inBuf[starIdx + 1]) && (l_crcPad[1] == inBuf[starIdx + 2])) ?  1 : 0;
    }
  }
  return 0;
}

static uint16_t prvGpscomNmeaParserStringParser__forwardColon(const char* inBuf, uint16_t inLen, uint16_t inBufIdx)
{
  char c = *(inBuf + inBufIdx++);

  /* Forward to next colon */
  while (c != ',') {
    if (inBufIdx >= inLen) {
      return min(inBufIdx, inLen);
    }

    c = *(inBuf + inBufIdx++);
  }

  /* Return one position after the colon */
  return inBufIdx;
}

static HAL_StatusTypeDef prvGpscomNmeaParserStringParser(const char* inBuf, uint16_t inLen, const char* fmtAry, float* fAry, int32_t* iAry, uint8_t* bAry)
{
  const uint8_t fmtLen  = strlen(fmtAry);
  uint8_t       fmtIdx  = 0;
  uint16_t      inIdx   = 0U;
  uint8_t       fAryIdx = 0U;
  uint8_t       iAryIdx = 0U;
  uint8_t       bAryIdx = 0U;

  while (fmtIdx < fmtLen) {
    char fmt = *(fmtAry + fmtIdx++);

    switch (fmt) {
    case 'f':
      {
        /* Floating point */
        {
          float f = atoff(inBuf + inIdx);
          fAry[fAryIdx++] = f;
        }

        inIdx = prvGpscomNmeaParserStringParser__forwardColon(inBuf, inLen, inIdx);
      }
      break;

    case 'i':
      {
        /* integer */
        {
          int i = atoi(inBuf + inIdx);
          iAry[iAryIdx++] = i;
        }

        inIdx = prvGpscomNmeaParserStringParser__forwardColon(inBuf, inLen, inIdx);
      }
      break;

    case 'b':
      {
        /* byte */
        if (inIdx < inLen) {
          uint8_t b = *(((uint8_t*) inBuf) + inIdx++);
          if (b != ',') {
            bAry[bAryIdx++] = b;
            inIdx++;

          } else {
            bAry[bAryIdx++] = 0U;
          }
        }
      }
      break;

    default:
      /* Bad format identifier */
      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

static void prvGpscomNmeaParser__GGA(const char* buf, uint16_t len)
{
  float   fAry[8] = { 0. };
  int32_t iAry[8] = { 0L };
  uint8_t bAry[8] = { 0U };
  uint8_t fIdx = 0, iIdx = 0, bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "ffbfbiiffbfbii", fAry, iAry, bAry);

  xSemaphoreTake(gpscomGpsCtxMutex, portMAX_DELAY);
  {
    gpscomGpsCtx.time     = fAry[fIdx++];
    gpscomGpsCtx.lat_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.lon_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.mode     = (GpsMode_t) iAry[iIdx++];
    gpscomGpsCtx.satsUse  = iAry[iIdx++];
    // TODO: . . .
  }
  xSemaphoreGive(gpscomGpsCtxMutex);

  __asm volatile( "ISB" );
}

static void prvGpscomNmeaParser__GLL(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser__GSA(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser__GSV(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser__RMC(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser__VTG(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser__PMT(const char* buf, uint16_t len)
{
}

static void prvGpscomNmeaParser(const char* buf, uint16_t len)
{
  if        ((!strncmp(buf + 1, "GPGGA", 5)) ||
             (!strncmp(buf + 1, "GNGGA", 5))) {
    /* Global Positioning System Fix Data */
    prvGpscomNmeaParser__GGA(buf, len);

  } else if ((!strncmp(buf + 1, "GPGLL", 5)) ||
             (!strncmp(buf + 1, "GNGLL", 5))) {
    /* GLL - Geographic Position - Latitude / Longitude */
    prvGpscomNmeaParser__GLL(buf, len);

  } else if ((!strncmp(buf + 1, "GPGSA", 5)) ||
             (!strncmp(buf + 1, "GNGSA", 5))) {
    /* GSA - GNSS DOP and Active Satellites */
    prvGpscomNmeaParser__GSA(buf, len);

  } else if ((!strncmp(buf + 1, "GPGSV", 5)) ||
             (!strncmp(buf + 1, "GLGSV", 5))) {
    /* GSV - GNSS Satellites in View */
    prvGpscomNmeaParser__GSV(buf, len);

  } else if ((!strncmp(buf + 1, "GPRMC", 5)) ||
             (!strncmp(buf + 1, "GNRMC", 5))) {
    /* RMC - Recommended Minimum Specific GNSS Data */
    prvGpscomNmeaParser__RMC(buf, len);

  } else if ( !strncmp(buf + 1, "GPVTG", 5)) {
    /* VTG - Course Over Ground and Ground Speed */
    prvGpscomNmeaParser__VTG(buf, len);

  } else if (!strncmp(buf + 1, "PMT", 3)) {
    /* System response messages */
    prvGpscomNmeaParser__PMT(buf, len);
  }
}

static void prvGpscomInterpreter(const uint8_t* buf, uint16_t len)
{
  uint8_t chkVld = prvGpscomNmeaChecksum(NULL, buf, len);

#if 1
  /* Debugging */
  {
    char  l_buf[64];
    int   l_len = sprintf(l_buf, "%c%c%c%c%c%c ... %c%c%c - len=%3u   valid=%u\r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],  buf[len - 5], buf[len - 4], buf[len - 3], len, chkVld);

    usbLogLen(l_buf, l_len);
  }
#endif

  if (chkVld) {
    prvGpscomNmeaParser((const char*) buf, len);
  }
}

static void prvGpscomGpsTX(const uint8_t* cmdBuf, uint8_t cmdLen)
{
  EventBits_t eb = xEventGroupGetBits(gpscomEventGroupHandle);

  /* When TX is running wait for the end of the previous transfer */
  if (eb & Gpscom_EGW__DMA_TX_RUN) {
    /* Block until end of transmission - at max. 1 sec */
    eb = xEventGroupWaitBits(gpscomEventGroupHandle,
        Gpscom_EGW__DMA_TX_END,
        0,
        0, 1000 / portTICK_PERIOD_MS);
  }

  /* Copy to DMA TX buffer */
  memcpy(gpscomGpsTxDmaBuf, cmdBuf, cmdLen);
  memset(gpscomGpsTxDmaBuf + cmdLen, 0, sizeof(gpscomGpsTxDmaBuf) - cmdLen);

  /* Re-set flags for TX */
  xEventGroupClearBits(gpscomEventGroupHandle, Gpscom_EGW__DMA_TX_END);
  xEventGroupSetBits(  gpscomEventGroupHandle, Gpscom_EGW__DMA_TX_RUN);

  /* Start transmission */
  if (HAL_OK != HAL_UART_Transmit_DMA(usartHuart3, gpscomGpsTxDmaBuf, cmdLen))
  {
    /* Drop packet */
  }
}

static void prvGpscomGpsServiceRX(void)
{
  taskDISABLE_INTERRUPTS();

  /* Interrupt locked block */
  {
    /* Find last written byte */
    gpscomGpsRxDmaBufIdx = gpscomGpsRxDmaBufLast + strnlen((char*)gpscomGpsRxDmaBuf + gpscomGpsRxDmaBufLast, sizeof(gpscomGpsRxDmaBuf) - gpscomGpsRxDmaBufLast);

    /* Slice DMA buffer data to the line buffer */
    while (gpscomGpsRxDmaBufLast < gpscomGpsRxDmaBufIdx) {
      char c;

      /* Byte copy */
      gpscomGpsRxLineBuf[gpscomGpsRxLineBufIdx++] = c = gpscomGpsRxDmaBuf[gpscomGpsRxDmaBufLast];
      gpscomGpsRxDmaBuf[gpscomGpsRxDmaBufLast++]  = 0;

      /* Process line buffer after line feed (LF) */
      if (c == '\n' || gpscomGpsRxLineBufIdx == sizeof(gpscomGpsRxLineBuf)) {
        uint8_t buf[ sizeof(gpscomGpsRxLineBuf) ];
        uint8_t len;

        /* Create a work copy of the line buffer - DMA disabled block */
        {
          /* Make a work copy */
          len = gpscomGpsRxLineBufIdx;
          memcpy(buf, gpscomGpsRxLineBuf, len);

          /* Clear the line buffer */
          gpscomGpsRxLineBufIdx = 0;
          memset(gpscomGpsRxLineBuf, 0, sizeof(gpscomGpsRxLineBuf));
        }

        /* Work on line buffer content */
        prvGpscomInterpreter(buf, len);
      }
    }

    /* Loop around */
    if (gpscomGpsRxDmaBufLast >= sizeof(gpscomGpsRxDmaBuf)) {
      gpscomGpsRxDmaBufLast = 0U;
    }
  }

  taskENABLE_INTERRUPTS();
}

static void prvGpscomGpsRX(void)
{
  const uint16_t dmaBufSize = sizeof(gpscomGpsRxDmaBuf);

  /* Reset working indexes */
  gpscomGpsRxDmaBufLast = gpscomGpsRxDmaBufIdx = 0U;

  /* Start RX DMA */
  if (HAL_UART_Receive_DMA(usartHuart3, gpscomGpsRxDmaBuf, dmaBufSize) != HAL_OK)
  {
    //Error_Handler();
  }

  /* Set RX running flag */
  xEventGroupSetBits(gpscomEventGroupHandle, Gpscom_EGW__DMA_RX_RUN);
}


void gpscomtRXTimerCallbackImpl(TimerHandle_t argument)
{
  /* Set bit */
  xEventGroupSetBits(gpscomEventGroupHandle, Gpscom_EGW__TIMER_SERVICE_RX);
}


void gpscomGpscomTaskInit(void)
{
  uint8_t inChr = 0;

  /* Wait until RTOS is ready */
  while (!usartHuart3) {
    osDelay(100);
  }

  /* Clear queue */
  while (xQueueReceive(gpscomInQueueHandle, &inChr, 0) == pdPASS) {
  }

  /* Start timer */
  xTimerStart(gpscomtRXTimerHandle, 1);
  xTimerChangePeriod(gpscomtRXTimerHandle, 50 / portTICK_PERIOD_MS, 1);                         // 50 ms
}

void gpscomGpscomTaskLoop(void)
{
  static uint8_t  s_buf[32]   = { 0 };
  static uint8_t  s_bufCtr    = 0;
  static uint8_t  s_bufMsgLen = 0;
  EventBits_t     eb;
  BaseType_t      xStatus;
  uint8_t         inChr;

  /* Re-activate RX com */
  eb = xEventGroupGetBits(gpscomEventGroupHandle);
  if (!(eb & Gpscom_EGW__DMA_RX_RUN)) {
    prvGpscomGpsRX();
  }

  /* Wait for events to occur */
  eb = xEventGroupWaitBits(gpscomEventGroupHandle,
      Gpscom_EGW__QUEUE_IN | Gpscom_EGW__DMA_TX_END | Gpscom_EGW__DMA_RX50 | Gpscom_EGW__DMA_RX100_END | Gpscom_EGW__DMA_TXRX_ERROR | Gpscom_EGW__TIMER_SERVICE_RX,
      Gpscom_EGW__QUEUE_IN | Gpscom_EGW__DMA_TX_END | Gpscom_EGW__DMA_RX50 | Gpscom_EGW__DMA_RX100_END | Gpscom_EGW__DMA_TXRX_ERROR | Gpscom_EGW__TIMER_SERVICE_RX,
      0, portMAX_DELAY);

  if (eb & Gpscom_EGW__DMA_TXRX_ERROR) {
    //Error_Handler();
  }

  if (eb & Gpscom_EGW__TIMER_SERVICE_RX) {
    prvGpscomGpsServiceRX();
  }

  if (eb & Gpscom_EGW__DMA_RX50) {
    /* Clear last state */
    xEventGroupClearBits(gpscomEventGroupHandle, Gpscom_EGW__DMA_RX50);

    /* Work on lower part of RX DMA buffer */
    prvGpscomGpsServiceRX();
  }

  if (eb & Gpscom_EGW__DMA_RX100_END) {
    /* Clear last state */
    xEventGroupClearBits(gpscomEventGroupHandle, Gpscom_EGW__DMA_RX100_END);

    /* Start RX again and work on higher part of RX DMA buffer */
    prvGpscomGpsServiceRX();
  }

  if (eb & Gpscom_EGW__DMA_TX_END) {
    /* Nothing to be done */
  }

  if (eb & Gpscom_EGW__QUEUE_IN) {
    /* Wait for input from the controller */
    do {
      /* Take next character from the queue, if any */
      inChr = 0;
      xStatus = xQueueReceive(gpscomInQueueHandle, &inChr, 100 / portTICK_PERIOD_MS);           // Wait max. 100 ms for completion
      if (pdPASS == xStatus) {
        if (!s_bufMsgLen) {
          s_bufMsgLen = inChr;

        } else {
          /* Process incoming message */
          s_buf[s_bufCtr++] = inChr;

          if (s_bufCtr == s_bufMsgLen) {
            /* Message complete */
            break;
          }
        }  // if (!s_bufMsgLen) else

      } else {
        /* Reset the state of the queue */
        goto gpscom_Error_clrInBuf;
      }
    } while (1);

    /* Process the message */
    switch (s_buf[0]) {
    case 0:  //GpscomInQueueCmds__XXX:
      {
  //      /* Set event mask bit for INIT */
  //      xEventGroupSetBits(loraEventGroupHandle, LORAWAN_EGW__DO_INIT);
      }
      break;
    default:
      /* Nothing to do */
      { }
    }  // switch (s_buf[0])

gpscom_Error_clrInBuf:
    {
      /* Clear the buffer to sync */
      s_bufCtr = s_bufMsgLen = 0;
      memset(s_buf, 0, sizeof(s_buf));
    }
  }  // if (eb & Gpscom_EGW__QUEUE_IN)
}
