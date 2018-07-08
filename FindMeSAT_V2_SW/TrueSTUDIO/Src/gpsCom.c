/*
 * gpsCom.c
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#include <sys/_stdint.h>
#include <string.h>
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

static uint8_t              gpscomGpsTxDmaBuf[128]            = { 0 };

static uint16_t             gpscomGpsRxDmaBufLast             = 0;
static uint16_t             gpscomGpsRxDmaBufIdx              = 0;
static uint8_t              gpscomGpsRxDmaBuf[128]            = { 0 };

static uint8_t              gpscomGpsRxLineBufIdx             = 0;
static uint8_t              gpscomGpsRxLineBuf[128]           = { 0 };


/* Global functions ----------------------------------------------------------*/

static void prvGpscomInterpreter(const uint8_t* buf, uint16_t len)
{
  char l_buf[32];

  /* Debugging */
  int l_len = sprintf(l_buf, "%c%c%c%c - len=%3u\r\n", buf[0], buf[1], buf[2], buf[3], len);
  usbLogLen((const char*) l_buf, l_len);
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
        //HAL_UART_DMAPause(usartUart5Handle);
        taskENTER_CRITICAL();

        /* Make a work copy */
        len = gpscomGpsRxLineBufIdx;
        memcpy(buf, gpscomGpsRxLineBuf, len);

        /* Clear the line buffer */
        gpscomGpsRxLineBufIdx = 0;
        memset(gpscomGpsRxLineBuf, 0, sizeof(gpscomGpsRxLineBuf));

        //HAL_UART_DMAResume(usartUart5Handle);
        taskEXIT_CRITICAL();
      }

      /* Work on line buffer content */
      prvGpscomInterpreter(buf, len);
    }
  }

  if (gpscomGpsRxDmaBufIdx >= sizeof(gpscomGpsRxDmaBuf)) {
    gpscomGpsRxDmaBufLast = gpscomGpsRxDmaBufIdx = 0U;
  }
}

static void prvGpscomGpsRX(void)
{
  const uint16_t dmaBufSize = sizeof(gpscomGpsRxDmaBuf);

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
