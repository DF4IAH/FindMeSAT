/*
 * controller.h
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_


/* Bit-mask for the controllerEventGroup */
typedef enum Controller_EGW_BM {
  Controller_EGW__DO_SEND             = 0x00000001UL,
} Controller_EGW_BM_t;


void controllerSendTimerCallbackImpl(TimerHandle_t xTimer);

void controllerControllerTaskInit(void);
void controllerControllerTaskLoop(void);

#endif /* CONTROLLER_H_ */
