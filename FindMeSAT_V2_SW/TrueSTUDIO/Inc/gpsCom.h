/*
 * gpsCom.h
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#ifndef GPSCOM_H_
#define GPSCOM_H_
#ifdef __cplusplus
 extern "C" {
#endif


 /* Bit-mask for the gpscomEventGroup */
typedef enum Gpscom_EGW_BM {
  Gpscom_EGW__QUEUE_IN                = 0x00000001UL,
  Gpscom_EGW__DMA_TX_END              = 0x00000010UL,
  Gpscom_EGW__DMA_RX50                = 0x00000020UL,
  Gpscom_EGW__DMA_RX100_END           = 0x00000040UL,
  Gpscom_EGW__DMA_TXRX_ERROR          = 0x00000080UL,
  Gpscom_EGW__DMA_TX_RUN              = 0x00000100UL,
  Gpscom_EGW__DMA_RX_RUN              = 0x00000200UL,
  Gpscom_EGW__TIMER_SERVICE_RX        = 0x00002000UL,
} Gpscom_EGW_BM_t;

 /* Command types for the gpscomInQueue */
typedef enum gpscomInQueueCmds {
  gpscomInQueueCmds__NOP              = 0,
} gpscomInQueueCmds_t;


void gpscomtRXTimerCallbackImpl(TimerHandle_t argument);

void gpscomGpscomTaskInit(void);
void gpscomGpscomTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* GPSCOM_H_ */
