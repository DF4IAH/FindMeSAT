/**
 * \file
 *
 * \brief FindMeSAT
 * serial.c
 *
 * Created: 08.08.2017 22:36:31
 * Author : DF4IAH
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

#include "main.h"
#include "interpreter.h"
#include "usb.h"
#include "usart_serial.h"

#include "serial.h"


extern bool							g_adc_enabled;
extern bool							g_dac_enabled;
extern int16_t						g_backlight_mode_pwm;
extern uint8_t						g_bias_pm;
extern uint8_t						g_pitch_tone_mode;
extern bool							g_errorBeep_enable;
extern bool							g_keyBeep_enable;

extern bool							g_usb_cdc_stdout_enabled;
extern bool							g_usb_cdc_printStatusLines;
extern bool							g_usb_cdc_rx_received;
extern bool							g_usb_cdc_transfers_authorized;
extern bool							g_usb_cdc_access_blocked;
extern WORKMODE_ENUM_t				g_workmode;

extern usart_serial_options_t		g_usart1_options;
extern bool							g_usart1_rx_ready;
extern uint8_t						g_usart1_rx_idx;
extern uint8_t						g_usart1_rx_buf[C_USART1_RX_BUF_LEN];

extern bool							g_twi1_gyro_valid;
extern uint8_t						g_twi1_gyro_1_version;
extern int16_t						g_twi1_gyro_1_temp;
extern int16_t						g_twi1_gyro_1_temp_RTofs;
extern int16_t						g_twi1_gyro_1_temp_sens;
extern int16_t						g_twi1_gyro_1_temp_deg_100;
extern int16_t						g_twi1_gyro_1_accel_x;
extern int16_t						g_twi1_gyro_1_accel_y;
extern int16_t						g_twi1_gyro_1_accel_z;
extern int16_t						g_twi1_gyro_1_accel_ofsx;
extern int16_t						g_twi1_gyro_1_accel_ofsy;
extern int16_t						g_twi1_gyro_1_accel_ofsz;
extern int16_t						g_twi1_gyro_1_accel_factx;
extern int16_t						g_twi1_gyro_1_accel_facty;
extern int16_t						g_twi1_gyro_1_accel_factz;
extern int16_t						g_twi1_gyro_1_accel_x_mg;
extern int16_t						g_twi1_gyro_1_accel_y_mg;
extern int16_t						g_twi1_gyro_1_accel_z_mg;
extern int16_t						g_twi1_gyro_1_gyro_x;
extern int16_t						g_twi1_gyro_1_gyro_y;
extern int16_t						g_twi1_gyro_1_gyro_z;
extern int16_t						g_twi1_gyro_1_gyro_ofsx;
extern int16_t						g_twi1_gyro_1_gyro_ofsy;
extern int16_t						g_twi1_gyro_1_gyro_ofsz;
extern int32_t						g_twi1_gyro_1_gyro_x_mdps;
extern int32_t						g_twi1_gyro_1_gyro_y_mdps;
extern int32_t						g_twi1_gyro_1_gyro_z_mdps;
extern uint8_t						g_twi1_gyro_2_version;
extern int8_t						g_twi1_gyro_2_asax;
extern int8_t						g_twi1_gyro_2_asay;
extern int8_t						g_twi1_gyro_2_asaz;
extern int16_t						g_twi1_gyro_2_ofsx;
extern int16_t						g_twi1_gyro_2_ofsy;
extern int16_t						g_twi1_gyro_2_ofsz;
extern int16_t						g_twi1_gyro_2_mag_x;
extern int16_t						g_twi1_gyro_2_mag_y;
extern int16_t						g_twi1_gyro_2_mag_z;
extern int16_t						g_twi1_gyro_2_mag_factx;
extern int16_t						g_twi1_gyro_2_mag_facty;
extern int16_t						g_twi1_gyro_2_mag_factz;
extern int32_t						g_twi1_gyro_2_mag_x_nT;
extern int32_t						g_twi1_gyro_2_mag_y_nT;
extern int32_t						g_twi1_gyro_2_mag_z_nT;

extern bool							g_twi1_baro_valid;
extern uint16_t						g_twi1_baro_version;
extern uint16_t						g_twi1_baro_c[C_TWI1_BARO_C_CNT];
extern uint32_t						g_twi1_baro_d1;
extern uint32_t						g_twi1_baro_d2;
extern int32_t						g_twi1_baro_temp_100;
extern int32_t						g_twi1_baro_p_100;

extern uint8_t						g_twi1_lock;
extern bool							g_twi1_hygro_valid;
extern uint8_t						g_twi1_hygro_status;
extern uint16_t						g_twi1_hygro_S_T;
extern uint16_t						g_twi1_hygro_S_RH;
extern int16_t						g_twi1_hygro_T_100;
extern int16_t						g_twi1_hygro_RH_100;

extern int16_t						g_adc_vctcxo_volt_1000;
extern int16_t						g_adc_5v0_volt_1000;
extern int16_t						g_adc_vbat_volt_1000;
extern int16_t						g_adc_io_adc4_volt_1000;
extern int16_t						g_adc_io_adc5_volt_1000;
extern int16_t						g_adc_silence_volt_1000;
extern int16_t						g_adc_temp_deg_100;

extern uint8_t						g_interpreter_lock;

extern char							g_prepare_buf[C_TX_BUF_SIZE];


/* ISR routines */


