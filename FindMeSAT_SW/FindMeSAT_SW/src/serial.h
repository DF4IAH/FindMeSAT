/*
 * \file
 *
 * \brief FindMeSAT
 * serial.h
 *
 * Created: 08.08.2017 22:36:17
 * Author : DF4IAH
 */


/*
 * ATI
 * AT+GSV
 *
 * AT+CMEE=2
 * AT+CFUN=1
 *
 * AT+ECHARGE=1
 * AT+CBC
 *
 * AT+CGNSPWR=1
 * AT+CGNSINF
 *
 * AT+CANT=1,0,10
 * AT+CNETSCAN=1;+CNETSCAN
 *
 * AT+CPIN?
 * AT+CREG?
 * AT+CGATT?
 * AT+CSQ
 * AT+C
 * AT+SAPBR=3,1,"Contype","GPRS"
 * AT+SAPBR=3,1,"APN","live.vodafone.com"
 * AT+SAPBR=1,1
 * AT+SAPBR=2,1
 * ... @see https://stackoverflow.com/questions/41418880/sim808-error-601-network-error
 * AT+SAPBR=0,1
 */


#ifndef SERIAL_H_
#define SERIAL_H_


#define C_USART1_RX_BUF_LEN			128U
#define C_USART1_RX_BUF_CHUNK		64


void serial_sim808_send(const char*, uint8_t len);

void serial_init(void);
void serial_start(void);

void serial_send_gns_info_req(void);
void serial_filter_inStream(int16_t len);

void task_serial(uint32_t now);


#endif /* SERIAL_H_ */