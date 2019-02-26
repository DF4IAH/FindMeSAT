/*
 * gpsCom.c
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#include <sys/_stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "main.h"
#include "usb.h"
#include "controller.h"

#include "gpsCom.h"


/* Variables -----------------------------------------------------------------*/
extern osThreadId           gpsComTaskHandle;
extern osMessageQId         gpscomInQueueHandle;
extern osMessageQId         gpscomOutQueueHandle;
extern osTimerId            gpscomtRXTimerHandle;
extern osMutexId            gpscomCtxMutexHandle;
extern EventGroupHandle_t   gpscomEventGroupHandle;
extern EventGroupHandle_t   controllerEventGroupHandle;

extern UART_HandleTypeDef*  usartHuart3;

extern ENABLE_MASK_t        g_enableMsk;
extern MON_MASK_t           g_monMsk;


volatile GpscomGpsCtx_t     gpscomGpsCtx                      = { 0 };


static uint8_t              gpscomGpsTxDmaBuf[128]            = { 0 };

static uint16_t             gpscomGpsRxDmaBufLast             = 0;
static uint16_t             gpscomGpsRxDmaBufIdx              = 0;
static uint8_t              gpscomGpsRxDmaBuf[2048]           = { 0 };

static uint8_t              gpscomGpsRxLineBufIdx             = 0;
static uint8_t              gpscomGpsRxLineBuf[128]           = { 0 };


/* Local functions ----------------------------------------------------------*/

static void prvGpscom_QueueOut_Process(gpscomOutQueueCmds_t cmd)
{
  switch (cmd) {
  case gpscomOutQueueCmds__EndOfParse:
    {
      const uint8_t c_qM[2] = { 1, gpscomOutQueueCmds__EndOfParse };

      for (uint8_t idx = 0; idx < sizeof(c_qM); idx++) {
        xQueueSendToBack(gpscomOutQueueHandle, c_qM + idx, 5 / portTICK_PERIOD_MS);
      }

      /* Set QUEUE_OUT bit */
      xEventGroupSetBits(controllerEventGroupHandle, Controller_EGW__GPSCOM_QUEUE_OUT);
    }
    break;

  default:
    { /* Nothing to be done */ }
  }  // switch (cmd)
}

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

