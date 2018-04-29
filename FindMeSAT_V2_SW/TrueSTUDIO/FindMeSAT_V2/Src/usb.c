/*
 * usb.c
 *
 *  Created on: 29.04.2018
 *      Author: espero
 */

/* Includes ------------------------------------------------------------------*/
#include "usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"


/* Variables -----------------------------------------------------------------*/
extern osMessageQId usbToHostQueueHandle;


void usbToHost(const uint8_t* buf, uint8_t len)
{
	const char nulBuf = 0;

	if (buf && len) {
		while (len--) {
			xQueueSendToBack(usbToHostQueueHandle, buf++, 0);
		}
		xQueueSendToBack(usbToHostQueueHandle, &nulBuf, 0);
	}
}
