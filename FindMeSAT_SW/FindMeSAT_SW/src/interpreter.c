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

#include "interpreter.h"


extern bool					g_adc_enabled;
extern bool					g_dac_enabled;
extern bool					g_usb_cdc_printStatusLines;
extern bool					g_usb_cdc_rx_received;
extern char					g_prepare_buf[C_TX_BUF_SIZE];

static char					s_rx_cmdLine_buf[C_RX_CMDLINE_BUF_SIZE]	= "";
static uint8_t				s_rx_cmdLine_idx						= 0;


static bool udi_write_tx_char(int chr, bool stripControl)
{
	if (stripControl) {
		/* Drop control character and return putc() success */
		if ((chr < 0x20) || (chr >= 0x80)) {
			return true;
		}
	}
	return (bool) udi_cdc_putc(chr);
}

uint8_t udi_write_tx_buf(const char* buf, uint8_t len, bool stripControl)
{
	uint8_t ret = 0;

	/* Write out each character - avoiding to use the block write function */
	while (ret < len) {
		uint8_t repeat_cnt = 10;

		while((!udi_write_tx_char(*(buf + ret), stripControl)) && --repeat_cnt) {
			delay_ms(10);
		}
		if (!repeat_cnt) {
			return ret;
		}
		++ret;
	}
	return ret;
}


const char					PM_HELP_01[]							= "\r\n\r\n\r\n************\r\n* COMMANDS *\r\n************\r\n\r\n";
const char					PM_HELP_02[]							= "adc=\t\t0: turn ADCA and ADCB off, 1: turn ADCA and ADCB on\r\n";
const char					PM_HELP_03[]							= "dac=\t\t0: turn DACB off, 1: turn DACB on\r\n";
const char					PM_HELP_04[]							= "help\t\tThis information page about all available commands\r\n";
const char					PM_HELP_05[]							= "info=\t\t0: OFF, 1: ON\r\n";
const char					PM_IP_CMD_NewLine[]						= "\r\n";
const char					PM_IP_CMD_CmdLine[]						= "\r\n> ";
PROGMEM_DECLARE(const char, PM_HELP_01[]);
PROGMEM_DECLARE(const char, PM_HELP_02[]);
PROGMEM_DECLARE(const char, PM_HELP_03[]);
PROGMEM_DECLARE(const char, PM_HELP_04[]);
PROGMEM_DECLARE(const char, PM_HELP_05[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_NewLine[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_CmdLine[]);

void printHelp(void)
{
	static bool again = false;

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_01);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_02);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_03);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_04);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_HELP_05);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_NewLine);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	if (!again) {
		again = true;

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_CmdLine);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	}
}


const char					PM_IP_CMD_adc[]							= "adc=";
const char					PM_IP_CMD_dac[]							= "dac=";
const char					PM_IP_CMD_info[]						= "info=";
const char					PM_IP_CMD_help[]						= "help";
const char					PM_UNKNOWN_01[]							= "\r\n??? unknown command - for assistance enter  help\r\n";
const char					PM_IP_CMD_1INTARG[]						= "%d";
PROGMEM_DECLARE(const char, PM_IP_CMD_adc[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_dac[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_info[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_help[]);
PROGMEM_DECLARE(const char, PM_UNKNOWN_01[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_1INTARG[]);

static void executeCmdLine(char* cmdLine_buf, uint8_t cmdLine_len)
{
	/* Process command */
	{
		if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_adc, sizeof(PM_IP_CMD_adc) - 1)) {
			int val = 0;
			sscanf_P((char*)cmdLine_buf + (sizeof(PM_IP_CMD_adc) - 1), PM_IP_CMD_1INTARG, &val);
			adc_app_enable(val);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_dac, sizeof(PM_IP_CMD_dac) - 1)) {
			int val = 0;
			sscanf_P((char*)cmdLine_buf + (sizeof(PM_IP_CMD_dac) - 1), PM_IP_CMD_1INTARG, &val);
			dac_app_enable(val);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_info, sizeof(PM_IP_CMD_info) - 1)) {
			int val = 0;
			sscanf_P((char*)cmdLine_buf + (sizeof(PM_IP_CMD_info) - 1), PM_IP_CMD_1INTARG, &val);
			printStatusLines_enable(val);

		} else if (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_help, sizeof(PM_IP_CMD_help) - 1)) {
			printHelp();

		} else {
			int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_UNKNOWN_01);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
		}
	}

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_IP_CMD_CmdLine);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}


void interpreter_doProcess(char rx_buf[], iram_size_t rx_len)
{
	/* Sanity checks */
	if (!rx_buf || !rx_len) {
		return;
	}

	char* pos = memchr(rx_buf, '\r', rx_len);

	/* Stash new data into static cmdLine buffer */
	if (!pos) {
		if ((s_rx_cmdLine_idx + rx_len) > C_RX_CMDLINE_BUF_SIZE) {
			/* Over sized - drop all buffered data and use this chunk as new begin */
			memcpy(s_rx_cmdLine_buf, rx_buf, rx_len);
			s_rx_cmdLine_idx = rx_len;

		} else {
			/* Add new chunk to buffer */
			memcpy(s_rx_cmdLine_buf + s_rx_cmdLine_idx, rx_buf, rx_len);
			s_rx_cmdLine_idx += rx_len;
		}

	} else {
		uint8_t pos_len = (pos - rx_buf) + 1;

		/* Add current chunk to the line buffer */
		memcpy(s_rx_cmdLine_buf + s_rx_cmdLine_idx, rx_buf, pos_len);
		s_rx_cmdLine_idx += pos_len;

		/* Feed interpreter with line data */
		executeCmdLine(s_rx_cmdLine_buf, s_rx_cmdLine_idx);
		s_rx_cmdLine_idx = 0;

		/* Attach trailing data to the buffer */
		if (rx_len > pos_len) {
			memcpy(s_rx_cmdLine_buf, pos + 1, rx_len - pos_len);
			s_rx_cmdLine_idx = rx_len - pos_len;
		}
	}
}
