/*
 * sensors.h
 *
 *  Created on: 04.07.2018
 *      Author: DF4IAH
 */

#ifndef SENSORS_H_
#define SENSORS_H_
#ifdef __cplusplus
 extern "C" {
#endif


/* Command types for the sensorsInQueue */
typedef enum sensorsInQueueCmds {
  sensorsInQueueCmds__NOP             = 0,
} sensorsInQueueCmds_t;


void sensorsSensorsTaskInit(void);
void sensorsSensorsTaskLoop(void);


#ifdef __cplusplus
}
#endif
#endif /* SENSORS_H_ */
