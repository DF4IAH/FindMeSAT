/**
 * \file
 *
 * \brief FindMeSAT
 * main.c
 *
 * Created: 22.01.2017 13:13:47
 * Author : DF4IAH
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
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

#include <ctype.h>
#include <math.h>

#include "conf_dac.h"
#include "dds.h"
#include "usb.h"
#include "serial_sim.h"
#include "twi_1_2.h"
#include "spi_ax.h"
#include "interpreter.h"

#include "main.h"


/* GLOBAL section */

char						g_dbg_buf[C_DBG_BUF_LEN]						= { 0 };
uint16_t					g_dbg_buf_len									= 0;

bool						g_adc_enabled									= true;
bool						g_dac_enabled									= false;
int16_t						g_backlight_mode_pwm							= 0;		// EEPROM
uint8_t						g_bias_pm										= 22;
uint8_t						g_pitch_tone_mode								= 0;		// EEPROM
bool						g_errorBeep_enable								= false;	// EEPROM
bool						g_keyBeep_enable								= false;	// EEPROM

uint64_t					g_milliseconds_cnt64							= 0ULL;
uint32_t					g_boot_time_ts									= 0UL;

uint16_t					g_1pps_last_lo									= 0U;		// measuring time
uint64_t					g_1pps_last_hi									= 0ULL;
int16_t						g_1pps_last_diff								= 0;
bool						g_1pps_last_inSpan								= false;
bool						g_1pps_last_new									= false;
bool						g_1pps_last_adjust								= false;
uint16_t					g_1pps_processed_lo								= 0U;		// corrected time to use
uint64_t					g_1pps_processed_hi								= 0ULL;
bool						g_1pps_proceeded_avail							= false;
uint8_t						g_1pps_processed_outOfSync						= 0;
int16_t						g_1pps_deviation								= 0;
bool						g_1pps_printtwi_avail							= false;
bool						g_1pps_printusb_avail							= false;
uint8_t						g_1pps_phased_cntr								= 0;
uint8_t						g_1pps_led										= 0;
bool						g_1pps_twi_new									= false;

uint8_t						g_gns_run_status								= 0;
uint8_t						g_gns_fix_status								= 0;
float						g_gns_lat										= 0.;
float						g_gns_lon										= 0.;
float						g_gns_msl_alt_m									= 0.;
float						g_gns_speed_kn									= 0.;
float						g_gns_speed_kmPh								= 0.;
float						g_gns_course_deg								= 0.;
uint8_t						g_gns_fix_mode									= 0;
float						g_gns_dop_h										= 0.;
float						g_gns_dop_p										= 0.;
float						g_gns_dop_v										= 0.;
uint8_t						g_gns_gps_sats_inView							= 0;
uint8_t						g_gns_gnss_sats_used							= 0;
uint8_t						g_gns_glonass_sats_inView						= 0;
uint8_t						g_gns_cPn0_dBHz									= 0;
//uint16_t					g_gns_hpa_m										= 0;
//uint16_t					g_gns_vpa_m										= 0;

uint8_t						g_aprs_mode										= 0;
char						g_aprs_link_service[C_APRS_LINK_SERVICE_LEN]	= { 0 };
char						g_aprs_link_user[C_APRS_LINK_USER_LEN]			= { 0 };
char						g_aprs_link_pwd[C_APRS_LINK_PWD_LEN]			= { 0 };
uint8_t						g_aprs_ip_proto									= C_GSM_IP_PROTO_NONE;
char						g_aprs_ip_name[C_APRS_IP_NAME_LEN]				= { 0 };
uint16_t					g_aprs_ip_port									= 0U;
char						g_aprs_source_callsign[C_APRS_S_LEN]			= { 0 };	// EEPROM
char						g_aprs_source_ssid[C_APRS_SSID_LEN]				= { 0 };	// EEPROM
char						g_aprs_login_user[C_APRS_USER_LEN]				= { 0 };	// EEPROM
char						g_aprs_login_pwd[C_APRS_PWD_LEN]				= { 0 };	// EEPROM
uint64_t					g_aprs_alert_last								= 0ULL;
APRS_ALERT_FSM_STATE_ENUM_t	g_aprs_alert_fsm_state							= APRS_ALERT_FSM_STATE__NOOP;
APRS_ALERT_REASON_ENUM_t	g_aprs_alert_reason								= APRS_ALERT_REASON__NONE;
float						g_aprs_pos_anchor_lat							= 0.f;
float						g_aprs_pos_anchor_lon							= 0.f;
int32_t						g_aprs_alert_1_gyro_x_mdps						= 0L;
int32_t						g_aprs_alert_1_gyro_y_mdps						= 0L;
int32_t						g_aprs_alert_1_gyro_z_mdps						= 0L;
int16_t						g_aprs_alert_1_accel_x_mg						= 0;
int16_t						g_aprs_alert_1_accel_y_mg						= 0;
int16_t						g_aprs_alert_1_accel_z_mg						= 0;
int32_t						g_aprs_alert_2_mag_x_nT							= 0L;
int32_t						g_aprs_alert_2_mag_y_nT							= 0L;
int32_t						g_aprs_alert_2_mag_z_nT							= 0L;

bool						g_usb_cdc_stdout_enabled						= false;
bool						g_usb_cdc_printStatusLines_atxmega				= false;	// EEPROM
bool						g_usb_cdc_printStatusLines_sim808				= false;	// EEPROM
bool						g_usb_cdc_printStatusLines_1pps					= false;	// EEPROM
bool						g_usb_cdc_rx_received							= false;
bool						g_usb_cdc_transfers_authorized					= false;
bool						g_usb_cdc_access_blocked						= false;
WORKMODE_ENUM_t				g_workmode										= WORKMODE_OFF;

uint8_t						g_usart_gprs_auto_response_state				= 0;
bool						g_usart1_rx_ready								= false;
bool						g_usart1_rx_OK									= false;
uint16_t					g_usart1_rx_idx									= 0;
char						g_usart1_rx_buf[C_USART1_RX_BUF_LEN]			= { 0 };
uint16_t					g_usart1_tx_len									= 0;
char						g_usart1_tx_buf[C_USART1_TX_BUF_LEN]			= { 0 };

bool						g_gsm_enable									= false;	// EEPROM
bool						g_gsm_aprs_enable								= false;	// EEPROM
bool						g_gsm_aprs_gprs_connected						= false;
bool						g_gsm_aprs_ip_connected							= false;
char						g_gsm_cell_lac[C_GSM_CELL_LAC_LEN]				= { 0 };
char						g_gsm_cell_ci[C_GSM_CELL_CI_LEN]				= { 0 };
char						g_gsm_login_pwd[C_GSM_PIN_BUF_LEN]				= { 0 };	// EEPROM
uint8_t						g_gsm_ring										= 0;


bool						g_twi1_gsm_valid								= false;
uint8_t						g_twi1_gsm_version								= 0;

bool						g_twi1_gyro_valid								= false;
uint8_t						g_twi1_gyro_1_version							= 0;
volatile int16_t			g_twi1_gyro_1_temp								= 0;
volatile int16_t			g_twi1_gyro_1_temp_RTofs						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_temp_sens							= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_temp_deg_100						= 0;
volatile int16_t			g_twi1_gyro_1_accel_x							= 0;
volatile int16_t			g_twi1_gyro_1_accel_y							= 0;
volatile int16_t			g_twi1_gyro_1_accel_z							= 0;
volatile int16_t			g_twi1_gyro_1_accel_ofsx						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_ofsy						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_ofsz						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_factx						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_facty						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_factz						= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_accel_x_mg						= 0;
volatile int16_t			g_twi1_gyro_1_accel_y_mg						= 0;
volatile int16_t			g_twi1_gyro_1_accel_z_mg						= 0;
volatile int16_t			g_twi1_gyro_1_gyro_x							= 0;
volatile int16_t			g_twi1_gyro_1_gyro_y							= 0;
volatile int16_t			g_twi1_gyro_1_gyro_z							= 0;
volatile int16_t			g_twi1_gyro_1_gyro_ofsx							= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_gyro_ofsy							= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_1_gyro_ofsz							= 0;		// EEPROM
volatile int32_t			g_twi1_gyro_1_gyro_x_mdps						= 0;
volatile int32_t			g_twi1_gyro_1_gyro_y_mdps						= 0;
volatile int32_t			g_twi1_gyro_1_gyro_z_mdps						= 0;
volatile bool				g_twi1_gyro_gyro_offset_set__flag				= false;
volatile bool				g_twi1_gyro_accel_offset_set__flag				= false;
uint8_t						g_twi1_gyro_2_version							= 0;
volatile int8_t				g_twi1_gyro_2_asax								= 0;
volatile int8_t				g_twi1_gyro_2_asay								= 0;
volatile int8_t				g_twi1_gyro_2_asaz								= 0;
volatile int16_t			g_twi1_gyro_2_ofsx								= 0;
volatile int16_t			g_twi1_gyro_2_ofsy								= 0;
volatile int16_t			g_twi1_gyro_2_ofsz								= 0;
volatile int16_t			g_twi1_gyro_2_mag_x								= 0;
volatile int16_t			g_twi1_gyro_2_mag_y								= 0;
volatile int16_t			g_twi1_gyro_2_mag_z								= 0;
volatile int16_t			g_twi1_gyro_2_mag_factx							= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_2_mag_facty							= 0;		// EEPROM
volatile int16_t			g_twi1_gyro_2_mag_factz							= 0;		// EEPROM
volatile int32_t			g_twi1_gyro_2_mag_x_nT							= 0;
volatile int32_t			g_twi1_gyro_2_mag_y_nT							= 0;
volatile int32_t			g_twi1_gyro_2_mag_z_nT							= 0;

bool						g_twi1_baro_valid								= false;
uint16_t					g_twi1_baro_version								= 0;
volatile uint16_t			g_twi1_baro_c[C_TWI1_BARO_C_CNT]				= { 0 };
volatile uint32_t			g_twi1_baro_d1									= 0UL;
volatile uint32_t			g_twi1_baro_d2									= 0UL;
volatile int32_t			g_twi1_baro_temp_100							= 0L;
volatile int32_t			g_twi1_baro_p_100								= 0L;

bool						g_twi1_hygro_valid								= false;
volatile uint16_t			g_twi1_hygro_status								= 0;
volatile uint16_t			g_twi1_hygro_S_T								= 0;
volatile uint16_t			g_twi1_hygro_S_RH								= 0;
volatile int16_t			g_twi1_hygro_T_100								= 0;
volatile int16_t			g_twi1_hygro_RH_100								= 0;
volatile int16_t			g_twi1_hygro_DP_100								= 0;

uint8_t						g_twi2_lcd_version								= 0;
volatile bool				g_twi2_lcd_repaint								= false;

bool						g_ax_enable										= false;	// EEPROM
bool						g_ax_pocsag_enable								= false;	// EEPROM
bool						g_ax_aprs_enable								= false;	// EEPROM
struct spi_device			g_ax_spi_device_conf							= { 0 };
volatile uint8_t			g_ax_spi_packet_buffer[C_SPI_AX_BUFFER_LENGTH]	= { 0 };
volatile uint32_t			g_ax_spi_freq_chan[2]							= { 0 };
volatile uint8_t			g_ax_spi_range_chan[2]							= { 0 };
volatile uint8_t			g_ax_spi_vcoi_chan[2]							= { 0 };

volatile int32_t			g_xo_mode_pwm									= 0L;		// EEPROM

struct adc_config			g_adc_a_conf									= { 0 };
struct adc_channel_config	g_adcch_vctcxo_5v0_vbat_conf					= { 0 };
struct adc_channel_config	g_adcch_io_adc4_conf							= { 0 };
struct adc_channel_config	g_adcch_io_adc5_conf							= { 0 };
struct adc_channel_config	g_adcch_silence_conf							= { 0 };

struct adc_config			g_adc_b_conf									= { 0 };
struct adc_channel_config	g_adcch_temp_conf								= { 0 };

volatile uint32_t			g_adc_vctcxo_cur								= 0;
volatile uint32_t			g_adc_vctcxo_sum								= 0;
volatile uint16_t			g_adc_vctcxo_cnt								= 0;
volatile uint32_t			g_adc_5v0_cur									= 0;
volatile uint32_t			g_adc_5v0_sum									= 0;
volatile uint16_t			g_adc_5v0_cnt									= 0;
volatile uint32_t			g_adc_vbat_cur									= 0;
volatile uint32_t			g_adc_vbat_sum									= 0;
volatile uint16_t			g_adc_vbat_cnt									= 0;
volatile uint32_t			g_adc_io_adc4_cur								= 0;
volatile uint32_t			g_adc_io_adc4_sum								= 0;
volatile uint16_t			g_adc_io_adc4_cnt								= 0;
volatile uint32_t			g_adc_io_adc5_cur								= 0;
volatile uint32_t			g_adc_io_adc5_sum								= 0;
volatile uint16_t			g_adc_io_adc5_cnt								= 0;
volatile uint32_t			g_adc_silence_cur								= 0;
volatile uint32_t			g_adc_silence_sum								= 0;
volatile uint16_t			g_adc_silence_cnt								= 0;
volatile uint32_t			g_adc_temp_cur									= 0;
volatile uint32_t			g_adc_temp_sum									= 0;
volatile uint16_t			g_adc_temp_cnt									= 0;
volatile int16_t			g_adc_vctcxo_volt_1000							= 0;
volatile int16_t			g_adc_5v0_volt_1000								= 0;
volatile int16_t			g_adc_vbat_volt_1000							= 0;
volatile int16_t			g_adc_io_adc4_volt_1000							= 0;
volatile int16_t			g_adc_io_adc5_volt_1000							= 0;
volatile int16_t			g_adc_silence_volt_1000							= 0;
volatile int16_t			g_adc_temp_deg_100								= 0;

volatile int16_t			g_env_temp_delta_100							= 0;		// EEPROM
volatile int16_t			g_env_temp_deg_100								= 0;
volatile int16_t			g_env_hygro_RH_100								= 0;

volatile bool				g_qnh_is_auto									= false;	// EEPROM
volatile int16_t			g_qnh_height_m									= 0;		// EEPROM
volatile int32_t			g_qnh_p_h_100									= 0L;

fifo_desc_t					g_fifo_sched_desc;
uint32_t					g_fifo_sched_buffer[FIFO_SCHED_BUFFER_LENGTH]	= { 0 };

struct pwm_config			g_pwm_vctcxo_cfg								= { 0 };
struct pwm_config			g_pwm_ctr_pll_cfg								= { 0 };

volatile uint8_t			g_sched_lock									= 0;
volatile uint8_t			g_interpreter_lock								= 0;
volatile uint8_t			g_twi1_lock										= 0;
volatile bool				g_sched_yield									= false;
volatile bool				g_sched_pop_again								= false;
volatile sched_entry_t		g_sched_data[C_SCH_SLOT_CNT]					= { 0 };
volatile uint8_t			g_sched_sort[C_SCH_SLOT_CNT]					= { 0 };

char						g_prepare_buf[C_TX_BUF_SIZE]					= { 0 };

bool						g_ax_pocsag_chime_enable						= false;
uint32_t					g_ax_pocsag_individual_ric						= 0UL;


const uint16_t				g_ax_pwr_ary[C_AX_PRW_LENGTH]					= {
	0x0000UL,
	0x000aUL,

	// -10.0 .. - 9.1
	  170,	  173,	  175,	  178,	  180,	  182,	  184,	  186,	  188,	  189,

	// - 9.0 .. - 8.1
	  191,	  193,	  194,	  196,	  198,	  199,	  201,	  203,	  205,    207,

	// - 8.0 .. - 7.1
	  209,	  211,	  214,	  216,	  219,	  221,	  224,	  227,	  230,	  233,

	// - 7.0 .. - 6.1
	  236,	  239,	  243,	  246,	  250,	  253,	  257,	  260,	  264,	  267,

	// - 6.0 .. - 5.1
	  271,	  275,	  278,	  282,	  285,	  289,	  292,	  296,	  299,	  303,

	// - 5.0 .. - 4.1
	  306,	  309,	  313,	  317,	  320,	  324,	  327,	  331,	  335,	  338,

	// - 4.0 .. - 3.1
	  342,	  346,	  350,	  354,	  358,	  362,	  366,	  370,	  374,	  379,

	// - 3.0 .. - 2.1
	  383,	  387,	  392,	  397,	  401,	  406,	  411,	  416,	  421,	  426,

	// - 2.0 .. - 1.1
	  431,	  436,	  441,	  447,	  452,	  457,	  462,	  467,	  471,	  476,

	// - 1.0 .. - 0.1
	  480,	  484,	  488,	  492,	  495,	  499,	  502,	  506,	  510,	  514,

	//   0.0 .. + 0.9
	  519,	  524,	  529,	  535,	  540,	  547,	  553,	  559,	  566,	  573,

	// + 1.0 .. + 1.9
	  580,	  587,	  594,	  602,	  609,	  616,	  624,	  632,	  640,	  648,

	// + 2.0 .. + 2.1
	  656,	  664,	  673,	  681,	  690,	  699,	  708,	  718,	  727,	  737,

	// + 3.0 .. + 3.9
	  747,	  757,	  768,	  778,	  789,	  800,	  812,	  824,	  836,	  849,

	// + 4.0 .. + 4.9
	  862,	  876,	  889,	  903,	  917,	  930,	  942,	  954,	  965,	  974,

	// + 5.0 .. + 5.9
	  982,	  988,	  993,	  997,	 1001,	 1004,	 1008,	 1012,	 1016,	 1022,

	// + 6.0 .. + 6.9
	 1030,	 1039,	 1051,	 1064,	 1078,	 1095,	 1112,	 1131,	 1151,	 1171,

	// + 7.0 .. + 7.9
	 1193,	 1215,	 1238,	 1261,	 1284,	 1307,	 1329,	 1350,	 1369,	 1388,

	// + 8.0 .. + 8.9
	 1404,	 1419,	 1431,	 1443,	 1454,	 1466,	 1477,	 1489,	 1503,	 1518,

	// + 9.0 .. + 9.9
	 1536,	 1556,	 1579,	 1604,	 1630,	 1657,	 1685,	 1712,	 1740,	 1766,

	// +10.0 .. +10.9
	 1792,	 1816,	 1839,	 1861,	 1884,	 1907,	 1931,	 1956,	 1984,	 2014,

	// +11.0 .. +11.9
	 2048,	 2085,	 2126,	 2169,	 2215,	 2263,	 2313,	 2363,	 2414,	 2465,

	// +12.0 .. +12.9
	 2516,	 2566,	 2616,	 2666,	 2717,	 2770,	 2824,	 2881,	 2941,	 3004,

	// +13.0 .. +13.9
	 3072,	 3144,	 3220,	 3298,	 3378,	 3459,	 3540,	 3619,	 3696,	 3770,

	// +14.0 .. +14.9
	 3840,	 3905,	 3963,	 4015,	 4058,	 4093,	 4117,	 4131,	 4132,	 4120,

	// +15.0
	 4095
};

uint8_t						g_ax_pocsag_news_idx							= 0;

uint8_t						g_ax_pocsag_activation_code_len;
const uint8_t				g_ax_pocsag_activation_code[][3]				= {
	{ 0, 7, 0x32},
	{ 0, 7, 0x22},
	{ 0, 7, 0x35},
	{ 0, 7, 0x33},
	{ 0, 7, 0x33},
	{ 0, 7, 0x34},
	{ 0, 7, 0x34},
	{ 0, 7, 0x34}
};


const char					PM_POCSAG_TX_MSG_SRCCALL[]						= "%s:  ";
PROGMEM_DECLARE(const char, PM_POCSAG_TX_MSG_SRCCALL[]);
const char					PM_POCSAG_TX_SPD_HDG_POS_RIC[]					= "alt=%5dm %c %08.5f %c%09.5f spd=%03dkm/h hdg=%03d";
PROGMEM_DECLARE(const char, PM_POCSAG_TX_SPD_HDG_POS_RIC[]);
const char					PM_POCSAG_TX_SPD_HDG_POS_NEWS[]					= "alt=%5dm %c %08.5f %4d%02d%02d %c%09.5f %02d:%02d  %c spd=%03dkm/h hdg=%03d";
PROGMEM_DECLARE(const char, PM_POCSAG_TX_SPD_HDG_POS_NEWS[]);

const char					PM_APRS_TX_HTTP_L1[]							= "POST / HTTP/1.1\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_HTTP_L1[]);
const char					PM_APRS_TX_HTTP_L2[]							= "Content-Length: %d\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_HTTP_L2[]);
const char					PM_APRS_TX_HTTP_L3[]							= "Content-Type: application/octet-stream\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_HTTP_L3[]);
const char					PM_APRS_TX_HTTP_L4[]							= "Accept-Type: text/plain\r\n\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_HTTP_L4[]);
const char					PM_APRS_TX_LOGIN[]								= "user %s pass %s vers %s %s\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_LOGIN[]);
const char					PM_APRS_TX_FORWARD[]							= "%s%s>APRS,TCPIP*:";							// USER, SSID with prefixing "-"
PROGMEM_DECLARE(const char, PM_APRS_TX_FORWARD[]);
const char					PM_APRS_TX_SYMBOL_TABLE_ID						= '/';
const char					PM_APRS_TX_SYMBOL_CODE							= 'j';	// /j: Jeep, /R: RV, \k: SUV, /W: WX
const char					PM_APRS_TX_POS_SPD_HDG[]						= "!%02d%5.2f%c%c%03d%5.2f%c%c%03d/%03d";
PROGMEM_DECLARE(const char, PM_APRS_TX_POS_SPD_HDG[]);
const char					PM_APRS_TX_N1[]									= "%cGx=%+06.1fd Gy=%+06.1fd Gz=%+06.1fd";
PROGMEM_DECLARE(const char, PM_APRS_TX_N1[]);
const char					PM_APRS_TX_N2[]									= "%cAx=%+6.3fg Ay=%+6.3fg Az=%+6.3fg";
PROGMEM_DECLARE(const char, PM_APRS_TX_N2[]);
const char					PM_APRS_TX_N3[]									= "%cMx=%+6.1fuT My=%+6.1fuT Mz=%+6.1fuT";
PROGMEM_DECLARE(const char, PM_APRS_TX_N3[]);
const char					PM_APRS_TX_N4[]									= "%c/A=%06ld DP=%+5.2fC QNH=%7.2fhPa";
PROGMEM_DECLARE(const char, PM_APRS_TX_N4[]);
const char					PM_APRS_TX_MSGEND[]								= "\r\n";
PROGMEM_DECLARE(const char, PM_APRS_TX_MSGEND[]);

