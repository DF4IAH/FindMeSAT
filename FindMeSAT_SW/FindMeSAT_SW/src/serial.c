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
#include "twi.h"
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
extern bool							g_usb_cdc_printStatusLines_atxmega;
extern bool							g_usb_cdc_rx_received;
extern bool							g_usb_cdc_transfers_authorized;
extern bool							g_usb_cdc_access_blocked;
extern WORKMODE_ENUM_t				g_workmode;

extern usart_serial_options_t		g_usart1_options;
extern bool							g_usart1_rx_ready;
extern uint16_t						g_usart1_rx_idx;
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


/* Functions */

void serial_sim808_send(const char* cmd, uint8_t len)
{
	/* Make a copy */
	for (uint8_t cnt = len, idx = len - 1; cnt; --cnt, --idx) {
		g_prepare_buf[idx] = cmd[idx];
	}
	g_prepare_buf[len]		= '\r';
	g_prepare_buf[len + 1]	= 0;

	/* Send the string to the SIM808 */
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
}

/* Set up serial connection to the SIM808 */
void serial_init(void)
{
	/* Set TXD high */
	ioport_set_pin_level(GSM_DTR1_DRV,				HIGH);	// DTR inactive
	ioport_set_pin_level(GSM_TXD1_DRV,				HIGH);	// TXD pausing
	ioport_set_pin_level(GSM_RTS1_DRV,				HIGH);	// RTS inactive

	ioport_set_pin_level(GSM_RESET_DRV_GPIO,		LOW);	// RESETn active
	ioport_set_pin_level(GSM_PWRKEY_DRV_GPIO,		LOW);	// Power key not pressed

	ioport_set_pin_level(GSM_PCM_CLK_DRV_GPIO,		LOW);	// PCM clock not used
	ioport_set_pin_level(GSM_PCM_IN_DRV_GPIO,		LOW);	// PCM data  not used


	/* Set limited slew rate */
	ioport_set_pin_mode(GSM_DTR1_DRV,				IOPORT_MODE_TOTEM    | IOPORT_MODE_SLEW_RATE_LIMIT);
	ioport_set_pin_mode(GSM_TXD1_DRV,				IOPORT_MODE_TOTEM    | IOPORT_MODE_SLEW_RATE_LIMIT);
	ioport_set_pin_mode(GSM_RTS1_DRV,				IOPORT_MODE_TOTEM    | IOPORT_MODE_SLEW_RATE_LIMIT);

	ioport_set_pin_mode(GSM_RESET_DRV_GPIO,			IOPORT_MODE_PULLDOWN | IOPORT_MODE_SLEW_RATE_LIMIT);
	ioport_set_pin_mode(GSM_PWRKEY_DRV_GPIO,		IOPORT_MODE_PULLDOWN | IOPORT_MODE_SLEW_RATE_LIMIT);

	ioport_set_pin_mode(GSM_PCM_CLK_DRV_GPIO,		IOPORT_MODE_PULLDOWN | IOPORT_MODE_SLEW_RATE_LIMIT);
	ioport_set_pin_mode(GSM_PCM_IN_DRV_GPIO,		IOPORT_MODE_PULLDOWN | IOPORT_MODE_SLEW_RATE_LIMIT);


	/* Set TXD line as output PIN */
	ioport_set_pin_dir(GSM_DTR1_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_TXD1_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_RXD1,					IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_RTS1_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_CTS1,					IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_RI1,						IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_DCD1,					IOPORT_DIR_INPUT);

	ioport_set_pin_dir(GSM_RESET_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_PWRKEY_DRV,				IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(GSM_POWERED,					IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_NETLIGHT,				IOPORT_DIR_INPUT);
	ioport_set_pin_dir(GSM_RF_SYNC,					IOPORT_DIR_INPUT);

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


const char					PM_SIM808_OK[]							= "OK";
const char					PM_SIM808_RDY[]							= "RDY";
const char					PM_SIM808_INFO_START[]					= "SIM808 ser1:  Starting the device ...\r\n";
const char					PM_SIM808_INFO_RESTART[]				= "SIM808 ser1:  Starting the device ...\r\n";
const char					PM_SIM808_INFO_SYNCED[]					= "SIM808 ser1:   baud rate synced\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_START[]		= "Init: SIM808 starting ...";
const char					PM_TWI1_INIT_ONBOARD_SIM808_RESTART[]	= "Init: SIM808 restarting ...";
const char					PM_TWI1_INIT_ONBOARD_SIM808_OK[]		= "Init: SIM808 success";
const char					PM_TWI1_INIT_ONBOARD_SIM808_IPR[]		= "AT+IPR=%ld\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_CFUN0[]		= "AT+CFUN=0\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_01[]	= "ATI\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_02[]	= "AT+GSV\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_03[]	= "AT+CIMI\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_04[]	= "AT+COPN\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_GPS_01[]	= "AT+CGNSPWR=%d\r\n";
const char					PM_TWI1_INIT_ONBOARD_SIM808_GPS_02[]	= "AT+CGNSINF\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_OK[]);
PROGMEM_DECLARE(const char, PM_SIM808_RDY[]);
PROGMEM_DECLARE(const char, PM_SIM808_INFO_START[]);
PROGMEM_DECLARE(const char, PM_SIM808_INFO_RESTART[]);
PROGMEM_DECLARE(const char, PM_SIM808_INFO_SYNCED[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_START[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_RESTART[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_OK[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_IPR[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_CFUN0[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_01[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_02[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_03[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_04[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GPS_01[]);
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GPS_02[]);

/* USB device stack start function to enable stack and start USB */
void serial_start(void)
{
	uint16_t loop_ctr = 0;
	uint8_t  line = 7;

	/* Init and start of the ASF USART service/device */
	usart_serial_init(USART_SERIAL1, &g_usart1_options);

	/* ISR interrupt levels */
	((USART_t*)USART_SERIAL1)->CTRLA = USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_OFF_gc | USART_DREINTLVL_OFF_gc;

	/* Inform about to start the SIM808 - LCD */
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_START);
	task_twi2_lcd_str(8, (line++) * 10, g_prepare_buf);

	/* Inform about to start the SIM808 - USB */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_START);
	udi_write_tx_buf(g_prepare_buf, len, false);

	/* Release the GSM_RESETn */
	ioport_set_pin_level(GSM_RESET_DRV_GPIO, HIGH);
	delay_ms(500);

	/* Data Terminal Ready is true */
	ioport_set_pin_level(GSM_DTR1_DRV, LOW);	// Activate SIM808 (non SLEEP mode)
	delay_ms(100);
	ioport_set_pin_level(GSM_RTS1_DRV, LOW);	// Serial line ready
	delay_ms(1);

	/* Synchronize with SIM808 */
	while (true) {
		usart_serial_write_packet(USART_SERIAL1, (const uint8_t*)"AT\r", 3);
		delay_ms(100);
		if (g_usart1_rx_ready) {
			{
				irqflags_t flags = cpu_irq_save();
				for (int16_t idx = g_usart1_rx_idx - 1; idx >= 0; --idx) {
					g_prepare_buf[idx] = g_usart1_rx_buf[idx];
				}
				g_usart1_rx_idx = 0;
				g_usart1_rx_ready = false;
				cpu_irq_restore(flags);
			}

			/* Leave loop when SIM808 responds */
			if (g_prepare_buf[0] && strstr_P(g_prepare_buf, PM_SIM808_OK))
				break;

			if (loop_ctr++ > 10) {
				loop_ctr = 0;

				/* Turn off SIM808 */
				ioport_set_pin_level(GSM_RTS1_DRV,			HIGH);	// Serial line NOT ready
				ioport_set_pin_level(GSM_DTR1_DRV,			HIGH);	// DTR inactive
				ioport_set_pin_level(GSM_RESET_DRV_GPIO,	LOW);	// RESETn active
				delay_ms(150);

				/* Inform about restart - LCD */
				if (line > 12) {
					task_twi2_lcd_header();
					line = 3;
				}
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_RESTART);
				task_twi2_lcd_str(8, (line++) * 10, g_prepare_buf);

				/* Inform about restart - USB */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_RESTART);
				udi_write_tx_buf(g_prepare_buf, len, false);

				/* Restart SIM808 */
				ioport_set_pin_level(GSM_RESET_DRV_GPIO,	HIGH);	// Release the RESETn line
				delay_ms(500);
				ioport_set_pin_level(GSM_DTR1_DRV,			LOW);	// DTR active
				delay_ms(100);
				ioport_set_pin_level(GSM_RTS1_DRV,			LOW);	// Serial line ready
				delay_ms(1);
			}
		}
	}

	/* Set the auto baud rate to fix rate */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_IPR, USART_SERIAL1_BAUDRATE);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(500);

#if 1
	/* Request the version number of the firmware */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_01);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(1000);
#endif

#if 0
	/* Request more details about the firmware */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_02);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(1000);
#endif

#if 0
	/* Request the IMSI number of the GSM device */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_03);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(1000);
#endif

#if 0
	/* Show providers of the GSM networks */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_04);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(1000);
#endif

#if 1
	/* Minimize functionality - do not do GSM */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_CFUN0);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(500);
#endif

#if 1
	/* Enable GNSS (GPS, Glonass, ...) */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GPS_01, 1);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(500);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GPS_02, 1);
	usart_serial_write_packet(USART_SERIAL1, (const uint8_t*) g_prepare_buf, len);
	yield_ms(500);
