/*
 * main.h
 *
 * Created: 25.01.2017 09:17:52
 *  Author: DF4IAH
 */


#ifndef MAIN_H_
#define MAIN_H_

#include <asf.h>


/* VERSION: YYM, MDD */
#define VERSION														20180330

#define APPLICATION_NAME											"FindMeSAT"
#define APPLICATION_VERSION											"1.0"

#define C_CLOCK_MHZ_F												30e6


#define C_DBG_BUF_LEN												256

#define C_TWI1_BARO_C_CNT											8

#define C_0DEGC_K													273.15f

#define C_ADC_STEPS													0x00100000UL
#define C_ADC_SUM_CNT												256

#define C_ADC_0V0_DELTA												190
#define C_VCC_3V3_VOLTS												3.318f
#define C_VCC_3V0_AREF_VOLTS										3

#define C_VCTCXO_PWM_HI_VOLTS										3.253f
#define C_VCTCXO_DEFAULT_VOLTS										1.500f
#define C_VCTCXO_DELTA_VOLTS										0.047f

#define C_VCC_5V0_VOLTS												4.890f
#define C_VCC_5V0_MULT												2.41948528f
#define C_VCC_VBAT_MULT												2.42614048f
#define C_TEMPSENSE_MULT											629.65f

#define C_TCC1_PERIOD												30000
#define C_TCC1_MEAN_OFFSET											0
#define C_TCC1_BORDER_OFFSET										3000
#define C_TCC1_CLOCKSETTING_AFTER_SECS								30U

#ifdef DEBUG
# define C_TCC1_CLOCKSETTING_OFFSET									27U
#else
# define C_TCC1_CLOCKSETTING_OFFSET									11U
#endif

#define C_TCC1_SPAN_HALF											50							// 30 equals to 1 µs


/* DEFAULT VALUES for the Sensors */

#define C_TWI1_GYRO_1_ACCEL_OFSX_DEFAULT							((int16_t) (-46672L / 16))
#define C_TWI1_GYRO_1_ACCEL_OFSY_DEFAULT							((int16_t) (+41120L / 16))
#define C_TWI1_GYRO_1_ACCEL_OFSZ_DEFAULT							((int16_t) (+76672L / 16))
#define C_TWI1_GYRO_1_ACCEL_FACTX_DEFAULT							9980
#define C_TWI1_GYRO_1_ACCEL_FACTY_DEFAULT							9975
#define C_TWI1_GYRO_1_ACCEL_FACTZ_DEFAULT							9950
#define C_TWI1_GYRO_1_GYRO_OFSX_DEFAULT								( -32 / 4)
#define C_TWI1_GYRO_1_GYRO_OFSY_DEFAULT								( -80 / 4)
#define C_TWI1_GYRO_1_GYRO_OFSZ_DEFAULT								(+148 / 4)
#define C_TWI1_GYRO_2_MAG_FACTX_DEFAULT								7760
#define C_TWI1_GYRO_2_MAG_FACTY_DEFAULT								8390
#define C_TWI1_GYRO_2_MAG_FACTZ_DEFAULT								9490

#define C_1PPS_PWM_DIFF_ARY_CNT										16


/* FIFO */
#define FIFO_SCHED_BUFFER_LENGTH									32


/* SCHEDULER */
#define C_SCH_SLOT_CNT												32


typedef enum SCHED_ENTRY_CB_TYPE_ENUM {
	SCHED_ENTRY_CB_TYPE__LISTTIME									= 0,
	SCHED_ENTRY_CB_TYPE__LISTTIME_ISSYNC,
} SCHED_ENTRY_CB_TYPE_ENUM_t;

typedef struct sched_entry {
	uint32_t			wakeTime;
	void*				callback;

	uint8_t				occupied		: 1;
	uint8_t				isIntDis		: 1;
	uint8_t				isSync			: 1;
	uint8_t				cbType			: 2;
	uint8_t				reserved1		: 3;
} sched_entry_t;

typedef void(*sched_callback_type0)(uint32_t listTime);
typedef void(*sched_callback_type1)(uint32_t listTime, bool isSync);


typedef enum MY_STRING_TO_VAR_ENUM {
	MY_STRING_TO_VAR_MOOP											= 0,
	MY_STRING_TO_VAR_FLOAT,
	MY_STRING_TO_VAR_LONG,
	MY_STRING_TO_VAR_INT
} MY_STRING_TO_VAR_ENUM_t;

typedef enum WORKMODE_ENUM {
	WORKMODE_OFF													= 0,
	WORKMODE_INIT,
	WORKMODE_RUN,
	WORKMODE_SUSPENDED,
	WORKMODE_END,
} WORKMODE_ENUM_t;