const char					PM_GSM_IP_PROTO_TCP_STR[]						= "TCP";
PROGMEM_DECLARE(const char, PM_GSM_IP_PROTO_TCP_STR[]);
const char					PM_GSM_IP_PROTO_UDP_STR[]						= "UDP";
PROGMEM_DECLARE(const char, PM_GSM_IP_PROTO_UDP_STR[]);

const char					PM_SIM808_INFO_LCD_READY[]						= "Init: SIM808      done.       ";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_LCD_READY[]);

const char					PM_SIM808_INFO_READY[]							= "SIM808 ser1:  --> READY.\r\n\r\n";
PROGMEM_DECLARE(const char, PM_SIM808_INFO_READY[]);

const char					PM_TWI1_SHUT_01[]								= "Shutting down ...";
PROGMEM_DECLARE(const char, PM_TWI1_SHUT_01[]);

const char					PM_CALIBRATION_GYRO_START[]						= "GYRO   calibration running   ...\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_GYRO_START[]);
const char					PM_CALIBRATION_GYRO_END[]						= "GYRO   calibration done.\r\n\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_GYRO_END[]);
const char					PM_CALIBRATION_ACCELX_START[]					= "ACCELX calibration running   ...\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELX_START[]);
const char					PM_CALIBRATION_ACCELX_END[]						= "ACCELX calibration done.\r\n\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELX_END[]);
const char					PM_CALIBRATION_ACCELY_START[]					= "ACCELY calibration running   ...\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELY_START[]);
const char					PM_CALIBRATION_ACCELY_END[]						= "ACCELY calibration done.\r\n\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELY_END[]);
const char					PM_CALIBRATION_ACCELZ_START[]					= "ACCELZ calibration running   ...\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELZ_START[]);
const char					PM_CALIBRATION_ACCELZ_END[]						= "ACCELZ calibration done.\r\n\r\n";
PROGMEM_DECLARE(const char, PM_CALIBRATION_ACCELZ_END[]);


twi_options_t g_twi1_options = {
	.speed     = TWI1_SPEED,
//	.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI1_SPEED),
	.speed_reg = TWI_BAUD((BOARD_XOSC_HZ * CONFIG_PLL0_MUL) / 2, TWI1_SPEED),
	.chip      = TWI1_MASTER_ADDR
};

uint8_t g_twi1_m_data[TWI_DATA_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

twi_package_t g_twi1_packet = {
	.buffer      = (void *)g_twi1_m_data,
	.no_wait     = true
};


twi_options_t g_twi2_options = {
	.speed     = TWI2_SPEED,
//	.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI2_SPEED),
	.speed_reg = TWI_BAUD((BOARD_XOSC_HZ * CONFIG_PLL0_MUL) / 2, TWI2_SPEED),
	.chip      = TWI2_MASTER_ADDR
};

uint8_t g_twi2_m_data[TWI_DATA_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

twi_package_t g_twi2_packet = {
	.chip        = TWI2_SLAVE_ADDR,
	.buffer      = (void *)g_twi2_m_data,
	.no_wait     = true
};

#ifdef TWI1_SLAVE
TWI_Slave_t		g_twi1_slave;
uint8_t			g_twi1_recv_data[DATA_LENGTH];
#endif

#ifdef TWI2_SLAVE
TWI_Slave_t		g_twi2_slave;
uint8_t			g_twi2_recv_data[DATA_LENGTH];
#endif


/* STATIC section for this module */

static struct dac_config			dac_conf				= { 0 };

static struct dma_channel_config	dmach_dma0_conf			= { 0 };
static struct dma_channel_config	dmach_dma1_conf			= { 0 };

static uint32_t						dds0_freq_mHz			= 2000000UL;		// 2 kHz
static uint32_t						dds0_inc				= 0UL;
static uint32_t						dds0_reg				= 0UL;				// Sine
static uint32_t						dds1_freq_mHz			= 4000010UL;		// 4 kHz
static uint32_t						dds1_inc				= 0UL;
static uint32_t						dds1_reg				= 0x40000000UL;		// Cosine

static dma_dac_buf_t dac_io_dac0_buf[2][DAC_NR_OF_SAMPLES]	= { 0 };


/* Forward declarations */

static void tc_init(void);
static void tc_start(void);

static void adc_init(void);
static void adc_start(void);
static void adc_stop(void);

static void dac_init(void);
static void dac_start(void);
static void dac_stop(void);

//static void isr_10ms(void);
static void isr_100ms(void);
static void isr_500ms(void);
static void isr_sparetime(void);

static void dma_init(void);
static void dma_start(void);
static void isr_dma_dac_ch0_A(enum dma_channel_status status);
static void isr_dma_dac_ch0_B(enum dma_channel_status status);

static void task_dac(uint32_t now);
static void task_adc(uint32_t now);


/* UTILS section */

static void init_globals(void)
{
	g_ax_pocsag_activation_code_len			= sizeof(g_ax_pocsag_activation_code) / 3;

	/* 1PPS */
	{
		g_milliseconds_cnt64				= 0ULL;
		g_boot_time_ts						= 0UL;

		g_1pps_last_lo						= 0U;		// measuring time
		g_1pps_last_hi						= 0ULL;
		g_1pps_last_diff					= 0;
		g_1pps_last_inSpan					= false;
		g_1pps_last_new						= false;
		g_1pps_last_adjust					= false;
		g_1pps_processed_lo					= 0U;		// corrected time to use
		g_1pps_processed_hi					= 0ULL;
		g_1pps_proceeded_avail				= false;
		g_1pps_processed_outOfSync			= 0;
		g_1pps_deviation					= 0;
		g_1pps_printtwi_avail				= false;
		g_1pps_printusb_avail				= false;
		g_1pps_phased_cntr					= 0;
		g_1pps_led							= 0;
		g_1pps_twi_new						= false;
	}

	/* USART */
	{
		g_usart1_rx_ready					= false;
		g_usart1_rx_OK						= false;
		g_usart1_rx_idx						= 0;
		g_usart1_tx_len						= 0;
	}

	/* GNS */
	{
		g_gns_run_status					= 0;
		g_gns_fix_status					= 0;
		g_gns_lat							= 0.;
		g_gns_lon							= 0.;
		g_gns_msl_alt_m						= 0.;
		g_gns_speed_kn						= 0.;
		g_gns_speed_kmPh					= 0.;
		g_gns_course_deg					= 0.;
		g_gns_fix_mode						= 0;
		g_gns_dop_h							= 0.;
		g_gns_dop_p							= 0.;
		g_gns_dop_v							= 0.;
		g_gns_gps_sats_inView				= 0;
		g_gns_gnss_sats_used				= 0;
		g_gns_glonass_sats_inView			= 0;
		g_gns_cPn0_dBHz						= 0;
//		g_gns_hpa_m							= 0;
//		g_gns_vpa_m							= 0;
	}

	/* VCTCXO */
	{
		int32_t val_i32 = 0L;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__VCTCXO, &val_i32, sizeof(val_i32)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_xo_mode_pwm = val_i32;
			cpu_irq_restore(flags);
		}
	}

	/* LCD backlight */
	{
		int16_t val_i16 = 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__LCDBL, &val_i16, sizeof(val_i16)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_backlight_mode_pwm = val_i16;
			cpu_irq_restore(flags);
		}
	}

	/* Beepers */
	{
		uint8_t val_ui8 = 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__BEEP, &val_ui8, sizeof(val_ui8)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_errorBeep_enable	= val_ui8 & 0x01;
			g_keyBeep_enable	= val_ui8 & 0x02;
			cpu_irq_restore(flags);
		}
	}

	/* Pitch tone variants */
	{
		uint8_t val_i8 = 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__PITCHTONE, &val_i8, sizeof(val_i8)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_pitch_tone_mode = val_i8;
			cpu_irq_restore(flags);
		}
	}

	/* Environment and QNH settings */
	{
		int16_t val_i16	= 0;
		uint8_t val_ui8	= 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__ENV_TEMP_DELTA, &val_i16, sizeof(val_i16)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_env_temp_delta_100 = val_i16;
			cpu_irq_restore(flags);

			/* Validity check */
			if (g_env_temp_delta_100 < -1000 || g_env_temp_delta_100 > 10000) {
				g_env_temp_delta_100 = 0;
				save_globals(EEPROM_SAVE_BF__ENV);
			}
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__ENV_QNH_AUTO, &val_ui8, sizeof(val_ui8)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_qnh_is_auto = (val_ui8 != 0);
			cpu_irq_restore(flags);
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__ENV_QNH_METERS, &val_i16, sizeof(val_i16)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_qnh_height_m = val_i16;
			cpu_irq_restore(flags);

			/* Validity check */
			if (g_qnh_height_m < -1000 || g_qnh_height_m > 30000) {
				g_qnh_is_auto	= true;
				g_qnh_height_m	= 0;
				save_globals(EEPROM_SAVE_BF__ENV);
			}
		}
	}

	/* Status lines */
	{
		uint8_t val_ui8 = 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__PRINT_STATUS, &val_ui8, sizeof(val_ui8)) == STATUS_OK) {
			irqflags_t flags = cpu_irq_save();
			g_usb_cdc_printStatusLines_atxmega	= val_ui8 & PRINT_STATUS_LINES__ATXMEGA;
			g_usb_cdc_printStatusLines_sim808	= val_ui8 & PRINT_STATUS_LINES__SIM808;
			g_usb_cdc_printStatusLines_1pps		= val_ui8 & PRINT_STATUS_LINES__1PPS;
			cpu_irq_restore(flags);
		}
	}

	/* 9-axis offset and gain corrections */
	{
		int16_t l_twi1_gyro_1_temp_RTofs	= 0;
		int16_t l_twi1_gyro_1_temp_sens		= 0;
		int16_t l_twi1_gyro_1_accel_ofsx	= 0;
		int16_t l_twi1_gyro_1_accel_ofsy	= 0;
		int16_t l_twi1_gyro_1_accel_ofsz	= 0;
		int16_t l_twi1_gyro_1_accel_factx	= 0;
		int16_t l_twi1_gyro_1_accel_facty	= 0;
		int16_t l_twi1_gyro_1_accel_factz	= 0;
		int16_t l_twi1_gyro_1_gyro_ofsx		= 0;
		int16_t l_twi1_gyro_1_gyro_ofsy		= 0;
		int16_t l_twi1_gyro_1_gyro_ofsz		= 0;
		int16_t l_twi1_gyro_2_mag_factx		= 0;
		int16_t l_twi1_gyro_2_mag_facty		= 0;
		int16_t l_twi1_gyro_2_mag_factz		= 0;

		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_TEMP_RTOFS, (void*)&l_twi1_gyro_1_temp_RTofs, sizeof(l_twi1_gyro_1_temp_RTofs));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_TEMP_SENS,  (void*)&l_twi1_gyro_1_temp_sens, sizeof(l_twi1_gyro_1_temp_sens));

		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_X, (void*)&l_twi1_gyro_1_accel_ofsx, sizeof(l_twi1_gyro_1_accel_ofsx));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_Y, (void*)&l_twi1_gyro_1_accel_ofsy, sizeof(l_twi1_gyro_1_accel_ofsy));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_Z, (void*)&l_twi1_gyro_1_accel_ofsz, sizeof(l_twi1_gyro_1_accel_ofsz));

		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_X, (void*)&l_twi1_gyro_1_accel_factx, sizeof(l_twi1_gyro_1_accel_factx));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_Y, (void*)&l_twi1_gyro_1_accel_facty, sizeof(l_twi1_gyro_1_accel_facty));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_Z, (void*)&l_twi1_gyro_1_accel_factz, sizeof(l_twi1_gyro_1_accel_factz));

		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_X, (void*)&l_twi1_gyro_1_gyro_ofsx, sizeof(l_twi1_gyro_1_gyro_ofsx));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_Y, (void*)&l_twi1_gyro_1_gyro_ofsy, sizeof(l_twi1_gyro_1_gyro_ofsy));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_Z, (void*)&l_twi1_gyro_1_gyro_ofsz, sizeof(l_twi1_gyro_1_gyro_ofsz));

		// EEPROM_ADDR__9AXIS_GYRO_FACT_XYZ
		// EEPROM_ADDR__9AXIS_MAG_OFS_XYZ

		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_X, (void*)&l_twi1_gyro_2_mag_factx, sizeof(l_twi1_gyro_2_mag_factx));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_Y, (void*)&l_twi1_gyro_2_mag_facty, sizeof(l_twi1_gyro_2_mag_facty));
		nvm_read(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_Z, (void*)&l_twi1_gyro_2_mag_factz, sizeof(l_twi1_gyro_2_mag_factz));

		/* In case EEPROM is not set yet - load with default values instead */
		if (!l_twi1_gyro_1_accel_factx || !l_twi1_gyro_1_accel_facty || !l_twi1_gyro_1_accel_factz ||
			!l_twi1_gyro_2_mag_factx || !l_twi1_gyro_2_mag_facty || !l_twi1_gyro_2_mag_factz) {
			calibration_mode(CALIBRATION_MODE_ENUM__DEFAULTS);
			return;
		}

		irqflags_t flags = cpu_irq_save();
		g_twi1_gyro_1_temp_RTofs	= l_twi1_gyro_1_temp_RTofs;
		g_twi1_gyro_1_temp_sens		= l_twi1_gyro_1_temp_sens;

		g_twi1_gyro_1_accel_ofsx	= l_twi1_gyro_1_accel_ofsx;
		g_twi1_gyro_1_accel_ofsy	= l_twi1_gyro_1_accel_ofsy;
		g_twi1_gyro_1_accel_ofsz	= l_twi1_gyro_1_accel_ofsz;

		g_twi1_gyro_1_accel_factx	= l_twi1_gyro_1_accel_factx;
		g_twi1_gyro_1_accel_facty	= l_twi1_gyro_1_accel_facty;
		g_twi1_gyro_1_accel_factz	= l_twi1_gyro_1_accel_factz;

		g_twi1_gyro_1_gyro_ofsx		= l_twi1_gyro_1_gyro_ofsx;
		g_twi1_gyro_1_gyro_ofsy		= l_twi1_gyro_1_gyro_ofsy;
		g_twi1_gyro_1_gyro_ofsz		= l_twi1_gyro_1_gyro_ofsz;

		g_twi1_gyro_2_mag_factx		= l_twi1_gyro_2_mag_factx;
		g_twi1_gyro_2_mag_facty		= l_twi1_gyro_2_mag_facty;
		g_twi1_gyro_2_mag_factz		= l_twi1_gyro_2_mag_factz;
		cpu_irq_restore(flags);
	}

	/* AX5243 */
	{
		uint32_t	val_ui32	= 0UL;
		uint8_t		val_ui8		= 0U;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__AX_BF, &val_ui8, sizeof(val_ui8)) == STATUS_OK) {
			g_ax_enable					= val_ui8 & AX__ENABLE;
			g_ax_aprs_enable			= val_ui8 & AX__APRS_ENABLE;
			g_ax_pocsag_enable			= val_ui8 & AX__POCSAG_ENABLE;
			g_ax_pocsag_chime_enable	= val_ui8 & AX__POCSAG_CHIME_ENABLE;
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__AX_POCSAG_RIC, &val_ui32, sizeof(val_ui32)) == STATUS_OK) {
			g_ax_pocsag_individual_ric = val_ui32;
		}

		g_ax_spi_freq_chan[0]	= 0;
		g_ax_spi_freq_chan[1]	= 0;
		g_ax_spi_range_chan[0]	= C_SPI_AX_RANGE_NOT_SET;
		g_ax_spi_range_chan[1]	= C_SPI_AX_RANGE_NOT_SET;
		g_ax_spi_vcoi_chan[0]	= 0;
		g_ax_spi_vcoi_chan[1]	= 0;
		g_ax_pocsag_news_idx	= 0;
	}

	/* GSM */
	{
		uint8_t		val_ui8		= 0U;

		g_gsm_aprs_gprs_connected	= false;
		g_gsm_aprs_ip_connected		= false;
		g_gsm_ring					= 0;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__GSM_BF, &val_ui8, sizeof(val_ui8)) == STATUS_OK) {
			g_gsm_enable			= val_ui8 & GSM__ENABLE;
			g_gsm_aprs_enable		= val_ui8 & GSM__APRS_ENABLE;
		}
		nvm_read(INT_EEPROM, EEPROM_ADDR__GSM_PIN,			(void*)&g_gsm_login_pwd,		sizeof(g_gsm_login_pwd));

		memset(g_gsm_cell_lac, 0, sizeof(g_gsm_cell_lac));
		memset(g_gsm_cell_ci,  0, sizeof(g_gsm_cell_ci));
	}

	/* APRS */
	{
		g_aprs_alert_last					= 0ULL;
		g_aprs_alert_fsm_state				= APRS_ALERT_FSM_STATE__NOOP;
		g_aprs_alert_reason					= APRS_ALERT_REASON__NONE;
		g_aprs_pos_anchor_lat				= 0.f;
		g_aprs_pos_anchor_lon				= 0.f;
		g_aprs_alert_1_gyro_x_mdps			= 0L;
		g_aprs_alert_1_gyro_y_mdps			= 0L;
		g_aprs_alert_1_gyro_z_mdps			= 0L;
		g_aprs_alert_1_accel_x_mg			= 0;
		g_aprs_alert_1_accel_y_mg			= 0;
		g_aprs_alert_1_accel_z_mg			= 0;
		g_aprs_alert_2_mag_x_nT				= 0L;
		g_aprs_alert_2_mag_y_nT				= 0L;
		g_aprs_alert_2_mag_z_nT				= 0L;

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_LINK_SERVICE, &g_aprs_link_service, sizeof(g_aprs_link_service)) != STATUS_OK) {
			memset(g_aprs_link_service, 0, sizeof(g_aprs_link_service));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_LINK_USER, &g_aprs_link_user, sizeof(g_aprs_link_user)) != STATUS_OK) {
			memset(g_aprs_link_user, 0, sizeof(g_aprs_link_user));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_LINK_PWD, &g_aprs_link_pwd, sizeof(g_aprs_link_pwd)) != STATUS_OK) {
			memset(g_aprs_link_pwd, 0, sizeof(g_aprs_link_pwd));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_IP_PROTO, &g_aprs_ip_proto, sizeof(g_aprs_ip_proto)) != STATUS_OK) {
			g_aprs_ip_proto = C_GSM_IP_PROTO_NONE;
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_IP_NAME, &g_aprs_ip_name, sizeof(g_aprs_ip_name)) != STATUS_OK) {
			memset(g_aprs_ip_name, 0, sizeof(g_aprs_ip_name));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_IP_PORT, &g_aprs_ip_port, sizeof(g_aprs_ip_port)) != STATUS_OK) {
			g_aprs_ip_port = 0U;
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_CALLSIGN, &g_aprs_source_callsign, sizeof(g_aprs_source_callsign)) != STATUS_OK) {
			memset(g_aprs_source_callsign, 0, sizeof(g_aprs_source_callsign));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_SSID, &g_aprs_source_ssid, sizeof(g_aprs_source_ssid)) != STATUS_OK) {
			memset(g_aprs_source_ssid, 0, sizeof(g_aprs_source_ssid));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_LOGIN, &g_aprs_login_user, sizeof(g_aprs_login_user)) != STATUS_OK) {
			memset(g_aprs_login_user, 0, sizeof(g_aprs_login_user));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_PWD, &g_aprs_login_pwd, sizeof(g_aprs_login_pwd)) != STATUS_OK) {
			memset(g_aprs_login_pwd, 0, sizeof(g_aprs_login_pwd));
		}

		if (nvm_read(INT_EEPROM, EEPROM_ADDR__APRS_MODE, &g_aprs_mode, sizeof(g_aprs_mode)) != STATUS_OK) {
			g_aprs_mode = 0U;
		}
	}
}

