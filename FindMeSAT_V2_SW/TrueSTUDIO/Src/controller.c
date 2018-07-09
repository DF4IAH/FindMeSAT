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
#include <math.h>
#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"
#include "usb.h"
#include "interpreter.h"
#include "adc.h"
#include "spi.h"
#include "LoRaWAN.h"
#include "gpsCom.h"

#include "controller.h"

//#pragma GCC diagnostic ignored "-Wformat"


/* Private variables ---------------------------------------------------------*/
extern osMessageQId         loraInQueueHandle;
extern osMessageQId         loraOutQueueHandle;
extern osMessageQId         gpscomInQueueHandle;
extern osMessageQId         gpscomOutQueueHandle;
extern osMessageQId         sensorsInQueueHandle;
extern osMessageQId         sensorsOutQueueHandle;
extern osMessageQId         interOutQueueHandle;
extern osTimerId            controllerSendTimerHandle;
extern osMutexId            trackMeApplUpDataMutexHandle;
extern osMutexId            trackMeApplDnDataMutexHandle;
extern osMutexId            gpscomCtxMutexHandle;
extern osSemaphoreId        usbToHostBinarySemHandle;
extern osSemaphoreId        trackMeApplUpBinarySemHandle;
extern EventGroupHandle_t   usbToHostEventGroupHandle;
extern EventGroupHandle_t   loraEventGroupHandle;
extern EventGroupHandle_t   controllerEventGroupHandle;

extern LoRaWANctx_t         loRaWANctx;
extern GpscomGpsCtx_t       gpscomGpsCtx;

/* Application data for track_me */
extern TrackMeApp_up_t      trackMeApp_up;
extern TrackMeApp_down_t    trackMeApp_down;

extern LoraliveApp_up_t     loraliveApp_up;
extern LoraliveApp_down_t   loraliveApp_down;

const uint16_t              Controller_MaxWaitMs              = 100;


/* Private function prototypes -----------------------------------------------*/
static void prvControllerInitBeforeGreet(void);
static void prvControllerInitAfterGreet(void);
static void prvControllerUsbGreet(void);
static void prvControllerPrintMCU(void);


/* Private functions ---------------------------------------------------------*/
const char controllerGreetMsg01[] = "\r\n";
const char controllerGreetMsg02[] = "+===========================================================+\r\n";
const char controllerGreetMsg03[] = "*                                                           *\r\n";
const char controllerGreetMsg04[] = "*  FindMeSAT V2 - by DF4IAH - ARM powered by STM32L496ZG-P  *\r\n";

const char controllerGreetMsg11[] =
    "\tFindMeSAT_V2 version:\r\n"
    "\t=====================\r\n"
    "\r\n"
    "\t\tSoftware date\t%08u\r\n";
void prvControllerUsbGreet(void)
{
  char verBuf[128];

  sprintf(verBuf, controllerGreetMsg11, FINDMESAT_VERSION);

  /* usbToHost block */
  {
    osSemaphoreWait(usbToHostBinarySemHandle, 0);

    usbToHostWait((uint8_t*) controllerGreetMsg02, strlen(controllerGreetMsg02));
    usbToHostWait((uint8_t*) controllerGreetMsg03, strlen(controllerGreetMsg03));
    usbToHostWait((uint8_t*) controllerGreetMsg04, strlen(controllerGreetMsg04));
    usbToHostWait((uint8_t*) controllerGreetMsg03, strlen(controllerGreetMsg03));
    usbToHostWait((uint8_t*) controllerGreetMsg02, strlen(controllerGreetMsg02));
    usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));
    usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));

    usbToHostWait((uint8_t*) verBuf, strlen(verBuf));
    usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));
    usbToHostWait((uint8_t*) controllerGreetMsg01, strlen(controllerGreetMsg01));

    osSemaphoreRelease(usbToHostBinarySemHandle);
  }
}

void prvControllerInitBeforeGreet(void)
{
  /* USB typing echo */
  xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__ECHO_ON);                       // TODO: should be from Config-FLASH page

  /* Check for attached SX127x_mbed_shield */
  if (HAL_OK == spiDetectShieldSX127x()) {
#if 1
    /* Send INIT message to the LoRaWAN task */
    const uint8_t c_maxWaitMs = 25;
    const uint8_t c_msgToLoRaWAN[2] = { 1, LoraInQueueCmds__Init };

    /* Write message into loraInQueue */
    for (uint8_t idx = 0; idx < sizeof(c_msgToLoRaWAN); idx++) {
      xQueueSendToBack(loraInQueueHandle, c_msgToLoRaWAN + idx, c_maxWaitMs);
    }

    /* Set QUEUE_IN bit */
    xEventGroupSetBits(loraEventGroupHandle, Lora_EGW__QUEUE_IN);
#endif
  }
}

