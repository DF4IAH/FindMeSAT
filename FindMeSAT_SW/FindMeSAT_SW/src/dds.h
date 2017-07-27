/*
 * dds.h
 *
 * Created: 04.07.2017 15:01:41
 *  Author: DF4IAH
 */


#ifndef DDS_H_
#define DDS_H_

#include <asf.h>


#define PM_SINE_COUNT												4096
#define PM_SINE_MAX_STEP											52
#define PM_SINE_IP_COUNT											(PM_SINE_MAX_STEP  * 2 * 16)


uint16_t PROGMEM_T PM_SINE[PM_SINE_COUNT];
int8_t   PROGMEM_T PM_SINE_IP[PM_SINE_IP_COUNT];


uint16_t get_interpolated_sine(uint16_t phase);


#endif /* DDS_H_ */
