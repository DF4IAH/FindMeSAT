/*
 * interpreter.h
 *
 * Created: 22.07.2017 19:37:56
 *  Author: DF4IAH
 */ 


#ifndef INTERPRETER_H_
#define INTERPRETER_H_


#define C_RX_CMDLINE_BUF_SIZE										256
#define C_TX_BUF_SIZE												128
#define C_TX_RIC_BUF_SIZE											128
#define C_TX_NEWS_BUF_SIZE											256


void printHelp(void);
void interpreter_doProcess(char rx_buf[], iram_size_t rx_len);


#endif /* INTERPRETER_H_ */
