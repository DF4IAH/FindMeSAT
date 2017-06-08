/*
 * \file
 *
 * \brief FindMeSAT
 * twi.h
 *
 * Created: 07.06.2017 22:48:48
 * Author : DF4IAH
 */


#ifndef TWI_H_
#define TWI_H_

#include <asf.h>


#define TWI1_MASTER			TWIE
//#define TWI1_SLAVE		TWIE
#define TWI1_MASTER_PORT	PORTE
#define TWI1_MASTER_ADDR	0x50
#define TWI1_SLAVE_ADDR		0x60
#define TWI1_SPEED			400000

#define TWI2_MASTER			TWIC
//#define TWI2_SLAVE		TWIC
#define TWI2_MASTER_PORT	PORTC
#define TWI2_MASTER_ADDR	0x52
#define TWI2_SLAVE_ADDR		0x62
#define TWI2_SPEED			400000


#define TWI_DATA_LENGTH		TWIS_SEND_BUFFER_SIZE


void twi_init(void);
void twi_start(void);


#endif /* TWI_H_ */
