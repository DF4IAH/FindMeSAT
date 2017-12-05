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

#include <math.h>

#include "main.h"
#include "interpreter.h"
#include "usb.h"
#include "twi.h"

#include "serial.h"


/* Add access to the global vars */
#include "externals.h"


/* Note: APRS hosts of SW-Germany */
// TCP: nuremberg.aprs2.net 8080
// TCP: karlsruhe.aprs2.net 8080


/* Global string store in program memory */
const char					PM_SIM808_INFO_LCD_START[]								= "Init: SIM808 Starting  ...";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_LCD_START[]);
const char					PM_SIM808_INFO_LCD_RESTART[]							= "Init: SIM808 Restarting...";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_LCD_RESTART[]);
const char					PM_SIM808_INFO_LCD_INITED[]								= "Init: SIM808 - init'ed ...";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_LCD_INITED[]);
const char					PM_SIM808_INFO_LCD_WAIT[]								= "Init: SIM808 - GPRS up ...";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_LCD_WAIT[]);

const char					PM_SIM808_INFO_START[]									= "SIM808 ser1:  Starting the device ...\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_START[]);
const char					PM_SIM808_INFO_RESTART[]								= "SIM808 ser1:  Restarting the device ...\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_RESTART[]);
const char					PM_SIM808_INFO_SYNCED[]									= "SIM808 ser1:  --> now synced ....\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_SYNCED[]);
const char					PM_SIM808_INFO_WAIT_CONNECT[]							= "SIM808 ser1:  --> device registering and enabling GPRS connection ...\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_WAIT_CONNECT[]);

const char					PM_SET_FUNC_1[]											= "AT+CFUN=%d\r\n";
PROGMEM_DECLARE(const char, PM_SET_FUNC_1[]);
const char					PM_SET_PIN_1[]											= "AT+CPIN=\"%.14s\"\r\n";
PROGMEM_DECLARE(const char, PM_SET_PIN_1[]);
const char					PM_SET_CPOWD_X[]										= "AT+CPOWD=%d\r\n";
PROGMEM_DECLARE(const char, PM_SET_CPOWD_X[]);

