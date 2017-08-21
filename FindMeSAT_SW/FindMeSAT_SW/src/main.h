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
#define VERSION_HIGH												170
#define VERSION_LOW													821


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
#define C_TEMPSENSE_MULT											629.20f

#define C_TCC1_PERIOD												30000U


/* FIFO */
#define FIFO_SCHED_BUFFER_LENGTH									32


/* SCHEDULER */
#define C_SCH_SLOT_CNT												32


typedef enum SCHED_ENTRY_CB_TYPE_ENUM {
	SCHED_ENTRY_CB_TYPE__LISTTIME = 0,
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
	MY_STRING_TO_VAR_MOOP = 0,
	MY_STRING_TO_VAR_FLOAT,
	MY_STRING_TO_VAR_LONG,
	MY_STRING_TO_VAR_INT
} MY_STRING_TO_VAR_ENUM_t;

typedef enum WORKMODE_ENUM {
	WORKMODE_OFF = 0,
	WORKMODE_INIT,
	WORKMODE_RUN,
	WORKMODE_SUSPENDED,
	WORKMODE_END,
} WORKMODE_ENUM_t;

typedef enum PRINT_STATUS_BF_ENUM {
	PRINT_STATUS_LINES__ATXMEGA		= 0b00000001,
	PRINT_STATUS_LINES__SIM808		= 0b00000010,
} PRINT_STATUS_BF_ENUM_t;

typedef enum CALIBRATION_MODE_ENUM {
	CALIBRATION_MODE_ENUM__DEFAULTS = 0,
	CALIBRATION_MODE_ENUM__GYRO,
} CALIBRATION_MODE_ENUM_t;

typedef enum ADC_CH0_SCAN_ENUM {
	ADC_CH0_SCAN_3V0 = 255,											// PIN = PA0, ADC0 - used as AREFA
	ADC_CH0_SCAN_VCTCXO = 0,										// PIN = PA1, ADC1
	ADC_CH0_SCAN_5V0,												// PIN = PA2, ADC2
	ADC_CH0_SCAN_VBAT,												// PIN = PA3, ADC3
} ADC_CH0_SCAN_ENUM_t;

typedef enum DMA_CHANNEL_ENUM {
	DMA_CHANNEL_DACB_CH0_A = 0,
	DMA_CHANNEL_DACB_CH0_B,
} DMA_CHANNEL_ENUM_t;

typedef enum EEPROM_ADDR_ENUM {
	EEPROM_ADDR__VERSION			= 0x0000,						// i32
	EEPROM_ADDR__VCTCXO				= 0x0010,						// i32
	EEPROM_ADDR__LCDBL				= 0x0014,						// i16
	EEPROM_ADDR__BEEP				= 0x0016,						// ui8
	EEPROM_ADDR__PITCHTONE			= 0x0017,						// i8
	EEPROM_ADDR__PRINT_STATUS		= 0x0018,						// ui8

	EEPROM_ADDR__9AXIS_TEMP_RTOFS	= 0x001C,						// i16
	EEPROM_ADDR__9AXIS_TEMP_SENS	= 0x001E,						// i16

	EEPROM_ADDR__9AXIS_ACCEL_OFS_X	= 0x0020,						// i16
	EEPROM_ADDR__9AXIS_ACCEL_OFS_Y	= 0x0022,						// i16
	EEPROM_ADDR__9AXIS_ACCEL_OFS_Z	= 0x0024,						// i16

	EEPROM_ADDR__9AXIS_ACCEL_FACT_X	= 0x0028,						// i16
	EEPROM_ADDR__9AXIS_ACCEL_FACT_Y	= 0x002A,						// i16
	EEPROM_ADDR__9AXIS_ACCEL_FACT_Z	= 0x002C,						// i16

	EEPROM_ADDR__9AXIS_GYRO_OFS_X	= 0x0030,						// i16
	EEPROM_ADDR__9AXIS_GYRO_OFS_Y	= 0x0032,						// i16
	EEPROM_ADDR__9AXIS_GYRO_OFS_Z	= 0x0034,						// i16

	EEPROM_ADDR__9AXIS_GYRO_FACT_X	= 0x0038,						// i16
	EEPROM_ADDR__9AXIS_GYRO_FACT_Y	= 0x003A,						// i16
	EEPROM_ADDR__9AXIS_GYRO_FACT_Z	= 0x003C,						// i16

	EEPROM_ADDR__9AXIS_MAG_OFS_X	= 0x0040,						// i16
	EEPROM_ADDR__9AXIS_MAG_OFS_Y	= 0x0042,						// i16
	EEPROM_ADDR__9AXIS_MAG_OFS_Z	= 0x0044,						// i16

	EEPROM_ADDR__9AXIS_MAG_FACT_X	= 0x0048,						// i16
	EEPROM_ADDR__9AXIS_MAG_FACT_Y	= 0x004A,						// i16
	EEPROM_ADDR__9AXIS_MAG_FACT_Z	= 0x004C,						// i16
} EEPROM_ADDR_ENUM_t;

typedef enum EEPROM_SAVE_BF_ENUM {
	EEPROM_SAVE_BF__VCTCXO			= 0b00000001,
	EEPROM_SAVE_BF__LCDBL			= 0b00000010,
	EEPROM_SAVE_BF__BEEP			= 0b00000100,
	EEPROM_SAVE_BF__PITCHTONE		= 0b00001000,
	EEPROM_SAVE_BF__PRINT_STATUS	= 0b00010000,
	EEPROM_SAVE_BF__9AXIS_OFFSETS	= 0b00100000,
} EEPROM_SAVE_BF_ENUM_t;


#define C_XO_BF_PLL					0x40000000L
#define C_XO_VAL_MASK				0x0000ffffL


typedef struct dma_dac_buf_s {
	uint16_t	ch0;
	uint16_t	ch1;
} dma_dac_buf_t;


void save_globals(EEPROM_SAVE_BF_ENUM_t bf);
int myStringToVar(char *str, uint32_t format, float out_f[], long out_l[], int out_i[]);

void adc_app_enable(bool enable);
void backlight_mode_pwm(int16_t mode_pwm);
void bias_update(uint8_t bias);
void calibration_mode(CALIBRATION_MODE_ENUM_t mode);
void dac_app_enable(bool enable);
void dds_update(float dds0_hz, float dds1_hz, float phase);
void errorBeep_enable(bool enable);
void keyBeep_enable(bool enable);
void pitchTone_mode(uint8_t mode);
void printStatusLines_bitfield(PRINT_STATUS_BF_ENUM_t bf);
void xoPwm_set(int32_t mode_pwm);
void halt(void);

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
