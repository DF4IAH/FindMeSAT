/**
 * \file
 *
 * \brief FindMeSAT
 * interpreter.c
 *
 * Created: 22.07.2017 19:37:37
 *  Author: DF4IAH
 */ 

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "usb.h"
#include "twi.h"

#include "interpreter.h"


extern bool					g_adc_enabled;
extern int16_t				g_backlight_mode_pwm;
extern uint8_t				g_bias_pm;
extern bool					g_dac_enabled;
extern bool					g_errorBeep_enable;
extern bool					g_keyBeep_enable;
extern bool					g_usb_cdc_printStatusLines;
extern bool					g_usb_cdc_rx_received;
extern bool					g_usb_cdc_access_blocked;
extern uint8_t				g_pitch_tone_mode;
extern char					g_prepare_buf[C_TX_BUF_SIZE];

static char					s_rx_cmdLine_buf[C_RX_CMDLINE_BUF_SIZE]	= "";
static uint8_t				s_rx_cmdLine_idx						= 0;


const char					PM_HELP_HDR_1[]							= "\r\n\r\n\r\n************\r\n";
const char					PM_HELP_HDR_2[]							= "* COMMANDS *\r\n************\r\n\r\n";
const char					PM_HELP_ADC_1[]							= "adc=\t\t0: turn ADCA and ADCB off, ";
const char					PM_HELP_ADC_2[]							= "1: turn ADCA and ADCB on\r\n";
const char					PM_HELP_AT_1[]							= "AT\t\tCMD to send to the SIM808\r\n";
const char					PM_HELP_BIAS_1[]						= "bias=\t\t0-63: bias voltage ";
const char					PM_HELP_BIAS_2[]						= "for LCD contrast\r\n";
const char					PM_HELP_BL_1[]							= "bl=\t\t0-255: backlight PWM, ";
const char					PM_HELP_BL_2[]							= "-1: AUTO, -2: SPECIAL\r\n";
const char					PM_HELP_DAC_1[]							= "dac=\t\t0: turn DACB off, ";
const char					PM_HELP_DAC_2[]							= "1: turn DACB on\r\n";
const char					PM_HELP_DDS_1[]							= "dds=a,b,c\ta: DDS0 frequency mHz, ";
const char					PM_HELP_DDS_2[]							= "b: DDS1 mHz, ";
const char					PM_HELP_DDS_3[]							= "c: starting phase of DDS1-DDS0 deg\r\n";
const char					PM_HELP_EB_1[]							= "eb=\t\t0: error beep OFF, 1: ON\r\n";
const char					PM_HELP_HELP_1[]						= "help\t\tThis information page ";
const char					PM_HELP_HELP_2[]						= "about all available commands\r\n";
const char					PM_HELP_INFO_1[]						= "info=\t\t0: OFF, 1: ON\r\n";
const char					PM_HELP_KB_1[]							= "kb=\t\t0: key beep OFF, 1: ON\r\n";
const char					PM_HELP_PT_1[]							= "pt=\t\t0: pitch tone OFF, ";
const char					PM_HELP_PT_2[]							= "1: turn speed, 2: variometer\r\n";
const char					PM_HELP_RESET_1[]						= "reset=\t\t1: reboot ALL\r\n";
const char					PM_IP_CMD_NewLine[]						= "\r\n";
const char					PM_IP_CMD_CmdLine[]						= "\r\n> ";
PROGMEM_DECLARE(const char, PM_HELP_HDR_1[]);
PROGMEM_DECLARE(const char, PM_HELP_HDR_2[]);
PROGMEM_DECLARE(const char, PM_HELP_ADC_1[]);
PROGMEM_DECLARE(const char, PM_HELP_ADC_2[]);
PROGMEM_DECLARE(const char, PM_HELP_AT_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BIAS_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BIAS_2[]);
PROGMEM_DECLARE(const char, PM_HELP_BL_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BL_2[]);
PROGMEM_DECLARE(const char, PM_HELP_DAC_1[]);
PROGMEM_DECLARE(const char, PM_HELP_DAC_2[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_1[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_2[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_3[]);
PROGMEM_DECLARE(const char, PM_HELP_EB_1[]);
PROGMEM_DECLARE(const char, PM_HELP_HELP_1[]);
PROGMEM_DECLARE(const char, PM_HELP_HELP_2[]);
PROGMEM_DECLARE(const char, PM_HELP_INFO_1[]);
PROGMEM_DECLARE(const char, PM_HELP_KB_1[]);
PROGMEM_DECLARE(const char, PM_HELP_PT_1[]);
PROGMEM_DECLARE(const char, PM_HELP_PT_2[]);
PROGMEM_DECLARE(const char, PM_HELP_RESET_1[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_NewLine[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_CmdLine[]);

void printHelp(void)
{
	static bool s_again = false;

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_HDR_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_HDR_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_ADC_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_ADC_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_AT_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_BIAS_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_BIAS_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_BL_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_BL_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_DAC_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_DAC_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_DDS_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_DDS_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_DDS_3);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_EB_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_HELP_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_HELP_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_INFO_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_KB_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_PT_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_PT_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_RESET_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_NewLine);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	if (!s_again) {
		s_again = true;
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_CmdLine);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	}
}


const char					PM_IP_CMD_adc[]							= "adc=";
const char					PM_IP_CMD_AT[]							= "AT";
const char					PM_IP_CMD_A_slash[]						= "A/";
const char					PM_IP_CMD_bias[]						= "bias=";
const char					PM_IP_CMD_bl[]							= "bl=";
const char					PM_IP_CMD_dac[]							= "dac=";
const char					PM_IP_CMD_dds[]							= "dds=";
const char					PM_IP_CMD_eb[]							= "eb=";
const char					PM_IP_CMD_info[]						= "info=";
const char					PM_IP_CMD_help[]						= "help";
const char					PM_IP_CMD_kb[]							= "kb=";
const char					PM_IP_CMD_pt[]							= "pt=";
const char					PM_IP_CMD_reset[]						= "reset=";
const char					PM_UNKNOWN_01[]							= "\r\n??? unknown command - for assistance enter  help\r\n";
PROGMEM_DECLARE(const char, PM_IP_CMD_adc[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_AT[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_A_slash[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_bias[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_bl[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_dac[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_dds[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_eb[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_info[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_help[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_kb[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_pt[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_reset[]);
PROGMEM_DECLARE(const char, PM_UNKNOWN_01[]);

static void executeCmdLine(char* cmdLine_buf, uint8_t cmdLine_len)
{
	/* Process command */
	{
		if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_adc, sizeof(PM_IP_CMD_adc) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_adc) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				adc_app_enable(val[0]);
			}

		} else if ((!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_AT,		sizeof(PM_IP_CMD_AT) - 1))		||
				   (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_A_slash,	sizeof(PM_IP_CMD_A_slash) - 1))) {
				serial_sim808_send(cmdLine_buf, cmdLine_len);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_bias, sizeof(PM_IP_CMD_bias) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_bias) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				bias_update(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_bl, sizeof(PM_IP_CMD_bl) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_bl) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				backlight_mode_pwm(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_dac, sizeof(PM_IP_CMD_dac) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_dac) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				dac_app_enable(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_dds, sizeof(PM_IP_CMD_dds) - 1)) {
			float val[3] = { -1.f, -1.f, -1.f };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_dds) - 1), MY_STRING_TO_VAR_FLOAT | (MY_STRING_TO_VAR_FLOAT << 2) | (MY_STRING_TO_VAR_FLOAT << 4), &(val[0]), NULL, NULL)) {
				dds_update(val[0], val[1], val[2]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_eb, sizeof(PM_IP_CMD_eb) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_eb) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				errorBeep_enable(val[0]);
			}

		} else if (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_help, sizeof(PM_IP_CMD_help) - 1)) {
			printHelp();

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_info, sizeof(PM_IP_CMD_info) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_info) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				printStatusLines_enable(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_kb, sizeof(PM_IP_CMD_kb) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_kb) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				keyBeep_enable(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_pt, sizeof(PM_IP_CMD_pt) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_pt) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				pitchTone_mode(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_reset, sizeof(PM_IP_CMD_reset) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_reset) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				if (val[0] == 1) {
					/* Terminate the USB connection */
					stdio_usb_disable();
					udc_stop();

					asm volatile(
						"jmp 0 \n\t"
						:
						:
						:
					);
				}
			}

		} else {
			int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_UNKNOWN_01);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

			if (g_errorBeep_enable) {
				twi2_set_beep(100, 10);  // Bad sound
				yield_ms(125);
			}
		}
	}

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_CmdLine);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}