typedef enum PRINT_STATUS_BF_ENUM {
	PRINT_STATUS_LINES__ATXMEGA										= 0b00000001,
	PRINT_STATUS_LINES__SIM808										= 0b00000010,
	PRINT_STATUS_LINES__1PPS										= 0b00000100,
} PRINT_STATUS_BF_ENUM_t;

typedef enum CALIBRATION_MODE_ENUM {
	CALIBRATION_MODE_ENUM__DEFAULTS = 0,
	CALIBRATION_MODE_ENUM__ACCEL_X,
	CALIBRATION_MODE_ENUM__ACCEL_Y,
	CALIBRATION_MODE_ENUM__ACCEL_Z,
	CALIBRATION_MODE_ENUM__GYRO,
} CALIBRATION_MODE_ENUM_t;

typedef enum ADC_CH0_SCAN_ENUM {
	ADC_CH0_SCAN_3V0												= 255,						// PIN = PA0, ADC0 - used as AREFA
	ADC_CH0_SCAN_VCTCXO												= 0,						// PIN = PA1, ADC1
	ADC_CH0_SCAN_5V0,																			// PIN = PA2, ADC2
	ADC_CH0_SCAN_VBAT,																			// PIN = PA3, ADC3
} ADC_CH0_SCAN_ENUM_t;

typedef enum DMA_CHANNEL_ENUM {
	DMA_CHANNEL_DACB_CH0_A = 0,
	DMA_CHANNEL_DACB_CH0_B,
} DMA_CHANNEL_ENUM_t;

typedef enum GSM_BF_ENUM {
	GSM__ENABLE														= 0b00000001,
	GSM__APRS_ENABLE												= 0b00000010,
} GSM_BF_ENUM_t;

typedef enum APRS_MODE_ENUM {
	APRS_MODE__OFF													= 0,
	APRS_MODE__ON,
} APRS_MODE_ENUM_t;

typedef enum AX_BF_ENUM {
	AX__ENABLE														= 0b00000001,
	AX__APRS_ENABLE													= 0b00000010,
	AX__POCSAG_ENABLE												= 0b00000100,
	AX__POCSAG_CHIME_ENABLE											= 0b00001000,
} AX_BF_ENUM_t;

/* find AX_SET_TX_RX_MODE_t in spi_ax.h, also */
#ifndef DEFINED_AX_SET_TX_RX_MODE
typedef enum AX_SET_TX_RX_MODE {
	AX_SET_TX_RX_MODE_OFF											= 0x00,

	AX_SET_TX_RX_MODE_APRS_TX										= 0x31,
	AX_SET_TX_RX_MODE_APRS_RX_WOR,
	AX_SET_TX_RX_MODE_APRS_RX_CONT,
	AX_SET_TX_RX_MODE_APRS_RX_CONT_SINGLEPARAMSET,

	AX_SET_TX_RX_MODE_POCSAG_TX										= 0x71,
	AX_SET_TX_RX_MODE_POCSAG_RX_WOR,
	AX_SET_TX_RX_MODE_POCSAG_RX_CONT,
	AX_SET_TX_RX_MODE_POCSAG_RX_CONT_SINGLEPARAMSET,
} AX_SET_TX_RX_MODE_t;
#define DEFINED_AX_SET_TX_RX_MODE
#endif

/* find AX_SET_MON_MODE_t in spi_ax.h, also */
#ifndef DEFINED_AX_SET_MON_MODE
typedef enum AX_SET_MON_MODE {
	AX_SET_MON_MODE_OFF												= 0x00,

	AX_SET_MON_MODE_APRS_RX_WOR										= AX_SET_TX_RX_MODE_APRS_RX_WOR,
	AX_SET_MON_MODE_APRS_RX_CONT									= AX_SET_TX_RX_MODE_APRS_RX_CONT,
	AX_SET_MON_MODE_APRS_RX_CONT_SINGLEPARAMSET						= AX_SET_TX_RX_MODE_APRS_RX_CONT_SINGLEPARAMSET,

	AX_SET_MON_MODE_POCSAG_RX_WOR									= AX_SET_TX_RX_MODE_POCSAG_RX_WOR,
	AX_SET_MON_MODE_POCSAG_RX_CONT									= AX_SET_TX_RX_MODE_POCSAG_RX_CONT,
	AX_SET_MON_MODE_POCSAG_RX_CONT_SINGLEPARAMSET					= AX_SET_TX_RX_MODE_POCSAG_RX_CONT_SINGLEPARAMSET,
} AX_SET_MON_MODE_t;
#define DEFINED_AX_SET_MON_MODE
#endif

