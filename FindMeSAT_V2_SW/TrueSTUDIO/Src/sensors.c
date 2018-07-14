/*
 * sensors.c
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#include <sys/_stdint.h>
#include <cmsis_os.h>
#include <string.h>
#include "FreeRTOS.h"
#include "main.h"

#include "sensors.h"


/* Variables -----------------------------------------------------------------*/
extern osThreadId         sensorsTaskHandle;
extern osMessageQId       sensorsOutQueueHandle;
extern osMessageQId       sensorsInQueueHandle;
extern EventGroupHandle_t sensorsEventGroupHandle;


/* Global functions ----------------------------------------------------------*/

void sensorsSensorsTaskInit(void)
{
  uint8_t inChr = 0;

  /* Clear queue */
  while (xQueueReceive(sensorsInQueueHandle, &inChr, 0) == pdPASS) {
  }
}

void sensorsSensorsTaskLoop(void)
{
  static uint8_t  s_buf[32]   = { 0 };
  static uint8_t  s_bufCtr    = 0;
  static uint8_t  s_bufMsgLen = 0;
  BaseType_t      xStatus;
  uint8_t         inChr;

  do {
    /* Wait for input from the controller */
    do {
      /* Take next character from the queue, if any */
      inChr = 0;
      xStatus = xQueueReceive(sensorsInQueueHandle, &inChr, 100 / portTICK_PERIOD_MS);            // Wait max. 100 ms for completion
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
        goto sensors_Error_clrInBuf;
      }
    } while (1);

    /* Process the message */
    switch (s_buf[0]) {
    case 0:  //SensorsInQueueCmds__XXX:
      {
  //      /* Set event mask bit for INIT */
  //      xEventGroupSetBits(loraEventGroupHandle, LORAWAN_EGW__DO_INIT);
      }
      break;
    default:
      /* Nothing to do */
      { }
    }  // switch (s_buf[0])
  } while (!xQueueIsQueueEmptyFromISR(sensorsInQueueHandle));

sensors_Error_clrInBuf:
  {
    /* Clear the buffer to sync */
    s_bufCtr = s_bufMsgLen = 0;
    memset(s_buf, 0, sizeof(s_buf));
  }
}
