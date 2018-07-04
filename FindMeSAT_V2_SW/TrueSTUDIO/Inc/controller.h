/*
 * controller.h
 *
 *  Created on: 01.05.2018
 *      Author: DF4IAH
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_
#ifdef __cplusplus
 extern "C" {
#endif


/* Bit-mask for the controllerEventGroup */
typedef enum Controller_EGW_BM {
  Controller_EGW__DO_SEND             = 0x00000001UL,
  Controller_EGW__DO_LINKCHECKREQ     = 0x00000002UL,
  Controller_EGW__DO_DEVICETIMEREQ    = 0x00000004UL,
} Controller_EGW_BM_t;


void controllerSendTimerCallbackImpl(TimerHandle_t xTimer);

void controllerControllerTaskInit(void);
void controllerControllerTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* CONTROLLER_H_ */