typedef enum EEPROM_ADDR_ENUM {
	EEPROM_ADDR__VERSION											= 0x0000,					// i32
	//														next:	= 0x0004,

	EEPROM_ADDR__GSM_BF												= 0x0008,					// ui8
	EEPROM_ADDR__APRS_MODE											= 0x0009,					// ui8
	EEPROM_ADDR__APRS_IP_PROTO										= 0x000A,					// ui8
	//														next:	= 0x000B,

	EEPROM_ADDR__APRS_IP_PORT										= 0x000C,					// ui16
	EEPROM_ADDR__ENV_TEMP_DELTA										= 0x000E,					// i16
	EEPROM_ADDR__VCTCXO												= 0x0010,					// i32
	EEPROM_ADDR__LCDBL												= 0x0014,					// i16
	EEPROM_ADDR__BEEP												= 0x0016,					// ui8
	EEPROM_ADDR__PITCHTONE											= 0x0017,					// i8
	EEPROM_ADDR__PRINT_STATUS										= 0x0018,					// ui8
	EEPROM_ADDR__ENV_QNH_AUTO										= 0x0019,					// ui8
	EEPROM_ADDR__ENV_QNH_METERS										= 0x001A,					// i16
	EEPROM_ADDR__9AXIS_TEMP_RTOFS									= 0x001C,					// i16
	EEPROM_ADDR__9AXIS_TEMP_SENS									= 0x001E,					// i16
	EEPROM_ADDR__9AXIS_ACCEL_OFS_X									= 0x0020,					// i16
	EEPROM_ADDR__9AXIS_ACCEL_OFS_Y									= 0x0022,					// i16
	EEPROM_ADDR__9AXIS_ACCEL_OFS_Z									= 0x0024,					// i16
	//														next:	= 0x0026,

	EEPROM_ADDR__9AXIS_ACCEL_FACT_X									= 0x0028,					// i16
	EEPROM_ADDR__9AXIS_ACCEL_FACT_Y									= 0x002A,					// i16
	EEPROM_ADDR__9AXIS_ACCEL_FACT_Z									= 0x002C,					// i16
	//														next:	= 0x002E,

	EEPROM_ADDR__9AXIS_GYRO_OFS_X									= 0x0030,					// i16
	EEPROM_ADDR__9AXIS_GYRO_OFS_Y									= 0x0032,					// i16
	EEPROM_ADDR__9AXIS_GYRO_OFS_Z									= 0x0034,					// i16
	//														next:	= 0x0036,

	EEPROM_ADDR__9AXIS_GYRO_FACT_X									= 0x0038,					// i16
	EEPROM_ADDR__9AXIS_GYRO_FACT_Y									= 0x003A,					// i16
	EEPROM_ADDR__9AXIS_GYRO_FACT_Z									= 0x003C,					// i16
	//														next:	= 0x003E,

	EEPROM_ADDR__9AXIS_MAG_OFS_X									= 0x0040,					// i16
	EEPROM_ADDR__9AXIS_MAG_OFS_Y									= 0x0042,					// i16
	EEPROM_ADDR__9AXIS_MAG_OFS_Z									= 0x0044,					// i16
	//														next:	= 0x0046,

	EEPROM_ADDR__9AXIS_MAG_FACT_X									= 0x0048,					// i16
	EEPROM_ADDR__9AXIS_MAG_FACT_Y									= 0x004A,					// i16
	EEPROM_ADDR__9AXIS_MAG_FACT_Z									= 0x004C,					// i16
	//														next:	= 0x004E,

	EEPROM_ADDR__ENV_BARO_TEMP_DELTA								= 0x0050,					// i16
	EEPROM_ADDR__ENV_BARO_PRES_DELTA								= 0x0052,					// i16
	EEPROM_ADDR__ENV_HYGRO_TEMP_DELTA								= 0x0054,					// i16
	EEPROM_ADDR__ENV_HYGRO_RH_DELTA									= 0x0056,					// i16
	//														next:	= 0x0058,

	EEPROM_ADDR__APRS_CALLSIGN										= 0x0080,					// char[12]
	EEPROM_ADDR__APRS_SSID											= 0x008C,					// char[4]
	EEPROM_ADDR__APRS_LOGIN											= 0x0090,					// char[10]
	EEPROM_ADDR__APRS_PWD											= 0x009A,					// char[6]

	EEPROM_ADDR__GSM_PIN											= 0x00A2,					// char[14]

	EEPROM_ADDR__AX_BF												= 0x00B0,					// ui8
	EEPROM_ADDR__AX_MON_MODE										= 0x00B1,					// ui8
	EEPROM_ADDR__AX_POCSAG_BEACON									= 0x00B2,					// ui8
	//														next:	= 0x00B3,

	EEPROM_ADDR__AX_POCSAG_RIC										= 0x00B4,					// ui32
	//														next:	= 0x00B8,

	EEPROM_ADDR__APRS_LINK_SERVICE									= 0x0100,					// char[32]
	EEPROM_ADDR__APRS_LINK_USER										= 0x0120,					// char[16]
	EEPROM_ADDR__APRS_LINK_PWD										= 0x0130,					// char[16]
	EEPROM_ADDR__APRS_IP_NAME										= 0x0140,					// char[32]
	//														next:	= 0x0160,

} EEPROM_ADDR_ENUM_t;

