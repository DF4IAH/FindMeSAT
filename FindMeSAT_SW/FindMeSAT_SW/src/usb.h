/*
 * \file
 *
 * \brief FindMeSAT
 * usb.h
 *
 * Created: 08.08.2017 21:51:48
 * Author : DF4IAH
 */


#ifndef USB_H_
#define USB_H_


#define C_USB_LINE_DELAY_MS											25


uint8_t udi_write_tx_msg_P(const char* msg_P);
uint8_t udi_write_tx_buf(const char* buf, uint8_t len, bool stripControl);
void udi_write_serial_line(const char* buf, uint16_t len);

void usb_init(void);

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

void task_usb(void);


#endif /* USB_H_ */