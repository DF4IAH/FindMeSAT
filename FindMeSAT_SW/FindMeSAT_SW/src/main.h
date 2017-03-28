/*
 * main.h
 *
 * Created: 25.01.2017 09:17:52
 *  Author: DF4IAH
 */ 


#ifndef MAIN_H_
#define MAIN_H_


/* INIT section */

static void evsys_init(void);
static void tc_init(void);
static void tc_start(void);
static void adc_init(void);
static void dac_init(void);
static void usb_init(void);

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

static void task_dac(void);
static void task_usb(void);
static void task(void);

void halt(void);

int main(void);


#endif /* MAIN_H_ */