uint8_t gpscomNmeaChecksum(char* outBuf, const uint8_t* inBuf, uint16_t len)
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
        if (*(inBuf + inIdx) != ',') {
          float f = atoff(inBuf + inIdx);
          fAry[fAryIdx++] = f;
          inIdx = prvGpscomNmeaParserStringParser__forwardColon(inBuf, inLen, inIdx);

        } else {
          fAry[fAryIdx++] = 0.f;
        }

      }
      break;

    case 'i':
      {
        /* integer */
        if (*(inBuf + inIdx) != ',') {
          int i = atoi(inBuf + inIdx);
          iAry[iAryIdx++] = i;

        } else {
          iAry[iAryIdx++] = 0;
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
  float   fAry[6] = { 0. };
  int32_t iAry[4] = { 0L };
  uint8_t bAry[4] = { 0U };
  uint8_t fIdx = 0, iIdx = 0, bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "ffbfbiiffbfbii", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    gpscomGpsCtx.time     = fAry[fIdx++];
    gpscomGpsCtx.lat_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.lon_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.piv      = (GpsPosIndicatorValues_t) iAry[iIdx++];
    gpscomGpsCtx.satsUse  = iAry[iIdx++];
    gpscomGpsCtx.hdop     = fAry[fIdx++];
    gpscomGpsCtx.alt_m    = fAry[fIdx++];

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__GLL(const char* buf, uint16_t len)
{
  float   fAry[3] = { 0. };
  int32_t iAry[0] = {    };
  uint8_t bAry[4] = { 0U };
  uint8_t fIdx = 0, /* iIdx = 0, */ bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "fbfbfbb", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    // TODO: function not verified, yet!
    //memset(&gpscomGpsCtx, 0, sizeof(gpscomGpsCtx));    // TODO: remove me!

    gpscomGpsCtx.lat_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.lon_deg  = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.time     = fAry[fIdx++];

    const char status     = bAry[bIdx++];
    gpscomGpsCtx.status   = (status == 'A') ?  GpsStatus__valid : (status == 'V') ?  GpsStatus__notValid : gpscomGpsCtx.status;

    const char mode       = bAry[bIdx++];
    switch (mode) {
    case 'A':
      gpscomGpsCtx.mode   = GpsMode__A_Autonomous;
      break;

    case 'D':
      gpscomGpsCtx.mode   = GpsMode__D_DGPS;
      break;

    case 'N':
      gpscomGpsCtx.mode   = GpsMode__N_DatNotValid;
      break;

    case 'R':
      gpscomGpsCtx.mode   = GpsMode__R_CoarsePosition;
      break;

    case 'S':
      gpscomGpsCtx.mode   = GpsMode__S_Simulator;
      break;

    default:
      { /* Skip setting */ }
    }

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__GSA(const char* buf, uint16_t len)
{
  float   fAry[ 3] = { 0. };
  int32_t iAry[13] = { 0L };
  uint8_t bAry[ 1] = { 0U };
  uint8_t fIdx = 0, iIdx = 0, bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "biiiiiiiiiiiiifff", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    /* Status and modes */
    const char mode1      = bAry[bIdx++];
    gpscomGpsCtx.mode1    = (mode1 == 'A') ?  GpsMode1__A_Automatic : (mode1 == 'M') ?  GpsMode1__M_Manual : gpscomGpsCtx.mode1;
    gpscomGpsCtx.mode2    = (GpsMode2_t) iAry[iIdx++];

#if 0  // Done by GSV
    /* Satellite receiver channels */
    for (uint8_t ch = 1; ch <= Gps_Rcvr_Channels; ch++) {
      gpscomGpsCtx.sv[ch - 1] = iAry[iIdx++];
    }
#endif

    /* Dilution of precision */
    gpscomGpsCtx.pdop     = fAry[fIdx++];
    gpscomGpsCtx.hdop     = fAry[fIdx++];
    gpscomGpsCtx.vdop     = fAry[fIdx++];

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__GSV(const char* buf, uint16_t len)
{
  float   fAry[ 0] = {    };
  int32_t iAry[19] = { 0L };
  uint8_t bAry[ 0] = {    };
  uint8_t /* fIdx = 0, */ iIdx = 0 /*, bIdx = 0 */ ;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "iiiiiiiiiiiiiiiiiii", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    /* Pages */
    const int32_t pages     = iAry[iIdx++];
    const int32_t thisPage  = iAry[iIdx++];
    gpscomGpsCtx.satsView   = iAry[iIdx++];

    /* Sanity check */
    if (thisPage < 1 || thisPage > pages) {
      return;
    }

    /* Satellite channels (GPS / GLONASS) */
    {
      uint8_t chStart = 0;
      uint8_t chEnd   = 0;
      if (buf[2] == 'P') {
        /* GPS entries */
        chStart =  1 + ((thisPage - 1) << 2);
        chEnd   =  min((thisPage << 2), gpscomGpsCtx.satsView);

      } else if (buf[2] == 'N' ||
                 buf[2] == 'D') {
        /* GLONASS / BEIDOU entries */
        chStart = Gps_Channels + 1 + ((thisPage - 1) << 2);
        chEnd   = Gps_Channels + min((thisPage << 2), gpscomGpsCtx.satsView);
      }

      if (chStart && chEnd) {
        /* All channels in use */
        for (uint8_t ch = chStart; ch <= chEnd; ch++) {
          gpscomGpsCtx.sv   [ch - 1]  = iAry[iIdx++];
          gpscomGpsCtx.sElev[ch - 1]  = iAry[iIdx++];
          gpscomGpsCtx.sAzim[ch - 1]  = iAry[iIdx++];
          gpscomGpsCtx.sSNR [ch - 1]  = iAry[iIdx++];
        }
      }
    }

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__RMC(const char* buf, uint16_t len)
{
  float   fAry[5] = { 0. };
  int32_t iAry[2] = { 0L };
  uint8_t bAry[5] = { 0U };
  uint8_t fIdx = 0, iIdx = 0, bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "fbfbfbffiibb", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    gpscomGpsCtx.time       = fAry[fIdx++];

    const char status       = bAry[bIdx++];
    gpscomGpsCtx.status     = (status == 'A') ?  GpsStatus__valid : (status == 'V') ?  GpsStatus__notValid : gpscomGpsCtx.status;

    gpscomGpsCtx.lat_deg    = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);
    gpscomGpsCtx.lon_deg    = prvGpscomCalc_NmeaLonLat_float(fAry[fIdx++], bAry[bIdx++]);

    gpscomGpsCtx.speed_kts  = fAry[fIdx++];
    gpscomGpsCtx.course_deg = fAry[fIdx++];

    gpscomGpsCtx.date       = iAry[iIdx++];

    const int32_t magVar    = iAry[iIdx++];
    (void) magVar;

    const uint8_t magSen    = bAry[bIdx++];
    (void) magSen;

    const char mode         = bAry[bIdx++];
    switch (mode) {
    case 'A':
      gpscomGpsCtx.mode     = GpsMode__A_Autonomous;
      break;

    case 'D':
      gpscomGpsCtx.mode     = GpsMode__D_DGPS;
      break;

    case 'N':
      gpscomGpsCtx.mode     = GpsMode__N_DatNotValid;
      break;

    case 'R':
      gpscomGpsCtx.mode     = GpsMode__R_CoarsePosition;
      break;

    case 'S':
      gpscomGpsCtx.mode     = GpsMode__S_Simulator;
      break;

    default:
      { /* Skip setting */ }
    }

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__VTG(const char* buf, uint16_t len)
{
  float   fAry[4] = { 0. };
  int32_t iAry[0] = {    };
  uint8_t bAry[5] = { 0U };
  uint8_t fIdx = 0, /* iIdx = 0, */ bIdx = 0;

  prvGpscomNmeaParserStringParser(buf + 7, len - 7, "fbfbfbfbb", fAry, iAry, bAry);

  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {
    // TODO: function not verified, yet!
    //memset(&gpscomGpsCtx, 0, sizeof(gpscomGpsCtx));    // TODO: remove me!

    gpscomGpsCtx.course_deg = fAry[fIdx++];
    const uint8_t cRef1     = bAry[bIdx++];
    (void) cRef1;

    const float c2          = fAry[fIdx++];
    const uint8_t cRef2     = bAry[bIdx++];
    (void) c2;
    (void) cRef2;

    gpscomGpsCtx.speed_kts  = fAry[fIdx++];
    const uint8_t spdU      = bAry[bIdx++];
    (void) spdU;

    const float   spd2      = fAry[fIdx++];
    const uint8_t spdU2     = bAry[bIdx++];
    (void) spd2;
    (void) spdU2;

    const char mode         = bAry[bIdx++];
    switch (mode) {
    case 'A':
      gpscomGpsCtx.mode     = GpsMode__A_Autonomous;
      break;

    case 'D':
      gpscomGpsCtx.mode     = GpsMode__D_DGPS;
      break;

    case 'N':
      gpscomGpsCtx.mode     = GpsMode__N_DatNotValid;
      break;

    case 'R':
      gpscomGpsCtx.mode     = GpsMode__R_CoarsePosition;
      break;

    case 'S':
      gpscomGpsCtx.mode     = GpsMode__S_Simulator;
      break;

    default:
      { /* Skip setting */ }
    }

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
}

static void prvGpscomNmeaParser__TXT(const char* buf, uint16_t len)
{

#if 0
  /* Try to get the mutex within that short time or drop the message */
  if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
  {

    /* Give mutex back */
    xSemaphoreGive(gpscomCtxMutexHandle);
  }
#endif

  /* Last NMEA data of a second processed - send end of parse to controller */
  prvGpscom_QueueOut_Process(gpscomOutQueueCmds__EndOfParse);
}

static void prvGpscomNmeaParser__PMT(const char* buf, uint16_t len)
{
  float   fAry[0] = {    };
  int32_t iAry[1] = { 0  };
  uint8_t bAry[0] = {    };
  uint8_t /* fIdx = 0, */ iIdx = 0 /*,  bIdx = 0 */ ;

  if (!strncmp(buf + 1, "PMTK010", 7)) {
    prvGpscomNmeaParserStringParser(buf + 7, len - 7, "i", fAry, iAry, bAry);

    /* Try to get the mutex within that short time or drop the message */
    if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, GpscomNmeaDataMutexWaitMax / portTICK_PERIOD_MS))
    {
      const int32_t msg     = iAry[iIdx++];
      gpscomGpsCtx.bootMsg  = msg;

      /* Give mutex back */
      xSemaphoreGive(gpscomCtxMutexHandle);
    }
  }
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
             (!strncmp(buf + 1, "GNGSA", 5)) ||
             (!strncmp(buf + 1, "BDGSA", 5))) {
    /* GSA - GNSS DOP and Active Satellites */
    prvGpscomNmeaParser__GSA(buf, len);

  } else if ((!strncmp(buf + 1, "GPGSV", 5)) ||
             (!strncmp(buf + 1, "GLGSV", 5)) ||
             (!strncmp(buf + 1, "BDGSV", 5))) {
    /* GSV - GNSS Satellites in View */
    prvGpscomNmeaParser__GSV(buf, len);

  } else if ((!strncmp(buf + 1, "GPRMC", 5)) ||
             (!strncmp(buf + 1, "GNRMC", 5))) {
    /* RMC - Recommended Minimum Specific GNSS Data */
    prvGpscomNmeaParser__RMC(buf, len);

  } else if ( !strncmp(buf + 1, "GPVTG", 5)) {
    /* VTG - Course Over Ground and Ground Speed */
    prvGpscomNmeaParser__VTG(buf, len);

  } else if ( !strncmp(buf + 1, "GPTXT", 5)) {
    /* VTG - Course Over Ground and Ground Speed */
    prvGpscomNmeaParser__TXT(buf, len);

  } else if (!strncmp(buf + 1, "PMT", 3)) {
    /* System response messages */
    prvGpscomNmeaParser__PMT(buf, len);
  }
}

// #define NMEA_DEBUG
static void prvGpscomInterpreter(const uint8_t* buf, uint16_t len)
{
  uint8_t chkVld = gpscomNmeaChecksum(NULL, buf, len);

  /* Debugging */
  if (g_monMsk & MON_MASK__GPS_RX) {
    char  l_buf[64];
    int   l_len;

    if (!strncmp((const char*) buf, "$PMTK", 5)) {
      usbLogLen((const char*) buf, len);

    } else {
      l_len = sprintf(l_buf, "%c%c%c%c%c%c ... %c%c%c - len=%3u   valid=%u\r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],  buf[len - 5], buf[len - 4], buf[len - 3], len, chkVld);
      usbLogLen(l_buf, l_len);
    }
  }

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
    if (!(eb & Gpscom_EGW__DMA_TX_RUN)) {
      /* Failed to send NMEA message */
      return;
    }
  }

  /* Copy to DMA TX buffer */
  memcpy(gpscomGpsTxDmaBuf, cmdBuf, cmdLen);
  memset(gpscomGpsTxDmaBuf + cmdLen, 0, sizeof(gpscomGpsTxDmaBuf) - cmdLen);                    // Clear end of buffer

  /* Re-set flags for TX */
  xEventGroupClearBits(gpscomEventGroupHandle, Gpscom_EGW__DMA_TX_END);
  xEventGroupSetBits(  gpscomEventGroupHandle, Gpscom_EGW__DMA_TX_RUN);

  /* Start transmission */
  if (HAL_OK != HAL_UART_Transmit_DMA(usartHuart3, gpscomGpsTxDmaBuf, cmdLen))
  {
    /* Drop packet */
  }
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

static void prvGpscomSendNMEA(const uint8_t* nmeaStr, uint8_t nmeaStrLen)
{
  char      pushAry[64]   = { 0 };

  if (nmeaStrLen > 3) {
    /* Get command into own buffer */
    for (int len = nmeaStrLen, idx = 0; len; --len, ++idx) {
      *(pushAry + idx) = toupper(*(nmeaStr + idx));
    }

    /* Right trim string */
    while ((*(pushAry + (nmeaStrLen - 1)) == '\r') || (*(pushAry + (nmeaStrLen - 1)) ==  '\n')) {
      if (--nmeaStrLen < 5) {
        return;
      }
      *(pushAry + nmeaStrLen) = 0;
    }

    /* Check if checksum is present */
    if (*(pushAry + (nmeaStrLen - 3)) != '*') {
      uint8_t nmeaChk = 0U;

      /* Calculate the checksum*/
      for (uint16_t idx = 1; idx < nmeaStrLen; idx++) {
        nmeaChk ^= *(pushAry + idx);
      }

      /* Concatenate checksum data */
      nmeaStrLen += sprintf(pushAry + nmeaStrLen, "*%02X", nmeaChk);
    }
    nmeaStrLen += sprintf(pushAry + nmeaStrLen, "\r\n");

    /* Send message to the GPS device */
    prvGpscomGpsTX((uint8_t*) pushAry, nmeaStrLen);
  }
}


/* Global functions ----------------------------------------------------------*/

void gpscomtRXTimerCallbackImpl(TimerHandle_t argument)
{
  /* Set bit */
  xEventGroupSetBits(gpscomEventGroupHandle, Gpscom_EGW__TIMER_SERVICE_RX);
}


/* Calculate Date and Time to unx time */
uint32_t gpscomCalcDataTime_to_unx_s(float* out_s_frac, uint32_t gnss_date, float gnss_time)
{
  time_t l_unx_s;

  {
    struct tm timp  = { 0 };

    uint16_t year   = gnss_date % 100;
    uint8_t  month  = (gnss_date / 100) % 100;
    uint8_t  mday   = (gnss_date / 10000);
    uint8_t  hour   = (uint8_t) (((uint32_t)  gnss_time) / 10000UL);
    uint8_t  minute = (uint8_t) ((((uint32_t) gnss_time) / 100) % 100);
    uint8_t  second = (uint8_t) (((uint32_t)  gnss_time) % 100);

    /* Sanity checks */
    if (month < 1) {
      return 0.f;
    }

    timp.tm_year    = year + 100;
    timp.tm_mon     = month - 1;
    timp.tm_mday    = mday;
    timp.tm_hour    = hour;
    timp.tm_min     = minute;
    timp.tm_sec     = second;
    l_unx_s         = mktime(&timp);
  }

  if (out_s_frac){
    float l_s_frac = gnss_time - floor(gnss_time);
    *out_s_frac = l_s_frac;
  }

  return l_unx_s;
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

    /* Process message if enabled */
    if (ENABLE_MASK__GPS_DATA & g_enableMsk) {
      /* Process the message */
      switch (s_buf[0]) {
      case gpscomInQueueCmds__NmeaSendToGPS:
        {
          prvGpscomSendNMEA(s_buf + 1, s_bufMsgLen - 1);
        }
        break;
      default:
        /* Nothing to do */
        { }
      }  // switch (s_buf[0])
    }  // if (ENABLE_MASK__GPS_DATA .. )

gpscom_Error_clrInBuf:
    {
      /* Clear the buffer to sync */
      s_bufCtr = s_bufMsgLen = 0;
      memset(s_buf, 0, sizeof(s_buf));
    }
  }  // if (eb & Gpscom_EGW__QUEUE_IN)
}