void save_globals(EEPROM_SAVE_BF_ENUM_t bf)
{
	/* Version information */
	{
		uint32_t version_ui32 = ((uint32_t)(20000 + VERSION_HIGH) * 1000) + (uint32_t)VERSION_LOW;
		for (uint8_t idx = 0; idx < 8; ++idx) {
			uint8_t pad = '0' + (version_ui32 % 10);
			nvm_write(INT_EEPROM, EEPROM_ADDR__VERSION + (7 - idx), (void*)&pad,					sizeof(pad));
			version_ui32 /= 10;
		}
	}

	/* VCTCXO */
	if (bf & EEPROM_SAVE_BF__VCTCXO) {
		irqflags_t flags = cpu_irq_save();
		int32_t val_i32 = g_xo_mode_pwm;
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__VCTCXO,				(void*)&val_i32,					sizeof(val_i32));
	}

	/* LCD backlight */
	if (bf & EEPROM_SAVE_BF__LCDBL) {
		irqflags_t flags = cpu_irq_save();
		int16_t val_i16 = g_backlight_mode_pwm;
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__LCDBL,				(void*)&val_i16,					sizeof(val_i16));
	}

	/* Beepers */
	if (bf & EEPROM_SAVE_BF__BEEP) {
		irqflags_t flags = cpu_irq_save();
		uint8_t val_ui8 = (g_keyBeep_enable ?  0x02 : 0x00) | (g_errorBeep_enable ?  0x01 : 0x00);
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__BEEP,				(void*)&val_ui8,					sizeof(val_ui8));
	}

	/* Pitch tone variants */
	if (bf & EEPROM_SAVE_BF__PITCHTONE) {
		irqflags_t flags = cpu_irq_save();
		int8_t val_i8 = g_pitch_tone_mode;
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__PITCHTONE,			(void*)&val_i8,						sizeof(val_i8));
	}

	/* Environment and QNH settings */
	if (bf & EEPROM_SAVE_BF__ENV) {
		irqflags_t flags = cpu_irq_save();
		int16_t val_i16_1	= g_env_temp_delta_100;
		uint8_t val_ui8		= g_qnh_is_auto;
		int16_t val_i16_2	= g_qnh_height_m;
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__ENV_TEMP_DELTA,		(void*)&val_i16_1,					sizeof(val_i16_1));
		nvm_write(INT_EEPROM, EEPROM_ADDR__ENV_QNH_AUTO,		(void*)&val_ui8,					sizeof(val_ui8));
		nvm_write(INT_EEPROM, EEPROM_ADDR__ENV_QNH_METERS,		(void*)&val_i16_2,					sizeof(val_i16_2));
	}

	/* Status lines */
	if (bf & EEPROM_SAVE_BF__PRINT_STATUS) {
		irqflags_t flags = cpu_irq_save();
		uint8_t val_ui8 = (g_usb_cdc_printStatusLines_atxmega	?  PRINT_STATUS_LINES__ATXMEGA	: 0x00)
						| (g_usb_cdc_printStatusLines_sim808	?  PRINT_STATUS_LINES__SIM808	: 0x00)
						| (g_usb_cdc_printStatusLines_1pps		?  PRINT_STATUS_LINES__1PPS		: 0x00);
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__PRINT_STATUS,		(void*)&val_ui8,					sizeof(val_ui8));
	}

	/* 9-axis offset and gain corrections */
	if (bf & EEPROM_SAVE_BF__9AXIS_OFFSETS) {
		irqflags_t flags = cpu_irq_save();
		int16_t l_twi1_gyro_1_temp_RTofs	= g_twi1_gyro_1_temp_RTofs;
		int16_t l_twi1_gyro_1_temp_sens		= g_twi1_gyro_1_temp_sens;

		int16_t l_twi1_gyro_1_accel_ofsx	= g_twi1_gyro_1_accel_ofsx;
		int16_t l_twi1_gyro_1_accel_ofsy	= g_twi1_gyro_1_accel_ofsy;
		int16_t l_twi1_gyro_1_accel_ofsz	= g_twi1_gyro_1_accel_ofsz;

		int16_t l_twi1_gyro_1_accel_factx	= g_twi1_gyro_1_accel_factx;
		int16_t l_twi1_gyro_1_accel_facty	= g_twi1_gyro_1_accel_facty;
		int16_t l_twi1_gyro_1_accel_factz	= g_twi1_gyro_1_accel_factz;

		int16_t l_twi1_gyro_1_gyro_ofsx		= g_twi1_gyro_1_gyro_ofsx;
		int16_t l_twi1_gyro_1_gyro_ofsy		= g_twi1_gyro_1_gyro_ofsy;
		int16_t l_twi1_gyro_1_gyro_ofsz		= g_twi1_gyro_1_gyro_ofsz;

		int16_t l_twi1_gyro_2_mag_factx		= g_twi1_gyro_2_mag_factx;
		int16_t l_twi1_gyro_2_mag_facty		= g_twi1_gyro_2_mag_facty;
		int16_t l_twi1_gyro_2_mag_factz		= g_twi1_gyro_2_mag_factz;
		cpu_irq_restore(flags);

		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_TEMP_RTOFS,	(void*)&l_twi1_gyro_1_temp_RTofs,	sizeof(l_twi1_gyro_1_temp_RTofs));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_TEMP_SENS,		(void*)&l_twi1_gyro_1_temp_sens,	sizeof(l_twi1_gyro_1_temp_sens));

		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_X,	(void*)&l_twi1_gyro_1_accel_ofsx,	sizeof(l_twi1_gyro_1_accel_ofsx));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_Y,	(void*)&l_twi1_gyro_1_accel_ofsy,	sizeof(l_twi1_gyro_1_accel_ofsy));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_OFS_Z,	(void*)&l_twi1_gyro_1_accel_ofsz,	sizeof(l_twi1_gyro_1_accel_ofsz));

		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_X,	(void*)&l_twi1_gyro_1_accel_factx,	sizeof(l_twi1_gyro_1_accel_factx));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_Y,	(void*)&l_twi1_gyro_1_accel_facty,	sizeof(l_twi1_gyro_1_accel_facty));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_ACCEL_FACT_Z,	(void*)&l_twi1_gyro_1_accel_factz,	sizeof(l_twi1_gyro_1_accel_factz));

		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_X,	(void*)&l_twi1_gyro_1_gyro_ofsx,	sizeof(l_twi1_gyro_1_gyro_ofsx));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_Y,	(void*)&l_twi1_gyro_1_gyro_ofsy,	sizeof(l_twi1_gyro_1_gyro_ofsy));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_GYRO_OFS_Z,	(void*)&l_twi1_gyro_1_gyro_ofsz,	sizeof(l_twi1_gyro_1_gyro_ofsz));

		// EEPROM_ADDR__9AXIS_GYRO_FACT_XYZ
		// EEPROM_ADDR__9AXIS_MAG_OFS_XYZ

		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_X,	(void*)&l_twi1_gyro_2_mag_factx,	sizeof(l_twi1_gyro_2_mag_factx));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_Y,	(void*)&l_twi1_gyro_2_mag_facty,	sizeof(l_twi1_gyro_2_mag_facty));
		nvm_write(INT_EEPROM, EEPROM_ADDR__9AXIS_MAG_FACT_Z,	(void*)&l_twi1_gyro_2_mag_factz,	sizeof(l_twi1_gyro_2_mag_factz));
	}

	/* AX5243 */
	if (bf & EEPROM_SAVE_BF__AX) {
		uint8_t val_ui8 = (g_ax_enable				?  AX__ENABLE				: 0x00)
						| (g_ax_aprs_enable			?  AX__APRS_ENABLE			: 0x00)
						| (g_ax_pocsag_enable		?  AX__POCSAG_ENABLE		: 0x00)
						| (g_ax_pocsag_chime_enable	?  AX__POCSAG_CHIME_ENABLE	: 0x00);

		nvm_write(INT_EEPROM, EEPROM_ADDR__AX_BF,				&val_ui8,							sizeof(val_ui8));
		nvm_write(INT_EEPROM, EEPROM_ADDR__AX_POCSAG_RIC,		&g_ax_pocsag_individual_ric,		sizeof(g_ax_pocsag_individual_ric));
	}

	/* GSM */
	if (bf & EEPROM_SAVE_BF__GSM) {
		uint8_t val_ui8 = (g_gsm_enable			?  GSM__ENABLE		: 0x00)
						| (g_gsm_aprs_enable	?  GSM__APRS_ENABLE	: 0x00);

		nvm_write(INT_EEPROM, EEPROM_ADDR__GSM_BF,				&val_ui8,							sizeof(val_ui8));
		nvm_write(INT_EEPROM, EEPROM_ADDR__GSM_PIN,				(void*)&g_gsm_login_pwd,			sizeof(g_gsm_login_pwd));
	}

	/* APRS */
	if (bf & EEPROM_SAVE_BF__APRS) {
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_LINK_SERVICE,	(void*)&g_aprs_link_service,		sizeof(g_aprs_link_service));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_LINK_USER,		(void*)&g_aprs_link_user,			sizeof(g_aprs_link_user));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_LINK_PWD,		(void*)&g_aprs_link_pwd,			sizeof(g_aprs_link_pwd));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_IP_PROTO,		(void*)&g_aprs_ip_proto,			sizeof(g_aprs_ip_proto));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_IP_NAME,		(void*)&g_aprs_ip_name,				sizeof(g_aprs_ip_name));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_IP_PORT,		(void*)&g_aprs_ip_port,				sizeof(g_aprs_ip_port));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_CALLSIGN,		(void*)&g_aprs_source_callsign,		sizeof(g_aprs_source_callsign));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_SSID,			(void*)&g_aprs_source_ssid,			sizeof(g_aprs_source_ssid));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_LOGIN,			(void*)&g_aprs_login_user,			sizeof(g_aprs_login_user));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_PWD,			(void*)&g_aprs_login_pwd,			sizeof(g_aprs_login_pwd));
		nvm_write(INT_EEPROM, EEPROM_ADDR__APRS_MODE,			(void*)&g_aprs_mode,				sizeof(g_aprs_mode));
	}
}


char* cueBehind(char* ptr, char delim)
{
	do {
		char c = *ptr;

		if (c == delim) {
			return ++ptr;
		} else if (!c) {
			return ptr;
		}
		++ptr;
	} while (true);

	return NULL;
}

int myStringToFloat(const char* ptr, float* out)
{
	const char* start	= ptr;
	uint32_t	i1		= 0;
	uint32_t	f1		= 0;
	uint32_t	f2		= 1;
	bool		isNeg	= false;
	bool		isFrac	= false;

	if (ptr) {
		/* Eat any white space */
		while (isspace(*ptr)) {
			ptr++;
		}

		/* Get sign */
		if (*ptr == '+') {
			ptr++;
		} else if (*ptr == '-') {
			ptr++;
			isNeg = true;
		}

		/* Read integer part */
		do {
			char c = *(ptr++);

			if ('0' <= c && c <= '9') {
				i1 *= 10;
				i1 += c - '0';
			} else if (c == '.') {
				isFrac = true;
				break;
			} else {
				break;
			}
		} while (true);

		/* Read fractional part */
		if (isFrac) {
			do {
				char c = *(ptr++);

				if ('0' <= c && c <= '9') {
					f2 *= 10;
					f1 *= 10;
					f1 += c - '0';
				} else {
					break;
				}
			} while (true);
		}

		/* Write out float value */
		if (out) {
			float fl = i1 + ((float)f1 / (float)f2);

			if (isNeg) {
				fl = -fl;
			}
			*out = fl;
		}
	}
	return ptr - start;
}

int myStringToVar(char *str, uint32_t format, float out_f[], long out_l[], int out_i[])
{
	int ret = 0;
	int idx = 0;

	uint8_t mode = format & 0x03;
	while (mode) {
		char* item = str + idx;

		switch (mode) {
			case MY_STRING_TO_VAR_FLOAT:
			if (out_f) {
				*(out_f++) = atof(item);
				++ret;
			}
			break;

			case MY_STRING_TO_VAR_LONG:
			if (out_l) {
				*(out_l++) = atol(item);
				++ret;
			}
			break;

			case MY_STRING_TO_VAR_INT:
			if (out_i) {
				*(out_i++) = atoi(item);
				++ret;
			}
			break;

			default:
				return ret;
		}

		/* forward to next string position */
		char* next = strchr(item, ',');
		if (!next) {
			break;
		}
		idx += 1 + next - item;

		format >>= 2;
		mode = format & 0x03;
	}

	return ret;
}

char* ipProto_2_ca(uint8_t aprs_ip_proto)
{
	switch (aprs_ip_proto) {
		case C_GSM_IP_PROTO_TCP:
			return "TCP";
		break;

		case C_GSM_IP_PROTO_UDP:
			return "UDP";
		break;

		default:
			return "";
	}
}

char* copyStr(char* target, uint8_t targetSize, const char* source)
{
	if (target && targetSize && source) {
		irqflags_t flags = cpu_irq_save();

		memset(target, 0, targetSize);

		for (uint8_t idx = 0; idx < targetSize; idx++) {
			char c = *(source + idx);
			if (c < 0x20) {
				break;
			}
			*(target + idx) = c;
		}

		cpu_irq_restore(flags);
	}
	return target;
}

/* For the CRC-CCITT 16 implementation: @see <LINUX_SOURCE>/lib/crc-ccitt.c  and  include/linux/crc-ccitt.h  and  drivers/net/hamradio/hdlcdrv.c
 * as well as  @see http://practicingelectronics.com/articles/article-100003/article.php
 * This function is verified against the "123456789" sequence which has the result of 0x906E
 */
#define NEED_CRC_CCITT true
#ifdef  NEED_CRC_CCITT
uint8_t calc_CRC16_CCITT(CALC_CRC16_CCITT_ENUM_t selection, uint8_t byte_LSB_first)
{
	static uint16_t s_crc16 = 0xffff;

	switch (selection) {
		case CALC_CRC16_CCITT_RESET:
		{
			s_crc16 = 0xffff;
			return 0;
		}
		break;

		case CALC_CRC16_CCITT_ADD:
		{
			for (uint8_t idx = 0, inp = byte_LSB_first; idx < 8; idx++) {
				uint16_t fb	= (inp & 0x0001) ^ (s_crc16 & 0x0001);						// Step 1: Feedback = Input.bit[0] XOR CRC.bit[15]

				s_crc16 >>= 1;															// Step 2: CRC >>= 1;  Input >>= 1;
				inp		>>= 1;

				s_crc16	 ^= fb * 0x8408U;												// Step 3: CRC ^= (Feedback * POLY)
			}
		}
		break;

		case CALC_CRC16_CCITT_RETURN_LSB:
		{
			return (uint8_t) (~s_crc16 & 0xff);
		}
		break;

		case CALC_CRC16_CCITT_RETURN_MSB:
		{
			return (uint8_t) ((~s_crc16 >> 8) & 0xff);
		}
		break;

		default: { }
	}
	return byte_LSB_first;
}
#endif

uint32_t calc_BitReverse(uint8_t bitWidth, uint32_t in)
{
	uint32_t out = 0UL;

	for (uint8_t idx = 0; idx < bitWidth; idx++) {
		if (in & (1UL << idx)) {
			out |= (1UL << (bitWidth - idx - 1));
		}
	}

	return out;
}


void adc_app_enable(bool enable)
{
	if (g_adc_enabled != enable) {
		if (enable) {
			tc_init();
			adc_init();

			tc_start();
			adc_start();

		} else {
			adc_stop();
		}

		/* each of it is atomic */
		{
			g_adc_enabled = enable;
			g_twi2_lcd_repaint = true;
		}
	}
}

void aprs_num_update(uint8_t mode)
{
	if (mode != APRS_MODE__OFF) {
		mode = APRS_MODE__ON;
	}

	g_aprs_mode = mode;
	save_globals(EEPROM_SAVE_BF__APRS);
}

void aprs_link_service_update(const char service[])
{
	if (copyStr(g_aprs_link_service, sizeof(g_aprs_link_service), service)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_link_user_update(const char user[])
{
	if (copyStr(g_aprs_link_user, sizeof(g_aprs_link_user), user)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_link_pwd_update(const char pwd[])
{
	if (copyStr(g_aprs_link_pwd, sizeof(g_aprs_link_pwd), pwd)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_ip_proto_update(const char proto[])
{
	if (!strncmp_P(proto, PM_GSM_IP_PROTO_TCP_STR, 3)) {
		g_aprs_ip_proto = C_GSM_IP_PROTO_TCP;
		save_globals(EEPROM_SAVE_BF__APRS);

	} else if (!strncmp_P(proto, PM_GSM_IP_PROTO_UDP_STR, 3)) {
		g_aprs_ip_proto = C_GSM_IP_PROTO_UDP;
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_ip_name_update(const char name[])
{
	if (copyStr(g_aprs_ip_name, sizeof(g_aprs_ip_name), name)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_ip_port_update(uint16_t port)
{
	g_aprs_ip_port = port;
	save_globals(EEPROM_SAVE_BF__APRS);
}

void aprs_call_update(const char call[])
{
	if (copyStr(g_aprs_source_callsign, sizeof(g_aprs_source_callsign), call)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_ssid_update(const char ssid[])
{
	g_aprs_source_ssid[0] = 0;
	g_aprs_source_ssid[1] = 0;
	g_aprs_source_ssid[2] = 0;
	g_aprs_source_ssid[3] = 0;

	if (!ssid) {
		return;

	} else if (ssid[0] == '-') {
		g_aprs_source_ssid[0] = ssid[0];
		if (isdigit(ssid[1])) {
			g_aprs_source_ssid[1] = ssid[1];

			if (isdigit(ssid[2])) {
				g_aprs_source_ssid[2] = ssid[2];
			}
		}

	} else {
		if (isdigit(ssid[0])) {
			g_aprs_source_ssid[0] = '-';
			g_aprs_source_ssid[1] = ssid[0];

			if (isdigit(ssid[1])) {
				g_aprs_source_ssid[2] = ssid[1];
			}
		}
	}

	save_globals(EEPROM_SAVE_BF__APRS);
}

void aprs_user_update(const char user[])
{
	if (copyStr(g_aprs_login_user, sizeof(g_aprs_login_user), user)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void aprs_pwd_update(const char pwd[])
{
	if (copyStr(g_aprs_login_pwd, sizeof(g_aprs_login_pwd), pwd)) {
		save_globals(EEPROM_SAVE_BF__APRS);
	}
}

void ax_aprs_enable(bool enable)
{
	/* atomic */
	g_ax_aprs_enable = enable;
	save_globals(EEPROM_SAVE_BF__AX);
}

void ax_pocsag_enable(bool enable)
{
	/* atomic */
	g_ax_pocsag_enable = enable;
	save_globals(EEPROM_SAVE_BF__AX);
}

void ax_enable(bool enable)
{
	/* atomic */
	g_ax_enable = enable;
	save_globals(EEPROM_SAVE_BF__AX);

	if (enable) {
		spi_ax_init_PR1200_Tx();

	} else {
		spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);
	}
}

void backlight_mode_pwm(int16_t mode_pwm)
{
	uint8_t l_pwm = mode_pwm & 0xff;

	/* Setting the mode */
	g_backlight_mode_pwm = mode_pwm;
	save_globals(EEPROM_SAVE_BF__LCDBL);

	switch (mode_pwm) {
		case -2:
			;
		break;

		case -1:
			twi2_set_ledbl(1, 0);
		break;

		default:
			twi2_set_ledbl(0, l_pwm);
	}
}

void bias_update(uint8_t bias)
{
	uint8_t l_bias_pm = bias & 0x3f;

	g_bias_pm = l_bias_pm & 0x3f;
	twi2_set_bias(l_bias_pm);
}

void calibration_mode(CALIBRATION_MODE_ENUM_t mode)
{
	switch (mode) {
		case CALIBRATION_MODE_ENUM__DEFAULTS:
			{
				irqflags_t flags = cpu_irq_save();

				g_twi1_gyro_1_temp_RTofs	= 0;
				g_twi1_gyro_1_temp_sens		= 413;

				g_twi1_gyro_1_accel_ofsx	= C_TWI1_GYRO_1_ACCEL_OFSX_DEFAULT;		// 16LSB / OFS
				g_twi1_gyro_1_accel_ofsy	= C_TWI1_GYRO_1_ACCEL_OFSY_DEFAULT;		// 16LSB / OFS
				g_twi1_gyro_1_accel_ofsz	= C_TWI1_GYRO_1_ACCEL_OFSZ_DEFAULT;		// 16LSB / OFS
				g_twi1_gyro_1_accel_factx	= C_TWI1_GYRO_1_ACCEL_FACTX_DEFAULT;	// X = Xchip * factx / 10000
				g_twi1_gyro_1_accel_facty	= C_TWI1_GYRO_1_ACCEL_FACTY_DEFAULT;	// Y = Ychip * facty / 10000
				g_twi1_gyro_1_accel_factz	= C_TWI1_GYRO_1_ACCEL_FACTZ_DEFAULT;	// Z = Zchip * factz / 10000

				g_twi1_gyro_1_gyro_ofsx		= C_TWI1_GYRO_1_GYRO_OFSX_DEFAULT;		//  4LSB / OFS
				g_twi1_gyro_1_gyro_ofsy		= C_TWI1_GYRO_1_GYRO_OFSY_DEFAULT;		//  4LSB / OFS
				g_twi1_gyro_1_gyro_ofsz		= C_TWI1_GYRO_1_GYRO_OFSZ_DEFAULT;		//  4LSB / OFS

				g_twi1_gyro_2_mag_factx		= C_TWI1_GYRO_2_MAG_FACTX_DEFAULT;		// X = Xchip * factx / 10000
				g_twi1_gyro_2_mag_facty		= C_TWI1_GYRO_2_MAG_FACTY_DEFAULT;		// Y = Ychip * facty / 10000
				g_twi1_gyro_2_mag_factz		= C_TWI1_GYRO_2_MAG_FACTZ_DEFAULT;		// Z = Zchip * factz / 10000

				/* Update the offset registers in the I2C device */
				g_twi1_gyro_gyro_offset_set__flag	= true;
				g_twi1_gyro_accel_offset_set__flag	= true;

				cpu_irq_restore(flags);

				/* Write back current offset values to the EEPROM */
				save_globals(EEPROM_SAVE_BF__9AXIS_OFFSETS);
			}
		break;

		case CALIBRATION_MODE_ENUM__GYRO:
			{
				const uint8_t	iterations = 8;
				int32_t			l_twi1_gyro_1_gyro_mean_x = 0L, l_twi1_gyro_1_gyro_mean_y = 0L, l_twi1_gyro_1_gyro_mean_z = 0L;
				int				len;

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_GYRO_START);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8,  7 * 10, g_prepare_buf);

				twi1_gyro_get_mean_values(iterations, true, &l_twi1_gyro_1_gyro_mean_x, &l_twi1_gyro_1_gyro_mean_y, &l_twi1_gyro_1_gyro_mean_z);

				/* Adjust the settings */
				{
					irqflags_t flags;

					/* Add rounding values */
					l_twi1_gyro_1_gyro_mean_x += (l_twi1_gyro_1_gyro_mean_x >= 0) ?  2 : -2;
					l_twi1_gyro_1_gyro_mean_y += (l_twi1_gyro_1_gyro_mean_y >= 0) ?  2 : -2;
					l_twi1_gyro_1_gyro_mean_z += (l_twi1_gyro_1_gyro_mean_z >= 0) ?  2 : -2;

					flags = cpu_irq_save();

					g_twi1_gyro_1_gyro_ofsx -= (l_twi1_gyro_1_gyro_mean_x >> 2);  // offset value have 1/4th precision
					g_twi1_gyro_1_gyro_ofsy -= (l_twi1_gyro_1_gyro_mean_y >> 2);
					g_twi1_gyro_1_gyro_ofsz -= (l_twi1_gyro_1_gyro_mean_z >> 2);

					cpu_irq_restore(flags);
				}

				/* Write back current offset values to the EEPROM */
				save_globals(EEPROM_SAVE_BF__9AXIS_OFFSETS);

				/* Update the offset registers in the I2C device */
				g_twi1_gyro_gyro_offset_set__flag = true;

				if (service_twi1_gyro(true)) {
					sched_push(task_twi1_gyro, SCHED_ENTRY_CB_TYPE__LISTTIME, 0, true, false, false);
				}

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_GYRO_END);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8,  8 * 10, g_prepare_buf);
			}
		break;

		case CALIBRATION_MODE_ENUM__ACCEL_X:
			{
				const uint8_t	iterations = 8;
				int32_t			l_twi1_gyro_1_accel_mean_x = 0L, l_twi1_gyro_1_accel_mean_y = 0L, l_twi1_gyro_1_accel_mean_z = 0L;
				int				len;

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELX_START);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8,  9 * 10, g_prepare_buf);

				twi1_gyro_get_mean_values(iterations, false, &l_twi1_gyro_1_accel_mean_x, &l_twi1_gyro_1_accel_mean_y, &l_twi1_gyro_1_accel_mean_z);

				/* Adjust the settings */
				{
					irqflags_t flags;

					/* Add rounding values */
					l_twi1_gyro_1_accel_mean_y += (l_twi1_gyro_1_accel_mean_y >= 0) ?  8 : -8;
					l_twi1_gyro_1_accel_mean_z += (l_twi1_gyro_1_accel_mean_z >= 0) ?  8 : -8;

					flags = cpu_irq_save();

					/* Adjust X factor */
					if (g_twi1_gyro_1_accel_x_mg) {
						int16_t l_twi1_gyro_1_accel_mean_x_mg = calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_mean_x, g_twi1_gyro_1_accel_factx);
						g_twi1_gyro_1_accel_factx = (int16_t) (((int32_t)g_twi1_gyro_1_accel_factx * 1000L) / l_twi1_gyro_1_accel_mean_x_mg);

					} else {
						g_twi1_gyro_1_accel_factx = C_TWI1_GYRO_1_ACCEL_FACTX_DEFAULT;
					}

					/* Adjust Y/Z offsets */
					g_twi1_gyro_1_accel_ofsy -= (l_twi1_gyro_1_accel_mean_y >> 4);
					g_twi1_gyro_1_accel_ofsz -= (l_twi1_gyro_1_accel_mean_z >> 4);

					cpu_irq_restore(flags);
				}

				/* Write back current offset values to the EEPROM */
				save_globals(EEPROM_SAVE_BF__9AXIS_OFFSETS);

				/* Update the offset registers in the I2C device */
				g_twi1_gyro_accel_offset_set__flag = true;
				if (service_twi1_gyro(true)) {
					sched_push(task_twi1_gyro, SCHED_ENTRY_CB_TYPE__LISTTIME, 0, true, false, false);
				}

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELX_END);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8, 10 * 10, g_prepare_buf);
			}
		break;

		case CALIBRATION_MODE_ENUM__ACCEL_Y:
			{
				const uint8_t	iterations = 8;
				int32_t			l_twi1_gyro_1_accel_mean_x = 0L, l_twi1_gyro_1_accel_mean_y = 0L, l_twi1_gyro_1_accel_mean_z = 0L;
				int				len;

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELY_START);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8,  9 * 10, g_prepare_buf);

				twi1_gyro_get_mean_values(iterations, false, &l_twi1_gyro_1_accel_mean_x, &l_twi1_gyro_1_accel_mean_y, &l_twi1_gyro_1_accel_mean_z);

				/* Adjust the settings */
				{
					irqflags_t flags;

					/* Add rounding values */
					l_twi1_gyro_1_accel_mean_x += (l_twi1_gyro_1_accel_mean_x >= 0) ?  8 : -8;
					l_twi1_gyro_1_accel_mean_z += (l_twi1_gyro_1_accel_mean_z >= 0) ?  8 : -8;

					flags = cpu_irq_save();

					/* Adjust Y factor */
					if (g_twi1_gyro_1_accel_y_mg) {
						int16_t l_twi1_gyro_1_accel_mean_y_mg = calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_mean_y, g_twi1_gyro_1_accel_facty);
						g_twi1_gyro_1_accel_facty = (int16_t) (((int32_t)g_twi1_gyro_1_accel_facty * 1000L) / l_twi1_gyro_1_accel_mean_y_mg);

					} else {
						g_twi1_gyro_1_accel_facty = C_TWI1_GYRO_1_ACCEL_FACTY_DEFAULT;
					}

					/* Adjust X/Z offsets */
					g_twi1_gyro_1_accel_ofsx -= (l_twi1_gyro_1_accel_mean_x >> 4);
					g_twi1_gyro_1_accel_ofsz -= (l_twi1_gyro_1_accel_mean_z >> 4);

					cpu_irq_restore(flags);
				}

				/* Write back current offset values to the EEPROM */
				save_globals(EEPROM_SAVE_BF__9AXIS_OFFSETS);

				/* Update the offset registers in the I2C device */
				g_twi1_gyro_accel_offset_set__flag = true;
				if (service_twi1_gyro(true)) {
					sched_push(task_twi1_gyro, SCHED_ENTRY_CB_TYPE__LISTTIME, 0, true, false, false);
				}

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELY_END);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8, 10 * 10, g_prepare_buf);
			}
		break;

		case CALIBRATION_MODE_ENUM__ACCEL_Z:
			{
				const uint8_t	iterations = 8;
				int32_t			l_twi1_gyro_1_accel_mean_x = 0L, l_twi1_gyro_1_accel_mean_y = 0L, l_twi1_gyro_1_accel_mean_z = 0L;
				int				len;

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELZ_START);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8,  9 * 10, g_prepare_buf);

				twi1_gyro_get_mean_values(iterations, false, &l_twi1_gyro_1_accel_mean_x, &l_twi1_gyro_1_accel_mean_y, &l_twi1_gyro_1_accel_mean_z);

				/* Adjust the settings */
				{
					irqflags_t flags;

					/* Add rounding values */
					l_twi1_gyro_1_accel_mean_x += (l_twi1_gyro_1_accel_mean_x >= 0) ?  8 : -8;
					l_twi1_gyro_1_accel_mean_y += (l_twi1_gyro_1_accel_mean_y >= 0) ?  8 : -8;

					flags = cpu_irq_save();

					/* Adjust Z factor */
					if (g_twi1_gyro_1_accel_z_mg) {
						int16_t l_twi1_gyro_1_accel_mean_z_mg = calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_mean_z, g_twi1_gyro_1_accel_factz);
						g_twi1_gyro_1_accel_factz = (int16_t) (((int32_t)g_twi1_gyro_1_accel_factz * 1000L) / l_twi1_gyro_1_accel_mean_z_mg);

					} else {
						g_twi1_gyro_1_accel_factz = C_TWI1_GYRO_1_ACCEL_FACTZ_DEFAULT;
					}

					/* Adjust X/Y offsets */
					g_twi1_gyro_1_accel_ofsx -= (l_twi1_gyro_1_accel_mean_x >> 4);
					g_twi1_gyro_1_accel_ofsy -= (l_twi1_gyro_1_accel_mean_y >> 4);

					cpu_irq_restore(flags);
				}

				/* Write back current offset values to the EEPROM */
				save_globals(EEPROM_SAVE_BF__9AXIS_OFFSETS);

				/* Update the offset registers in the I2C device */
				g_twi1_gyro_accel_offset_set__flag = true;
				if (service_twi1_gyro(true)) {
					sched_push(task_twi1_gyro, SCHED_ENTRY_CB_TYPE__LISTTIME, 0, true, false, false);
				}

				/* Inform via USB and LCD */
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_CALIBRATION_ACCELZ_END);
				udi_write_tx_buf(g_prepare_buf, len, false);
				task_twi2_lcd_str(8, 10 * 10, g_prepare_buf);
			}
		break;
	}
}