typedef enum EEPROM_SAVE_BF_ENUM {
	EEPROM_SAVE_BF__VCTCXO											= 0b0000000000000001,
	EEPROM_SAVE_BF__9AXIS_OFFSETS									= 0b0000000000000010,
	EEPROM_SAVE_BF__LCDBL											= 0b0000000000000100,
	EEPROM_SAVE_BF__PRINT_STATUS									= 0b0000000000001000,
	EEPROM_SAVE_BF__BEEP											= 0b0000000000010000,
	EEPROM_SAVE_BF__PITCHTONE										= 0b0000000000100000,
	EEPROM_SAVE_BF__ENV												= 0b0000000001000000,
	EEPROM_SAVE_BF__GSM												= 0b0000000010000000,
	EEPROM_SAVE_BF__APRS											= 0b0000000100000000,
	EEPROM_SAVE_BF__AX												= 0b0000001000000000,
} EEPROM_SAVE_BF_ENUM_t;


#define C_XO_BF_PLL													0x40000000L
#define C_XO_VAL_FLAGS_MASK											0xff000000L
#define C_XO_VAL_INT_MASK											0x00ffff00L
#define C_XO_VAL_INT_SHIFT											8
#define C_XO_VAL_FRAC_MASK											0x000000ffL


typedef struct dma_dac_buf_s {
	uint16_t	ch0;
	uint16_t	ch1;
} dma_dac_buf_t;


/* APRS related */

typedef enum APRS_ALERT_FSM_STATE_ENUM {
	APRS_ALERT_FSM_STATE__NOOP										= 0,
	APRS_ALERT_FSM_STATE__DO_N1,
	APRS_ALERT_FSM_STATE__DO_N2,
	APRS_ALERT_FSM_STATE__DO_N3,
	APRS_ALERT_FSM_STATE__DO_N4,
} APRS_ALERT_FSM_STATE_ENUM_t;

#define C_APRS_LINK_SERVICE_LEN										32
#define C_APRS_LINK_USER_LEN										16
#define C_APRS_LINK_PWD_LEN											16
#define C_APRS_IP_PROTO_LEN											4
#define C_APRS_IP_NAME_LEN											32
#define C_APRS_IP_PORT_LEN											6
#define C_APRS_S_LEN												12
#define C_APRS_SSID_LEN												4
#define C_APRS_USER_LEN												10
#define C_APRS_PWD_LEN												6
#define C_APRS_ALERT_MESSAGE_DELAY_SEC								10
#define C_APRS_ALERT_TIME_SEC										900
#define C_APRS_ALERT_REQ_HOLDOFF_SEC								60
#define C_APRS_ALERT_POS_DELTA_M									2500
#define C_APRS_ALERT_POS_HOLDOFF_SEC								64
#define C_APRS_ALERT_GYRO_DPS_1000									2500
#define C_APRS_ALERT_GYRO_HOLDOFF_SEC								61
#define C_APRS_ALERT_ACCEL_G_1000									10
#define C_APRS_ALERT_ACCEL_HOLDOFF_SEC								63
#define C_APRS_ALERT_MAG_DELTA_NT									6500
#define C_APRS_ALERT_MAG_HOLDOFF_SEC								62

typedef enum APRS_ALERT_REASON_ENUM {															// Shorthand in APRS message
	APRS_ALERT_REASON__NONE											= 0,						// '?'
	APRS_ALERT_REASON__TIME,																	// 'T'
	APRS_ALERT_REASON__POSITION,																// 'P'
	APRS_ALERT_REASON__GYRO,																	// 'G'
	APRS_ALERT_REASON__ACCEL,																	// 'A'
	APRS_ALERT_REASON__MAGNET,																	// 'M'
	APRS_ALERT_REASON__REQUEST,																	// 'R'

	APRS_ALERT_REASON_COUNT
} APRS_ALERT_REASON_ENUM_t;
#define APRS_ALERT_REASON_SHORTHAND									"?TPGAMR"