#endif

	/* Inform about baud rate match - LCD */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_OK);
	task_twi2_lcd_str(8, (line++) * 10, g_prepare_buf);

	/* Inform about baud rate match - USB */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_SYNCED);
	udi_write_tx_buf(g_prepare_buf, len, false);
}


void task_serial(uint32_t now)
{
	uint16_t len = 0;
	uint16_t move = 0;

	/* Leave when nothing to do */
	if (!g_usart1_rx_ready) {
		return;
	}

	/* Take out own copy and reset */
	{
		irqflags_t flags = cpu_irq_save();
		if (g_usart1_rx_idx) {
			/* Do a chunk each time */
			len = g_usart1_rx_idx;
			if (len > C_USART1_RX_BUF_CHUNK) {
				move = len - C_USART1_RX_BUF_CHUNK;
				len = C_USART1_RX_BUF_CHUNK;
			}

			/* Make a copy of the chunk */
			for (int16_t idx = len - 1; idx >= 0; --idx) {
				g_prepare_buf[idx] = g_usart1_rx_buf[idx];
			}

			/* If more data is available move that part down */
			if (move) {
				for (int16_t mov_idx = 0; mov_idx < move; ++mov_idx) {
					g_usart1_rx_buf[mov_idx] = g_usart1_rx_buf[mov_idx + C_USART1_RX_BUF_CHUNK];
				}
				g_usart1_rx_idx = move;

			} else {
				/* Buffer empty */
				g_usart1_rx_idx = 0;
				g_usart1_rx_ready = false;
			}
		}
		cpu_irq_restore(flags);
	}

	/* Copy chunk of data to USB_CDC */
	if (len) {
		udi_write_tx_buf(g_prepare_buf, (uint8_t)len, false);
	}
}