void dac_app_enable(bool enable)
{
	if (g_dac_enabled != enable) {
		if (enable) {
			/* Setting the values */
			{
				irqflags_t flags	= cpu_irq_save();
				dds0_freq_mHz		= 2000000UL;		// 2 kHz
				dds0_reg			= 0UL;				// Sine
				dds1_freq_mHz		= 4000010UL;		// 4 kHz
				dds1_reg			= 0x40000000UL;		// Cosine
				cpu_irq_restore(flags);
			}

			dac_init();
			tc_start();
			dac_start();

		} else {
			dac_stop();
		}

		/* atomic */
		g_dac_enabled = enable;
	}
}

void dds_update(float dds0_hz, float dds1_hz, float phase)
{
	uint32_t l_dds0_freq_mHz = 0UL;
	uint32_t l_dds1_freq_mHz = 0UL;
	uint32_t l_dds1_reg = 0UL;

	/* Update only when mHz value for DDS0 is given */
	if (dds0_hz >= 0.f) {
		l_dds0_freq_mHz = (uint32_t) (dds0_hz * 1000.f);
	}

	/* Update only when mHz value for DDS1 is given */
	if (dds1_hz >= 0.f) {
		l_dds1_freq_mHz = (uint32_t) (dds1_hz * 1000.f);
	}

	/* Set the phase between two starting oscillators */
	if (phase >= 0.f) {
		l_dds1_reg = (uint32_t) (0x40000000UL * (phase / 90.f));
	}

	/* Modifying the DDS registers */
	{
		irqflags_t flags = cpu_irq_save();

		/* Update only when mHz value for DDS0 is given */
		if (dds0_hz >= 0.f) {
			dds0_freq_mHz = l_dds0_freq_mHz;
		}

		/* Update only when mHz value for DDS1 is given */
		if (dds1_hz >= 0.f) {
			dds1_freq_mHz = l_dds1_freq_mHz;
		}

		/* Set the phase between two starting oscillators */
		if (phase >= 0.f) {
			dds0_reg = 0UL;
			dds1_reg = l_dds1_reg;
		}

		cpu_irq_restore(flags);
	}

	/* Calculate new increment values */
	sched_push(task_dac, SCHED_ENTRY_CB_TYPE__LISTTIME, 10, true, false, false);
}

void errorBeep_enable(bool enable)
{
	/* atomic */
	g_errorBeep_enable = enable;

	save_globals(EEPROM_SAVE_BF__BEEP);
}

void env_temp(float temp)
{
	irqflags_t flags = cpu_irq_save();
	int16_t l_env_temp_delta_100	= g_env_temp_delta_100;
	int16_t l_env_temp_deg_100		= g_env_temp_deg_100;
	cpu_irq_restore(flags);

	l_env_temp_delta_100 = (int16_t) ((l_env_temp_deg_100 + l_env_temp_delta_100) - (100.f * temp));

	flags = cpu_irq_save();
	g_env_temp_delta_100 = l_env_temp_delta_100;
	cpu_irq_restore(flags);

	save_globals(EEPROM_SAVE_BF__ENV);
}

void gsm_aprs_enable(bool enable)
{
	/* atomic */
	g_gsm_aprs_enable = enable;

	save_globals(EEPROM_SAVE_BF__GSM);
}

void gsm_pin_update(const char pin[])
{
	if (copyStr(g_gsm_login_pwd, (uint8_t) sizeof(g_gsm_login_pwd), pin)) {
		save_globals(EEPROM_SAVE_BF__GSM);
	}
}

void gsm_enable(bool enable)
{
	/* atomic */
	g_gsm_enable = enable;

	save_globals(EEPROM_SAVE_BF__GSM);

	/* Switch to the desired activation */
	serial_gsm_activation(enable);

	/* Activate GPRS link*/
	if (enable && g_gsm_aprs_enable) {
		serial_gsm_gprs_link_openClose(true);

		/* Inform about the serial communication success */
		{
			int len;

			/* LCD information */
			if (g_workmode != WORKMODE_RUN) {
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_LCD_READY);
				task_twi2_lcd_str(8, 12 * 10, g_prepare_buf);
			}

			/* USB information */
			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_SIM808_INFO_READY);
			udi_write_tx_buf(g_prepare_buf, len, false);
		}
	}
}

void keyBeep_enable(bool enable)
{
	/* atomic */
	g_keyBeep_enable = enable;

	save_globals(EEPROM_SAVE_BF__BEEP);
}

void pitchTone_mode(uint8_t mode)
{
	/* atomic */
	g_pitch_tone_mode = mode;

	save_globals(EEPROM_SAVE_BF__PITCHTONE);
}

void pocsag_chime_update(bool enable)
{
	/* atomic */
	g_ax_pocsag_chime_enable = enable;

	save_globals(EEPROM_SAVE_BF__AX);
}

void pocsag_message_send(const char msg[])
{
	if (g_ax_enable && g_ax_pocsag_individual_ric) {
		char* ptr = strchr(msg, 0x0d);
		uint8_t msgLen = ptr ?  (ptr - msg) : 0;

		/* FIFOCMD / FIFOSTAT */
		spi_ax_transport(false, "< a8 03 >");													// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

		/* Switch to POCSAG mode */
		spi_ax_init_POCSAG_Tx();

		/* Transmit POCSAG message */
		if (spi_ax_run_POCSAG_Tx_FIFO_Msg(g_ax_pocsag_individual_ric, ax_pocsag_analyze_msg_tgtFunc_get(msg, msgLen), msg, msgLen)) {
			/* Some configuration data error has happened */									// TODO: add config error message code here.
			nop();
		}

		do {
			/* FIFOSTAT */
			spi_ax_transport(false, "< 28 R1 >");
		} while (!(g_ax_spi_packet_buffer[0] & 0x01));

		do {
			/* RADIOSTATE */
			spi_ax_transport(false, "< 1c R1 >");												// RD Address 0x1C: RADIOSTATE - IDLE
		} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
	}
}

void pocsag_ric_update(uint32_t ric)
{
	irqflags_t flags = cpu_irq_save();
	g_ax_pocsag_individual_ric = ric;
	cpu_irq_restore(flags);

	save_globals(EEPROM_SAVE_BF__AX);
}

void pocsag_send_skyper_activation(void)
{
	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	/* Switch to POCSAG mode */
	spi_ax_init_POCSAG_Tx();

	/* Activate Skyper */
	{
		uint8_t actBufLen = g_ax_pocsag_activation_code_len;
		char actBuf[++actBufLen];

		uint8_t activationLen = spi_ax_pocsag_skyper_RIC2ActivationString(actBuf, actBufLen, g_ax_pocsag_individual_ric);
		spi_ax_run_POCSAG_Tx_FIFO_Msg(g_ax_pocsag_individual_ric, AX_POCSAG_CW2_MODE2_ACTIVATION, actBuf, activationLen);
	}

	do {
		/* FIFOSTAT */
		spi_ax_transport(false, "< 28 R1 >");
	} while (!(g_ax_spi_packet_buffer[0] & 0x01));

	do {
		/* RADIOSTATE */
		spi_ax_transport(false, "< 1c R1 >");													// RD Address 0x1C: RADIOSTATE - IDLE
	} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
}

void qnh_setAuto(void)
{
	/* atomic */
	g_qnh_is_auto	= true;

	save_globals(EEPROM_SAVE_BF__ENV);
}

void qnh_setHeightM(int16_t heightM)
{
	irqflags_t flags = cpu_irq_save();
	g_qnh_is_auto	= false;
	g_qnh_height_m	= heightM;
	cpu_irq_restore(flags);

	save_globals(EEPROM_SAVE_BF__ENV);
}

void printStatusLines_bitfield(PRINT_STATUS_BF_ENUM_t bf)
{
	/* atomic */
	g_usb_cdc_printStatusLines_atxmega	= bf & PRINT_STATUS_LINES__ATXMEGA	?  true : false;
	g_usb_cdc_printStatusLines_sim808	= bf & PRINT_STATUS_LINES__SIM808	?  true : false;
	g_usb_cdc_printStatusLines_1pps		= bf & PRINT_STATUS_LINES__1PPS		?  true : false;

	save_globals(EEPROM_SAVE_BF__PRINT_STATUS);
}

void shutdown(bool doReset)
{
	/* Print the information */
	{
		task_twi2_lcd_cls();
		task_twi2_lcd_str(70, 5 * 10, strcpy_P(g_prepare_buf, PM_TWI1_SHUT_01));

		g_workmode = WORKMODE_END;
	}

	/* Shutdown SIM808 */
	//if (g_gsm_mode != OFF) {
	{
		serial_sim808_gsm_setFuncMode(C_SERIAL_SIM808_GSM_SETFUNC_OFF);
		serial_sim808_gsm_shutdown();
		serial_shutdown();
	}

	/* Terminate the USB connection */
	{
		stdio_usb_disable();
		udc_stop();

		/* Power reduction: disable power of the USB */
		PR_PRPF |= PR_USB_bm;
	}

	/* Reset the LCD */
	{
		/* Turn LED off */
		twi2_set_leds(0);

		/* Switch backlight off */
		twi2_set_ledbl(0, 0);

		/* Short high voltage of LCD */
		task_twi2_lcd_reset();
	}

	/* Power off subsystems */
	{
		PR_PRPF |= PR_TWI_bm;
		PR_PRPF |= PR_SPI_bm;
		PR_PRPF |= PR_AC_bm;
		PR_PRPF |= PR_ADC_bm;
		PR_PRPF |= PR_DAC_bm;
		PR_PRPF |= PR_DMA_bm;
		PR_PRPF |= PR_RTC_bm;
		PR_PRPF |= PR_EVSYS_bm;
	}

	if (doReset) {
		asm volatile(
			"jmp 0 \n\t"
			:
			:
			:
		);

	} else {
		/* Stop main loop */
		halt();
	}
}

void xoPwm_set(int32_t mode_pwm)
{
	int32_t l_xo_mode_pwm = INT16_MIN;

	if (mode_pwm >= 0) {
		/* Set the PWM value for the VCTCXO pull voltage (should be abt. 1.5 V) */
		l_xo_mode_pwm = ((mode_pwm << C_XO_VAL_INT_SHIFT) & C_XO_VAL_INT_MASK);
		tc_write_cc_buffer(&TCC0, TC_CCC, (uint16_t) (l_xo_mode_pwm >> C_XO_VAL_INT_SHIFT));

	} else {
		switch (mode_pwm) {
			case -2:
				/* Preset value */
				l_xo_mode_pwm = ((int32_t) (0.5f + g_pwm_vctcxo_cfg.period * C_VCTCXO_DEFAULT_VOLTS / C_VCTCXO_PWM_HI_VOLTS)) & UINT16_MAX;
				tc_write_cc_buffer(&TCC0, TC_CCC, (uint16_t)l_xo_mode_pwm);	// Preset value with no flags
				l_xo_mode_pwm <<= C_XO_VAL_INT_SHIFT;
			break;

			case -1:
				/* PLL mode - get current PWM value and set PLL bit */
				{
					irqflags_t flags = cpu_irq_save();
					l_xo_mode_pwm = g_xo_mode_pwm;
					cpu_irq_restore(flags);
				}
				l_xo_mode_pwm &= (C_XO_VAL_INT_MASK | C_XO_VAL_FRAC_MASK);	// Mask out all flags
				l_xo_mode_pwm |=  C_XO_BF_PLL;								// Flag: use PLL feature
			break;
		}
	}

	/* Write back global var */
	if (l_xo_mode_pwm != INT16_MIN) {
		irqflags_t flags = cpu_irq_save();
		g_xo_mode_pwm = l_xo_mode_pwm;
		cpu_irq_restore(flags);

		save_globals(EEPROM_SAVE_BF__VCTCXO);
	}
}

void halt(void)
{
	/* MAIN Loop Shutdown */
	/* atomic */
	{
		g_workmode = WORKMODE_END;
	}
}


static void calc_next_frame(dma_dac_buf_t buf[DAC_NR_OF_SAMPLES], uint32_t* dds0_reg_p, uint32_t* dds0_inc_p, uint32_t* dds1_reg_p, uint32_t* dds1_inc_p)
{
	/* Filling the DMA block for a dual connected DAC channel */
	for (uint8_t idx = 0; idx < DAC_NR_OF_SAMPLES; ++idx, *dds0_reg_p += *dds0_inc_p, *dds1_reg_p += *dds1_inc_p) {
		uint16_t dds0_phase = *dds0_reg_p >> 16;
		buf[idx].ch0 = get_interpolated_sine(dds0_phase);

		uint16_t dds1_phase = *dds1_reg_p >> 16;
		buf[idx].ch1 = get_interpolated_sine(dds1_phase);
	}
}