void prvControllerInitAfterGreet(void)
{
  /* Print MCU infos */
  prvControllerPrintMCU();

  /* Print help table */
  interpreterPrintHelp();

  /* At the last position show the cursor */
  interpreterShowCursor();

  /* Test LoRaWAN access */
  if (loRaWANctx.bkpRAM)
  {
#if 0
    loraliveApp_up.id = 'E';

    loraliveApp_up.voltage_32_v  = (uint8_t) (3.3f * 32 + 0.5f);

    loraliveApp_up.dust025_10_hi = 0U; loraliveApp_up.dust025_10_lo = 0U;
    loraliveApp_up.dust025_10_hi = 0U; loraliveApp_up.dust025_10_lo = 0U;

    uint32_t latitude_1000 = 49473;
    loraliveApp_up.u.l14.latitude_1000_sl24  = (uint8_t) ((latitude_1000  >> 24) & 0xffUL);
    loraliveApp_up.u.l14.latitude_1000_sl16  = (uint8_t) ((latitude_1000  >> 16) & 0xffUL);
    loraliveApp_up.u.l14.latitude_1000_sl08  = (uint8_t) ((latitude_1000  >>  8) & 0xffUL);
    loraliveApp_up.u.l14.latitude_1000_sl00  = (uint8_t) ((latitude_1000  >>  0) & 0xffUL);

    uint32_t longitude_1000 = 8615;
    loraliveApp_up.u.l14.longitude_1000_sl24 = (uint8_t) ((longitude_1000 >> 24) & 0xffUL);
    loraliveApp_up.u.l14.longitude_1000_sl16 = (uint8_t) ((longitude_1000 >> 16) & 0xffUL);
    loraliveApp_up.u.l14.longitude_1000_sl08 = (uint8_t) ((longitude_1000 >>  8) & 0xffUL);
    loraliveApp_up.u.l14.longitude_1000_sl00 = (uint8_t) ((longitude_1000 >>  0) & 0xffUL);

    /* TODO_ DEBUG Loop to be removed */
    for (uint8_t i = 0; i < 3; i++) {
      usbLog("\r\nTX:\r\n");
      LoRaWAN_App_trackMeApp_pushUp(&loRaWANctx, &loraliveApp_up, 14);

      usbLog("\r\nRX:\r\n");
      LoRaWAN_App_trackMeApp_receiveLoop(&loRaWANctx);
    }

    while (1) {
      spiPreviousWakeTime = xTaskGetTickCount();

      usbLog("\r\nRX:\r\n");
      LoRaWAN_App_trackMeApp_receiveLoop(&loRaWANctx);
    }
#endif
  }
}

const char controllerPackages0x00[] = "LQFP64";
const char controllerPackages0x02[] = "LQFP100";
const char controllerPackages0x03[] = "UFBGA132";
const char controllerPackages0x04[] = "LQFP144, WLCSP81 or WLCSP72";
const char controllerPackages0x10[] = "UFBGA169";
const char controllerPackages0x11[] = "WLCSP100";
const char controllerPackages0xXX[] = "(reserved)";
void prvControllerPrintMCU(void)
{
  char lotBuf[8];
  char buf[220] = { 0 };
  const char *packagePtr = NULL;

  uint32_t uidPosX    = (*((uint32_t*)  UID_BASE     )      ) & 0X0000ffffUL;
  uint32_t uidPosY    = (*((uint32_t*)  UID_BASE     ) >> 16) & 0X0000ffffUL;
  uint32_t uidWaf     = (*((uint32_t*) (UID_BASE + 4))      ) & 0X000000ffUL;
  char* uidLot        = ((char*)       (UID_BASE + 5));
  memcpy((void*)lotBuf, (const void*)uidLot, 7);
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

  uint16_t flashSize      = (uint16_t) ((*((uint32_t*) FLASHSIZE_BASE)) & 0x0000ffffUL);

  /* Request ADC1 Vdda */
  uint32_t adc1Vdda_mV = adcGetVdda_mV();

  /* Request ADC1 Vbat */
  uint32_t adc1Vbat_mV    = adcGetVbat_mV();

  /* Request ADC1 Temp */
  int32_t  adc1Temp_100   = adcGetTemp_100();
  int16_t  adc1Temp_100_i = adc1Temp_100 / 100;
  uint16_t adc1Temp_100_f = adc1Temp_100 >= 0 ?  (adc1Temp_100 % 100) : (-adc1Temp_100 % 100);

  int len = sprintf(buf,
      "\r\n"
      "\tMCU Info:\r\n"
      "\t=========\r\n"
      "\r\n"
      "\t\tLot-ID\t\t%s\r\n"
      "\t\tWafer\t\t%lu\r\n"
      "\t\tPos. X/Y\t%2lu/%2lu\r\n"
      "\t\tPackage(s)\t%s\r\n"
      "\t\tFlash size\t%4u kB\r\n"
      "\t\tVdda\t\t%4lu mV\r\n"
      "\t\tVbat\t\t%4lu mV\r\n"
      "\t\tMCU Temp.\t%+02d.%02u C\r\n\r\n\r\n",
      lotBuf, uidWaf, uidPosX, uidPosY, packagePtr, flashSize, adc1Vdda_mV, adc1Vbat_mV, adc1Temp_100_i, adc1Temp_100_f);
  usbLogLen(buf, len);
}


