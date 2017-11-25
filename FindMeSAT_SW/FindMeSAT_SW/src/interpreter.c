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


/* Add access to the global vars */
#include "externals.h"


static char					s_rx_cmdLine_buf[C_RX_CMDLINE_BUF_SIZE]	= "";
static uint8_t				s_rx_cmdLine_idx						= 0;


const char					PM_HELP_HDR_1[]							= "\r\n\r\n\r\n************\r\n";
const char					PM_HELP_HDR_2[]							= "* COMMANDS *\r\n************\r\n\r\n";
const char					PM_HELP__HASH_1[]						= "#\t\tany text is transfered to the SIM808.\r\n";
const char					PM_HELP_ADC_1[]							= "adc=\t\t0: turn ADCA and ADCB off, ";
const char					PM_HELP_ADC_2[]							=	"1: turn ADCA and ADCB on.\r\n";
const char					PM_HELP_APRS_1[]						= "aprs=\t\t0: OFF, 1: ON.\r\n";
const char					PM_HELP_APRS_2[]						=	"\t\tcall=<str>: callsign.\r\n";
const char					PM_HELP_APRS_3[]						=	"\t\tssid=[-]0-15: SSID.\r\n";
const char					PM_HELP_APRS_4[]						=	"\t\ts_name=<str>: APRS Service name.\r\n";
const char					PM_HELP_APRS_5[]						=	"\t\ts_user=<str>: APRS Service user login.\r\n";
const char					PM_HELP_APRS_6[]						=	"\t\ts_pwd=<str>: APRS Service password.\r\n";
const char					PM_HELP_APRS_7[]						=	"\t\tip_proto=<str>: APRS IP protocol (TCP, UDP).\r\n";
const char					PM_HELP_APRS_8[]						=	"\t\tip_name=<str>: APRS IP host name.\r\n";
const char					PM_HELP_APRS_9[]						=	"\t\tip_port=0-65535: APRS IP port.\r\n";
const char					PM_HELP_APRS_10[]						=	"\t\th_user=<str>: APRS Host user login.\r\n";
const char					PM_HELP_APRS_11[]						=	"\t\th_pwd=<str>: APRS Host password.\r\n";
const char					PM_HELP_AT_1[]							= "AT\t\tCMD to send to the SIM808.\r\n";
const char					PM_HELP_BIAS_1[]						= "bias=\t\t0-63: bias voltage ";
const char					PM_HELP_BIAS_2[]						=	"for LCD contrast.\r\n";
const char					PM_HELP_BL_1[]							= "bl=\t\t0-255: backlight PWM, ";
const char					PM_HELP_BL_2[]							= "-1: AUTO, -2: TURNLIGHT special.\r\n";
const char					PM_HELP_CAL_1[]							= "cal=\t\tdefaults: save default values ";
const char					PM_HELP_CAL_2[]							=	"to EEPROM.\r\n";
const char					PM_HELP_CAL_3[]							=	"\t\taccelx: X-axis 1g fact-cal, Y/Z offset-cal.\r\n";
const char					PM_HELP_CAL_4[]							=	"\t\taccely: Y-axis 1g fact-cal, X/Z offset-cal.\r\n";
const char					PM_HELP_CAL_5[]							=	"\t\taccelz: Z-axis 1g fact-cal, X/Y offset-cal.\r\n";
const char					PM_HELP_CAL_6[]							=	"\t\tgyro: reduce GYRO offset errors.\r\n";
const char					PM_HELP_DAC_1[]							= "dac=\t\t0: turn DACB off, ";
const char					PM_HELP_DAC_2[]							=	"1: turn DACB on.\r\n";
const char					PM_HELP_DDS_1[]							= "dds=a,b,c\ta: DDS0 frequency mHz, ";
const char					PM_HELP_DDS_2[]							=	"b: DDS1 mHz, ";
const char					PM_HELP_DDS_3[]							=	"c: starting phase of DDS1-DDS0 deg.\r\n";
const char					PM_HELP_EB_1[]							= "eb=\t\t0: error beep OFF, 1: ON.\r\n";
const char					PM_HELP_ENV_T_1[]						= "env_t=\t\ttemp: environment temp in degC.\r\n";
const char					PM_HELP_GSM_1[]							= "gsm=\t\t0: OFF, 1: ON.\r\n";
const char					PM_HELP_GSM_2[]							=	"\t\taprs=0: APRS via GSM OFF, 1: ON.\r\n";
const char					PM_HELP_GSM_3[]							=	"\t\tpin=<pin>: the PIN of the GSM smart card.\r\n";
const char					PM_HELP_HELP_1[]						= "help\t\tThis information page ";
const char					PM_HELP_HELP_2[]						=	"about all available commands.\r\n";
const char					PM_HELP_INFO_1[]						= "info=\t\t0-255: (bitmask) --> 0: OFF,\r\n";
const char					PM_HELP_INFO_2[]						=	"\t\t- 0x01: ATxmega.\r\n";
const char					PM_HELP_INFO_3[]						=	"\t\t- 0x02: SIM808.\r\n";
const char					PM_HELP_INFO_4[]						=	"\t\t- 0x04: 1PPS/PLL.\r\n";
const char					PM_HELP_KB_1[]							= "kb=\t\t0: key beep OFF, 1: ON.\r\n";
const char					PM_HELP_PT_1[]							= "pt=\t\t0: pitch tone OFF, ";
const char					PM_HELP_PT_2[]							=	"1: turn speed, 2: variometer.\r\n";
const char					PM_HELP_QNH_AUTO_1[]					= "qnh=\t\tauto: height is taken from GPS.\r\n";
const char					PM_HELP_QNH_M_1[]						= "qnh_m=\t\theight: fixed value in meters.\r\n";
const char					PM_HELP_RESET_1[]						= "reset=\t\t1: reboot ALL.\r\n";
const char					PM_HELP_SHUT_1[]						= "shut\t\tShutdown this device.\r\n";
const char					PM_HELP_XO_1[]							= "xo=\t\t0-65535: VCTCXO pull voltage, ";
const char					PM_HELP_XO_2[]							=	"-1: PLL ON.\r\n";
const char					PM_IP_CMD_NewLine[]						= "\r\n";
const char					PM_IP_CMD_CmdLine[]						= "\r\n> ";
PROGMEM_DECLARE(const char, PM_HELP_HDR_1[]);
PROGMEM_DECLARE(const char, PM_HELP_HDR_2[]);
PROGMEM_DECLARE(const char, PM_HELP__HASH_1[]);
PROGMEM_DECLARE(const char, PM_HELP_ADC_1[]);
PROGMEM_DECLARE(const char, PM_HELP_ADC_2[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_1[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_2[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_3[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_4[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_5[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_6[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_7[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_8[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_9[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_10[]);
PROGMEM_DECLARE(const char, PM_HELP_APRS_11[]);
PROGMEM_DECLARE(const char, PM_HELP_AT_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BIAS_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BIAS_2[]);
PROGMEM_DECLARE(const char, PM_HELP_BL_1[]);
PROGMEM_DECLARE(const char, PM_HELP_BL_2[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_1[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_2[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_3[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_4[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_5[]);
PROGMEM_DECLARE(const char, PM_HELP_CAL_6[]);
PROGMEM_DECLARE(const char, PM_HELP_DAC_1[]);
PROGMEM_DECLARE(const char, PM_HELP_DAC_2[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_1[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_2[]);
PROGMEM_DECLARE(const char, PM_HELP_DDS_3[]);
PROGMEM_DECLARE(const char, PM_HELP_EB_1[]);
PROGMEM_DECLARE(const char, PM_HELP_ENV_T_1[]);
PROGMEM_DECLARE(const char, PM_HELP_GSM_1[]);
PROGMEM_DECLARE(const char, PM_HELP_GSM_2[]);
PROGMEM_DECLARE(const char, PM_HELP_GSM_3[]);
PROGMEM_DECLARE(const char, PM_HELP_HELP_1[]);
PROGMEM_DECLARE(const char, PM_HELP_HELP_2[]);
PROGMEM_DECLARE(const char, PM_HELP_INFO_1[]);
PROGMEM_DECLARE(const char, PM_HELP_INFO_2[]);
PROGMEM_DECLARE(const char, PM_HELP_INFO_3[]);
PROGMEM_DECLARE(const char, PM_HELP_INFO_4[]);
PROGMEM_DECLARE(const char, PM_HELP_KB_1[]);
PROGMEM_DECLARE(const char, PM_HELP_PT_1[]);
PROGMEM_DECLARE(const char, PM_HELP_PT_2[]);
PROGMEM_DECLARE(const char, PM_HELP_QNH_AUTO_1[]);
PROGMEM_DECLARE(const char, PM_HELP_QNH_M_1[]);
PROGMEM_DECLARE(const char, PM_HELP_RESET_1[]);
PROGMEM_DECLARE(const char, PM_HELP_SHUT_1[]);
PROGMEM_DECLARE(const char, PM_HELP_XO_1[]);
PROGMEM_DECLARE(const char, PM_HELP_XO_2[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_NewLine[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_CmdLine[]);

void printHelp(void)
{
	static bool s_again = false;

	udi_write_tx_msg_P(PM_HELP_HDR_1);
	udi_write_tx_msg_P(PM_HELP_HDR_2);

	udi_write_tx_msg_P(PM_HELP__HASH_1);

	udi_write_tx_msg_P(PM_HELP_ADC_1);
	udi_write_tx_msg_P(PM_HELP_ADC_2);

	udi_write_tx_msg_P(PM_HELP_APRS_1);
	udi_write_tx_msg_P(PM_HELP_APRS_2);
	udi_write_tx_msg_P(PM_HELP_APRS_3);
	udi_write_tx_msg_P(PM_HELP_APRS_4);
	udi_write_tx_msg_P(PM_HELP_APRS_5);
	udi_write_tx_msg_P(PM_HELP_APRS_6);
	udi_write_tx_msg_P(PM_HELP_APRS_7);
	udi_write_tx_msg_P(PM_HELP_APRS_8);
	udi_write_tx_msg_P(PM_HELP_APRS_9);
	udi_write_tx_msg_P(PM_HELP_APRS_10);
	udi_write_tx_msg_P(PM_HELP_APRS_11);

	udi_write_tx_msg_P(PM_HELP_AT_1);

	udi_write_tx_msg_P(PM_HELP_BIAS_1);
	udi_write_tx_msg_P(PM_HELP_BIAS_2);

	udi_write_tx_msg_P(PM_HELP_BL_1);
	udi_write_tx_msg_P(PM_HELP_BL_2);

	udi_write_tx_msg_P(PM_HELP_CAL_1);
	udi_write_tx_msg_P(PM_HELP_CAL_2);
	udi_write_tx_msg_P(PM_HELP_CAL_3);
	udi_write_tx_msg_P(PM_HELP_CAL_4);
	udi_write_tx_msg_P(PM_HELP_CAL_5);
	udi_write_tx_msg_P(PM_HELP_CAL_6);

	udi_write_tx_msg_P(PM_HELP_DAC_1);
	udi_write_tx_msg_P(PM_HELP_DAC_2);

	udi_write_tx_msg_P(PM_HELP_DDS_1);
	udi_write_tx_msg_P(PM_HELP_DDS_2);
	udi_write_tx_msg_P(PM_HELP_DDS_3);

	udi_write_tx_msg_P(PM_HELP_EB_1);

	udi_write_tx_msg_P(PM_HELP_ENV_T_1);

	udi_write_tx_msg_P(PM_HELP_GSM_1);
	udi_write_tx_msg_P(PM_HELP_GSM_2);
	udi_write_tx_msg_P(PM_HELP_GSM_3);

	udi_write_tx_msg_P(PM_HELP_HELP_1);
	udi_write_tx_msg_P(PM_HELP_HELP_2);

	udi_write_tx_msg_P(PM_HELP_INFO_1);
	udi_write_tx_msg_P(PM_HELP_INFO_2);
	udi_write_tx_msg_P(PM_HELP_INFO_3);
	udi_write_tx_msg_P(PM_HELP_INFO_4);

	udi_write_tx_msg_P(PM_HELP_KB_1);

	udi_write_tx_msg_P(PM_HELP_PT_1);
	udi_write_tx_msg_P(PM_HELP_PT_2);

	udi_write_tx_msg_P(PM_HELP_QNH_AUTO_1);

	udi_write_tx_msg_P(PM_HELP_QNH_M_1);

	udi_write_tx_msg_P(PM_HELP_RESET_1);

	udi_write_tx_msg_P(PM_HELP_SHUT_1);

	udi_write_tx_msg_P(PM_HELP_XO_1);
	udi_write_tx_msg_P(PM_HELP_XO_2);

	udi_write_tx_msg_P(PM_IP_CMD_NewLine);

	if (!s_again) {
		s_again = true;
		udi_write_tx_msg_P(PM_IP_CMD_CmdLine);
	}
}


const char					PM_IP_CMD_adc[]							= "adc=";
const char					PM_IP_CMD_aprs_num[]					= "aprs=";
const char					PM_IP_CMD_aprs_call[]					= "aprs=call=";
const char					PM_IP_CMD_aprs_ssid[]					= "aprs=ssid=";
const char					PM_IP_CMD_aprs_link_name[]				= "aprs=s_name=";
const char					PM_IP_CMD_aprs_link_user[]				= "aprs=s_user=";
const char					PM_IP_CMD_aprs_link_pwd[]				= "aprs=s_pwd=";
const char					PM_IP_CMD_aprs_ip_proto[]				= "aprs=ip_proto=";
const char					PM_IP_CMD_aprs_ip_name[]				= "aprs=ip_name=";
const char					PM_IP_CMD_aprs_ip_port[]				= "aprs=ip_port=";
const char					PM_IP_CMD_aprs_user[]					= "aprs=user=";
const char					PM_IP_CMD_aprs_pwd[]					= "aprs=pwd=";
const char					PM_IP_CMD_AT[]							= "AT";
const char					PM_IP_CMD_A_slash[]						= "A/";
const char					PM_IP_CMD_bias[]						= "bias=";
const char					PM_IP_CMD_bl[]							= "bl=";
const char					PM_IP_CMD_cal_accelx[]					= "cal=accelx";
const char					PM_IP_CMD_cal_accely[]					= "cal=accely";
const char					PM_IP_CMD_cal_accelz[]					= "cal=accelz";
const char					PM_IP_CMD_cal_defaults[]				= "cal=defaults";
const char					PM_IP_CMD_cal_gyro[]					= "cal=gyro";
const char					PM_IP_CMD_dac[]							= "dac=";
const char					PM_IP_CMD_dds[]							= "dds=";
const char					PM_IP_CMD_eb[]							= "eb=";
const char					PM_IP_CMD_env_t[]						= "env_t=";
const char					PM_IP_CMD_gsm_num[]						= "gsm=";
const char					PM_IP_CMD_gsm_aprs[]					= "gsm=aprs=";
const char					PM_IP_CMD_gsm_pin[]						= "gsm=pin=";
const char					PM_IP_CMD_help[]						= "help";
const char					PM_IP_CMD_info[]						= "info=";
const char					PM_IP_CMD_kb[]							= "kb=";
const char					PM_IP_CMD_pt[]							= "pt=";
const char					PM_IP_CMD_qnh_auto[]					= "qnh=auto";
const char					PM_IP_CMD_qnh_m[]						= "qnh_m=";
const char					PM_IP_CMD_reset[]						= "reset=";
const char					PM_IP_CMD_shut[]						= "shut";
const char					PM_IP_CMD_xo[]							= "xo=";
const char					PM_UNKNOWN_01[]							= "\r\n??? unknown command - for assistance enter  help\r\n";
PROGMEM_DECLARE(const char, PM_IP_CMD_adc[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_num[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_call[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_ssid[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_link_name[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_link_user[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_link_pwd[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_ip_proto[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_ip_name[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_ip_port[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_user[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_aprs_pwd[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_AT[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_A_slash[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_bias[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_bl[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_cal_accelx[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_cal_accely[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_cal_accelz[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_cal_defaults[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_cal_gyro[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_dac[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_dds[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_eb[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_env_t[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_gsm_num[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_gsm_aprs[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_gsm_pin[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_help[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_info[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_kb[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_pt[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_qnh_auto[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_qnh_m[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_reset[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_shut[]);
PROGMEM_DECLARE(const char, PM_IP_CMD_xo[]);
PROGMEM_DECLARE(const char, PM_UNKNOWN_01[]);


static void executeCmdLine(char* cmdLine_buf, uint8_t cmdLine_len)
{
	/* Process command */
	{
		if (cmdLine_buf[0] == '#') {
			/* Send every line starting with # to the SIM808 device, excluding the first character itself */
			serial_sim808_send(&(cmdLine_buf[1]), cmdLine_len - 1, false);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_adc,			sizeof(PM_IP_CMD_adc) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_adc) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				adc_app_enable(val[0] != 0);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_call,		sizeof(PM_IP_CMD_aprs_call) - 1)) {
			aprs_call_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_call) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_ssid,		sizeof(PM_IP_CMD_aprs_ssid) - 1)) {
			aprs_ssid_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_ssid) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_link_name,	sizeof(PM_IP_CMD_aprs_link_name) - 1)) {
			aprs_link_service_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_link_name) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_link_user,	sizeof(PM_IP_CMD_aprs_link_user) - 1)) {
			aprs_link_user_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_link_user) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_link_pwd,	sizeof(PM_IP_CMD_aprs_link_pwd) - 1)) {
			aprs_link_pwd_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_link_pwd) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_ip_proto,	sizeof(PM_IP_CMD_aprs_ip_proto) - 1)) {
			aprs_ip_proto_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_ip_proto) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_ip_name,	sizeof(PM_IP_CMD_aprs_ip_name) - 1)) {
			aprs_ip_name_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_ip_name) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_ip_port,	sizeof(PM_IP_CMD_aprs_ip_port) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_aprs_ip_port) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				aprs_ip_port_update(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_user,		sizeof(PM_IP_CMD_aprs_user) - 1)) {
			aprs_user_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_user) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_pwd,		sizeof(PM_IP_CMD_aprs_pwd) - 1)) {
			aprs_pwd_update(cmdLine_buf + (sizeof(PM_IP_CMD_aprs_pwd) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_aprs_num,		sizeof(PM_IP_CMD_aprs_num) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_aprs_num) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				aprs_num_update(val[0]);
			}

		} else if ((!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_AT,		sizeof(PM_IP_CMD_AT) - 1))		||
				   (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_A_slash,	sizeof(PM_IP_CMD_A_slash) - 1)) ||
				   ((1 <= cmdLine_buf[0]) && (cmdLine_buf[0] <= 26))) {
				serial_sim808_send(cmdLine_buf, cmdLine_len, false);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_bias,			sizeof(PM_IP_CMD_bias) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_bias) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				bias_update(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_bl,				sizeof(PM_IP_CMD_bl) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_bl) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				backlight_mode_pwm(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_cal_defaults,	sizeof(PM_IP_CMD_cal_defaults) - 1)) {
			calibration_mode(CALIBRATION_MODE_ENUM__DEFAULTS);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_cal_accelx,		sizeof(PM_IP_CMD_cal_accelx) - 1)) {
			calibration_mode(CALIBRATION_MODE_ENUM__ACCEL_X);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_cal_accely,		sizeof(PM_IP_CMD_cal_accely) - 1)) {
			calibration_mode(CALIBRATION_MODE_ENUM__ACCEL_Y);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_cal_accelz,		sizeof(PM_IP_CMD_cal_accelz) - 1)) {
			calibration_mode(CALIBRATION_MODE_ENUM__ACCEL_Z);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_cal_gyro,		sizeof(PM_IP_CMD_cal_gyro) - 1)) {
			calibration_mode(CALIBRATION_MODE_ENUM__GYRO);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_dac,			sizeof(PM_IP_CMD_dac) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_dac) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				dac_app_enable(val[0] != 0);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_dds,			sizeof(PM_IP_CMD_dds) - 1)) {
			float val[3] = { -1.f, -1.f, -1.f };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_dds) - 1), MY_STRING_TO_VAR_FLOAT | (MY_STRING_TO_VAR_FLOAT << 2) | (MY_STRING_TO_VAR_FLOAT << 4), &(val[0]), NULL, NULL)) {
				dds_update(val[0], val[1], val[2]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_eb,				sizeof(PM_IP_CMD_eb) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_eb) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				errorBeep_enable(val[0] != 0);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_env_t,			sizeof(PM_IP_CMD_env_t) - 1)) {
			float val[3] = { -1.f, -1.f, -1.f };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_env_t) - 1), MY_STRING_TO_VAR_FLOAT, &(val[0]), NULL, NULL)) {
				env_temp(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_gsm_aprs,		sizeof(PM_IP_CMD_gsm_aprs) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_gsm_aprs) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				gsm_aprs_enable(val[0] != 0);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_gsm_pin,		sizeof(PM_IP_CMD_gsm_pin) - 1)) {
			gsm_pin_update(cmdLine_buf + (sizeof(PM_IP_CMD_gsm_pin) - 1));

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_gsm_num,		sizeof(PM_IP_CMD_gsm_num) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_gsm_num) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				gsm_enable(val[0] != 0);
			}

		} else if (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_help,		sizeof(PM_IP_CMD_help) - 1)) {
			printHelp();

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_info,			sizeof(PM_IP_CMD_info) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_info) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				printStatusLines_bitfield(val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_kb,				sizeof(PM_IP_CMD_kb) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_kb) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				keyBeep_enable(val[0] != 0);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_pt,				sizeof(PM_IP_CMD_pt) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_pt) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				pitchTone_mode(val[0]);
			}

		} else if (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_qnh_auto,	sizeof(PM_IP_CMD_qnh_auto) - 1)) {
			qnh_setAuto();

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_qnh_m,			sizeof(PM_IP_CMD_qnh_m) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_qnh_m) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				qnh_setHeightM((int16_t)val[0]);
			}

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_reset,			sizeof(PM_IP_CMD_reset) - 1)) {
			int val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_reset) - 1), MY_STRING_TO_VAR_INT, NULL, NULL, &(val[0]))) {
				if (val[0] == 1) {
					shutdown(true);
				}
			}

		} else if (!strncasecmp_P((char*)cmdLine_buf, PM_IP_CMD_shut,		sizeof(PM_IP_CMD_shut) - 1)) {
			shutdown(false);

		} else if (!strncmp_P((char*)cmdLine_buf, PM_IP_CMD_xo,				sizeof(PM_IP_CMD_xo) - 1)) {
			long val[1] = { 0 };
			if (myStringToVar((char*)cmdLine_buf + (sizeof(PM_IP_CMD_xo) - 1), MY_STRING_TO_VAR_LONG, NULL, &(val[0]), NULL)) {
				xoPwm_set(val[0]);
			}

		} else {
			udi_write_tx_msg_P(PM_UNKNOWN_01);

			if (g_errorBeep_enable) {
				twi2_set_beep(100, 10);  // Bad sound
				yield_ms(125);
			}
		}
	}

	udi_write_tx_msg_P(PM_IP_CMD_CmdLine);
}


void interpreter_doProcess(char rx_buf[], iram_size_t rx_len)
{
	/* Sanity checks */
	if (!rx_buf || !rx_len || (rx_len >= (C_RX_CMDLINE_BUF_SIZE - 1))) {
		return;
	}

	/* Look for line termination or control characters */
	char* pos = memchr(rx_buf, '\r', rx_len);
	if (!pos) {
		if (1 <= rx_buf[rx_len - 1] && rx_buf[rx_len - 1] <= 26) {
			pos = &(rx_buf[rx_len - 1]);
		}
	}

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