const char					PM_DEBUG_MAIN_1PPS_1[]				= "DEBUG_MAIN_1PPS: diff=%+04d, last_adjust=%d, inSpan=%d, ";
const char					PM_DEBUG_MAIN_1PPS_2[]				= "doUpdate=%d, outOfSync=%d\r\n";
PROGMEM_DECLARE(const char, PM_DEBUG_MAIN_1PPS_1[]);
PROGMEM_DECLARE(const char, PM_DEBUG_MAIN_1PPS_2[]);
static void isr_100ms_main_1pps(void)
{
	/* Correct 1PPS time - called each second once */

	/* Leave when no new event has arrived */
	if (!g_1pps_last_new) {
		return;

	} else {
		bool doUpdate				= false;
		bool inSpan					= false;
		int16_t l_1pps_last_diff	= 0;

		/* Timer has adjust to 1PPS */
		if (g_1pps_last_adjust) {
			g_1pps_last_adjust			= false;
			g_1pps_processed_lo			= g_1pps_last_lo;
			g_1pps_processed_hi			= g_1pps_last_hi;
			g_1pps_processed_outOfSync	= 0;
			inSpan						= true;

		} else {
			/* Normal operation */
			if (g_1pps_processed_lo || g_1pps_processed_hi) {
				/* Calculate diff-time */
				l_1pps_last_diff = (int16_t)g_1pps_last_lo - (int16_t)g_1pps_processed_lo;

				/* Border values */
				if (l_1pps_last_diff < (C_TCC1_BORDER_OFFSET - C_TCC1_PERIOD)) {
					l_1pps_last_diff += C_TCC1_PERIOD;
				} else if ((C_TCC1_PERIOD - C_TCC1_BORDER_OFFSET) < l_1pps_last_diff) {
					l_1pps_last_diff -= C_TCC1_PERIOD;
				}

				/* Window check */
				if ((-C_TCC1_SPAN_HALF <= l_1pps_last_diff) && (l_1pps_last_diff <= C_TCC1_SPAN_HALF)) {	// +/- 1.67 µs
					inSpan		= true;
					doUpdate	= true;
				}

				/* Move slowly to meridian */
				if ((0 <= g_1pps_processed_lo) && (g_1pps_processed_lo < C_TCC1_MEAN_OFFSET)) {									// To early
					l_1pps_last_diff -= (C_TCC1_MEAN_OFFSET - g_1pps_processed_lo) / 40;

				} else if ((C_TCC1_MEAN_OFFSET < g_1pps_processed_lo) && (g_1pps_processed_lo <= (C_TCC1_MEAN_OFFSET << 1))) {	// To late
					l_1pps_last_diff += (g_1pps_processed_lo - C_TCC1_MEAN_OFFSET) / 40;
				}

				if (inSpan) {
					g_1pps_processed_outOfSync = 0;
				} else {
					/* Re-align meridian */
					if (g_1pps_processed_outOfSync > 10) {
						doUpdate			= true;
						l_1pps_last_diff	= 0;
					}

					if (g_1pps_processed_outOfSync < 255) {
						g_1pps_processed_outOfSync++;
					}
				}

			} else {
				/* Preset with first event */
				doUpdate = true;
			}

			if (doUpdate) {
				g_1pps_processed_lo  = g_1pps_last_lo;
				g_1pps_processed_hi  = g_1pps_last_hi;
			} else {
				g_1pps_processed_hi += 1000;	// Advance 1000 ms further, keep lo value
			}
		}

		/* Adjust the signals */
		g_1pps_last_new			= false;
		g_1pps_proceeded_avail	= true;
		g_1pps_last_diff		= l_1pps_last_diff;
		g_1pps_last_inSpan		= inSpan;
		g_1pps_printtwi_avail	= true;
		g_1pps_printusb_avail	= true;

		/* Blink when new second starts */
		g_1pps_led				= inSpan ?  0x02 : 0x01;  // Green / Red

		/* Show PLL inter-calculation values and states */
		if (g_usb_cdc_printStatusLines_1pps) {
			char l_prepare_buf[C_TX_BUF_SIZE];

			int len = snprintf_P(l_prepare_buf, sizeof(l_prepare_buf), PM_DEBUG_MAIN_1PPS_1, l_1pps_last_diff, g_1pps_last_adjust, inSpan);
			udi_write_tx_buf(l_prepare_buf, min(len, sizeof(l_prepare_buf)), false);
			len = snprintf_P(l_prepare_buf, sizeof(l_prepare_buf), PM_DEBUG_MAIN_1PPS_2, doUpdate, g_1pps_processed_outOfSync);
			udi_write_tx_buf(l_prepare_buf, min(len, sizeof(l_prepare_buf)), false);
		}
	}
}


/* APRS alarming - detected activities */

uint16_t aprs_pos_delta_m(void)
{
	float				l_aprs_pos_anchor_lat;
	float				l_aprs_pos_anchor_lon;
	float				l_gns_lat;
	float				l_gns_lon;

	/* Get the current position */
	{
		irqflags_t flags = cpu_irq_save();
		l_aprs_pos_anchor_lat = g_aprs_pos_anchor_lat;
		l_aprs_pos_anchor_lon = g_aprs_pos_anchor_lon;
		l_gns_lat = g_gns_lat;
		l_gns_lon = g_gns_lon;
		cpu_irq_restore(flags);
	}

	/* Initial setting */
	if (!l_aprs_pos_anchor_lat && !l_aprs_pos_anchor_lon) {
		aprs_pos_anchor();
		return 0;
	}

	/* Simplified plane calculations - 1deg = 60nm; 1nm = 1852m */
	float l_dist_m_x	= 1852.f * 60.f * cos(l_gns_lat * M_PI / 180.f) *	(l_gns_lon >= l_aprs_pos_anchor_lon ?  (l_gns_lon - l_aprs_pos_anchor_lon) : (l_aprs_pos_anchor_lon - l_gns_lon));
	float l_dist_m_y	= 1852.f * 60.f * 									(l_gns_lat >= l_aprs_pos_anchor_lat ?  (l_gns_lat - l_aprs_pos_anchor_lat) : (l_aprs_pos_anchor_lat - l_gns_lat));
	float l_dist_m = 0.5f + sqrtf(l_dist_m_x * l_dist_m_x + l_dist_m_y * l_dist_m_y);

	return (uint16_t)l_dist_m;
}

void aprs_pos_anchor(void)
{
	irqflags_t flags = cpu_irq_save();
	g_aprs_pos_anchor_lat = g_gns_lat;
	g_aprs_pos_anchor_lon = g_gns_lon;
	cpu_irq_restore(flags);
}

uint16_t aprs_gyro_total_dps_1000(void)
{
	volatile float l_twi1_gyro_1_gyro_x_mdps;
	volatile float l_twi1_gyro_1_gyro_y_mdps;
	volatile float l_twi1_gyro_1_gyro_z_mdps;

	/* Get the current position */
	{
		irqflags_t flags = cpu_irq_save();
		l_twi1_gyro_1_gyro_x_mdps = g_twi1_gyro_1_gyro_x_mdps;
		l_twi1_gyro_1_gyro_y_mdps = g_twi1_gyro_1_gyro_y_mdps;
		l_twi1_gyro_1_gyro_z_mdps = g_twi1_gyro_1_gyro_z_mdps;
		cpu_irq_restore(flags);
	}

	float l_gyro_total_dps_1000 = sqrt(l_twi1_gyro_1_gyro_x_mdps * l_twi1_gyro_1_gyro_x_mdps + l_twi1_gyro_1_gyro_y_mdps * l_twi1_gyro_1_gyro_y_mdps + l_twi1_gyro_1_gyro_z_mdps * l_twi1_gyro_1_gyro_z_mdps);
	return (uint16_t) (0.5f + l_gyro_total_dps_1000);
}

uint16_t aprs_accel_xy_delta_g_1000(void)
{
	static float		s_twi1_gyro_1_accel_x_mg = 0.f;
	static float		s_twi1_gyro_1_accel_y_mg = 0.f;
	volatile float		l_twi1_gyro_1_accel_x_mg;
	volatile float		l_twi1_gyro_1_accel_y_mg;

	/* Initial setting */
	if (!s_twi1_gyro_1_accel_x_mg && !s_twi1_gyro_1_accel_y_mg) {
		irqflags_t flags = cpu_irq_save();
		s_twi1_gyro_1_accel_x_mg = g_twi1_gyro_1_accel_x_mg;
		s_twi1_gyro_1_accel_y_mg = g_twi1_gyro_1_accel_y_mg;
		cpu_irq_restore(flags);
		return 0;
	}

	/* Get the current position */
	{
		irqflags_t flags = cpu_irq_save();
		l_twi1_gyro_1_accel_x_mg = g_twi1_gyro_1_accel_x_mg;
		l_twi1_gyro_1_accel_y_mg = g_twi1_gyro_1_accel_y_mg;
		cpu_irq_restore(flags);
	}

	/* Delta and geometric additions */
	float l_accel_delta_x = l_twi1_gyro_1_accel_x_mg - s_twi1_gyro_1_accel_x_mg;
	float l_accel_delta_y = l_twi1_gyro_1_accel_y_mg - s_twi1_gyro_1_accel_y_mg;
	float l_accel_xy_delta_g_1000 = sqrt(l_accel_delta_x * l_accel_delta_x + l_accel_delta_y * l_accel_delta_y);

	/* Adjust to this values */
	s_twi1_gyro_1_accel_x_mg = l_twi1_gyro_1_accel_x_mg;
	s_twi1_gyro_1_accel_y_mg = l_twi1_gyro_1_accel_y_mg;

	return (uint16_t) (0.5f + l_accel_xy_delta_g_1000);
}

uint16_t aprs_mag_delta_nT(void)
{
	static int32_t		s_twi1_gyro_2_mag_x_nT	= 0;
	static int32_t		s_twi1_gyro_2_mag_y_nT	= 0;
	static int32_t		s_twi1_gyro_2_mag_z_nT	= 0;
	volatile int32_t	l_twi1_gyro_2_mag_x_nT;
	volatile int32_t	l_twi1_gyro_2_mag_y_nT;
	volatile int32_t	l_twi1_gyro_2_mag_z_nT;

	/* Initial setting */
	if (!s_twi1_gyro_2_mag_x_nT && !s_twi1_gyro_2_mag_y_nT && !s_twi1_gyro_2_mag_z_nT) {
		irqflags_t flags = cpu_irq_save();
		s_twi1_gyro_2_mag_x_nT = g_twi1_gyro_2_mag_x_nT;
		s_twi1_gyro_2_mag_y_nT = g_twi1_gyro_2_mag_y_nT;
		s_twi1_gyro_2_mag_z_nT = g_twi1_gyro_2_mag_z_nT;
		cpu_irq_restore(flags);
		return 0;
	}

	/* Get the current position */
	{
		irqflags_t flags = cpu_irq_save();
		l_twi1_gyro_2_mag_x_nT = g_twi1_gyro_2_mag_x_nT;
		l_twi1_gyro_2_mag_y_nT = g_twi1_gyro_2_mag_y_nT;
		l_twi1_gyro_2_mag_z_nT = g_twi1_gyro_2_mag_z_nT;
		cpu_irq_restore(flags);
	}

	/* Delta and geometric additions */
	float l_mag_delta_x = l_twi1_gyro_2_mag_x_nT - s_twi1_gyro_2_mag_x_nT;
	float l_mag_delta_y = l_twi1_gyro_2_mag_y_nT - s_twi1_gyro_2_mag_y_nT;
	float l_mag_delta_z = l_twi1_gyro_2_mag_z_nT - s_twi1_gyro_2_mag_z_nT;
	float l_mag_delta_total = sqrtf(l_mag_delta_x * l_mag_delta_x + l_mag_delta_y * l_mag_delta_y + l_mag_delta_z * l_mag_delta_z);

	/* Adjust to this values */
	s_twi1_gyro_2_mag_x_nT = l_twi1_gyro_2_mag_x_nT;
	s_twi1_gyro_2_mag_y_nT = l_twi1_gyro_2_mag_y_nT;
	s_twi1_gyro_2_mag_z_nT = l_twi1_gyro_2_mag_z_nT;

	return (uint16_t) (0.5f + l_mag_delta_total);
}

void aprs_message_send(const char* msg, uint16_t content_message_len)
{
	/* GSM DPRS transportation */
	if (g_gsm_enable && g_gsm_aprs_enable) {
		char l_content_hdr[C_USART1_TX_BUF_LEN];
		uint16_t content_hdr_len = (uint16_t) snprintf_P(l_content_hdr, sizeof(l_content_hdr), PM_APRS_TX_LOGIN, g_aprs_login_user, g_aprs_login_pwd, APPLICATION_NAME, APPLICATION_VERSION);
		uint16_t len = content_hdr_len + 2 + content_message_len;

		/* Transport message */
		{
			char		l_msg_buf[C_USART1_TX_BUF_LEN];
			uint16_t	l_msg_buf_len;

			/* Line 1 */
			l_msg_buf_len = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_HTTP_L1);
			serial_sim808_send(l_msg_buf, l_msg_buf_len, true);

			/* Line 2 */
			l_msg_buf_len = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_HTTP_L2, len);
			serial_sim808_send(l_msg_buf, l_msg_buf_len, true);

			/* Line 3 */
			l_msg_buf_len = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_HTTP_L3);
			serial_sim808_send(l_msg_buf, l_msg_buf_len, true);

			/* Line 4 */
			l_msg_buf_len = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_HTTP_L4);
			serial_sim808_send(l_msg_buf, l_msg_buf_len, true);

			/* Content header - authentication */
			serial_sim808_send(l_content_hdr, content_hdr_len, true);

			/* Content message */
			serial_sim808_send(msg, content_message_len, true);

			/* Ending line */
			l_msg_buf_len = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_MSGEND);
			serial_sim808_send(l_msg_buf, l_msg_buf_len, true);
		}
	}
}


/* Simple scheduler concept */

bool sched_getLock(volatile uint8_t* lockVar)
{
	bool status = false;

	/* IRQ secured access */	
	{
		irqflags_t flags = cpu_irq_save();
		barrier();
		if (!*lockVar) {	// No use before
			++*lockVar;
			barrier();
			status = true;
		}
		cpu_irq_restore(flags);
	}
	return status;
}

void sched_freeLock(volatile uint8_t* lockVar)
{
	irqflags_t flags	= cpu_irq_save();
	*lockVar			= 0;
	cpu_irq_restore(flags);
}

void sched_set_alarm(uint32_t alarmTime)
{
	/* Set next time to wake up */
	uint32_t checkTime = rtc_get_time() + 2;
	if (alarmTime < checkTime) {
		alarmTime = checkTime;
	}
	rtc_set_alarm(alarmTime);
}

void sched_doSleep(void)
{
#if 1
	/* Power down until IRQ or event calls in */
	sleepmgr_enter_sleep();
#endif
}

void sched_push(void* cb, SCHED_ENTRY_CB_TYPE_ENUM_t cbType, uint32_t wakeTime, bool isDelay, bool isIntDis, bool isSync)
{
	const uint32_t pushTime = rtc_get_time();
	uint32_t alarmTime = 0UL;
	uint32_t sfCb = (isIntDis ?  0x8F000000UL : 0x0F00000UL) | (isSync ?  0x40000000UL : 0x0000000UL) | ((uint32_t)cbType << 24) | ((uint32_t)(uint16_t)cb);

	if (isDelay) {
		/* Sanity checks */
		if (wakeTime < 2) {
			wakeTime = 2;														// Min value to use to work properly
		} else if (wakeTime > 30000U) {
			wakeTime = ((30000U << 10) / 1024);									// Max value 30 sec
		} else {
			wakeTime = ((wakeTime << 10) / 1024);								// Time correction for 1024 ticks per second
		}

		/* Absolute time ticks */
		wakeTime += pushTime;
	}

	/* Lock access to the scheduler entries */
	if (!sched_getLock(&g_sched_lock)) {
		/* Push entry to the stack due to blocked access */
		if (!fifo_is_full(&g_fifo_sched_desc)) {
			irqflags_t flags = cpu_irq_save();
			fifo_push_uint32(&g_fifo_sched_desc, wakeTime);
			fifo_push_uint32(&g_fifo_sched_desc, sfCb);
			cpu_irq_restore(flags);
		}
		return;
	}

// -- single entry section for g_sched_data[] / g_sched_sort[]
// v

	/* Get next free slot */
	bool dataEntryStored = false;
	uint8_t slot = 0;

	for (int idx = 0; idx < C_SCH_SLOT_CNT; ++idx) {
		if (!(g_sched_data[idx].occupied)) {
			g_sched_data[idx].occupied	= true;
			g_sched_data[idx].callback	= cb;
			g_sched_data[idx].cbType	= cbType & 0x03;
			g_sched_data[idx].isIntDis	= (isIntDis ?  1 : 0);
			g_sched_data[idx].isSync	= (isSync	?  1 : 0);
			g_sched_data[idx].wakeTime	= wakeTime;
			slot = idx + 1;
			dataEntryStored = true;
			break;
		}
	}

	if (!dataEntryStored) {
		goto sched_push_out;  // should not happen
	}

	/* Bind to sort list */
	for (int pos = 0; pos < C_SCH_SLOT_CNT; ++pos) {
		uint8_t idx = g_sched_sort[pos];
		if (!idx) {
			/* Fill in after last entry */
			g_sched_sort[pos] = slot;
			break;
		}
		--idx;

		/* Place new item in front of that entry */
		if (g_sched_data[idx].occupied && (wakeTime < g_sched_data[idx].wakeTime)) {
			/* Move all entries of position list one up to give space for our new entry */
			for (int mvidx = C_SCH_SLOT_CNT - 2; pos <= mvidx; --mvidx) {
				g_sched_sort[mvidx + 1] = g_sched_sort[mvidx];
			}

			/* Fill in new item */
			g_sched_sort[pos] = slot;
			break;
		}
	}

	/* Get next time for wake-up */
	alarmTime = g_sched_data[g_sched_sort[0] - 1].wakeTime;

sched_push_out:

// ^
// -- single entry section for g_sched_data[] / g_sched_sort[]

	/* Release lock */
	sched_freeLock(&g_sched_lock);

	/* Pop back all FIFO entries */
	while (!fifo_is_empty(&g_fifo_sched_desc)) {
		uint32_t sfCb = 0UL;
		wakeTime = 0UL;

		/* Get next entries */
		{
			irqflags_t flags = cpu_irq_save();
			fifo_pull_uint32(&g_fifo_sched_desc, &wakeTime);
			fifo_pull_uint32(&g_fifo_sched_desc, &sfCb);
			cpu_irq_restore(flags);
		}

		/* FIFO check */
		if ((sfCb & 0x0f000000UL) != 0x0f000000UL) {
			/* Signature not found - clear all entries to synchronize again */
			irqflags_t flags = cpu_irq_save();
			while (!fifo_is_empty(&g_fifo_sched_desc)) {
				fifo_pull_uint32(&g_fifo_sched_desc, &sfCb);
			}
			sfCb = 0UL;
			cpu_irq_restore(flags);
		}

		if ((sfCb & 0x0000ffffUL) && wakeTime) {
			sched_push((void*)(uint16_t)(sfCb & 0x0000ffffUL), ((sfCb & 0x00030000) >> 24), wakeTime, false, (sfCb & 0x80000000UL ?  true : false), (sfCb & 0x40000000UL ?  true : false));
		}
	}

	/* Set next time to wake up */
	if (alarmTime) {
		sched_set_alarm(alarmTime);
	}
}

void sched_pop(uint32_t wakeNow)
{
	uint32_t alarmTime = 0UL;

	if (!sched_getLock(&g_sched_lock)) {
		/* Locked by another one, sched_pop() later */
		g_sched_pop_again = true;
		return;
	}

// -- single entry section for g_sched_data[] / g_sched_sort[]
// v

	uint8_t idx = g_sched_sort[0];
	if (!idx) {
		/* No jobs at the scheduler */
		goto sched_pop_out;
	}
	if (!(g_sched_data[idx - 1].occupied)) {
		/* Sanity failed */
		goto sched_pop_out;
	}

	/* Process each entry until the current timestamp */
	alarmTime = g_sched_data[idx - 1].wakeTime;
	while (alarmTime <= rtc_get_time()) {
		/* Get callback */
		void* cbVal		= g_sched_data[idx - 1].callback;
		uint8_t cbType	= g_sched_data[idx - 1].cbType;
		bool isIntDis	= g_sched_data[idx - 1].isIntDis;
		bool isSync		= g_sched_data[idx - 1].isSync;

		/* Free entry */
		g_sched_data[idx - 1].occupied = false;

		/* Move all items down by one */
		for (int mvidx = 0; mvidx < (C_SCH_SLOT_CNT - 1); ++mvidx) {
			g_sched_sort[mvidx] = g_sched_sort[mvidx + 1];
		}
		g_sched_sort[C_SCH_SLOT_CNT - 1] = 0;	// clear top-most index

		/* Call the CB function */
		if (cbVal) {
			irqflags_t flags = cpu_irq_save();
			if (isIntDis) {
				cpu_irq_disable();
			} else {
				cpu_irq_enable();
			}
			switch (cbType) {
				case SCHED_ENTRY_CB_TYPE__LISTTIME:
				{
					sched_callback_type0 cb = cbVal;
					cb(alarmTime);
				}
				break;

				case SCHED_ENTRY_CB_TYPE__LISTTIME_ISSYNC:
				{
					sched_callback_type1 cb = cbVal;
					cb(alarmTime, isSync);
				}
				break;
			}
			cpu_irq_restore(flags);
		}

		/* Get the next alarm time */
		{
			idx = g_sched_sort[0];
			if (!idx) {
				/* No jobs at the scheduler */
				goto sched_pop_out;
			}
			if (!(g_sched_data[idx - 1].occupied)) {
				/* Sanity failed */
				goto sched_pop_out;
			}
			alarmTime = g_sched_data[idx - 1].wakeTime;
		}
	}

sched_pop_out:

// ^
// -- single entry section for g_sched_data[] / g_sched_sort[]

	sched_freeLock(&g_sched_lock);

	/* Set next time to wake up */
	if (alarmTime) {
		sched_set_alarm(alarmTime);
	}

	/* Restart due to another guest rang the door bell */
	if (g_sched_pop_again) {
		g_sched_pop_again = false;
		sched_pop(wakeNow);
	}
}

void yield_ms(uint16_t ms)
{
	irqflags_t flags = cpu_irq_save();
	cpu_irq_enable();

	/* A yield job is on the scheduler */
	g_sched_yield = true;

	/* Push ourself to the scheduler */
	sched_push(yield_ms_cb, SCHED_ENTRY_CB_TYPE__LISTTIME, ms, true, false, false);

	/* Continued sleep until our callback is done */
	do {
		/* Enter sleep mode */
		sched_doSleep();

		/* Woke up for any reason - check if we were called */
	} while (g_sched_yield);

	cpu_irq_restore(flags);
}

void yield_ms_cb(uint32_t listTime)
{
	g_sched_yield = false;
}


/* INIT section */

static void interrupt_init(void)
{
	PORTR_DIRCLR	= (1 << 0);																	// Pin R0 direction is cleared = INPUT
	PORTR_PIN0CTRL	= PORT_ISC_RISING_gc;														// GNSS 1PPS has a rising edge signal

	PORTR_INT0MASK	= (1 << 0);																	// Port R0	--> INT0
	PORTR_INT1MASK	= 0;																		// (none)	--> INT1

	PORTR_INTCTRL	= PORT_INT1LVL_OFF_gc | PORT_INT0LVL_HI_gc;									// Enable interrupt for port R0 with high level
}

ISR(PORTR_INT0_vect)
{
	if (g_1pps_phased_cntr != C_TCC1_CLOCKSETTING_AFTER_SECS) {
		{
			irqflags_t flags = cpu_irq_save();

			/* Take the time */
			g_1pps_last_lo	= tc_read_count(&TCC1);
			g_1pps_last_hi	= g_milliseconds_cnt64;

			/* Correction for late ISR */
			if (g_1pps_last_hi % 1000 == 999U) {
				g_1pps_last_hi++;
			}

			cpu_irq_restore(flags);
		}
		g_1pps_last_new	= true;

	} else {
		{
			irqflags_t flags = cpu_irq_save();

			/* Phased-in time correction */
			g_1pps_last_lo = (uint16_t) (C_TCC1_CLOCKSETTING_OFFSET + C_TCC1_MEAN_OFFSET);		// Time difference between 1PPS interrupt to this position in code
			tc_write_count(&TCC1, g_1pps_last_lo);
			tc_update(&TCC1);

			/* Rounding up to next full second */
			g_1pps_last_hi			+= 1000U;
			g_1pps_last_hi			-= g_1pps_last_hi % 1000U;
			g_milliseconds_cnt64	 = g_1pps_last_hi;

			cpu_irq_restore(flags);
		}

		/* Step ahead avoiding to trap in here, again */
		++g_1pps_phased_cntr;

		/* Inform about the adjustment */
		g_1pps_last_adjust = true;
	}

	/* TWI2 - LCD line scheduler synchronization */
	g_1pps_twi_new	= true;
}


