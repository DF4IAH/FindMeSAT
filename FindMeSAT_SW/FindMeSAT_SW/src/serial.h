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
#define C_USART1_TX_BUF_LEN			64

#define C_USART_SERIAL1_BAUDRATE	9600
#define C_USART_SIM808_RESP_MS		25
#define C_USART_SIM808_RESP_ITER	80

#define C_GSM_PIN_BUF_LEN			14

typedef enum C_GSM_CREG_STAT_ENUM {
	C_GSM_CREG_STAT_DISABLED						= 0,
	C_GSM_CREG_STAT_REGHOME							= 1,
	C_GSM_CREG_STAT_SEARCHING						= 2,
	C_GSM_CREG_STAT_DENIED							= 3,
	C_GSM_CREG_STAT_UNKNOWN							= 4,
	C_GSM_CREG_STAT_REGROAMING						= 5
} C_GSM_CREG_STAT_ENUM_t;

typedef enum C_GSM_CGATT_STAT_ENUM {
	C_GSM_CREG_STAT_DETACHED						= 0,
	C_GSM_CREG_STAT_ATTACHED						= 1
} C_GSM_CGATT_STAT_ENUM_t;


typedef enum SERIAL_SIM808_GSM_SETFUNC_ENUM {
	SERIAL_SIM808_GSM_SETFUNC_OFF					= 0,
	SERIAL_SIM808_GSM_SETFUNC_ON,
} SERIAL_SIM808_GSM_SETFUNC_ENUM_t;

void serial_sim808_send(const char* msg, uint8_t len, bool doWait);
bool serial_sim808_sendAndResponse(const char* msg, uint8_t len);
void serial_sim808_gsm_setFunc(SERIAL_SIM808_GSM_SETFUNC_ENUM_t funcMode);
void serial_sim808_gsm_setPin(const char* pin);

void serial_gsm_activation(bool enable);
void serial_gsm_gprs_link_openClose(bool isStart);
void serial_gsm_rx_creg(C_GSM_CREG_STAT_ENUM_t stat);
void serial_gsm_rx_cgatt(C_GSM_CGATT_STAT_ENUM_t stat);
void serial_gsm_gprs_ip_openClose(bool isStart);
void serial_sim808_gsm_shutdown(void);


void serial_init(void);
void serial_start(void);

void serial_send_gns_urc(uint8_t val);
bool serial_filter_inStream(const char* buf, uint16_t len);

void task_serial(void);
void task(void);


#endif /* SERIAL_H_ */