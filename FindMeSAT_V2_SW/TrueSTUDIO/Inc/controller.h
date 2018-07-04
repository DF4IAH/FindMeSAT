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
  Controller_EGW__INTER_QUEUE_OUT     = 0x00000002UL,
  Controller_EGW__INTER_SENS_DO_SEND  = 0x00000003UL,
  Controller_EGW__LORA_QUEUE_IN       = 0x00000010UL,
  Controller_EGW__LORA_QUEUE_OUT      = 0x00000020UL,
  Controller_EGW__GPSCOM_QUEUE_IN     = 0x00000100UL,
  Controller_EGW__GPSCOM_QUEUE_OUT    = 0x00000200UL,
  Controller_EGW__SENSORS_QUEUE_IN    = 0x00001000UL,
  Controller_EGW__SENSORS_QUEUE_OUT   = 0x00002000UL,
} Controller_EGW_BM_t;


void controllerSendTimerCallbackImpl(TimerHandle_t xTimer);

void controllerControllerTaskInit(void);
void controllerControllerTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* CONTROLLER_H_ */
