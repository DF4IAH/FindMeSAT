/*
 * usb.c
 *
 *  Created on: 29.04.2018
 *      Author: DF4IAH
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"

#include <stddef.h>
#include <sys/_stdint.h>
#include <stdio.h>
#include <usbd_cdc_if.h>
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"

#include "usb.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId         usbToHostQueueHandle;
extern osMessageQId         usbFromHostQueueHandle;
extern EventGroupHandle_t   usbToHostEventGroupHandle;
extern osSemaphoreId        usbToHostBinarySemHandle;

extern ENABLE_MASK_t        g_enableMsk;
extern MON_MASK_t           g_monMsk;

uint8_t                     usbFromHostISRBuf[64]                  = { 0 };
uint32_t                    usbFromHostISRBufLen                   = 0;



const uint8_t usbToHost_MaxWaitQueueMs = 100;
void usbToHost(const uint8_t* buf, uint32_t len)
{
	if (buf && len) {
		while (len--) {
			osMessagePut(usbToHostQueueHandle, *(buf++), usbToHost_MaxWaitQueueMs);
		}
		osMessagePut(usbToHostQueueHandle, 0, usbToHost_MaxWaitQueueMs);
	}
}

const uint16_t usbToHostWait_MaxWaitSemMs = 500;
void usbToHostWait(const uint8_t* buf, uint32_t len)
{
  EventBits_t eb = xEventGroupWaitBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__BUF_EMPTY, 0, 0, usbToHostWait_MaxWaitSemMs);
  if (eb & USB_TO_HOST_EG__BUF_EMPTY) {
    usbToHost(buf, len);
  }
}

void usbLogLen(const char* str, int len)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*)str, len);
  osSemaphoreRelease(usbToHostBinarySemHandle);
}

inline
void usbLog(const char* str)
{
  usbLogLen(str, strlen(str));
}


void usbFromHostFromIRQ(const uint8_t* buf, uint32_t len)
{
	if (buf && len && !usbFromHostISRBufLen) {
		BaseType_t	lMaxLen = sizeof(usbFromHostISRBuf) - 1;
		BaseType_t	lLen = len;

		if (lLen > lMaxLen) {
			lLen = lMaxLen;
		}
		memcpy((void*)usbFromHostISRBuf, (const void*)buf, lLen);
		usbFromHostISRBuf[lLen] = 0;
		__asm volatile( "ISB" );
		usbFromHostISRBufLen = lLen;
	}
}


const char usbClrScrBuf[4] = { 0x0c, 0x0d, 0x0a, 0 };
void usbUsbToHostTaskInit(void)
{
  uint8_t inChr = 0;

  //HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);                                  // Red on

  /* Give time for USB CDC to come up */
  osDelay(2500);

  /* Clear queue */
  while (xQueueReceive(usbToHostQueueHandle, &inChr, 0) == pdPASS) {
  }
  xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__BUF_EMPTY);

  //HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);                                // Red off
  //HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);                                  // Blue on

  /* Init connection with dummy data */
  for (uint32_t cnt = 15; cnt; cnt--) {
    CDC_Transmit_FS((uint8_t*) usbClrScrBuf, 3);
    osDelay(10);
  }
  osDelay(250);
  //HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);                                // Blue off
}

void usbUsbToHostTaskLoop(void)
{
  static uint8_t buf[32] = { 0 };
  static uint8_t bufCtr = 0;
  uint8_t inChr;
  BaseType_t xStatus;

  do {
    /* Take next character from the queue - at least update each 100ms */
    inChr = 0;
    xStatus = xQueueReceive(usbToHostQueueHandle, &inChr, 100 / portTICK_PERIOD_MS);
    if ((pdPASS == xStatus) && inChr) {
      /* Group-Bit for empty queue cleared */
      xEventGroupClearBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__BUF_EMPTY);

      buf[bufCtr++] = inChr;

    } else {
      if (pdPASS != xStatus) {
        /* Group-Bit for empty queue set */
        xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__BUF_EMPTY);
      }
    }

    /* Flush when 0 or when buffer is full */
    if (!inChr || (bufCtr >= (sizeof(buf) - 1))) {
      uint32_t retryCnt;

      /* Do not send empty buffer */
      if (!bufCtr) {
        return;
      }

      buf[bufCtr] = 0;
      for (retryCnt = 25; retryCnt; --retryCnt) {
        /* Transmit to USB host */
        uint8_t ucRetVal = CDC_Transmit_FS(buf, bufCtr);
        if (USBD_BUSY != ucRetVal) {
          /* Data accepted for transmission */
          bufCtr = 0;
          buf[0] = 0;

          /* Group-Bit for empty queue set */
          xEventGroupSetBits(usbToHostEventGroupHandle, USB_TO_HOST_EG__BUF_EMPTY);

          //HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);                          // Red off
          break;

        } else {
          /* USB EP busy - try again */
          //HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);                            // Red on

          /* Delay for next USB packet to come and go */
          osDelay(1);
        }
      }

      if (!retryCnt) {
        /* USB EP still busy - drop whole buffer content */
        bufCtr = 0;
        //HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);                              // Red on
      }
    }
  } while (!xQueueIsQueueEmptyFromISR(usbToHostQueueHandle));
}


void usbUsbFromHostTaskInit(void)
{
  //HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);                                  // Green on

  /* Give time for USB CDC to come up */
  osDelay(2500);

  //HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);                                // Green off
}

void usbUsbFromHostTaskLoop(void)
{
  const uint8_t nulBuf[1] = { 0 };
  const uint8_t lightOnMax = 2;
  const uint8_t maxWaitMs = 25;
  static uint8_t lightOnCtr = 0;

  if (usbFromHostISRBufLen) {
    lightOnCtr = lightOnMax;
    //HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);                                // Green on

    /* USB OUT EP from host put data into the buffer */
    uint8_t* bufPtr = usbFromHostISRBuf;
    for (BaseType_t idx = 0; idx < usbFromHostISRBufLen; ++idx, ++bufPtr) {
      xQueueSendToBack(usbFromHostQueueHandle, bufPtr, maxWaitMs);
    }
    xQueueSendToBack(usbFromHostQueueHandle, nulBuf, maxWaitMs);

    memset((char*) usbFromHostISRBufLen, 0, sizeof(usbFromHostISRBufLen));
    __asm volatile( "ISB" );
    usbFromHostISRBufLen = 0;

  } else {
    /* Delay for the next attempt */
    osDelay(25);
  }

  /* Show state */
  switch (lightOnCtr) {
  case 0:
    break;

  case 1:
    --lightOnCtr;
    //HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);                            // Green off
    break;

  default:
    --lightOnCtr;
  }
}
