/*
 * interpreter.h
 *
 * Created: 22.07.2017 19:37:56
 *  Author: DF4IAH
 */ 


#ifndef INTERPRETER_H_
#define INTERPRETER_H_


#define C_RX_CMDLINE_BUF_SIZE										256
#define C_TX_BUF_SIZE												64


uint8_t udi_write_tx_buf(const char* buf, uint8_t len, bool stripControl);
void printHelp(void);
void interpreter_doProcess(char rx_buf[], iram_size_t rx_len);


#endif /* INTERPRETER_H_ */