const char					PM_TWI1_INIT_ONBOARD_SIM808_IPR_X[]						= "AT+IPR=%ld\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_IPR_X[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_IFC_XX[]					= "AT+IFC=%d,%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_IFC_XX[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_ATE_X[]						= "ATE%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_ATE_X[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_CMEE_X[]					= "AT+CMEE=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_CMEE_X[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_CREG_X[]					= "AT+CREG=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_CREG_X[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_CFUN_X[]					= "AT+CFUN=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_CFUN_X[]);

const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_01[]					= "ATI\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_01[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_02[]					= "AT+GSV\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_02[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_03[]					= "AT+CIMI\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_03[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_04[]					= "AT+COPN\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_04[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_INFO_05[]					= "AT+CNETSCAN=1;+CNETSCAN\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_INFO_05[]);

const char					PM_TWI1_INIT_ONBOARD_SIM808_GPS_01[]					= "AT+CGNSPWR=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GPS_01[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GPS_02[]					= "AT+CGNSINF\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GPS_02[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GPS_03[]					= "AT+CGNSURC=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GPS_03[]);

const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CSQ[]				= "AT+CSQ\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CSQ[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CREG[]				= "AT+CREG?\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CREG[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CGATT[]			= "AT+CGATT?\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CGATT[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CSTT_PARAM[]		= "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CSTT_PARAM[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIICR[]			= "AT+CIICR\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIICR[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIFSR[]			= "AT+CIFSR\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIFSR[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSTART[]			= "AT+CIPSTART=\"%s\",\"%s\",\"%d\"\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSTART[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSEND[]			= "AT+CIPSEND\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSEND[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CTRL_Z[]			= "\x1A\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CTRL_Z[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPCLOSE[]			= "AT+CIPCLOSE\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPCLOSE[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSERVER[]		= "AT+CIPSERVER=%d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSERVER[]);
const char					PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSHUT[]			= "AT+CIPSHUT\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSHUT[]);

const char					PM_TWI1_UTIL_ONBOARD_SIM808_OK_R[]						= "OK";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_OK_R[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_RDY_R[]						= "RDY";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_RDY_R[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_UGNSINF_R[]					= "+UGNSINF:";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_UGNSINF_R[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DD[]					= "+CREG: %d,%d";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DD[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DSS[]				= "+CREG: %d,\"%c%c%c%c\",\"%c%c%c%c\"";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DSS[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DDSS[]				= "+CREG: %d,%d,\"%c%c%c%c\",\"%c%c%c%c\"";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DDSS[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_CGATT_R[]					= "+CGATT: ";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_CGATT_R[]);
const char					PM_TWI1_UTIL_ONBOARD_SIM808_RING_R[]					= "RING";
PROGMEM_DECLARE(const char, PM_TWI1_UTIL_ONBOARD_SIM808_RING_R[]);


/* ISR routines */

/* USART, RX - Complete */
ISR(USARTF0_RXC_vect, ISR_BLOCK)
{
	uint8_t ser1_rxd = USARTF0_DATA;

	if (g_usart1_rx_idx < (C_USART1_RX_BUF_LEN - 1)) {
		g_usart1_rx_buf[g_usart1_rx_idx++]	= (char)ser1_rxd;
		g_usart1_rx_buf[g_usart1_rx_idx]	= 0;
	}

	/* Handshaking - STOP data */
	if (g_usart1_rx_idx > (C_USART1_RX_BUF_LEN - C_USART1_RX_BUF_DIFF_OFF)) {
		ioport_set_pin_level(GSM_RTS1_DRV_GPIO, IOPORT_PIN_LEVEL_HIGH);
	}

	/* Input string ready to read */
	g_usart1_rx_ready = true;
}

/* USART, TX - Complete */
ISR(USARTF0_TXC_vect, ISR_BLOCK)
{
	if (g_usart1_tx_len > 0) {
		if (ioport_get_pin_level(GSM_CTS1_GPIO) == IOPORT_PIN_LEVEL_LOW) {
			/* Send next character */
			USARTF0_DATA = g_usart1_tx_buf[0];

			/* Move string one position ahead */
			for (uint8_t idx = 0; idx < (g_usart1_tx_len - 1); idx++) {
				g_usart1_tx_buf[idx] = g_usart1_tx_buf[idx + 1];
			}

			/* Finish TX string and resize length to be sent */
			g_usart1_tx_buf[--g_usart1_tx_len] = 0;
		}
	}
}


/* Functions */

static void isr_serial_tx_kickstart(void)
{
	/* No ongoing transmission, but data is available to be transfered and SIM808 is ready for that */
	if ((ioport_get_pin_level(GSM_RTS1_DRV_GPIO) == IOPORT_PIN_LEVEL_LOW) && g_usart1_tx_len && (USARTF0_STATUS & _BV(USART_DREIF_bp))) {
		/* Send next character */
		USARTF0_DATA = g_usart1_tx_buf[0];

		/* Move string one position ahead */
		for (uint8_t idx = 0; idx < (g_usart1_tx_len - 1); idx++) {
			g_usart1_tx_buf[idx] = g_usart1_tx_buf[idx + 1];
		}

		/* Finish TX string and resize length to be sent */
		g_usart1_tx_buf[--g_usart1_tx_len] = 0;
	}
}

void serial_sim808_send(const char* msg, uint8_t len, bool doWait)
{
	int16_t max;

	/* IRQ disabled section */
	{
		irqflags_t flags = cpu_irq_save();

		/* Append message to TX buffer */
		max = min(g_usart1_tx_len + len, sizeof(g_usart1_tx_buf) - 3);

		int16_t idx_s = g_usart1_tx_len;
		for (uint8_t idx_b = 0; idx_s < max; idx_b++) {
			g_usart1_tx_buf[idx_s++] = msg[idx_b];
		}
		g_usart1_tx_buf[idx_s]		= 0;
		g_usart1_tx_len				= idx_s;

		/* No ongoing transmission, but data is available to be transfered and SIM808 is ready for that */
		isr_serial_tx_kickstart();

		cpu_irq_restore(flags);
	}

	/* Wait for message to be sent */
	if (doWait) {
		yield_ms(C_USART_SIM808_RESP_MS + max);
		task();
	}
}

bool serial_sim808_sendAndResponse(const char* msg, uint8_t len)
{
	/* Reset the OK response flag */
	g_usart1_rx_OK = false;

	/* Send the string direct to the SIM808 */
	serial_sim808_send(msg, len, true);

	/* Wait until 2 s for the response */
	for (uint8_t ycnt = C_USART_SIM808_RESP_ITER; ycnt; --ycnt) {
		if (g_usart1_rx_OK) {
			/* OK response received */
			return true;
		}

		yield_ms(C_USART_SIM808_RESP_MS);
		task_serial();
	}

	/* No OK response */
	return false;
}

void serial_sim808_gsm_setFunc(C_SERIAL_SIM808_GSM_SETFUNC_ENUM_t funcMode)
{
	/* Prepare message to the SIM808 */
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SET_FUNC_1, funcMode);

	/* Send the string to the SIM808 */
	if (len) {
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
}

void serial_sim808_gsm_setPin(const char* pin)
{
	/* Prepare message to the SIM808 */
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SET_PIN_1, pin);

	/* Send the string to the SIM808 */
	if (len) {
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
}

void serial_gsm_activation(bool enable)
{
	if (enable) {
		if (g_gsm_enable) {
			serial_sim808_gsm_setFunc(C_SERIAL_SIM808_GSM_SETFUNC_ON);
			serial_sim808_gsm_setPin(g_gsm_login_pwd);
		}

	} else {
		if (g_gsm_enable && g_gsm_aprs_enable) {
			serial_gsm_gprs_link_openClose(false);
		}
		serial_sim808_gsm_setFunc(C_SERIAL_SIM808_GSM_SETFUNC_OFF);
	}
}

void serial_gsm_gprs_link_openClose(bool isStart) {
	static bool s_gprs_isOpen = false;
	int len;

	if (!g_gsm_enable || !g_gsm_aprs_enable) {
		g_gsm_aprs_gprs_connected	= false;
		g_gsm_aprs_ip_connected		= false;
		return;
	}

	if (isStart && !s_gprs_isOpen) {
		/* Enable auto reply chain */
		g_usart_gprs_auto_response_state = 1;

		/* Inform about starting the serial communication process */
		{
			int len;

			/* LCD information */
			if (g_workmode == WORKMODE_INIT) {
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_LCD_WAIT);
				task_twi2_lcd_str(8,  9 * 10, g_prepare_buf);
			}

			/* USB information */
			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_WAIT_CONNECT);
			udi_write_tx_buf(g_prepare_buf, len, false);
		}

		#if 0
		len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#11 DEBUG: +CREG? sending ...\r\n");
		udi_write_tx_buf(g_prepare_buf, len, false);
		#endif

		/* Shutdown any open connections and drop GPRS link, just in case */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSHUT);
		serial_sim808_sendAndResponse(g_prepare_buf, len);

		/* GPRS activation - check for registration, first */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CREG);
		serial_sim808_sendAndResponse(g_prepare_buf, len);

		/* Wait until the auto_responder chain has ended */
		{
			uint32_t l_now		= tcc1_get_time();
			uint32_t l_until	= l_now + 5000;

			while (l_until > l_now) {
				if (g_gsm_aprs_gprs_connected) {
					s_gprs_isOpen = true;
					break;
				}
				task();
				l_now = tcc1_get_time();
			}
		}

	} else if (!isStart && s_gprs_isOpen) {
		s_gprs_isOpen = false;

		/* Shut down IP connection */
		serial_gsm_gprs_ip_openClose(false);

		#if 0
		/* Close any listening servers */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSERVER, 0);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
		#endif

		/* Shutdown any open connections and drop GPRS link */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSHUT);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
}

void serial_gsm_rx_creg(C_GSM_CREG_STAT_ENUM_t stat)
{
	static bool s_done = false;
	static bool s_lock = false;
	int len;

	if (!g_gsm_enable || !g_gsm_aprs_enable || s_done || s_lock || (g_usart_gprs_auto_response_state != 1)) {
		return;
	}

	if ((stat == C_GSM_CREG_STAT_REGHOME) || (stat == C_GSM_CREG_STAT_REGROAMING)) {
		s_lock = true;
		barrier();
		s_done = true;

		#if 0
		len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#21 DEBUG: +CREG: 1 (REGHOME) / 5 (REGROAMING) received.\r\n");
		udi_write_tx_buf(g_prepare_buf, len, false);
		#endif

		g_usart_gprs_auto_response_state = 2;

		#if 0
		len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#22 DEBUG: +CGATT? sending ...\r\n");
		udi_write_tx_buf(g_prepare_buf, len, false);
		#endif

		/* Check and push device to activate GPRS */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CGATT);
		s_lock = false;
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
}

void serial_gsm_rx_cgatt(C_GSM_CGATT_STAT_ENUM_t stat)
{
	static bool s_done = false;
	static bool s_lock = false;
	int len;

	if (!g_gsm_enable || !g_gsm_aprs_enable || s_done || s_lock || (g_usart_gprs_auto_response_state != 2)) {
		return;
	}


	if (stat == C_GSM_CREG_STAT_ATTACHED) {
		s_lock = true;
		barrier();
		s_done = true;

		#if 0
		len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#31 DEBUG: +CGATT: 1 (ATTACHED) received.\r\n");
		udi_write_tx_buf(g_prepare_buf, len, false);
		#endif

		#if 0
		len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#32 DEBUG: +CSTT, +CIICR, +CIFSR sending ...\r\n");
		udi_write_tx_buf(g_prepare_buf, len, false);
		#endif

		/* Start task for GPRS service */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CSTT_PARAM, g_aprs_link_service, g_aprs_link_user, g_aprs_link_pwd);  // "web.vodafone.de"
		serial_sim808_sendAndResponse(g_prepare_buf, len);

		/* Establish GPRS connection */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIICR);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
		yield_ms(2500);

		/* Request local IP address */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIFSR);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
		yield_ms(500);

		g_usart_gprs_auto_response_state = 0;
		g_gsm_aprs_gprs_connected = true;
		barrier();
		s_lock = false;
	}
}

void serial_gsm_gprs_ip_openClose(bool isStart)
{
	static bool s_ip_isOpen = false;
	char buf[C_TX_BUF_SIZE];

	/* GSM / APRS service not enabled */
	if (!g_gsm_enable || !g_gsm_aprs_enable) {
		g_gsm_aprs_ip_connected = false;
		return;
	}

	/* GPRS not established */
	if (!g_gsm_aprs_gprs_connected || g_usart_gprs_auto_response_state) {
		return;
	}

	/* Activation of IP not during GPRS link set-up */
	{
		if (isStart && !s_ip_isOpen) {
			int len;

			irqflags_t flags = cpu_irq_save();
			uint16_t l_aprs_ip_port = g_aprs_ip_port;
			cpu_irq_restore(flags);

			#if 0
			len = snprintf(buf, sizeof(buf), "#41 DEBUG: Opening TCP connection ...\r\n");
			udi_write_tx_buf(buf, len, false);
			#endif

			/* Connect TCP/IP port */
			len = snprintf_P(buf, sizeof(buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSTART, ipProto_2_ca(g_aprs_ip_proto), g_aprs_ip_name, l_aprs_ip_port);
			serial_sim808_sendAndResponse(buf, len);
			yield_ms(2750);

			len = snprintf_P(buf, sizeof(buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPSEND);
			serial_sim808_sendAndResponse(buf, len);
			yield_ms(250);

			g_gsm_aprs_ip_connected = true;
			barrier();
			s_ip_isOpen = true;

		} else if (!isStart && s_ip_isOpen) {
			int len;

			g_gsm_aprs_ip_connected = false;

			/* Stop message block by a ^Z character (0x1a) */
			len = snprintf_P(buf, sizeof(buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CTRL_Z);
			serial_sim808_send(buf, len, true);
			yield_ms(1000);

			#if 0
			len = snprintf(buf, sizeof(buf), "#46 DEBUG: Closing TCP connection ...\r\n");
			udi_write_tx_buf(buf, len, false);
			#endif

			/* Close TCP/IP port */
			len = snprintf_P(buf, sizeof(buf), PM_TWI1_INIT_ONBOARD_SIM808_GSM_GPRS_CIPCLOSE);
			serial_sim808_sendAndResponse(buf, len);

			s_ip_isOpen = false;
		}
	}
}

void serial_sim808_gsm_shutdown(void)
{
	if (g_gsm_enable && g_gsm_aprs_enable) {
		serial_gsm_gprs_link_openClose(false);
	}
	g_gsm_aprs_gprs_connected	= false;
	g_gsm_aprs_ip_connected		= false;

	/* Power down the SIM808 device */
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SET_CPOWD_X, 1);
	serial_sim808_sendAndResponse(g_prepare_buf, len);
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
}


/* USB device stack start function to enable stack and start USB */
void serial_start(void)
{
	uint16_t loop_ctr = 0;
	uint8_t  line = 7;
	uint8_t ctrl_a, ctrl_b;

	/* Init and start of the ASF USART service/device */
	{
		int8_t bscale;
		uint32_t bsel;
		irqflags_t flags = cpu_irq_save();

		/* Power reduction: enable power of USARTF0 */
		PR_PRPF &= ~PR_USART0_bm;

		/* Baud rate setting */
		{
			for (bscale = -7; bscale <= 7; bscale++) {
				if (bscale < 0) {
					float bsel_f = ((C_CLOCK_MHZ_F / (16.f * (float)C_USART_SERIAL1_BAUDRATE)) - 1.f) / pow(2., (double)bscale);
					bsel = (uint32_t) (bsel_f + 0.5f);
				} else {
					float bsel_f = (C_CLOCK_MHZ_F / (pow(2., (double)bscale) * 16.f * (float)C_USART_SERIAL1_BAUDRATE)) - 1.f;
					bsel = (uint32_t) (bsel_f + 0.5f);
				}

				if (bsel < 4096) {
					break;
				}
			}

			ctrl_b  = (uint8_t) ((bscale & 0x0f) << USART_BSCALE0_bp);
			ctrl_b |= (uint8_t) ((bsel >> 8) & 0x0f);
			ctrl_a  = (uint8_t) (bsel & 0xff);
			USARTF0_BAUDCTRLA = ctrl_a;
			USARTF0_BAUDCTRLB = ctrl_b;
		}

		/* 8N1 */
		USARTF0_CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;

		/* ISR interrupt levels */
		USARTF0_CTRLA = USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_LO_gc | USART_DREINTLVL_OFF_gc;

		/* RX and TX enable */
		USARTF0_CTRLB = USART_RXEN_bm | USART_TXEN_bm;

		cpu_irq_restore(flags);
	}

	/* Inform about to start the SIM808 - LCD */
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_LCD_START);
	task_twi2_lcd_str(8,  7 * 10, g_prepare_buf);

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
	delay_ms(10);

	/* Synchronize with SIM808 */
	while (true) {
		serial_sim808_send("AT\r", 3, false);
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
			if (g_prepare_buf[0] && strstr_P(g_prepare_buf, PM_TWI1_UTIL_ONBOARD_SIM808_OK_R)) {
				break;
			}

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
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_LCD_RESTART);
				task_twi2_lcd_str(8,  7 * 10, g_prepare_buf);

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

	#if 0
	/* Set the baud rate to AUTO or fixed rate */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_IPR_X, (long) C_USART_SERIAL1_BAUDRATE);
	#else
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_IPR_X, 0L);
	#endif
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	/* Set handshaking of both directions to hardware CTS/RTS */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_IFC_XX, 2, 2);
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	/* Turn off echoing */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_ATE_X, 0);
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	/* Turn on error descriptions */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_CMEE_X, 2);
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	/* Turn on registering information */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_CREG_X, 2);
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	/* Activation of all functionalities */
	serial_gsm_activation(g_gsm_enable);

	#if 0
	/* Request the version number of the firmware */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_01);
	serial_sim808_sendAndResponse(g_prepare_buf, len);
	#endif

	#if 0
	/* Request more details about the firmware */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_02);
	serial_sim808_sendAndResponse(g_prepare_buf, len);
	#endif

	#if 0
	if (g_gsm_enable) {
		/* Request the IMSI number of the GSM device */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_03);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
	#endif

	/* Enable GNSS (GPS, Glonass, ...) and send a position fix request */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GPS_01, 1);
	serial_sim808_sendAndResponse(g_prepare_buf, len);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GPS_02);
	serial_sim808_sendAndResponse(g_prepare_buf, len);

	#if 0
	if (g_gsm_enable) {
		/* Show providers of the GSM networks */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_04);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
	#endif

	#if 0
	if (g_gsm_enable) {
		/* Scan all networks */
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_INFO_05);
		serial_sim808_sendAndResponse(g_prepare_buf, len);
	}
	#endif


	/* Inform about baud rate match - LCD */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_LCD_INITED);
	task_twi2_lcd_str(8,  8 * 10, g_prepare_buf);

	/* Inform about baud rate match - USB */
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_SYNCED);
	udi_write_tx_buf(g_prepare_buf, len, false);
}