static void evsys_init(void)
{
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);

	/* ADC - event channels 0, 1, 2, 3 */
	EVSYS.CH0MUX  = EVSYS_CHMUX_TCC0_CCC_gc;													// TCC0 CC-C goes to EVSYS CH0
	EVSYS.CH0CTRL = EVSYS_DIGFILT_1SAMPLE_gc;													// EVSYS CH0 no digital filtering
	EVSYS.CH1MUX  = EVSYS_CHMUX_TCC0_CCC_gc;													// TCC0 CC-C goes to EVSYS CH1
	EVSYS.CH1CTRL = EVSYS_DIGFILT_1SAMPLE_gc;													// EVSYS CH1 no digital filtering
	EVSYS.CH2MUX  = EVSYS_CHMUX_TCC0_CCC_gc;													// TCC0 CC-C goes to EVSYS CH2
	EVSYS.CH2CTRL = EVSYS_DIGFILT_1SAMPLE_gc;													// EVSYS CH2 no digital filtering
	EVSYS.CH3MUX  = EVSYS_CHMUX_TCC0_CCC_gc;													// TCC0 CC-C goes to EVSYS CH3
	EVSYS.CH3CTRL = EVSYS_DIGFILT_1SAMPLE_gc;													// EVSYS CH3 no digital filtering

	/* DAC - event 4 */
	EVSYS.CH4MUX  = EVSYS_CHMUX_TCE1_OVF_gc;													// TCE1 overflow goes to EVSYS CH4
	EVSYS.CH4CTRL = EVSYS_DIGFILT_1SAMPLE_gc;													// EVSYS CH4 no digital filtering
}


static void tc_init(void)
{
	/* Get the PWM value */
	int32_t l_xo_mode_pwm;
	{
		irqflags_t flags = cpu_irq_save();
		l_xo_mode_pwm = g_xo_mode_pwm;
		cpu_irq_restore(flags);
	}

	/* TCC0: VCTCXO PWM signal generation and ADCA & ADCB */
	pwm_init(&g_pwm_vctcxo_cfg, PWM_TCC0, PWM_CH_C, 2048);										// Init PWM structure and enable timer - running with 2048 Hz --> 2 Hz averaged data
	pwm_start(&g_pwm_vctcxo_cfg, 45);															// Start PWM here. Percentage with 1% granularity is to coarse, use driver access instead
	tc_write_cc_buffer(&TCC0, TC_CCC, (uint16_t) ((l_xo_mode_pwm & C_XO_VAL_INT_MASK) >> C_XO_VAL_INT_SHIFT));	// Setting the PWM value

	/* TCC1: Free running clock for 30 MHz PLL */
	g_pwm_ctr_pll_cfg.tc = &TCC1;
	tc_enable(&TCC1);																			// Enable TCC1 and power up
	tc_set_wgm(&TCC1, TC_WG_NORMAL);															// Normal counting up
	tc_write_clock_source(&TCC1, PWM_CLK_OFF);													// Disable counter until all is ready
	pwm_set_frequency(&g_pwm_ctr_pll_cfg, 1000);												// Prepare structure for 1 ms overflow frequency
	tc_write_period(&TCC1, g_pwm_ctr_pll_cfg.period);											// Calculate period count
	tc_write_period_buffer(&TCC1, C_TCC1_PERIOD - 1);											// Overflows every 1 ms

	/* TCE1: DAC clock */
	tc_enable(&TCE1);
	tc_set_wgm(&TCE1, TC_WG_NORMAL);															// Internal clock for DAC convertion
	tc_write_period(&TCE1, (sysclk_get_per_hz() / DAC_RATE_OF_CONV) - 1);						// DAC clock of 100 kHz for DDS (Direct Digital Synthesis)
}

static void tc_start(void)
{
	/* ADC clock */
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV1_gc);											// VCTCXO PWM start, output still is Z-state
	tc_set_overflow_interrupt_callback(&TCC0, isr_tcc0_ovfl);
	tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_LO);

	/* Free running clock for 30 MHz PLL */
	tc_write_clock_source(&TCC1, TC_CLKSEL_DIV1_gc);
	tc_set_overflow_interrupt_callback(&TCC1, isr_tcc1_ovfl);
	tc_set_overflow_interrupt_level(&TCC1, TC_INT_LVL_HI);

//	tc_write_clock_source(&TCD0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCD1, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCE0, TC_CLKSEL_DIV1_gc);

	/* DAC clock */
	tc_write_clock_source(&TCE1, TC_CLKSEL_DIV1_gc);

//	tc_write_clock_source(&TCF0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCF1, TC_CLKSEL_DIV1_gc);
}

//ISR(TCC0_OVF_vect)
void isr_tcc0_ovfl(void)
{	// This ISR is called 2048 per second
//	static uint32_t	last_10ms  = 0UL;
	static uint32_t	last_100ms = 0UL;
	static uint32_t	last_500ms = 0UL;
	
	/* Time downscaling */
	uint32_t now = tcc1_get_time();

	/* Clear IF bit to allow interrupt enabled section */
	TCC0_INTFLAGS = TC0_OVFIF_bm;

	if (g_workmode == WORKMODE_RUN) {
#if 0
		/* Group, which needs to be called about 100x per second */
		if (((now - last_10ms) >= 10) || (now < last_10ms)) {
			last_10ms = now;
			isr_10ms();
			return;
		}
#endif

		/* Group, which needs to be called about 10x per second */
		if (((now - last_100ms) >= 102) || (now < last_100ms)) {
			last_100ms = now;
			isr_100ms();
			return;
		}

		/* Group, which needs to be called about 2x per second */
		if (((now - last_500ms) >= 500) || (now < last_500ms)) {
			last_500ms = now;
			isr_500ms();
			return;
		}

		isr_sparetime();
	}
}

uint32_t tcc1_get_time(void)
{
	static uint64_t	last = 0ULL;
	uint64_t		now;
	irqflags_t		flags;
	uint8_t			sureCtr = 0;

	do {
		flags = cpu_irq_save();
		now = g_milliseconds_cnt64;
		cpu_irq_restore(flags);

		/* Avoid to return wrong value */
		if (((now >= last) && ((now - last) < 0x00100000)) || !++sureCtr) {
			last = now;
			return (uint32_t)now;
		}
	} while(true);
}

#if 0
static void isr_10ms(void)
{
	isr_10ms_twi1_onboard(now);
}
#endif

static void isr_100ms(void)
{
	isr_100ms_main_1pps();
	isr_100ms_twi1_onboard();
}

static void isr_500ms(void)
{
	isr_500ms_twi1_onboard();

	/* CPU ADC values */
	sched_push(task_adc, SCHED_ENTRY_CB_TYPE__LISTTIME, 100, true, false, false);

	/* CPU DAC reconfiguration */
	sched_push(task_dac, SCHED_ENTRY_CB_TYPE__LISTTIME, 100, true, false, false);

	/* Kick RTC32 */
	rtc_set_alarm(rtc_get_time() + 2);
}

static void isr_sparetime(void)
{
	isr_sparetime_twi1_onboard();
}

//ISR(TCC1_OVF_vect)
void isr_tcc1_ovfl(void)
{	// This ISR is called every millisecond for tracking the PLL
	++g_milliseconds_cnt64;
}

static void rtc_start(void)
{
	rtc_set_callback(isr_rtc_alarm);
}

void isr_rtc_alarm(uint32_t rtc_time)
{	// Alarm call-back with the current time
	cpu_irq_enable();
	sched_pop(rtc_time);
}


static void adc_init(void)
{
	/* Disable digital circuits of ADC pins */
	PORTA_PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA_PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA_PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA_PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA_PIN4CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA_PIN5CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTB_PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTB_PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;

	/* Prepare the structures */
	adc_read_configuration(&ADC_VCTCXO_5V0_VBAT,							&g_adc_a_conf);
	adcch_read_configuration(&ADC_VCTCXO_5V0_VBAT, ADC_VCTCXO_5V0_VBAT_CH,	&g_adcch_vctcxo_5v0_vbat_conf);
	adcch_read_configuration(&ADC_IO_ADC4, ADC_IO_ADC4_CH,					&g_adcch_io_adc4_conf);
	adcch_read_configuration(&ADC_IO_ADC5, ADC_IO_ADC5_CH,					&g_adcch_io_adc5_conf);
	adcch_read_configuration(&ADC_SILENCE, ADC_SILENCE_CH,					&g_adcch_silence_conf);
	adc_read_configuration(&ADC_TEMP,										&g_adc_b_conf);
	adcch_read_configuration(&ADC_TEMP, ADC_TEMP_CH,						&g_adcch_temp_conf);

	/* ADC-clock request */
	adc_set_clock_rate(&g_adc_a_conf, 1000000UL);												// External signals: 100kHz .. 2000kHz
	adc_set_clock_rate(&g_adc_b_conf,  115000UL);												// Internal signals: 100kHz ..  125kHz

	/* Enable internal ADC-B input for temperature measurement */
	adc_disable_internal_input(&g_adc_a_conf,		ADC_INT_TEMPSENSE				   );
	adc_enable_internal_input (&g_adc_a_conf,							ADC_INT_BANDGAP);
	adc_enable_internal_input (&g_adc_b_conf,		ADC_INT_TEMPSENSE | ADC_INT_BANDGAP);

	/* Current limitation */
	adc_set_current_limit(&g_adc_a_conf,			ADC_CURRENT_LIMIT_MED);
	adc_set_current_limit(&g_adc_b_conf,			ADC_CURRENT_LIMIT_HIGH);

	/* ADC impedance */
	adc_set_gain_impedance_mode(&g_adc_a_conf,		ADC_GAIN_HIGHIMPEDANCE);
	adc_set_gain_impedance_mode(&g_adc_b_conf,		ADC_GAIN_HIGHIMPEDANCE);

	/* PIN assignment */
	adcch_set_input(&g_adcch_vctcxo_5v0_vbat_conf,	ADCCH_POS_PIN1,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_io_adc4_conf,			ADCCH_POS_PIN4,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_io_adc5_conf,			ADCCH_POS_PIN5,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_silence_conf,			ADCCH_POS_BANDGAP,		ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_temp_conf,				ADCCH_POS_TEMPSENSE,	ADCCH_NEG_NONE, 1);

	/* Convertion and reference */
	adc_set_conversion_parameters(&g_adc_a_conf,	ADC_SIGN_OFF, ADC_RES_12, ADC_REF_AREFA);	// ADC-A: ADC0 (Pin 62 3V0 as reference pin)
	adc_set_conversion_parameters(&g_adc_b_conf,	ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);	// ADC-B: bandgap diode (1V0)

	/* PIN scan on ADC-channel 0 */
	adcch_set_pin_scan(&g_adcch_vctcxo_5v0_vbat_conf, 0, 2);									// ADC-A: scan between ADC1 .. ADC3

	/* Trigger */
	adc_set_conversion_trigger(&g_adc_a_conf, ADC_TRIG_EVENT_SINGLE, 4, 1);
	adc_set_conversion_trigger(&g_adc_b_conf, ADC_TRIG_EVENT_SINGLE, 1, 0);

	/* Interrupt service routine */
	adc_set_callback(&ADCA, isr_adc_a);
	adc_set_callback(&ADCB, isr_adc_b);

	/* Interrupt type */
	adcch_set_interrupt_mode(&g_adcch_vctcxo_5v0_vbat_conf,	ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_io_adc4_conf,			ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_io_adc5_conf,			ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_silence_conf,			ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_temp_conf,			ADCCH_MODE_COMPLETE);

	/* Interrupt enable */
	adcch_enable_interrupt(&g_adcch_vctcxo_5v0_vbat_conf);
	adcch_enable_interrupt(&g_adcch_io_adc4_conf);
	adcch_enable_interrupt(&g_adcch_io_adc5_conf);
	adcch_enable_interrupt(&g_adcch_silence_conf);
	adcch_enable_interrupt(&g_adcch_temp_conf);

	/* Execute the new settings */
	adc_write_configuration(&ADCA,											&g_adc_a_conf);
	adcch_write_configuration(&ADC_VCTCXO_5V0_VBAT,	ADC_VCTCXO_5V0_VBAT_CH,	&g_adcch_vctcxo_5v0_vbat_conf);
	adcch_write_configuration(&ADC_IO_ADC4,			ADC_IO_ADC4_CH,			&g_adcch_io_adc4_conf);
	adcch_write_configuration(&ADC_IO_ADC5,			ADC_IO_ADC5_CH,			&g_adcch_io_adc5_conf);
	adcch_write_configuration(&ADC_SILENCE,			ADC_SILENCE_CH,			&g_adcch_silence_conf);
	adc_write_configuration(&ADCB,											&g_adc_b_conf);
	adcch_write_configuration(&ADC_TEMP,			ADC_TEMP_CH,			&g_adcch_temp_conf);

	/* Get production signature for calibration */
	ADCA_CAL = adc_get_calibration_data(ADC_CAL_ADCA);
	ADCB_CAL = adc_get_calibration_data(ADC_CAL_ADCB);
}

static void adc_start(void)
{
	adc_enable(&ADCA);
	adc_enable(&ADCB);
}

static void adc_stop(void)
{
	adc_disable(&ADCA);
	adc_disable(&ADCB);
}

void isr_adc_a(ADC_t* adc, uint8_t ch_mask, adc_result_t res)
{
	uint8_t scan_ofs_next = (ADCA_CH0_SCAN >> 4);
	int16_t val = res - C_ADC_0V0_DELTA;

	if ((ch_mask & ADC_VCTCXO_5V0_VBAT_CH)) {
		switch (scan_ofs_next) {
			case ADC_CH0_SCAN_5V0:
				g_adc_vctcxo_sum += val;
				if (++g_adc_vctcxo_cnt >= C_ADC_SUM_CNT) {
					g_adc_vctcxo_cur = g_adc_vctcxo_sum;
					g_adc_vctcxo_sum = g_adc_vctcxo_cnt = 0;
				}
			break;

			case ADC_CH0_SCAN_VBAT:
				g_adc_5v0_sum += val;
				if (++g_adc_5v0_cnt >= C_ADC_SUM_CNT) {
					g_adc_5v0_cur = g_adc_5v0_sum;
					g_adc_5v0_sum = g_adc_5v0_cnt = 0;
				}
			break;

			case ADC_CH0_SCAN_VCTCXO:
				g_adc_vbat_sum += val;
				if (++g_adc_vbat_cnt >= C_ADC_SUM_CNT) {
					g_adc_vbat_cur = g_adc_vbat_sum;
					g_adc_vbat_sum = g_adc_vbat_cnt = 0;
				}
			break;
		}

	} else if (ch_mask & ADC_IO_ADC4_CH) {
		g_adc_io_adc4_sum += val;
		if (++g_adc_io_adc4_cnt >= C_ADC_SUM_CNT) {
			g_adc_io_adc4_cur = g_adc_io_adc4_sum;
			g_adc_io_adc4_sum = g_adc_io_adc4_cnt = 0;
		}

	} else if (ch_mask & ADC_IO_ADC5_CH) {
		g_adc_io_adc5_sum += val;
		if (++g_adc_io_adc5_cnt >= C_ADC_SUM_CNT) {
			g_adc_io_adc5_cur = g_adc_io_adc5_sum;
			g_adc_io_adc5_sum = g_adc_io_adc5_cnt = 0;
		}

	} else if (ch_mask & ADC_SILENCE_CH) {
		g_adc_silence_sum += val;
		if (++g_adc_silence_cnt >= C_ADC_SUM_CNT) {
			g_adc_silence_cur = g_adc_silence_sum;
			g_adc_silence_sum = g_adc_silence_cnt = 0;
		}
	}
}

void isr_adc_b(ADC_t* adc, uint8_t ch_mask, adc_result_t res)
{
	int16_t val = res - C_ADC_0V0_DELTA;

	if (ch_mask & ADC_TEMP_CH) {
		g_adc_temp_sum += val;
		if (++g_adc_temp_cnt >= C_ADC_SUM_CNT) {
			g_adc_temp_cur = g_adc_temp_sum;
			g_adc_temp_sum = g_adc_temp_cnt = 0;
		}
	}
}


static void dac_init(void)
{
	dac_read_configuration(&DAC_DAC, &dac_conf);
	dac_set_conversion_parameters(&dac_conf, DAC_REF_BANDGAP, DAC_ADJ_LEFT);
	dac_set_active_channel(&dac_conf, DAC_DAC1_CH | DAC_DAC0_CH, 0);
	dac_set_conversion_trigger(&dac_conf, DAC_DAC1_CH | DAC_DAC0_CH, 4);
	dac_write_configuration(&DAC_DAC, &dac_conf);

	/* Get production signature for calibration */
	DACB_CH0OFFSETCAL	= dac_get_calibration_data(DAC_CAL_DACB0_OFFSET);
	DACB_CH0GAINCAL		= dac_get_calibration_data(DAC_CAL_DACB0_GAIN);
	DACB_CH1OFFSETCAL	= dac_get_calibration_data(DAC_CAL_DACB1_OFFSET);
	DACB_CH1GAINCAL		= dac_get_calibration_data(DAC_CAL_DACB1_GAIN);

	dma_init();
}

static void dac_start(void)
{
	dac_enable(&DACB);

	/* Connect the DMA to the DAC periphery */
	dma_start();

	/* IRQ disabled section */
	{
		irqflags_t flags = cpu_irq_save();

		/* Prepare DMA blocks */
		calc_next_frame(&dac_io_dac0_buf[0][0], &dds0_reg, &dds0_inc, &dds1_reg, &dds1_inc);
		calc_next_frame(&dac_io_dac0_buf[1][0], &dds0_reg, &dds0_inc, &dds1_reg, &dds1_inc);

		/* DMA channels activation */
		dma_channel_enable(DMA_CHANNEL_DACB_CH0_A);

		cpu_irq_restore(flags);
	}
}

static void dac_stop(void)
{
	dma_disable();
	dac_disable(&DACB);
}


static void dma_init(void)
{
	memset(&dmach_dma0_conf, 0, sizeof(dmach_dma0_conf));										// DACB channel 0 - linked with dma1
	memset(&dmach_dma1_conf, 0, sizeof(dmach_dma1_conf));										// DACB channel 1 - linked with dma0

	dma_channel_set_burst_length(&dmach_dma0_conf, DMA_CH_BURSTLEN_4BYTE_gc);
	dma_channel_set_burst_length(&dmach_dma1_conf, DMA_CH_BURSTLEN_4BYTE_gc);

	dma_channel_set_transfer_count(&dmach_dma0_conf, DAC_NR_OF_SAMPLES * sizeof(dma_dac_buf_t));
	dma_channel_set_transfer_count(&dmach_dma1_conf, DAC_NR_OF_SAMPLES * sizeof(dma_dac_buf_t));

	dma_channel_set_src_reload_mode(&dmach_dma0_conf, DMA_CH_SRCRELOAD_TRANSACTION_gc);
	dma_channel_set_src_dir_mode(&dmach_dma0_conf, DMA_CH_SRCDIR_INC_gc);
	dma_channel_set_source_address(&dmach_dma0_conf, (uint16_t)(uintptr_t) &dac_io_dac0_buf[0][0]);
	dma_channel_set_dest_reload_mode(&dmach_dma0_conf, DMA_CH_DESTRELOAD_BURST_gc);
	dma_channel_set_dest_dir_mode(&dmach_dma0_conf, DMA_CH_DESTDIR_INC_gc);
	dma_channel_set_destination_address(&dmach_dma0_conf, (uint16_t)(uintptr_t) &DACB_CH0DATA);	// Access to CH0 and CH1

	dma_channel_set_src_reload_mode(&dmach_dma1_conf, DMA_CH_SRCRELOAD_TRANSACTION_gc);
	dma_channel_set_src_dir_mode(&dmach_dma1_conf, DMA_CH_SRCDIR_INC_gc);
	dma_channel_set_source_address(&dmach_dma1_conf, (uint16_t)(uintptr_t) &dac_io_dac0_buf[1][0]);
	dma_channel_set_dest_reload_mode(&dmach_dma1_conf, DMA_CH_DESTRELOAD_BURST_gc);
	dma_channel_set_dest_dir_mode(&dmach_dma1_conf, DMA_CH_DESTDIR_INC_gc);
	dma_channel_set_destination_address(&dmach_dma1_conf, (uint16_t)(uintptr_t) &DACB_CH0DATA);	// Access to CH0 and CH1

	dma_channel_set_trigger_source(&dmach_dma0_conf, DMA_CH_TRIGSRC_DACB_CH0_gc);
	dma_channel_set_single_shot(&dmach_dma0_conf);

	dma_channel_set_trigger_source(&dmach_dma1_conf, DMA_CH_TRIGSRC_DACB_CH0_gc);
	dma_channel_set_single_shot(&dmach_dma1_conf);

	task_dac(tcc1_get_time());																	// Calculate DDS increments
}

static void dma_start(void)
{
	dma_enable();

	dma_set_callback(DMA_CHANNEL_DACB_CH0_A, isr_dma_dac_ch0_A);
	dma_channel_set_interrupt_level(&dmach_dma0_conf, DMA_INT_LVL_MED);

	dma_set_callback(DMA_CHANNEL_DACB_CH0_B, isr_dma_dac_ch0_B);
	dma_channel_set_interrupt_level(&dmach_dma1_conf, DMA_INT_LVL_MED);

	dma_set_priority_mode(DMA_PRIMODE_CH01RR23_gc);
	dma_set_double_buffer_mode(DMA_DBUFMODE_CH01_gc);

	dma_channel_write_config(DMA_CHANNEL_DACB_CH0_A, &dmach_dma0_conf);
	dma_channel_write_config(DMA_CHANNEL_DACB_CH0_B, &dmach_dma1_conf);
}

