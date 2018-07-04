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


/* Command types for the gpscomInQueue */
typedef enum gpscomInQueueCmds {
  gpscomInQueueCmds__NOP              = 0,
} gpscomInQueueCmds_t;


void gpscomGpscomTaskInit(void);
void gpscomGpscomTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* GPSCOM_H_ */