void serial_shutdown(void)
{
	ioport_set_pin_level(GSM_RTS1_DRV, HIGH);	// Serial line not ready
	delay_ms(10);

	ioport_set_pin_level(GSM_DTR1_DRV, HIGH);	// Disable SIM808 (SLEEP mode)
	delay_ms(100);

	/* Enable the GSM_RESETn */
	ioport_set_pin_level(GSM_RESET_DRV_GPIO, LOW);

	/* Power reduction: disable power of USARTF0 */
	PR_PRPF |= PR_USART0_bm;
}

void serial_send_gns_urc(uint8_t val)
{
	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_SIM808_GPS_03, val);
	serial_sim808_sendAndResponse(g_prepare_buf, len);
}

/*static*/ bool serial_filter_inStream(const char* buf, uint16_t len)
{
	/* Sanity check for minimum length */
	if (!len) {
		return true;
	}

	/* Check for +UGNSINF sentence reply */
	const int gnsInf_len = strlen_P(PM_TWI1_UTIL_ONBOARD_SIM808_UGNSINF_R);

	const char* lineEnd_ptr = strchr(buf, '\n');
	const int lineEnd_idx = lineEnd_ptr ?  (lineEnd_ptr - buf) : (len - 1);

	const char* gnsInf_ptr = strstr_P(buf, PM_TWI1_UTIL_ONBOARD_SIM808_UGNSINF_R);
	const int gnsInf_idx = gnsInf_ptr ?  (gnsInf_ptr - buf) : (len - 1);

	const char* rxOk_ptr = strstr_P(buf, PM_TWI1_UTIL_ONBOARD_SIM808_OK_R);
	if (rxOk_ptr) {
		g_usart1_rx_OK = true;
	}

	/* Responders for SIM808 initiated requests/status */
	if (g_gsm_enable && g_gsm_aprs_enable) {
		int val[2] = { 0 };
		char outStr[2][4] = { 0 };

		if (10 == sscanf_P(buf, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DDSS,
			&(val[0]), &(val[1]),
			&(outStr[0][0]), &(outStr[0][1]), &(outStr[0][2]), &(outStr[0][3]), &(outStr[1][0]), &(outStr[1][1]), &(outStr[1][2]), &(outStr[1][3]))) {
			if ((val[1] != C_GSM_CREG_STAT_REGHOME) && (val[1] != C_GSM_CREG_STAT_REGROAMING)) {
				irqflags_t flags = cpu_irq_save();
				memset(g_gsm_cell_lac, 0, sizeof(g_gsm_cell_lac));
				memset(g_gsm_cell_ci,  0, sizeof(g_gsm_cell_ci));
				cpu_irq_restore(flags);
			} else {
				irqflags_t flags = cpu_irq_save();
				memcpy(g_gsm_cell_lac, &(outStr[0][0]), sizeof(g_gsm_cell_lac));
				memcpy(g_gsm_cell_ci,  &(outStr[1][0]), sizeof(g_gsm_cell_ci));
				cpu_irq_restore(flags);
			}
			serial_gsm_rx_creg((uint8_t)val[1]);

		} else if (9 == sscanf_P(buf, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DSS,
			&(val[0]),
			&(outStr[0][0]), &(outStr[0][1]), &(outStr[0][2]), &(outStr[0][3]), &(outStr[1][0]), &(outStr[1][1]), &(outStr[1][2]), &(outStr[1][3]))) {
			if ((val[0] != C_GSM_CREG_STAT_REGHOME) && (val[0] != C_GSM_CREG_STAT_REGROAMING)) {
				irqflags_t flags = cpu_irq_save();
				memset(g_gsm_cell_lac, 0, sizeof(g_gsm_cell_lac));
				memset(g_gsm_cell_ci,  0, sizeof(g_gsm_cell_ci));
				cpu_irq_restore(flags);
			} else {
				irqflags_t flags = cpu_irq_save();
				memcpy(g_gsm_cell_lac, &(outStr[0][0]), sizeof(g_gsm_cell_lac));
				memcpy(g_gsm_cell_ci,  &(outStr[1][0]), sizeof(g_gsm_cell_ci));
				cpu_irq_restore(flags);
			}
			serial_gsm_rx_creg((uint8_t)val[0]);

		} else if (2 == sscanf_P(buf, PM_TWI1_UTIL_ONBOARD_SIM808_CREG_R_DD,
			&(val[0]), &(val[1]))) {
			if ((val[1] != C_GSM_CREG_STAT_REGHOME) && (val[1] != C_GSM_CREG_STAT_REGROAMING)) {
				irqflags_t flags = cpu_irq_save();
				memset(g_gsm_cell_lac, 0, sizeof(g_gsm_cell_lac));
				memset(g_gsm_cell_ci,  0, sizeof(g_gsm_cell_ci));
				cpu_irq_restore(flags);
			}
			serial_gsm_rx_creg((uint8_t)val[1]);

		} else if (!strncmp_P((char*)buf, PM_TWI1_UTIL_ONBOARD_SIM808_CGATT_R, sizeof(PM_TWI1_UTIL_ONBOARD_SIM808_CGATT_R) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)buf + (sizeof(PM_TWI1_UTIL_ONBOARD_SIM808_CGATT_R) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				serial_gsm_rx_cgatt((uint8_t)val[0]);
			}

		} else if (!strncmp_P((char*)buf, PM_TWI1_UTIL_ONBOARD_SIM808_RING_R, sizeof(PM_TWI1_UTIL_ONBOARD_SIM808_RING_R) - 1)) {
			g_gsm_ring = 2;  // Number of repeats of first APRS packet
		}
	}

	char* ptr = (char*) gnsInf_ptr;
	if (!gnsInf_ptr) {
		return false;

	} else if (gnsInf_idx < lineEnd_idx) {
		irqflags_t flags;
		ptr += gnsInf_idx + gnsInf_len;

		uint8_t idx = 0;
		int16_t restLen = len - (ptr - buf);
		while (restLen > 0) {
			uint64_t u64	= 0;
			long	loVal	= 0;
			float	fVal	= 0.;
			char*	ptr2	= NULL;
			struct calendar_date calDat;

			switch (idx) {
				case  0:
				case  1:
				case  8:
				case 14:
				case 15:
				case 18:
					loVal = strtol(ptr, &ptr2, 10);
					ptr = ptr2 + 1;

					if        (idx ==  0) {
						g_gns_run_status = loVal;

					} else if (idx ==  1) {
						g_gns_fix_status = loVal;

					} else if (idx ==  8) {
						g_gns_fix_mode = loVal;

					} else if (idx == 14) {
						g_gns_gps_sats_inView = loVal;

					} else if (idx == 15) {
						g_gns_gnss_sats_used = loVal;

					} else if (idx == 16) {
						g_gns_glonass_sats_inView = loVal;

					} else if (idx == 18) {
						g_gns_cPn0_dBHz = loVal;
					}
				break;

				case  2:
					do {
						char c = *(ptr++);
						if ('0' <= c && c <= '9') {
							u64 *= 10;
							u64 += c - '0';
						} else if (c == '.') {
							// Skip decimal point
						} else {
							// Leave loop
							break;
						}
					} while (true);

					if (u64) {
						u64	/=	1000U;
						calDat.second	= (uint8_t) (u64 % 100U);
						u64	/= 	100U;
						calDat.minute	= (uint8_t) (u64 % 100U);
						u64	/= 	100U;
						calDat.hour		= (uint8_t) (u64 % 100U);
						u64	/= 	100U;
						calDat.date		= (uint8_t) (u64 % 100U) - 1;
						u64	/= 	100U;
						calDat.month	= (uint8_t) (u64 % 100U) - 1;
						u64	/= 	100U;
						calDat.year		= (uint16_t) u64;

						uint32_t l_ts	= calendar_date_to_timestamp(&calDat);

						flags = cpu_irq_save();
						l_ts		   -= (uint32_t)(g_milliseconds_cnt64 / 1000);
						g_boot_time_ts	= l_ts;
						cpu_irq_restore(flags);
					}
				break;

				case  3:
				case  4:
				case  5:
				case  6:
				case  7:
				case 10:
				case 11:
				case 12:
					ptr += myStringToFloat(ptr, &fVal);
					if        (idx ==  3) {
						if (fVal) {
							g_gns_lat = fVal;
						}

					} else if (idx ==  4) {
						if (fVal) {
							g_gns_lon = fVal;
						}

					} else if (idx ==  5) {
						if (fVal) {
							g_gns_msl_alt_m = fVal;
							if (g_qnh_is_auto) {
								g_qnh_height_m = (int16_t) (0.5f + fVal);
							}
						}

					} else if (idx ==  6) {
						g_gns_speed_kmPh = fVal;

					} else if (idx ==  7) {
						g_gns_course_deg = fVal;

					} else if (idx == 10) {
						g_gns_dop_h = fVal;

					} else if (idx == 11) {
						g_gns_dop_p = fVal;

					} else if (idx == 12) {
						g_gns_dop_v = fVal;
					}
				break;

				default:
					ptr = cueBehind(ptr, ',');
			}
			restLen = len - (ptr - buf);
			idx++;
		}
	}
	return true;
}