typedef enum CALC_CRC16_CCITT_ENUM {
	CALC_CRC16_CCITT_RESET											= 0,
	CALC_CRC16_CCITT_ADD,
	CALC_CRC16_CCITT_RETURN_LSB,
	CALC_CRC16_CCITT_RETURN_MSB
} CALC_CRC16_CCITT_ENUM_t;


void save_globals(EEPROM_SAVE_BF_ENUM_t bf);
char* cueBehind(char* ptr, char delim);
int myStringToFloat(const char* ptr, float* out);
int myStringToVar(char *str, uint32_t format, float out_f[], long out_l[], int out_i[]);
uint8_t doHexdump(char *target, const uint8_t *source, uint8_t inLen);
char* ipProto_2_ca(uint8_t aprs_ip_proto);
char* copyStr(char* target, uint8_t targetSize, const char* source);
uint8_t calc_CRC16_CCITT(CALC_CRC16_CCITT_ENUM_t selection, uint8_t byte_LSB_first);
uint32_t calc_BitReverse(uint8_t bitWidth, uint32_t in);
void adc_app_enable(bool enable);
void aprs_num_update(uint8_t mode);
void aprs_link_service_update(const char service[]);
void aprs_link_user_update(const char user[]);
void aprs_link_pwd_update(const char pwd[]);
void aprs_ip_proto_update(const char proto[]);
void aprs_ip_name_update(const char name[]);
void aprs_ip_port_update(uint16_t port);
void aprs_call_update(const char call[]);
void aprs_ssid_update(const char ssid[]);
void aprs_user_update(const char user[]);
void aprs_pwd_update(const char pwd[]);
void ax_aprs_enable(bool enable);
void ax_pocsag_enable(bool enable);
void ax_enable(bool enable);
void backlight_mode_pwm(int16_t mode_pwm);
void bias_update(uint8_t bias);
void calibration_mode(CALIBRATION_MODE_ENUM_t mode);
void dac_app_enable(bool enable);
void dds_update(float dds0_hz, float dds1_hz, float phase);
void errorBeep_enable(bool enable);
void env_temp(float temp);
void gsm_aprs_enable(bool enable);
void gsm_pin_update(const char pin[]);
void gsm_enable(bool enable);
void keyBeep_enable(bool enable);
void monitor_mode(AX_SET_MON_MODE_t mode);
void pitchTone_mode(uint8_t mode);
void pocsagBeacon_time(uint8_t secs);
void pocsag_chime_update(bool enable);
void pocsag_message_send(const char msg[]);
void pocsag_ric_update(uint32_t ric);
void pocsag_send_skyper_activation(void);
void qnh_setAuto(void);
void qnh_setHeightM(int16_t heightM);
void printStatusLines_bitfield(PRINT_STATUS_BF_ENUM_t bf);
void sens_baro_temp(float temp100);
void sens_baro_pres(float pres100);
void sens_hygro_temp(float temp100);
void sens_hygro_RH(float rh100);
void shutdown(bool doReset);
void xoPwm_set(int32_t mode_pwm);

uint32_t tcc1_get_time(void);
void halt(void);

uint16_t aprs_pos_delta_m(void);
void aprs_pos_anchor(void);
uint16_t aprs_gyro_total_dps_1000(void);
uint16_t aprs_accel_xy_delta_g_1000(void);
uint16_t aprs_mag_delta_nT(void);

void aprs_message_send(const char* msg, uint16_t len);

bool sched_getLock(volatile uint8_t* lockVar);
void sched_freeLock(volatile uint8_t* lockVar);
void sched_set_alarm(uint32_t alarmTime);
void sched_doSleep(void);
void sched_push(void* cb, SCHED_ENTRY_CB_TYPE_ENUM_t cbType, uint32_t wakeTime, bool isDelay, bool isIntDis, bool isSync);
void sched_pop(uint32_t wakeNow);
void yield_ms(uint16_t ms);
void yield_ms_cb(uint32_t listTime);

void isr_tcc0_ovfl(void);
void isr_tcc1_ovfl(void);
void isr_rtc_alarm(uint32_t rtc_time);
void isr_adc_a(ADC_t* adc, uint8_t ch_mask, adc_result_t res);
void isr_adc_b(ADC_t* adc, uint8_t ch_mask, adc_result_t res);

int main(void);


#endif /* MAIN_H_ */
