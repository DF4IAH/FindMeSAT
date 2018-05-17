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
#include "adc.h"
#include "spi.h"
#include "LoRaWAN.h"

#include "controller.h"

//#pragma GCC diagnostic ignored "-Wformat"


/* Private variables ---------------------------------------------------------*/
extern osSemaphoreId      usbToHostBinarySemHandle;
extern EventGroupHandle_t usbToHostEventGroupHandle;
extern LoRaWANctx_t       loRaWANctx;
extern LoraliveApp_t      loraliveApp;
extern uint32_t           spiPreviousWakeTime;

/* Private function prototypes -----------------------------------------------*/
static void prvControllerInitBeforeGreet(void);
static void prvControllerInitAfterGreet(void);
static void prvControllerUsbGreet(void);
static void prvControllerPrintMCU(void);


/* Global functions ----------------------------------------------------------*/
void controllerControllerTaskInit(void)
{
  uint32_t prevWakeTime = 0;

  /* Inits to be done before the USB CDC messaging is ready */
  prvControllerInitBeforeGreet();

  /* Delay until the USB CDC init phase is over */
  osDelayUntil(&prevWakeTime, 3500);

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
const char controllerGreetMsg02[] = "+===========================================================+\r\n";
const char controllerGreetMsg03[] = "*                                                           *\r\n";
const char controllerGreetMsg04[] = "*  FindMeSAT V2 - by DF4IAH - ARM powered by STM32L496ZG-P  *\r\n";

const char controllerGreetMsg11[] =
    "\tFindMeSAT_V2 version:\r\n"
    "\t=====================\r\n"
    "\r\n"
    "\t\tDate\t\t%08u\r\n";
void prvControllerUsbGreet(void)
{
  char verBuf[70];

  sprintf(verBuf, controllerGreetMsg11, FINDMESAT_VERSION);

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

void prvControllerInitBeforeGreet(void)
{
  /* USB typing echo */
  xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__ECHO_ON);   // TODO: should be from Config-FLASH page

  /* Check for attached SX1272_mbed_shield */
  if (spiDetectShieldSX1272()) {
    /* Init LoRaWAM module */
    LoRaWAN_Init();

    /* TODO: Test pushing of data */
    {
      loraliveApp.id = 'E';

      loraliveApp.voltage_32_v  = (uint8_t) (3.3f * 32 + 0.5f);

      loraliveApp.dust025_10_hi = 0U; loraliveApp.dust025_10_lo = 0U;
      loraliveApp.dust025_10_hi = 0U; loraliveApp.dust025_10_lo = 0U;

      uint32_t latitude_1000 = 49473;
      loraliveApp.u.l14.latitude_1000_sl24  = (uint8_t) ((latitude_1000  >> 24) & 0xffUL);
      loraliveApp.u.l14.latitude_1000_sl16  = (uint8_t) ((latitude_1000  >> 16) & 0xffUL);
      loraliveApp.u.l14.latitude_1000_sl08  = (uint8_t) ((latitude_1000  >>  8) & 0xffUL);
      loraliveApp.u.l14.latitude_1000_sl00  = (uint8_t) ((latitude_1000  >>  0) & 0xffUL);

      uint32_t longitude_1000 = 8615;
      loraliveApp.u.l14.longitude_1000_sl24 = (uint8_t) ((longitude_1000 >> 24) & 0xffUL);
      loraliveApp.u.l14.longitude_1000_sl16 = (uint8_t) ((longitude_1000 >> 16) & 0xffUL);
      loraliveApp.u.l14.longitude_1000_sl08 = (uint8_t) ((longitude_1000 >>  8) & 0xffUL);
      loraliveApp.u.l14.longitude_1000_sl00 = (uint8_t) ((longitude_1000 >>  0) & 0xffUL);

      /* TODO_ DEBUG Loop tp be removed */
      for (uint8_t i = 0; i < 3; i++) {
        LoRaWAN_App_loralive_pushUp(&loRaWANctx, &loraliveApp, 14);
        LoRaWAN_App_loralive_receiveLoop(&loRaWANctx);
      }

      while (1) {
        spiPreviousWakeTime = osKernelSysTick();
        LoRaWAN_App_loralive_receiveLoop(&loRaWANctx);
      }
    }
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

  uint16_t flashSize      = (uint16_t) ((*((uint32_t*) FLASHSIZE_BASE)) & 0x0000ffffUL);

  /* Request ADC1 Vdda */
  uint32_t adc1Vdda_mV = adcGetVdda_mV();

  /* Request ADC1 Vbat */
  uint32_t adc1Vbat_mV    = adcGetVbat_mV();

  /* Request ADC1 Temp */
  int32_t  adc1Temp_100   = adcGetTemp_100();
  int16_t  adc1Temp_100_i = adc1Temp_100 / 100;
  uint16_t adc1Temp_100_f = adc1Temp_100 >= 0 ?  (adc1Temp_100 % 100) : (-adc1Temp_100 % 100);

  sprintf(buf,
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

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) buf, strlen(buf));
  osSemaphoreRelease(usbToHostBinarySemHandle);
}