static void isr_dma_dac_ch0_A(enum dma_channel_status status)
{
	dma_channel_enable(DMA_CHANNEL_DACB_CH0_B);

	cpu_irq_enable();
	calc_next_frame(&dac_io_dac0_buf[0][0], &dds0_reg, &dds0_inc, &dds1_reg, &dds1_inc);
}

static void isr_dma_dac_ch0_B(enum dma_channel_status status)
{
	dma_channel_enable(DMA_CHANNEL_DACB_CH0_A);

	cpu_irq_enable();
	calc_next_frame(&dac_io_dac0_buf[1][0], &dds0_reg, &dds0_inc, &dds1_reg, &dds1_inc);
}


/* The LOOP section */

static void task_dac(uint32_t now)
{	/* Calculation of the DDS increments */
	static uint32_t s_dds0_freq_mHz = 0UL;
	static uint32_t s_dds1_freq_mHz = 0UL;
	uint32_t l_dds0_freq_mHz, l_dds1_freq_mHz;

	/* Setting the pair of frequencies */
	{
		irqflags_t flags		= cpu_irq_save();
		l_dds0_freq_mHz			= dds0_freq_mHz;
		l_dds1_freq_mHz			= dds1_freq_mHz;
		cpu_irq_restore(flags);
	}

	if ((l_dds0_freq_mHz != s_dds0_freq_mHz) || (l_dds1_freq_mHz != s_dds1_freq_mHz)) {
		/* DDS increment calculation */
		uint32_t l_dds0_inc = (uint32_t) (((uint64_t)dds0_freq_mHz * UINT32_MAX) / (DAC_RATE_OF_CONV * 1000UL));
		uint32_t l_dds1_inc = (uint32_t) (((uint64_t)dds1_freq_mHz * UINT32_MAX) / (DAC_RATE_OF_CONV * 1000UL));
		s_dds0_freq_mHz = l_dds0_freq_mHz;
		s_dds1_freq_mHz = l_dds1_freq_mHz;

		/* Setting the pair of increments */
		{
			irqflags_t flags	= cpu_irq_save();
			dds0_inc			= l_dds0_inc;
			dds1_inc			= l_dds1_inc;
			cpu_irq_restore(flags);
		}
	}
}

static void task_adc(uint32_t now)
{	/* Calculations of the ADC values for the presentation layer */
	static uint32_t adc_last = 0;

	if ((now - adc_last) >= 500 || (now < adc_last)) {
		uint32_t l_adc_vctcxo_cur, l_adc_5v0_cur, l_adc_vbat_cur, l_adc_io_adc4_cur, l_adc_io_adc5_cur;
		uint32_t l_adc_silence_cur, l_adc_temp_cur;

		adc_last = now;

		/* Getting a copy of the values */
		{
			irqflags_t flags	= cpu_irq_save();
			l_adc_vctcxo_cur	= g_adc_vctcxo_cur;
			l_adc_5v0_cur		= g_adc_5v0_cur;
			l_adc_vbat_cur		= g_adc_vbat_cur;
			l_adc_io_adc4_cur	= g_adc_io_adc4_cur;
			l_adc_io_adc5_cur	= g_adc_io_adc5_cur;
			l_adc_silence_cur	= g_adc_silence_cur;
			l_adc_temp_cur		= g_adc_temp_cur;
			cpu_irq_restore(flags);
		}

		int16_t l_adc_vctcxo_volt_1000	= (int16_t) (((( 1000UL * l_adc_vctcxo_cur  * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS))  - 1000 * C_VCTCXO_DELTA_VOLTS);
		int16_t l_adc_5v0_volt_1000		= (int16_t) (((  1000UL * l_adc_5v0_cur     * C_VCC_3V0_AREF_VOLTS * C_VCC_5V0_MULT  ) / C_ADC_STEPS));
		int16_t l_adc_vbat_volt_1000	= (int16_t) (((  1000UL * l_adc_vbat_cur    * C_VCC_3V0_AREF_VOLTS * C_VCC_VBAT_MULT ) / C_ADC_STEPS));
		int16_t l_adc_io_adc4_volt_1000	= (int16_t) (((  1000UL * l_adc_io_adc4_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_io_adc5_volt_1000	= (int16_t) (((  1000UL * l_adc_io_adc5_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_silence_volt_1000	= (int16_t) (((  1000UL * l_adc_silence_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_temp_deg_100		= (int16_t) ((((  100UL * l_adc_temp_cur                           * C_TEMPSENSE_MULT) / C_ADC_STEPS))  -  100 * C_0DEGC_K);

		/* Writing back the values */
		{
			irqflags_t flags		= cpu_irq_save();
			g_adc_vctcxo_volt_1000	= l_adc_vctcxo_volt_1000;
			g_adc_5v0_volt_1000		= l_adc_5v0_volt_1000;
			g_adc_vbat_volt_1000	= l_adc_vbat_volt_1000;
			g_adc_io_adc4_volt_1000	= l_adc_io_adc4_volt_1000;
			g_adc_io_adc5_volt_1000	= l_adc_io_adc5_volt_1000;
			g_adc_silence_volt_1000	= l_adc_silence_volt_1000;
			g_adc_temp_deg_100		= l_adc_temp_deg_100;
			cpu_irq_restore(flags);
		}
	}
}

static void task_main_pll(void)
{	/* Handling the 1PPS PLL system */
	static uint16_t s_xo_mode_pwm_val_frac = 0;
	static int16_t	s_diff_ary[C_1PPS_PWM_DIFF_ARY_CNT] = { 0 };
	static uint8_t	s_diff_ary_idx = 0;
	int16_t			l_1pps_last_diff;
	bool			l_1pps_last_inSpan;
	int16_t			l_1pps_deviation;
	int32_t			l_xo_mode_pwm;
	uint8_t			l_1pps_phased_cntr;

	/* Process each second once */
	if (!g_1pps_proceeded_avail) {
		return;
	}
	g_1pps_proceeded_avail = false;

	/* Get copy of global data */
	irqflags_t flags = cpu_irq_save();
	l_1pps_last_diff	= g_1pps_last_diff;
	l_1pps_last_inSpan	= g_1pps_last_inSpan;
	l_xo_mode_pwm		= g_xo_mode_pwm;
	l_1pps_phased_cntr	= g_1pps_phased_cntr;
	cpu_irq_restore(flags);

	/* Modify PWM only when last data was in span */
	if (l_1pps_last_inSpan) {
		/* Diverting into PWM value and flags */
		uint8_t l_xo_mode_pwm_flags = l_xo_mode_pwm &  C_XO_VAL_FLAGS_MASK;
		int32_t l_xo_mode_pwm_val	= l_xo_mode_pwm & (C_XO_VAL_INT_MASK | C_XO_VAL_FRAC_MASK);

		/* Entering new diff value into array */
		s_diff_ary[s_diff_ary_idx++] = l_1pps_last_diff;
		if (s_diff_ary_idx >= C_1PPS_PWM_DIFF_ARY_CNT) {
			s_diff_ary_idx = 0;
		}

		/* Calculate mean value on the total array */
		l_1pps_deviation = 0;
		for (uint8_t idx = C_1PPS_PWM_DIFF_ARY_CNT; idx; idx--) {
			l_1pps_deviation += s_diff_ary[idx - 1];
		}

		/* Low-pass filtering of deviation and PWM integration */
		l_xo_mode_pwm_val -= l_1pps_deviation;

		int16_t i_xo_mode_pwm_val_i	= (l_xo_mode_pwm_val & C_XO_VAL_INT_MASK) >> C_XO_VAL_INT_SHIFT;
		uint8_t i_xo_mode_pwm_val_f	=  l_xo_mode_pwm_val & C_XO_VAL_FRAC_MASK;

		/* Fractional part */
		s_xo_mode_pwm_val_frac += i_xo_mode_pwm_val_f;
		if (s_xo_mode_pwm_val_frac >= 256) {
			s_xo_mode_pwm_val_frac -= 256;
			i_xo_mode_pwm_val_i++;
		}

		/* Set the new integer PWM value */
		tc_write_cc_buffer(&TCC0, TC_CCC, i_xo_mode_pwm_val_i);

		/* Synced stability counter */
		if ((-C_TCC1_SPAN_HALF <= l_1pps_deviation) && (l_1pps_deviation <= C_TCC1_SPAN_HALF)) {
			if (++l_1pps_phased_cntr == (C_TCC1_CLOCKSETTING_AFTER_SECS + 3)) {
				/* Send GNS URC command for repeated GNSINF status lines */
				serial_send_gns_urc(1);

			} else if (l_1pps_phased_cntr > 250) {
				/* Saturated value*/
				l_1pps_phased_cntr = 250;
			}
		} else {
			l_1pps_phased_cntr = 0;
		}

		/* Adding previous flags */
		l_xo_mode_pwm = l_xo_mode_pwm_flags | (l_xo_mode_pwm_val & (C_XO_VAL_INT_MASK | C_XO_VAL_FRAC_MASK));

		/* Pushing global data */
		flags = cpu_irq_save();
		g_1pps_deviation	= l_1pps_deviation;
		g_xo_mode_pwm		= l_xo_mode_pwm;
		g_1pps_phased_cntr	= l_1pps_phased_cntr;
		cpu_irq_restore(flags);
	}
}

static void task_env_calc(void)
{
	static uint32_t s_last = 0UL;
	uint32_t now = tcc1_get_time();

	/* No more than 2 calculations per sec */
	if (s_last + 500 <= now) {
		irqflags_t flags;
		int32_t l_twi1_baro_temp_100;
		int16_t l_twi1_hygro_T_100;
		float temp_twi1_mean;
		float temp_env;
		float temp_env_round;
		int16_t	l_env_temp_deg_100;
		int16_t	l_env_hygro_RH_100;
		int16_t l_twi1_hygro_DP_100;
		int16_t l_env_temp_delta_100;

		/* Get the global data */
		flags = cpu_irq_save();
		l_twi1_baro_temp_100	= g_twi1_baro_temp_100;
		l_twi1_hygro_T_100		= g_twi1_hygro_T_100;
		l_twi1_hygro_DP_100		= g_twi1_hygro_DP_100;
		l_env_temp_delta_100	= g_env_temp_delta_100;
		cpu_irq_restore(flags);

		/* Mean temperature of the Barometer and the Hygrometer chip */
		temp_twi1_mean		= ((float)l_twi1_baro_temp_100 + (float)l_twi1_hygro_T_100) / 200.f;
		temp_env			= temp_twi1_mean - (l_env_temp_delta_100 / 100.f);
		temp_env_round		= (temp_env >= 0.f) ?  0.5f : -0.5f;
		l_env_temp_deg_100	= (int16_t) (temp_env_round + 100.f * temp_env);

		/* Rel. humidity correction for environment temperature */
		/* @see https://www.wetterochs.de/wetter/feuchte.html */
		{
			// const float K1	= 6.1078f;
			float td			= l_twi1_hygro_DP_100 / 100.f;
			float calc_a;
			float calc_b;
			float sdd_td;
			float sdd_tenv;

			/* Temperature over water or ice */
			if (temp_env >= 0.f) {
				/* over water */
				calc_a = 7.5f;
				calc_b = 237.3f;

			} else if (temp_env >= -5.f) {														// this temperature is a value given by the programmers will, only ...
				/* over water */
				calc_a = 7.6f;
				calc_b = 240.7f;

			} else {
				/* over ice */
				calc_a = 9.5f;
				calc_b = 265.5f;
			}

			sdd_td				= /* K1* */ pow(10.f, (calc_a * td)			/ (calc_b + td));
			sdd_tenv			= /* K1* */ pow(10.f, (calc_a * temp_env)	/ (calc_b + temp_env));
			l_env_hygro_RH_100	= (int16_t) (0.5f + 10000.f * sdd_td / sdd_tenv);
		}

		/* Pushing global data */
		flags = cpu_irq_save();
		g_env_temp_deg_100	= l_env_temp_deg_100;
		g_env_hygro_RH_100	= l_env_hygro_RH_100;
		cpu_irq_restore(flags);

		s_last = now;
	}
}

static void task_main_aprs_pocsag(void)
{
	static uint32_t				s_now_sec					= 0UL;
	static bool					s_lock						= false;
	uint32_t					l_now_sec					= tcc1_get_time() >> 10;
	uint32_t					l_boot_time_ts;
	uint32_t					l_ts;
	float						l_gns_lat_f;
	uint8_t						l_lat_deg_d;
	float						l_lat_minutes_f;
	char						l_lat_hemisphere;
	float						l_gns_lon_f;
	uint8_t						l_lon_deg_d;
	float						l_lon_minutes_f;
	char						l_lon_hemisphere;
	float						l_gns_course_deg;
	uint16_t					l_course_deg;
	float						l_gns_speed_kn;
	float						l_gns_speed_kmPh;
	uint16_t					l_speed_kn;
	uint64_t					l_aprs_alert_last;
	APRS_ALERT_FSM_STATE_ENUM_t	l_aprs_alert_fsm_state;
	APRS_ALERT_REASON_ENUM_t	l_aprs_alert_reason;
	irqflags_t					flags;
	char						l_pocsag_ric_msg_buf[C_TX_RIC_BUF_SIZE];
	char						l_pocsag_news_msg_buf[C_TX_NEWS_BUF_SIZE];
	char						l_msg_buf[C_TX_BUF_SIZE];
	uint16_t					l_pocsag_ric_msg_buf_len	= 0U;
	uint16_t					l_pocsag_news_msg_buf_len	= 0U;
	uint16_t					l_msg_buf_len				= 0U;
	char						l_mark						= ' ';
	struct calendar_date		calDat;

	/* Once a second to be processed - do not send when GPS not ready */
	if ( s_lock ||
		(s_now_sec == l_now_sec) ||
		!g_gns_fix_status) {
		return;
	}

	/* do not send when APRS and POCSAG is disabled */
	if (!((g_gsm_enable && g_gsm_aprs_enable) || (g_ax_enable && (g_ax_aprs_enable || g_ax_pocsag_enable)))) {
		return;
	}

	/* Single thread lock */
	s_lock = true;

	/* Store new second */
	s_now_sec = l_now_sec;

	/* Get a copy from the global variables */
	{
		flags = cpu_irq_save();
		l_boot_time_ts			= g_boot_time_ts;
		l_ts					= g_boot_time_ts + (uint32_t)(g_milliseconds_cnt64 / 1000);
		l_aprs_alert_last		= g_aprs_alert_last;
		l_aprs_alert_fsm_state	= g_aprs_alert_fsm_state;
		l_aprs_alert_reason		= g_aprs_alert_reason;
		cpu_irq_restore(flags);
	}

	/* GNSS has to set the clock first */
	if (!l_boot_time_ts) {
		/* Single thread finished */
		s_lock = false;
		return;
	}

	/* Get the date */
	calendar_timestamp_to_date(l_ts, &calDat);

	/* Prepare POCSAG mode */
	if (g_ax_enable && g_ax_pocsag_enable) {
		/* FIFOCMD / FIFOSTAT */
		spi_ax_transport(false, "< a8 03 >");													// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

		/* Switch to POCSAG mode */
		spi_ax_init_POCSAG_Tx();

		/* POCSAG Skyper clock for every new minute (chime) */
		if (!calDat.second && g_ax_pocsag_chime_enable) {
			l_pocsag_ric_msg_buf_len = spi_ax_pocsag_skyper_TimeString(l_pocsag_ric_msg_buf, (uint8_t) sizeof(l_pocsag_ric_msg_buf), &calDat);

			/* Transmit POCSAG message */
			spi_ax_run_POCSAG_Tx_FIFO_Msg(AX_POCSAG_SKYPER_RIC_CLOCK, AX_POCSAG_CW2_MODE0_NUMERIC, l_pocsag_ric_msg_buf, strlen(l_pocsag_ric_msg_buf));
			l_pocsag_ric_msg_buf_len = 0;
		}
	}

	do {
		/* Check for sensor and time boundaries */
		if (l_aprs_alert_fsm_state == APRS_ALERT_FSM_STATE__NOOP) {
			/* APRS messaging started */
			if (g_gsm_ring &&
				((l_aprs_alert_last + C_APRS_ALERT_REQ_HOLDOFF_SEC) <= l_now_sec)) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__REQUEST;

			} else if ((l_aprs_alert_last + C_APRS_ALERT_TIME_SEC) <= l_now_sec) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__TIME;

			} else if ((aprs_pos_delta_m() > C_APRS_ALERT_POS_DELTA_M) &&
				((l_aprs_alert_last + C_APRS_ALERT_POS_HOLDOFF_SEC) <= l_now_sec)) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__POSITION;
				aprs_pos_anchor();

			} else if ((aprs_gyro_total_dps_1000() > C_APRS_ALERT_GYRO_DPS_1000) &&
				((l_aprs_alert_last + C_APRS_ALERT_GYRO_HOLDOFF_SEC) <= l_now_sec)) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__GYRO;

			} else if ((aprs_accel_xy_delta_g_1000() > C_APRS_ALERT_ACCEL_G_1000) &&
				((l_aprs_alert_last + C_APRS_ALERT_ACCEL_HOLDOFF_SEC) <= l_now_sec)) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__ACCEL;

			} else if ((aprs_mag_delta_nT() > C_APRS_ALERT_MAG_DELTA_NT) &&
				((l_aprs_alert_last + C_APRS_ALERT_MAG_HOLDOFF_SEC) <= l_now_sec)) {
				l_aprs_alert_reason		= APRS_ALERT_REASON__MAGNET;
			}

			if (l_aprs_alert_reason != APRS_ALERT_REASON__NONE) {
				l_aprs_alert_fsm_state	= APRS_ALERT_FSM_STATE__DO_N1;
				l_aprs_alert_last		= l_now_sec;

				/* Make snapshot of alert environment */
				{
					flags = cpu_irq_save();

					/* Gyroscope */
					g_aprs_alert_1_gyro_x_mdps	= g_twi1_gyro_1_gyro_x_mdps;
					g_aprs_alert_1_gyro_y_mdps	= g_twi1_gyro_1_gyro_y_mdps;
					g_aprs_alert_1_gyro_z_mdps	= g_twi1_gyro_1_gyro_z_mdps;

					/* Acceleration */
					g_aprs_alert_1_accel_x_mg	= g_twi1_gyro_1_accel_x_mg;
					g_aprs_alert_1_accel_y_mg	= g_twi1_gyro_1_accel_y_mg;
					g_aprs_alert_1_accel_z_mg	= g_twi1_gyro_1_accel_z_mg;

					/* Magnetics */
					g_aprs_alert_2_mag_x_nT		= g_twi1_gyro_2_mag_x_nT;
					g_aprs_alert_2_mag_y_nT		= g_twi1_gyro_2_mag_y_nT;
					g_aprs_alert_2_mag_z_nT		= g_twi1_gyro_2_mag_z_nT;

					cpu_irq_restore(flags);
				}
			}
		}

		/* Preparations for message strings */
		if (l_aprs_alert_fsm_state != APRS_ALERT_FSM_STATE__NOOP) {
			/* Get copy from global variables */
			{
				flags = cpu_irq_save();
				l_gns_lat_f			= g_gns_lat;
				l_gns_lon_f			= g_gns_lon;
				l_gns_course_deg	= g_gns_course_deg;
				l_gns_speed_kn		= g_gns_speed_kn;
				l_gns_speed_kmPh	= g_gns_speed_kmPh;
				cpu_irq_restore(flags);
			}

			/* Calculations */
			{
				l_lat_deg_d			= (uint8_t) (l_gns_lat_f >= 0.f ?  l_gns_lat_f : -l_gns_lat_f);
				l_lat_minutes_f		= ((l_gns_lat_f >= 0.f ?  l_gns_lat_f : -l_gns_lat_f) - l_lat_deg_d) * 60.f + 0.005f;
				l_lat_hemisphere	= l_gns_lat_f >= 0.f ?  'N' : 'S';
				l_lon_deg_d			= (uint8_t) (l_gns_lon_f >= 0.f ?  l_gns_lon_f : -l_gns_lon_f);
				l_lon_minutes_f		= ((l_gns_lon_f >= 0.f ?  l_gns_lon_f : -l_gns_lon_f) - l_lon_deg_d) * 60.f + 0.005f;
				l_lon_hemisphere	= l_gns_lon_f >= 0.f ?  'E' : 'W';

				l_course_deg		= (uint16_t) (0.5f + l_gns_course_deg);
				l_course_deg		%= 360;
				if (!l_course_deg) {
					l_course_deg = 360;
				}

				l_speed_kn			= (uint16_t) (0.5f + l_gns_speed_kn);
			}
		}

		/* Check for reporting interval */
		switch (l_aprs_alert_fsm_state) {
			case APRS_ALERT_FSM_STATE__DO_N1:
			{
				int32_t l_aprs_alert_1_gyro_x_mdps;
				int32_t l_aprs_alert_1_gyro_y_mdps;
				int32_t l_aprs_alert_1_gyro_z_mdps;

				flags = cpu_irq_save();
				l_aprs_alert_1_gyro_x_mdps = g_aprs_alert_1_gyro_x_mdps;
				l_aprs_alert_1_gyro_y_mdps = g_aprs_alert_1_gyro_y_mdps;
				l_aprs_alert_1_gyro_z_mdps = g_aprs_alert_1_gyro_z_mdps;
				cpu_irq_restore(flags);

				if ((l_aprs_alert_reason == APRS_ALERT_REASON__GYRO) || (l_aprs_alert_reason == APRS_ALERT_REASON__REQUEST)) {
					l_mark = '*';
				}

				#if 0
				/* Debug info at USB console */
				int len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#81 DEBUG: message N1 in queue ...\r\n");
				udi_write_tx_buf(g_prepare_buf, len, false);
				#endif

				/* Message content POCSAG */
				if (g_ax_pocsag_enable) {
					if (g_ax_pocsag_individual_ric) {
						l_pocsag_ric_msg_buf_len  = (uint16_t) snprintf_P(l_pocsag_ric_msg_buf, sizeof(l_pocsag_ric_msg_buf), PM_POCSAG_TX_MSG_SRCCALL, g_aprs_source_callsign);
						l_pocsag_ric_msg_buf_len += (uint16_t) snprintf_P(&(l_pocsag_ric_msg_buf[0]) + l_pocsag_ric_msg_buf_len, sizeof(l_pocsag_ric_msg_buf) - l_pocsag_ric_msg_buf_len,
																PM_POCSAG_TX_SPD_HDG_POS_RIC,
																(int) (g_gns_msl_alt_m + 0.5f),
																l_lat_hemisphere, l_gns_lat_f,
																l_lon_hemisphere, l_gns_lon_f,
																(int) (l_gns_speed_kmPh + 0.5f), (int) (l_gns_course_deg + 0.5f));
					}

					{
						const char reasonAry[] = APRS_ALERT_REASON_SHORTHAND;
						l_pocsag_news_msg_buf_len  = (uint16_t) snprintf_P(l_pocsag_news_msg_buf, sizeof(l_pocsag_news_msg_buf), PM_POCSAG_TX_MSG_SRCCALL, g_aprs_source_callsign);
						l_pocsag_news_msg_buf_len += (uint16_t) snprintf_P(&(l_pocsag_news_msg_buf[0]) + l_pocsag_news_msg_buf_len, sizeof(l_pocsag_news_msg_buf) - l_pocsag_news_msg_buf_len,
																PM_POCSAG_TX_SPD_HDG_POS_NEWS,
																(int) (g_gns_msl_alt_m + 0.5f),
																l_lat_hemisphere, l_gns_lat_f,
																calDat.year, calDat.month + 1, calDat.date + 1,
																l_lon_hemisphere, l_gns_lon_f,
																calDat.hour, calDat.minute,
																reasonAry[l_aprs_alert_reason],
																(int) (l_gns_speed_kmPh + 0.5f), (int) (l_gns_course_deg + 0.5f));
					}
				}

				/* Message content APRS */
				l_msg_buf_len  = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_FORWARD, g_aprs_source_callsign, g_aprs_source_ssid);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_POS_SPD_HDG, l_lat_deg_d, l_lat_minutes_f, l_lat_hemisphere, PM_APRS_TX_SYMBOL_TABLE_ID, l_lon_deg_d, l_lon_minutes_f, l_lon_hemisphere, PM_APRS_TX_SYMBOL_CODE, l_course_deg, l_speed_kn);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_N1, l_mark, l_aprs_alert_1_gyro_x_mdps / 1000.f, l_aprs_alert_1_gyro_y_mdps / 1000.f, l_aprs_alert_1_gyro_z_mdps / 1000.f);

				l_aprs_alert_fsm_state = APRS_ALERT_FSM_STATE__DO_N2;

				/* Re-activate GPRS when call came in */
				if (g_gsm_enable && g_gsm_aprs_enable) {
					if (l_aprs_alert_reason == APRS_ALERT_REASON__REQUEST) {
						static uint8_t s_count = 0;

						if (++s_count < g_gsm_ring) {
							/* Repeat that packet as requested number of times */
							l_aprs_alert_fsm_state = APRS_ALERT_FSM_STATE__DO_N1;
							l_aprs_alert_last = l_now_sec + C_APRS_ALERT_MESSAGE_DELAY_SEC;

						} else {
							s_count = 0;
						}

						/* Reset the RING info */
						g_gsm_ring = 0;
					}
				}
			}
			break;

			case APRS_ALERT_FSM_STATE__DO_N2:
			if ((l_aprs_alert_last + C_APRS_ALERT_MESSAGE_DELAY_SEC) < l_now_sec) {
				int16_t l_aprs_alert_1_accel_x_mg;
				int16_t l_aprs_alert_1_accel_y_mg;
				int16_t l_aprs_alert_1_accel_z_mg;

				flags = cpu_irq_save();
				l_aprs_alert_1_accel_x_mg = g_aprs_alert_1_accel_x_mg;
				l_aprs_alert_1_accel_y_mg = g_aprs_alert_1_accel_y_mg;
				l_aprs_alert_1_accel_z_mg = g_aprs_alert_1_accel_z_mg;
				cpu_irq_restore(flags);

				if ((l_aprs_alert_reason == APRS_ALERT_REASON__ACCEL) || (l_aprs_alert_reason == APRS_ALERT_REASON__REQUEST)) {
					l_mark = '*';
				}

				#if 0
				/* Debug info at USB console */
				int len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#82 DEBUG: message N2 in queue ...\r\n");
				udi_write_tx_buf(g_prepare_buf, len, false);
				#endif

				/* Message content POCSAG */
				l_pocsag_ric_msg_buf_len	= 0;
				l_pocsag_news_msg_buf_len	= 0;

				/* Message content */
				l_msg_buf_len  = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_FORWARD, g_aprs_source_callsign, g_aprs_source_ssid);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_POS_SPD_HDG, l_lat_deg_d, l_lat_minutes_f, l_lat_hemisphere, PM_APRS_TX_SYMBOL_TABLE_ID, l_lon_deg_d, l_lon_minutes_f, l_lon_hemisphere, PM_APRS_TX_SYMBOL_CODE, l_course_deg, l_speed_kn);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_N2, l_mark, l_aprs_alert_1_accel_x_mg / 1000.f, l_aprs_alert_1_accel_y_mg / 1000.f, l_aprs_alert_1_accel_z_mg / 1000.f);

				l_aprs_alert_fsm_state = APRS_ALERT_FSM_STATE__DO_N3;
			}
			break;

			case APRS_ALERT_FSM_STATE__DO_N3:
			if ((l_aprs_alert_last + C_APRS_ALERT_MESSAGE_DELAY_SEC) < l_now_sec) {
				int32_t l_aprs_alert_2_mag_x_nT;
				int32_t l_aprs_alert_2_mag_y_nT;
				int32_t l_aprs_alert_2_mag_z_nT;

				flags = cpu_irq_save();
				l_aprs_alert_2_mag_x_nT = g_aprs_alert_2_mag_x_nT;
				l_aprs_alert_2_mag_y_nT = g_aprs_alert_2_mag_y_nT;
				l_aprs_alert_2_mag_z_nT = g_aprs_alert_2_mag_z_nT;
				cpu_irq_restore(flags);

				if ((l_aprs_alert_reason == APRS_ALERT_REASON__MAGNET) || (l_aprs_alert_reason == APRS_ALERT_REASON__REQUEST)) {
					l_mark = '*';
				}

				#if 0
				/* Debug info at USB console */
				int len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#83 DEBUG: message N3 in queue ...\r\n");
				udi_write_tx_buf(g_prepare_buf, len, false);
				#endif

				/* Message content POCSAG */
				l_pocsag_ric_msg_buf_len	= 0;
				l_pocsag_news_msg_buf_len	= 0;

				/* Message content */
				l_msg_buf_len  = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_FORWARD, g_aprs_source_callsign, g_aprs_source_ssid);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_POS_SPD_HDG, l_lat_deg_d, l_lat_minutes_f, l_lat_hemisphere, PM_APRS_TX_SYMBOL_TABLE_ID, l_lon_deg_d, l_lon_minutes_f, l_lon_hemisphere, PM_APRS_TX_SYMBOL_CODE, l_course_deg, l_speed_kn);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_N3, l_mark, l_aprs_alert_2_mag_x_nT / 1000.f, l_aprs_alert_2_mag_y_nT / 1000.f, l_aprs_alert_2_mag_z_nT / 1000.f);

				l_aprs_alert_fsm_state = APRS_ALERT_FSM_STATE__DO_N4;
			}
			break;

			case APRS_ALERT_FSM_STATE__DO_N4:
			if ((l_aprs_alert_last + C_APRS_ALERT_MESSAGE_DELAY_SEC) < l_now_sec) {
				float l_gns_msl_alt_m;
				int16_t l_twi1_hygro_DP_100;
				int32_t l_twi1_baro_p_h_100;

				flags = cpu_irq_save();
				l_gns_msl_alt_m		= g_gns_msl_alt_m;
				l_twi1_hygro_DP_100	= g_twi1_hygro_DP_100;
				l_twi1_baro_p_h_100	= g_qnh_p_h_100;
				cpu_irq_restore(flags);

				float l_gns_msl_alt_ft = l_gns_msl_alt_m >= 0.f ?  (0.5f + (l_gns_msl_alt_m / 0.3048f)) : (-0.5f + (l_gns_msl_alt_m / 0.3048f));

				if (l_aprs_alert_reason == APRS_ALERT_REASON__REQUEST) {
					l_mark = '*';
				}

				#if 0
				/* Debug info at USB console */
				int len = snprintf(g_prepare_buf, sizeof(g_prepare_buf), "#84 DEBUG: message N4 in queue ...\r\n");
				udi_write_tx_buf(g_prepare_buf, len, false);
				#endif

				/* Message content POCSAG */
				l_pocsag_ric_msg_buf_len	= 0;
				l_pocsag_news_msg_buf_len	= 0;

				/* Message content */
				l_msg_buf_len  = (uint16_t) snprintf_P(l_msg_buf, sizeof(l_msg_buf), PM_APRS_TX_FORWARD, g_aprs_source_callsign, g_aprs_source_ssid);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_POS_SPD_HDG, l_lat_deg_d, l_lat_minutes_f, l_lat_hemisphere, PM_APRS_TX_SYMBOL_TABLE_ID, l_lon_deg_d, l_lon_minutes_f, l_lon_hemisphere, PM_APRS_TX_SYMBOL_CODE, l_course_deg, l_speed_kn);
				l_msg_buf_len += (uint16_t) snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_N4, l_mark, (long)l_gns_msl_alt_ft, l_twi1_hygro_DP_100 / 100.f, l_twi1_baro_p_h_100 / 100.f);

				l_aprs_alert_fsm_state	= APRS_ALERT_FSM_STATE__NOOP;
				l_aprs_alert_reason		= APRS_ALERT_REASON__NONE;
			}
			break;

			default:
				l_aprs_alert_fsm_state	= APRS_ALERT_FSM_STATE__NOOP;
				l_aprs_alert_reason		= APRS_ALERT_REASON__NONE;
		}
	} while (false);

	/* POCSAG message content ready */
	if (g_ax_enable && g_ax_pocsag_enable) {
		/* Push POCSAG via AX5243 (VHF/UHF) */
		{
			/* Individual message to the RIC */
			if (g_ax_pocsag_individual_ric && l_pocsag_ric_msg_buf_len) {
				spi_ax_run_POCSAG_Tx_FIFO_Msg(g_ax_pocsag_individual_ric, AX_POCSAG_CW2_MODE3_ALPHANUM, l_pocsag_ric_msg_buf, strlen(l_pocsag_ric_msg_buf));

				do {
					/* RADIOSTATE */
					spi_ax_transport(false, "< 1c R1 >");												// RD Address 0x1C: RADIOSTATE - IDLE
				} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
			}

			/* Public news */
			if (l_pocsag_news_msg_buf_len) {
				/* Announce public Skyper rubric number 95 with label "Tracking" */
				{
					const char rubricLabel[] = "Tracking";
					char rubricBuf[32];

					uint16_t rubricBufLen = spi_ax_pocsag_skyper_RubricString(rubricBuf, sizeof(rubricBuf), 95, rubricLabel, strlen(rubricLabel));
					spi_ax_run_POCSAG_Tx_FIFO_Msg(AX_POCSAG_SKYPER_RIC_RUBRICS, AX_POCSAG_CW2_MODE3_ALPHANUM, rubricBuf, rubricBufLen);

					do {
						/* RADIOSTATE */
						spi_ax_transport(false, "< 1c R1 >");												// RD Address 0x1C: RADIOSTATE - IDLE
					} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
				}

				/* Send public Skyper news to rubric "Tracking" */
				{
					char newsBuf[C_TX_NEWS_BUF_SIZE];

					uint16_t newsBufLen = spi_ax_pocsag_skyper_NewsString(newsBuf, sizeof(newsBuf), 95, ++g_ax_pocsag_news_idx, l_pocsag_news_msg_buf, l_pocsag_news_msg_buf_len);
					spi_ax_run_POCSAG_Tx_FIFO_Msg(AX_POCSAG_SKYPER_RIC_NEWS, AX_POCSAG_CW2_MODE3_ALPHANUM, newsBuf, newsBufLen);

					g_ax_pocsag_news_idx %= 10;

					do {
						/* RADIOSTATE */
						spi_ax_transport(false, "< 1c R1 >");												// RD Address 0x1C: RADIOSTATE - IDLE
					} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
				}
			}
		}
	}

	/* APRS message content ready */
	if (l_msg_buf_len) {
		l_msg_buf_len += (uint16_t)  snprintf_P(&(l_msg_buf[0]) + l_msg_buf_len, sizeof(l_msg_buf) - l_msg_buf_len, PM_APRS_TX_MSGEND);

		/* Push APRS via AX5243 (VHF/UHF) */
		if (g_ax_enable && g_ax_aprs_enable) {
			const char addrAry[][C_PR1200_CALL_LENGTH]	= { "APXFMS", "DF4IAH", "WIDE1", "WIDE2" };
			const uint8_t ssidAry[]	= { 0, 8, 1, 2 };

			/* Switch APRS mode (handled by PR1200) */
			spi_ax_init_PR1200_Tx();

			/* Transmit APRS message */
			spi_ax_run_PR1200_Tx_FIFO_APRS(addrAry, ssidAry, sizeof(addrAry) / C_PR1200_CALL_LENGTH,  l_msg_buf, l_msg_buf_len);

			do {
				/* FIFOSTAT */
				spi_ax_transport(false, "< 28 R1 >");
			} while (!(g_ax_spi_packet_buffer[0] & 0x01));
		}

		/* Push APRS message via the GSM / GPRS network */
		if (g_gsm_enable && g_gsm_aprs_enable) {
			/* Init GPRS link opening */
			if (!g_gsm_aprs_gprs_connected) {
				serial_gsm_gprs_link_openClose(true);
			}

			/* GPRS IP transportation open - blocking */
			serial_gsm_gprs_ip_openClose(true);

			/* Sending APRS information to the server */
			if (g_gsm_aprs_ip_connected) {
				aprs_message_send(l_msg_buf, l_msg_buf_len);
			}

			/* GPRS IP transportation close */
			serial_gsm_gprs_ip_openClose(false);

			/* Mark end of message as last transmission time */
			l_aprs_alert_last = tcc1_get_time() >> 10;
		}
	}

	/* Shutdown GPRS link when state machine is idling */
	if (g_gsm_enable && g_gsm_aprs_enable) {
		if (l_aprs_alert_fsm_state == APRS_ALERT_FSM_STATE__NOOP) {
			serial_gsm_gprs_link_openClose(false);
		}
	}


	/* Write back to the global variables */
	{
		flags = cpu_irq_save();
		g_aprs_alert_last		= l_aprs_alert_last;
		g_aprs_alert_fsm_state	= l_aprs_alert_fsm_state;
		g_aprs_alert_reason		= l_aprs_alert_reason;
		cpu_irq_restore(flags);
	}

	/* Single thread finished */
	s_lock = false;
}

