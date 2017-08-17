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


#define C_USART1_RX_BUF_LEN			256U
#define C_USART1_RX_BUF_CHUNK		64


void serial_sim808_send(const char*, uint8_t len);

void serial_init(void);
void serial_start(void);

void task_serial(uint32_t now);


#endif /* SERIAL_H_ */