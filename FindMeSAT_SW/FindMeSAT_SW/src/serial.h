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


#define C_USART1_RX_BUF_LEN			512U
#define C_USART1_RX_BUF_DIFF_OFF	16
#define C_USART1_RX_BUF_DIFF_ON		32

#define C_GSM_PIN_BUF_LEN			14


typedef enum SERIAL_SIM808_GSM_SETFUNC_ENUM {
	SERIAL_SIM808_GSM_SETFUNC_OFF					= 0,
	SERIAL_SIM808_GSM_SETFUNC_ON					= 1,
} SERIAL_SIM808_GSM_SETFUNC_ENUM_t;

void serial_sim808_send(const char*, uint8_t len);
bool serial_sim808_sendAndResponse(const char* cmd, uint8_t len, bool doCopy);
void serial_sim808_gsm_setFunc(SERIAL_SIM808_GSM_SETFUNC_ENUM_t funcMode);
void serial_sim808_gsm_setPin(const char* pin);
void serial_gsm_activation(bool enable);
void serial_gsm_rx_creg(uint8_t val);
void serial_gsm_rx_cgatt(uint8_t val);
void serial_gsm_gprs_openClose(bool isStart);
void serial_sim808_gsm_shutdown(void);


void serial_init(void);
void serial_start(void);
void serial_gprs_establish(void);

void serial_send_gprs_open(void);
void serial_send_gns_urc(uint8_t val);

void task_serial(uint32_t now);


#endif /* SERIAL_H_ */