/*
 * \file
 *
 * \brief FindMeSAT
 * spi.h
 *
 * Created: 27.12.2017 18:37:04
 * Author : DF4IAH
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <asf.h>


#if 0

# define USART_SPI_AX												USARTC1

#else

# define SPI_AX														SPIC

#endif


void spi_init(void);
void spi_start(void);


#endif /* SPI_H_ */