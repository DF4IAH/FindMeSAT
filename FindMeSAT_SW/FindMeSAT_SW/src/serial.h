/*
 * \file
 *
 * \brief FindMeSAT
 * serial.h
 *
 * Created: 08.08.2017 22:36:17
 * Author : DF4IAH
 */


#ifndef SERIAL_H_
#define SERIAL_H_


#define C_USART1_RX_BUF_LEN			8


void serial_init(void);
void serial_start(void);

void task_serial(uint32_t now);


#endif /* SERIAL_H_ */