// #define GPS_SIMU
// #define WEATHER_SIMU
// #define CURRENT_SIMU
void prvControllerGetDataAndUpload(void)
{
#ifdef GPS_SIMU
  const  float  centerLat     =  49.473185f;
  const  float  centerLon     =   8.614806f;
  const  float  centerAlt     =  98.0f;
  const  float  centerAcc     =   5.0f;
  const  float  radius_m      = 100.0f;
  const  float  heightAmpl_m  =   3.0f;
  const  float  height_m_p_s  =   0.1f;
  const  float  humidityAmpl  = 100.0f;  // +/- 10%
  static float  simPhase      =   0.0f;
#endif

  /* Take mutex to access LoRaWAN TrackMeApp */
  if (pdTRUE == xSemaphoreTake(trackMeApplUpDataMutexHandle, 500 / portTICK_PERIOD_MS)) {
    /* Fill in data to send */
    {
#ifdef GPS_SIMU
      // This part is done by the gpsCom module
      /* TTN Mapper entities */
      trackMeApp_up.latitude_deg      = centerLat + ((radius_m / (60.f * 1852.f)) * sin(simPhase * PI/180));
      trackMeApp_up.longitude_deg     = centerLon + ((radius_m / (60.f * 1852.f)) * cos(simPhase * PI/180) / cos(2*PI * centerLat));
      trackMeApp_up.altitude_m        = centerAlt + sin(simPhase * PI/180) * heightAmpl_m;
      trackMeApp_up.accuracy_10thM    = centerAcc + sin(simPhase * PI/180) * 2.0f;

      /* Motion vector entities */
      trackMeApp_up.course_deg        = (uint8_t) (360U - (uint16_t)simPhase);
      trackMeApp_up.speed_m_s         = 2*PI*radius_m * (30.f / 360.f);
      trackMeApp_up.vertspeed_m_s     = cos(simPhase * PI/180) * height_m_p_s;
#endif

#ifdef WEATHER_SIMU
      /* Weather entities */
      trackMeApp_up.temp_100th_C      = adcGetTemp_100();
      trackMeApp_up.humitidy_1000th   = (uint16_t) (500 + sin(simPhase * PI/180) * humidityAmpl);
      trackMeApp_up.baro_Pa           = 101325;
#endif

      /* System entities */
      trackMeApp_up.vbat_mV           = adcGetVbat_mV();
#ifdef CURRENT_SIMU
      trackMeApp_up.ibat_uA           = -1000;  // drawing from the battery
#endif

#ifdef GPS_SIMU
      /* Simulator phase increment - each 10 secs */
      simPhase += 30.f;
      if (simPhase > 359.9f) {
        simPhase = 0.f;
      }
#endif
    }

    /* Free semaphore */
    xSemaphoreGive(trackMeApplUpDataMutexHandle);

    /* Signal to take-over data and do upload */
    {
      const uint8_t c_msgToLoRaWAN[2] = { 1, LoraInQueueCmds__TrackMeApplUp };

      /* Write message into loraInQueue */
      for (uint8_t idx = 0; idx < sizeof(c_msgToLoRaWAN); idx++) {
        xQueueSendToBack(loraInQueueHandle, c_msgToLoRaWAN + idx, Controller_MaxWaitMs);
      }

      /* Set QUEUE_IN bit */
      xEventGroupSetBits(loraEventGroupHandle, Lora_EGW__QUEUE_IN);
    }

  } else {
    /* No luck filling in current data - abort plan to upload data */
  }
}


