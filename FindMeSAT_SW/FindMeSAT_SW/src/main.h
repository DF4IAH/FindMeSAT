/*
 * main.h
 *
 * Created: 25.01.2017 09:17:52
 *  Author: DF4IAH
 */


#ifndef MAIN_H_
#define MAIN_H_

typedef enum ADC_CH0_SCAN_ENUM {
	ADC_CH0_SCAN_3V0 = 0,									// PIN = PA0, ADC0
	ADC_CH0_SCAN_VCTCXO,									// PIN = PA1, ADC1
	ADC_CH0_SCAN_5V0,										// PIN = PA2, ADC2
	ADC_CH0_SCAN_VBAT,										// PIN = PA3, ADC3
} ADC_CH0_SCAN_t;


/* INIT section */

void cb_rtc_alarm(uint32_t rtc_time);
void cb_adc(ADC_t* adc, uint8_t ch_mask, adc_result_t res);

void usb_callback_suspend_action(void);
void usb_callback_resume_action(void);

void usb_callback_remotewakeup_enable(void);
void usb_callback_remotewakeup_disable(void);
void usb_send_wakeup_event(void);

bool usb_callback_cdc_enable(void);
void usb_callback_cdc_disable(void);
void usb_callback_config(uint8_t port, usb_cdc_line_coding_t * cfg);
void usb_callback_cdc_set_dtr(uint8_t port, bool b_enable);
void usb_callback_cdc_set_rts(uint8_t port, bool b_enable);
void usb_callback_rx_notify(uint8_t port);
void usb_callback_tx_empty_notify(uint8_t port);


/* RUNNING section */

void halt(void);

int main(void);


#endif /* MAIN_H_ */