void task_serial(void /*uint32_t now*/)
{
	uint16_t len_out = 0;

	/* Handshaking SIM808 --> MPU, START data - IRQ disabled  */
	{
		irqflags_t flags = cpu_irq_save();

		/* No ongoing transmission, but data is available to be transfered and SIM808 is ready for that */
		isr_serial_tx_kickstart();

		cpu_irq_restore(flags);
	}

	/* Handshaking MPU --> SIM808, START data - IRQ disabled */
	{
		irqflags_t flags = cpu_irq_save();
		uint16_t l_usart1_rx_idx = g_usart1_rx_idx;
		cpu_irq_restore(flags);

		if (l_usart1_rx_idx < (C_USART1_RX_BUF_LEN - C_USART1_RX_BUF_DIFF_ON)) {
			ioport_set_pin_level(GSM_RTS1_DRV_GPIO, IOPORT_PIN_LEVEL_LOW);
		}
	}

	while (g_usart1_rx_ready) {
		/* Find delimiter as line end indicator - IRQ allowed */
		{
			const char* p = strchr(g_usart1_rx_buf, '\n');	// g_usart1_rx_buf has to be 0 terminated
			if (p) {
				len_out = 1 + (p - g_usart1_rx_buf);

			} else if (g_usart1_rx_idx >= (C_USART1_RX_BUF_LEN - C_USART1_RX_BUF_DIFF_ON)) {
				len_out = C_USART1_RX_BUF_LEN;	// indicates to flush the read buffer

			} else {
				return;	// not complete yet
			}
		}

		/* Process the line - IRQ allowed */
		if (len_out < C_USART1_RX_BUF_LEN) {
			static bool s_doNotPrint = false;

			/* Process line and get data */
			bool l_doNotPrint = serial_filter_inStream(g_usart1_rx_buf, len_out);

			/* Copy chunk of data to USB_CDC */
			if (!l_doNotPrint && (!s_doNotPrint || (len_out > 3)) && g_usb_cdc_printStatusLines_sim808) {
				udi_write_serial_line(g_usart1_rx_buf, len_out);
			}

			/* Store last line state */
			s_doNotPrint = l_doNotPrint;
		}

		/* Move serial buffer by one line and adjust index - IRQ disabled */
		{
			irqflags_t flags = cpu_irq_save();

			/* Move operation */
			if (len_out < C_USART1_RX_BUF_LEN) {
				char* l_usart1_rx_buf_ptr		= g_usart1_rx_buf;
				char* l_usart1_rx_buf_mov_ptr	= g_usart1_rx_buf + len_out;
				for (int16_t movItCnt = g_usart1_rx_idx; movItCnt; movItCnt--) {
					*(l_usart1_rx_buf_ptr++)	= *(l_usart1_rx_buf_mov_ptr++);
				}
			}

			/* Adjust index or reset buffer when stuck */
			if (g_usart1_rx_idx <= len_out) {
				g_usart1_rx_idx		= 0;
				g_usart1_rx_ready	= false;

			} else {
				g_usart1_rx_idx	   -= len_out;
			}

			/* Clean operation (for debugging) */
			char* l_usart1_rx_buf_ptr = g_usart1_rx_buf + g_usart1_rx_idx;
			for (int16_t movItCnt = (C_USART1_RX_BUF_LEN - g_usart1_rx_idx); movItCnt; movItCnt--) {
				*(l_usart1_rx_buf_ptr++) = 0;
			}

			cpu_irq_restore(flags);
		}

		/* Handshaking, START data - IRQ disabled */
		{
			irqflags_t flags = cpu_irq_save();
			uint16_t l_usart1_rx_idx = g_usart1_rx_idx;
			cpu_irq_restore(flags);

			if (l_usart1_rx_idx < (C_USART1_RX_BUF_LEN - C_USART1_RX_BUF_DIFF_ON)) {
				ioport_set_pin_level(GSM_RTS1_DRV_GPIO, IOPORT_PIN_LEVEL_LOW);
			}
		}
	}
}