void prvPushToLoraInQueue(const uint8_t* cmdAry, uint8_t cmdLen)
{
  for (uint8_t idx = 0; idx < cmdLen; ++idx) {
    xQueueSendToBack(loraInQueueHandle, cmdAry + idx, 1);
  }
  xEventGroupSetBits(loraEventGroupHandle, Lora_EGW__QUEUE_IN);
}

void prvPullFromInterpreterOutQueue(void)
{
  BaseType_t  xStatus;
  uint8_t     inAry[4]  = { 0 };
  uint8_t     inLen     = 0;

  do {
    xStatus = xQueueReceive(interOutQueueHandle, &inLen, 1);
    if (pdPASS == xStatus) {
      for (uint8_t idx = 0; idx < inLen; idx++) {
        xStatus = xQueueReceive(interOutQueueHandle, inAry + idx, Controller_MaxWaitMs / portTICK_PERIOD_MS);
        if (pdFALSE == xStatus) {
          /* Bad communication - drop */
          return;
        }
      }
    }

    switch (inAry[0]) {
    case InterOutQueueCmds__DoSendDataUp:
      {
        prvControllerGetDataAndUpload();
      }
      break;

    case InterOutQueueCmds__LinkCheckReq:
      {
        const uint8_t cmdAry[2] = { 1, LoraInQueueCmds__LinkCheckReq };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;

    case InterOutQueueCmds__DeviceTimeReq:
      {
        const uint8_t cmdAry[2] = { 1, LoraInQueueCmds__DeviceTimeReq };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;

    case InterOutQueueCmds__ConfirmedPackets:
      {
        const uint8_t confSet = inAry[1];
        const uint8_t cmdAry[3] = { 2, LoraInQueueCmds__ConfirmedPackets, confSet };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;

    case InterOutQueueCmds__ADRset:
      {
        const uint8_t adrSet = inAry[1];
        const uint8_t cmdAry[3] = { 2, LoraInQueueCmds__ADRset, adrSet };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;

    case InterOutQueueCmds__DRset:
      {
        const uint8_t drSet = inAry[1];
        const uint8_t cmdAry[3] = { 2, LoraInQueueCmds__DRset, drSet };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;

    case InterOutQueueCmds__PwrRedDb:
      {
        const uint8_t pwrRed_db = inAry[1];
        const uint8_t cmdAry[3] = { 2, LoraInQueueCmds__PwrRedDb, pwrRed_db };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;
    }  // switch
  } while (!xQueueIsQueueEmptyFromISR(interOutQueueHandle));
}

void prvPullFromLoraOutQueue(void)
{
  BaseType_t  xStatus;
  uint8_t     inAry[4]  = { 0 };
  uint8_t     inLen     = 0;

  do {
    xStatus = xQueueReceive(loraOutQueueHandle, &inLen, 1);
    if (pdPASS == xStatus) {
      for (uint8_t idx = 0; idx < inLen; idx++) {
        xStatus = xQueueReceive(loraOutQueueHandle, inAry + idx, Controller_MaxWaitMs / portTICK_PERIOD_MS);
        if (pdFALSE == xStatus) {
          /* Bad communication - drop */
          return;
        }
      }
    }

    switch (inAry[0]) {
    case LoraOutQueueCmds__Connected:
      {
        /* Ready from LoRaWAN task received - activate upload timer */
        xStatus = xTimerStart(controllerSendTimerHandle, 1);
        xStatus = xTimerChangePeriod(controllerSendTimerHandle, 5 * 60 * 1000 / portTICK_PERIOD_MS, 1);   // 5 minutes

        /* Request LoRaWAN link check and network time */
        const uint8_t cmdAry[4] = {
            1, LoraInQueueCmds__LinkCheckReq,
            1, LoraInQueueCmds__DeviceTimeReq };
        prvPushToLoraInQueue(cmdAry, sizeof(cmdAry));
      }
      break;
    }  // switch
  } while (!xQueueIsQueueEmptyFromISR(loraOutQueueHandle));
}

void prvPullFromGpscomOutQueue(void)
{
  BaseType_t  xStatus;
  uint8_t     inAry[4]  = { 0 };
  uint8_t     inLen     = 0;

  do {
    xStatus = xQueueReceive(gpscomOutQueueHandle, &inLen, 1);
    if (pdPASS == xStatus) {
      for (uint8_t idx = 0; idx < inLen; idx++) {
        xStatus = xQueueReceive(gpscomOutQueueHandle, inAry + idx, Controller_MaxWaitMs / portTICK_PERIOD_MS);
        if (pdFALSE == xStatus) {
          /* Bad communication - drop */
          return;
        }
      }
    }

    switch (inAry[0]) {
    case gpscomOutQueueCmds__EndOfParse:
      {
        /* Take over data important for the track_me App*/
        /* Try to get the mutex within that short time or drop the message */
        if (pdTRUE == xSemaphoreTake(gpscomCtxMutexHandle, 100 / portTICK_PERIOD_MS))
        {
          const float lat_deg         = gpscomGpsCtx.lat_deg;
          const float lon_deg         = gpscomGpsCtx.lon_deg;
          const float alt_m           = gpscomGpsCtx.alt_m;
          const float accuracy_10thM  = gpscomGpsCtx.hdop * 71.f;
          const float course_deg      = gpscomGpsCtx.course_deg;
          const float speed_m_s       = gpscomGpsCtx.speed_kts * 1.852f / 3.6f;

          /* Give mutex back */
          xSemaphoreGive(gpscomCtxMutexHandle);

          /* Try to get trackMeApplUpData mutex */
          if (pdTRUE == xSemaphoreTake(trackMeApplUpDataMutexHandle, 100 / portTICK_PERIOD_MS))
          {
            trackMeApp_up.latitude_deg    = lat_deg;
            trackMeApp_up.longitude_deg   = lon_deg;
            trackMeApp_up.altitude_m      = alt_m;
            trackMeApp_up.accuracy_10thM  = accuracy_10thM;
            trackMeApp_up.course_deg      = course_deg;
            trackMeApp_up.speed_m_s       = speed_m_s;

            /* Give mutex back */
            xSemaphoreGive(gpscomCtxMutexHandle);
          }
        }
      }
      break;
    }  // switch
  } while (!xQueueIsQueueEmptyFromISR(gpscomOutQueueHandle));
}


/* Global functions ----------------------------------------------------------*/
void controllerSendTimerCallbackImpl(TimerHandle_t xTimer)
{
  /* Set flag for sending and upload data */
  xEventGroupSetBits(controllerEventGroupHandle, Controller_EGW__INTER_SENS_DO_SEND);
}


void controllerControllerTaskInit(void)
{
  uint32_t prevWakeTime = 0;

  /* Inits to be done before the USB CDC messaging is ready */
  prvControllerInitBeforeGreet();

  /* Delay until the USB CDC init phase is over */
  osDelayUntil(&prevWakeTime, 3500);

  /* Greetings to the USB CDC */
  prvControllerUsbGreet();

  /* Inits to be done after USB/DCD connection is established */
  prvControllerInitAfterGreet();
}

void controllerControllerTaskLoop(void)
{
  /* Keep controllerEventGroupHandle for 25 ms */
  EventBits_t eb = xEventGroupWaitBits(controllerEventGroupHandle,
      Controller_EGW__INTER_QUEUE_OUT | Controller_EGW__LORA_QUEUE_OUT | Controller_EGW__GPSCOM_QUEUE_OUT | Controller_EGW__INTER_SENS_DO_SEND,
      0,
      0, 25 / portTICK_PERIOD_MS);

  if (eb & Controller_EGW__INTER_QUEUE_OUT) {
    xEventGroupClearBits(controllerEventGroupHandle, Controller_EGW__INTER_QUEUE_OUT);
    prvPullFromInterpreterOutQueue();
  }

  if (eb & Controller_EGW__LORA_QUEUE_OUT) {
    xEventGroupClearBits(controllerEventGroupHandle, Controller_EGW__LORA_QUEUE_OUT);
    prvPullFromLoraOutQueue();
  }

  if (eb & Controller_EGW__GPSCOM_QUEUE_OUT) {
    xEventGroupClearBits(controllerEventGroupHandle, Controller_EGW__GPSCOM_QUEUE_OUT);
    prvPullFromGpscomOutQueue();
  }

  if (eb & Controller_EGW__INTER_SENS_DO_SEND) {
    xEventGroupClearBits(controllerEventGroupHandle, Controller_EGW__INTER_SENS_DO_SEND);
    prvControllerGetDataAndUpload();
  }
}