void task(void)
{
	if (g_workmode == WORKMODE_RUN) {
		/* TASK when woken up and all ISRs are done */
		/* note: ADC and DAC are handled by the scheduler */
		task_serial();																			// Handle serial communication with the SIM808
		task_twi();																				// Handle (TWI1 and) TWI2 communications
		task_usb();																				// Handling the USB connection
		task_main_pll();																		// Handling the 1PPS PLL system
		task_env_calc();																		// Environment simulation calculations
		task_main_aprs_pocsag();																// Handling the APRS alerts and POCSAG messages

		/* Send debug message via Skyper */
		if (g_dbg_buf_len && g_ax_enable && g_ax_pocsag_enable && g_ax_pocsag_individual_ric && (g_milliseconds_cnt64 > 35000ULL)) {
			spi_ax_send_POCSAG_Msg(g_ax_pocsag_individual_ric, ax_pocsag_analyze_msg_tgtFunc_get(g_dbg_buf, g_dbg_buf_len), g_dbg_buf, g_dbg_buf_len);
			g_dbg_buf_len = 0;
		}
	}
}


int main(void)
{
	uint8_t retcode = 0;

	/* Init the IOPORT */
	ioport_init();

	/* Init the FIFO buffers */
	fifo_init(&g_fifo_sched_desc, g_fifo_sched_buffer, FIFO_SCHED_BUFFER_LENGTH);

	/* Init of interrupt system */
	g_workmode = WORKMODE_INIT;
	irq_initialize_vectors();
	pmic_init();
	pmic_set_scheduling(PMIC_SCH_FIXED_PRIORITY);

	sysclk_init();																				// Clock configuration set-up

	sleepmgr_init();																			// Unlocks all sleep mode levels

	rtc_init();
	rtc_start();

	init_globals();

	interrupt_init();																			// Port interrupts
	evsys_init();																				// Event system
	tc_init();																					// Timers


	/* Insert basic TEST-CODE here */


	serial_init();																				// Set up serial connection to the SIM808
	if (g_adc_enabled) {
		adc_init();																				// ADC
	}
	if (g_dac_enabled) {
		dac_init();																				// DAC
	}
	twi_init();																					// I2C / TWI to 1:Baro,Hygro,9axis 2:LCD
	spi_init();																					// SPI to AX5243

	board_init();																				// Activates all in/out pins not already handled above - transitions from Z to dedicated states

	nvm_init(INT_FLASH);

	/* All interrupt sources & PMIC are prepared until here - IRQ activation follows */
	cpu_irq_enable();

	/* Start of sub-modules */
	tc_start();																					// All clocks and PWM timers start here
	if (g_dac_enabled) {
		dac_start();																			// Start DA convertions
	}
	if (g_adc_enabled) {
		adc_start();																			// Start AD convertions
	}

	/* Init of USB system */
	usb_init();																					// USB device stack start function to enable stack and start USB

	/* Start TWI channels */
	twi_start();																				// Start TWI

	/* Start the AX5243 */
	spi_start();																				// Start SPI communication with the AX5243


	/* Insert prepared system TEST-CODE here */
#if 0
	/* TEST */
	for (int i = 1000; i; i--) {

		#if 1
		/* Calibration with the preamble '1010..' pattern */
		{
			/* FIFOCMD / FIFOSTAT */
			spi_ax_transport(false, "< a8 03 >");												// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

			/* Switch to POCSAG mode */
			spi_ax_init_POCSAG_Tx();

			for (int calCnt = 400; calCnt; calCnt--) {
				spi_ax_util_POCSAG_Tx_FIFO_Preamble();

				/* FIFO do a COMMIT */
				spi_ax_transport(false, "< a8 04 >");											// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
			}

			do {
				/* FIFOSTAT */
				spi_ax_transport(false, "< 28 R1 >");
			} while (!(g_ax_spi_packet_buffer[0] & 0x01));
		}

		#else
		/* Test POCSAG message */
		{
			const uint32_t tgtRic			= 143721UL;											// Skyper of DF4IAH
			char tstBuf[80];

			memset(tstBuf, 0, sizeof(tstBuf));
			//sprintf(tstBuf, "%02d%02d%d   %02d%02d%02d", 12, 1000 - i, 0,   12,  2, 18);
			sprintf(tstBuf, "DF4IAH: %02d%02d%02d   %02d%02d%02d", 12, 1000 - i, 0,   12,  2, 18);

			for (int count = 1; count; count--) {
				/* FIFOCMD / FIFOSTAT */
				spi_ax_transport(false, "< a8 03 >");											// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

				/* Switch from APRS mode to POCSAG */
				spi_ax_init_POCSAG_Tx();

				/* Transmit POCSAG message */
				uint8_t tstBufLen = (uint8_t) strlen(tstBuf);
				(void) spi_ax_run_POCSAG_Tx_FIFO_Msg(tgtRic, ax_pocsag_analyze_msg_tgtFunc_get(tstBuf, tstBufLen), tstBuf, tstBufLen);

				delay_ms(500);
				//delay_ms(7500);
			}

			/* FIFOCMD / FIFOSTAT */
			spi_ax_transport(false, "< a8 03 >");												// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

			do {
				/* FIFOSTAT */
				spi_ax_transport(false, "< 28 R1 >");
			} while (!(g_ax_spi_packet_buffer[0] & 0x01));

			do {
				/* RADIOSTATE */
				spi_ax_transport(false, "< 1c R1 >");											// RD Address 0x1C: RADIOSTATE - IDLE
			} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
		}
		#endif

		#if 0
		/* Test APRS message */
		{
			char addrAry[][6]	= { "APXFMS", "DF4IAH", "WIDE1", "WIDE2" };
			uint8_t ssidAry[]	= { 0, 8, 1, 2 };
			char aprsMsg[]		= "!4928.39N/00836.88Ej OP: Uli, QTH: Ladenburg, LOC: JN49hl.";

			for (int count = 1; count; count--) {
				/* FIFOCMD / FIFOSTAT */
				spi_ax_transport(false, "< a8 03 >");											// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

				/* Switch to APRS (PR1200) mode */
				spi_ax_init_PR1200_Tx();

				/* Enter an APRS UI frame */
				spi_ax_run_PR1200_Tx_FIFO_APRS(addrAry, ssidAry, 4, aprsMsg, strlen(aprsMsg));

				delay_ms(250);
				//delay_ms(7500);
			}

			do {
				/* FIFOSTAT */
				spi_ax_transport(false, "< 28 R1 >");
			} while (!(g_ax_spi_packet_buffer[0] & 0x01));

			do {
				/* RADIOSTATE */
				spi_ax_transport(false, "< 1c R1 >");											// RD Address 0x1C: RADIOSTATE - IDLE
			} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);
		}
		# endif
	}

	volatile bool loopWait = true;
	while (loopWait) {
		nop();
	}
#endif


	/* Start serial */
	serial_start();																				// Start communication with the SIM808 */

	/* LED green */
	twi2_set_leds(0x02);

	/* Show help page of command set */
	printHelp();

	/* LEDs off */
	twi2_set_leds(0x00);

	/* The application code */
	g_twi2_lcd_repaint = true;
	g_workmode = WORKMODE_RUN;
    while (g_workmode == WORKMODE_RUN) {
		/* Process all user space tasks */
		task();

		/* Work on the pushed back jobs */
		yield_ms(0);
    }

	/* LEDs off */
	twi2_set_leds(0x00);

	cpu_irq_disable();
	sleepmgr_enter_sleep();

	return retcode;
}
