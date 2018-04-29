/*
 * usb.h
 *
 *  Created on: 29.04.2018
 *      Author: DF4IAH
 */

#ifndef USB_H_
#define USB_H_

#include <stddef.h>
#include <sys/_stdint.h>


void usbToHost(const uint8_t* buf, uint8_t len);

#endif /* USB_H_ */