/* Serial data received */
ISR(USARTF0_RXC_vect)
{
	/* Byte received */
	uint8_t ser1_rxd = usart_getchar(USART_SERIAL1);

	if (g_usart1_rx_idx < C_USART1_RX_BUF_LEN) {
		g_usart1_rx_buf[g_usart1_rx_idx++] = ser1_rxd;
	}

	/* Input string ready to read */
	g_usart1_rx_ready = true;
}


/* Set up serial connection to the SIM808 */
void serial_init(void)
{
	/* Set TXD high */
	ioport_set_pin_level(GSM_TXD1_DRV, HIGH);
	ioport_set_pin_level(GSM_CTS1_DRV_GPIO, HIGH);

	ioport_set_pin_level(GSM_RESET_DRV_GPIO,		LOW);	// RESETn active
	ioport_set_pin_level(GSM_PWRKEY_DRV_GPIO,		LOW);	// Power key not pressed

	ioport_set_pin_level(GSM_PCM_CLK_DRV_GPIO,		LOW);	// PCM clock not used
	ioport_set_pin_level(GSM_PCM_IN_DRV_GPIO,		LOW);	// PCM data  not used


	/* Set TXD line as output PIN */
	ioport_set_pin_dir(GSM_TXD1_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_RXD1,					IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_CTS1_DRV_GPIO,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_RTS1_GPIO,				IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_DTR1_GPIO,				IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_RI1_GPIO,				IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_DCD1_GPIO,				IOPORT_DIR_INPUT);

	ioport_set_pin_dir(GSM_RESET_DRV_GPIO,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_PWRKEY_DRV_GPIO,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_POWERED_GPIO,			IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_NETLIGHT_GPIO,			IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_RF_SYNC_GPIO,			IOPORT_DIR_INPUT);

	ioport_set_pin_dir(GSM_PCM_CLK_DRV_GPIO,		IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_PCM_IN_DRV_GPIO,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_PCM_OUT_GPIO,			IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_PCM_SYNC_GPIO,			IOPORT_DIR_INPUT);

	ioport_set_pin_dir(GPS_GSM_1PPS_GPIO,			IOPORT_DIR_INPUT);

	/* Prepare to use ASF USART service */
	g_usart1_options.baudrate	= USART_SERIAL1_BAUDRATE;
	g_usart1_options.charlength	= USART_SERIAL1_CHAR_LENGTH;
	g_usart1_options.paritytype	= USART_SERIAL1_PARITY;
	g_usart1_options.stopbits	= USART_SERIAL1_STOP_BIT;
}

/* USB device stack start function to enable stack and start USB */
void serial_start(void)
{
	usart_serial_init(USART_SERIAL1, &g_usart1_options);
	
	/* Release the GSM_RESETn */
	ioport_set_pin_level(GSM_RESET_DRV_GPIO, HIGH);
}


void task_serial(uint32_t now)
{
	uint8_t len = 0;

	/* Leave when nothing to do */
	if (!g_usart1_rx_ready) {
		return;
	}

	/* Take out own copy and reset */
	{
		irqflags_t flags = cpu_irq_save();
		if (g_usart1_rx_idx) {
			len = g_usart1_rx_idx;
			for (int8_t idx = g_usart1_rx_idx - 1; idx; --idx) {
				g_prepare_buf[idx] = g_usart1_rx_buf[idx];
			}
			g_usart1_rx_idx = 0;
		}
		cpu_irq_restore(flags);
	}

	/* Copy stream to USB_CDC */
	if (len) {
		udi_write_tx_buf(g_prepare_buf, len, false);
	}

// usart_serial_getchar(USART_SERIAL, &received_byte);
// usart_serial_putchar(USART_SERIAL, transmit_byte);
}