void interpreter_doProcess(char rx_buf[], iram_size_t rx_len)
{
	/* Sanity checks */
	if (!rx_buf || !rx_len || (rx_len >= (C_RX_CMDLINE_BUF_SIZE - 1))) {
		return;
	}

	/* Look for line termination */
	char* pos = memchr(rx_buf, '\r', rx_len);

	/* Clipping */
	if ((s_rx_cmdLine_idx + rx_len) >= C_RX_CMDLINE_BUF_SIZE) {
		/* Over sized - clip incoming data on the buffer size limit */
		rx_len = (C_RX_CMDLINE_BUF_SIZE - 1) - s_rx_cmdLine_idx;

		/* Adjust pos if the line ending exists */
		if (pos) {
			pos = rx_buf + rx_len;
		}
	}

	/* Add new chunk to buffer */
	uint8_t pos_len = pos ?  ((pos - rx_buf) + 1) : rx_len;
	memcpy(s_rx_cmdLine_buf + s_rx_cmdLine_idx, rx_buf, pos_len);
	s_rx_cmdLine_idx += pos_len;

	/* Execute line */
	if (pos) {
		/* Feed interpreter with line data */
		executeCmdLine(s_rx_cmdLine_buf, s_rx_cmdLine_idx);
		s_rx_cmdLine_idx = 0;
	}
}
