/*
 * usb.c
 *
 *  Created on: 29.04.2018
 *      Author: DF4IAH
 */

/* Includes ------------------------------------------------------------------*/
#include "usb.h"

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId	usbToHostQueueHandle;
extern osMessageQId	usbFromHostQueueHandle;
uint8_t				usbFromHostISRBuf[64]	= { 0 };
uint32_t 			usbFromHostISRBufLen	= 0;


void usbToHost(const uint8_t* buf, uint32_t len)
{
	const uint8_t maxWaitMs = 25;

	if (buf && len) {
		while (len--) {
			osMessagePut(usbToHostQueueHandle, *(buf++), maxWaitMs);
		}
		osMessagePut(usbToHostQueueHandle, 0, maxWaitMs);
	}
}

void usbFromHostFromIRQ(const uint8_t* buf, uint32_t len)
{
	if (buf && len && !usbFromHostISRBufLen) {
		BaseType_t	lMaxLen = sizeof(usbFromHostISRBuf) - 1;
		BaseType_t	lLen = len;

		if (lLen > lMaxLen) {
			lLen = lMaxLen;
		}
		memcpy(usbFromHostISRBuf, buf, lLen);
		usbFromHostISRBuf[lLen] = 0;
		__asm volatile( "" );  // schedule barrier
		usbFromHostISRBufLen = lLen;
	}
}
