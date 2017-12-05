/**
 * \file
 *
 * \brief FindMeSAT
 * twi.c
 *
 * Created: 07.06.2017 22:39:16
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


/* Add access to the global vars */
#include "externals.h"

#ifdef TWI1_SLAVE
extern TWI_Slave_t		g_twi1_slave;
extern uint8_t			g_twi1_recv_data[TWI_DATA_LENGTH];
#endif

#ifdef TWI2_SLAVE
extern TWI_Slave_t		g_twi2_slave;
extern uint8_t			g_twi2_recv_data[TWI_DATA_LENGTH];
#endif


const char					PM_FORMAT_02LD[]					= "%02ld";
PROGMEM_DECLARE(const char, PM_FORMAT_02LD[]);
const char					PM_FORMAT_03LD[]					= "%03ld";
PROGMEM_DECLARE(const char, PM_FORMAT_03LD[]);
const char					PM_FORMAT_03F10[]					= "%03.10f";
PROGMEM_DECLARE(const char, PM_FORMAT_03F10[]);
const char					PM_FORMAT_4LD[]						= "%4ld";
PROGMEM_DECLARE(const char, PM_FORMAT_4LD[]);
const char					PM_FORMAT_4F1[]						= "%4.1f";
PROGMEM_DECLARE(const char, PM_FORMAT_4F1[]);
const char					PM_FORMAT_5F1[]						= "%5.1f";
PROGMEM_DECLARE(const char, PM_FORMAT_5F1[]);
const char					PM_FORMAT_5F3[]						= "%5.3f";
PROGMEM_DECLARE(const char, PM_FORMAT_5F3[]);
const char					PM_FORMAT_05LD[]					= "%05ld";
PROGMEM_DECLARE(const char, PM_FORMAT_05LD[]);
const char					PM_FORMAT_05F2[]					= "%05.2f";
PROGMEM_DECLARE(const char, PM_FORMAT_05F2[]);
const char					PM_FORMAT_6F1[]						= "%6.1f";
PROGMEM_DECLARE(const char, PM_FORMAT_6F1[]);
const char					PM_FORMAT_07F2[]					= "%07.2f";
PROGMEM_DECLARE(const char, PM_FORMAT_07F2[]);
const char					PM_FORMAT_KMPH[]					= "kmh";
PROGMEM_DECLARE(const char, PM_FORMAT_KMPH[]);

const char					PM_TWI1_INIT_HYGRO_01[]				= "\r\nTWI-onboard: Hygro SHT31-DIS - I2C address: 0x%02X\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_HYGRO_01[]);
const char					PM_TWI1_INIT_HYGRO_02[]				= "TWI-onboard: Hygro SHT31-DIS -   address NACK / 'break' bad response\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_HYGRO_02[]);
const char					PM_TWI1_INIT_HYGRO_03[]				= "TWI-onboard: Hygro SHT31-DIS -      status: 0x%04X\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_HYGRO_03[]);
const char					PM_TWI1_INIT_HYGRO_04[]				= "TWI-onboard:  INIT success.\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_HYGRO_04[]);
const char					PM_TWI1_INIT_HYGRO_05[]				= "TWI-onboard:  ... device not on board. (sc=%d)\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_HYGRO_05[]);
const char					PM_TWI1_INIT_ONBOARD_HYGRO_OK[]		= "Init: Hygro success";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_HYGRO_OK[]);

const char					PM_TWI1_INIT_GYRO_01[]				= "\r\nTWI-onboard: Gyro MPU-9250 - I2C address: 0x%02X, 0x%02X\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_GYRO_01[]);
const char					PM_TWI1_INIT_GYRO_02[]				= "TWI-onboard: Gyro MPU-9250   -   'reset 1' bad response\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_GYRO_02[]);
const char					PM_TWI1_INIT_GYRO_03[]				= "TWI-onboard: Gyro MPU-9250 -     version: 0x%02X, 0x%02X\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_GYRO_03[]);
const char					PM_TWI1_INIT_GYRO_04[]				= "TWI-onboard:  INIT success.\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_GYRO_04[]);
const char					PM_TWI1_INIT_GYRO_05[]				= "TWI-onboard:  ... device not on board. (sc=%d)\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_GYRO_05[]);
const char					PM_TWI1_INIT_ONBOARD_GYRO_OK[]		= "Init: Gyro  success";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_GYRO_OK[]);

const char					PM_TWI1_INIT_BARO_01[]				= "\r\nTWI-onboard: Baro MS560702BA03-50 - I2C address: 0x%02X\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_01[]);
const char					PM_TWI1_INIT_BARO_02[]				= "TWI-onboard:  BAD reading serial/CRC word. (sc=%d)\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_02[]);
const char					PM_TWI1_INIT_BARO_03[]				= "TWI-onboard: Baro MS560702BA03-50 -     serial#: %d\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_03[]);
const char					PM_TWI1_INIT_BARO_04[]				= "TWI-onboard:  BAD reading PROM address %d. (sc=%d)\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_04[]);
const char					PM_TWI1_INIT_BARO_05[]				= "TWI-onboard:  INIT success.\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_05[]);
const char					PM_TWI1_INIT_BARO_06[]				= "TWI-onboard:  ... device not on board. (sc=%d)\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_BARO_06[]);
const char					PM_TWI1_INIT_ONBOARD_BARO_OK[]		= "Init: Baro  success";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_BARO_OK[]);

const char					PM_TWI1_INIT_ONBOARD_01[]			= "-----------\r\n\r\n";
PROGMEM_DECLARE(const char, PM_TWI1_INIT_ONBOARD_01[]);

const char					PM_TWIHEADER_FINDMESAT[]			= "FindMeSAT";
PROGMEM_DECLARE(const char, PM_TWIHEADER_FINDMESAT[]);
const char					PM_TWIHEADER_BY_DF4IAH[]			= "by DF4IAH";
PROGMEM_DECLARE(const char, PM_TWIHEADER_BY_DF4IAH[]);

const char					PM_TWIINIT_DATE_TIME[]				= "    -  -     :  :   UTC";
PROGMEM_DECLARE(const char, PM_TWIINIT_DATE_TIME[]);
const char					PM_TWIINIT_MP_TEMP[]				= "mP Temp =";
PROGMEM_DECLARE(const char, PM_TWIINIT_MP_TEMP[]);
const char					PM_TWIINIT_MP_UUSB[]				=    "Uusb =";
PROGMEM_DECLARE(const char, PM_TWIINIT_MP_UUSB[]);
const char					PM_TWIINIT_MP_UBAT[]				=    "Ubat =";
PROGMEM_DECLARE(const char, PM_TWIINIT_MP_UBAT[]);
const char					PM_TWIINIT_MP_UVCTCXO[]				= "Uvctcxo =";
PROGMEM_DECLARE(const char, PM_TWIINIT_MP_UVCTCXO[]);
//const char				PM_TWIINIT_MP_UIOADC4[]				= "Uioadc4 =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_MP_UIOADC4[]);
//const char				PM_TWIINIT_MP_UIOADC5[]				= "Uioadc5 =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_MP_UIOADC5[]);
//const char				PM_TWIINIT_MP_USILEN[]				= "Usilen. =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_MP_USILEN[]);
//const char					PM_TWIINIT_BA_TEMP[]				= "Ba_Temp =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_BA_TEMP[]);
//const char					PM_TWIINIT_BA_PRES[]				= "Ba_Pres =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_BA_PRES[]);
//const char					PM_TWIINIT_HY_TEMP[]				= "Hy_Temp =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_HY_TEMP[]);
//const char					PM_TWIINIT_HY_RELH[]				= "Hy_RelH =";
//PROGMEM_DECLARE(const char, PM_TWIINIT_HY_RELH[]);
const char					PM_TWIINIT_ENV_TEMP[]				= "EnvTemp =";
PROGMEM_DECLARE(const char, PM_TWIINIT_ENV_TEMP[]);
const char					PM_TWIINIT_ENV_RELH[]				= "EnvRelH =";
PROGMEM_DECLARE(const char, PM_TWIINIT_ENV_RELH[]);
const char					PM_TWIINIT_DP_TEMP[]				= "DP_Temp =";
PROGMEM_DECLARE(const char, PM_TWIINIT_DP_TEMP[]);
const char					PM_TWIINIT_QNH[]					= "QNH hPa =";
PROGMEM_DECLARE(const char, PM_TWIINIT_QNH[]);
const char					PM_TWIINIT_C[]						= "C";
PROGMEM_DECLARE(const char, PM_TWIINIT_C[]);
const char					PM_TWIINIT_V[]						= "V";
PROGMEM_DECLARE(const char, PM_TWIINIT_V[]);
const char					PM_TWIINIT_P100[]					= "%";
PROGMEM_DECLARE(const char, PM_TWIINIT_P100[]);
//const char					PM_TWIINIT_HPA[]					= "hPa";
//PROGMEM_DECLARE(const char, PM_TWIINIT_HPA[]);
const char					PM_TWIINIT_GX[]						= "Gx";
PROGMEM_DECLARE(const char, PM_TWIINIT_GX[]);
const char					PM_TWIINIT_GY[]						= "Gy";
PROGMEM_DECLARE(const char, PM_TWIINIT_GY[]);
const char					PM_TWIINIT_GZ[]						= "Gz";
PROGMEM_DECLARE(const char, PM_TWIINIT_GZ[]);
const char					PM_TWIINIT_MAGNETICS[]				= "Magnetics";
PROGMEM_DECLARE(const char, PM_TWIINIT_MAGNETICS[]);
const char					PM_TWIINIT_ACCEL[]					= "Accel.";
PROGMEM_DECLARE(const char, PM_TWIINIT_ACCEL[]);


/* Forward declarations */

static void task_twi1_hygro(void);
static void task_twi1_gyro(void);
static void task_twi1_baro(void);
static void task_twi2_lcd_template(void);


/* Functions */

#ifdef TWI1_SLAVE
static void twi1_slave_process(void) {
	int i;

	for(i = 0; i < TWI_DATA_LENGTH; i++) {
		g_twi1_recv_data[i] = g_twi1_slave.receivedData[i];
	}
}

ISR(TWIE_TWIS_vect) {
	TWI_SlaveInterruptHandler(&g_twi1_slave);
}
#endif

#ifdef TWI2_SLAVE
static void twi2_slave_process(void) {
	int i;

	for(i = 0; i < TWI_DATA_LENGTH; i++) {
		g_twi2_recv_data[i] = g_twi2_slave.receivedData[i];
	}
}

ISR(TWIC_TWIS_vect) {
	TWI_SlaveInterruptHandler(&g_twi2_slave);
}
#endif


inline
static int16_t calc_gyro1_accel_raw2mg(int16_t raw, int16_t factor)
{
	return (((1000 * TWI1_SLAVE_GYRO_DTA_1_ACCEL_CONFIG__02G) * (int64_t)raw * (int64_t)factor) / 10000LL) >> 15;
}

inline
static int32_t calc_gyro1_gyro_raw2mdps(int16_t raw)
{
	return (int32_t) (((1000L * TWI1_SLAVE_GYRO_DTA_1_GYRO_CONFIG__0250DPS) * (int64_t)raw) >> 15);
}

inline
static int16_t calc_gyro1_temp_raw2C100(int16_t raw)
{
	return (int16_t) (((100L * (int32_t)(raw - g_twi1_gyro_1_temp_RTofs)) / (int32_t)g_twi1_gyro_1_temp_sens) + 2100);
}

inline
static int32_t calc_gyro2_correct_mag_2_nT(int16_t raw, int8_t asa, int16_t factor)
{
	if (raw >= 0) {
		//                                         asa decoding
		//                                                            rounding
		//                                                                    /256
		return (((int32_t)raw * factor * ((int32_t)asa + 128) / 10) + 128) >> 8;
	} else {
		return (((int32_t)raw * factor * ((int32_t)asa + 128) / 10) - 128) >> 8;
	}
}


bool twi2_waitUntilReady(bool retry)
{
	const uint8_t c_cntNum = 255;
	static bool s_LCD_offline = false;
	static bool s_lock = false;
	status_code_t status;
	uint8_t isBusy;
	uint8_t isValid;
	uint32_t l_timeout;
	uint8_t l_cnt;

	if (!s_lock) {
		/* Quick pre-check */
		if (s_LCD_offline && !retry) {
			return false;

		} else if (s_LCD_offline && retry) {
			s_LCD_offline = false;
			s_lock = true;
			start_twi2_lcd();
			s_lock = false;
		}
	}

	/* Calculate timeout value */
	l_timeout = TWI2_DISPLAY_TIMEOUT_MS + tcc1_get_time();

	g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_GET_STATE;
	g_twi2_packet.addr_length = 1;

	/* Wait until not BUSY */
	l_cnt = c_cntNum;
	do {
		g_twi2_packet.length = 1;
		status = twi_master_read(&TWI2_MASTER, &g_twi2_packet);
		isValid = g_twi2_m_data[0] & 0x80;
		isBusy  = g_twi2_m_data[0] & 0x01;

		if ((status != STATUS_OK) || !isValid) {
			delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
		} else {
			if (isBusy && isValid) {
				delay_us(TWI_SMART_LCD_DEVICE_BUSY_DELAY_MIN_US);
			}
		}

		if (!--l_cnt) {
			l_cnt = c_cntNum;

			/* Timeout check - expensive operation */
			if (l_timeout <= tcc1_get_time()) {
				s_LCD_offline = true;
				return false;
			}
		}
	} while ((status != STATUS_OK) || !isValid || isBusy);

	return true;
}


/* Setters for the interpreter */

void twi2_set_leds(uint8_t leds)
{
	if (twi2_waitUntilReady(false)) {
		/* Show green LED */
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_LEDS;
		g_twi2_m_data[0] = leds & 0x03;
		g_twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void twi2_set_ledbl(uint8_t mode, uint8_t pwm)
{
	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_BACKLIGHT;
		g_twi2_m_data[0] = mode;
		g_twi2_m_data[1] = pwm;
		g_twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void twi2_set_bias(uint8_t bias)
{
	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_CONTRAST;
		g_twi2_m_data[0] = bias;
		g_twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void twi2_set_beep(uint8_t pitch_10hz, uint8_t len_10ms)
{
	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_BEEP;
		g_twi2_m_data[0] = len_10ms;
		g_twi2_m_data[1] = pitch_10hz;
		g_twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}


/* INIT functions */

static void init_twi1_hygro(void)
{
	status_code_t sc;

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_HYGRO_01, TWI1_SLAVE_HYGRO_ADDR);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	g_twi1_hygro_status = 0;

	do {
		/* SHT31-DIS hygro: stop any running jobs */
		g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_BREAK_HI;
		g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_BREAK_LO;
		g_twi1_packet.addr_length = 2;
		g_twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_HYGRO_02);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
			break;
		}
		delay_ms(2);

		/* SHT31-DIS hygro: reset */
		g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_RESET_HI;
		g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_RESET_LO;
		g_twi1_packet.addr_length = 2;
		g_twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(2);

		/* SHT31-DIS hygro: return current status */
		g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_STATUS_HI;
		g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_STATUS_LO;
		g_twi1_packet.addr_length = 2;
		g_twi1_packet.length = 2;
		sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_hygro_status = (g_twi1_m_data[0] << 8) | g_twi1_m_data[1];
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_HYGRO_03, g_twi1_hygro_status);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

		/* SHT31-DIS hygro: start next measurement */
		g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_ONESHOT_HIPREC_NOCLKSTRETCH_HI;
		g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_ONESHOT_HIPREC_NOCLKSTRETCH_LO;
		g_twi1_packet.addr_length = 2;
		g_twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		g_twi1_hygro_valid = true;

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_HYGRO_OK);
		task_twi2_lcd_str(8, 3 * 10, g_prepare_buf);

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_HYGRO_04);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
		return;
	} while(false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_HYGRO_05, sc);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}

status_code_t twi1_gyro_gyro_offset_set(void)
{
	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_GYRO_XG_OFFSET_H;
	g_twi1_packet.addr_length = 1;
	g_twi1_m_data[0] = (uint8_t) (g_twi1_gyro_1_gyro_ofsx >> 8);
	g_twi1_m_data[1] = (uint8_t) (g_twi1_gyro_1_gyro_ofsx & 0xFF);
	g_twi1_m_data[2] = (uint8_t) (g_twi1_gyro_1_gyro_ofsy >> 8);
	g_twi1_m_data[3] = (uint8_t) (g_twi1_gyro_1_gyro_ofsy & 0xFF);
	g_twi1_m_data[4] = (uint8_t) (g_twi1_gyro_1_gyro_ofsz >> 8);
	g_twi1_m_data[5] = (uint8_t) (g_twi1_gyro_1_gyro_ofsz & 0xFF);
	g_twi1_packet.length = 6;
	return twi_master_write(&TWI1_MASTER, &g_twi1_packet);
}

status_code_t twi1_gyro_accel_offset_set(void)
{
	status_code_t sc;

	do {
			g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
			g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_XA_OFFSET_H;
			g_twi1_packet.addr_length = 1;
			g_twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsx & 0x7F80) >> 7);
			g_twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsx &   0x7F) << 1);
			g_twi1_packet.length = 2;
			sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
			if (sc != STATUS_OK) {
				break;
			}

			g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_YA_OFFSET_H;
			g_twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsy & 0x7F80) >> 7);
			g_twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsy &   0x7F) << 1);
			sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
			if (sc != STATUS_OK) {
				break;
			}

			g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ZA_OFFSET_H;
			g_twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsz & 0x7F80) >> 7);
			g_twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsz &   0x7F) << 1);
			sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
	} while (false);

	return sc;
}

void init_twi1_gyro(void)
{
	status_code_t sc;

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_GYRO_01, TWI1_SLAVE_GYRO_ADDR_1, TWI1_SLAVE_GYRO_ADDR_2);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	g_twi1_gyro_1_version = 0;
	g_twi1_gyro_2_version = 0;

	do {
		/* MPU-9250 6 axis: RESET */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_PWR_MGMT_1;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_PWR_MGMT_1__HRESET | TWI1_SLAVE_GYRO_DTA_1_PWR_MGMT_1__CLKSEL_VAL;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_GYRO_02);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
			break;
		}
		delay_ms(10);

		/* MPU-9250 6 axis: read Who Am I control value */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_WHOAMI;
		g_twi1_packet.addr_length = 1;
		g_twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_1_version = g_twi1_m_data[0];

		/* MPU-9250 6 axis: I2C bypass on to access the Magnetometer chip */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_INT_PIN_CFG;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_INT_PIN_CFG__BYPASS_EN;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* Magnetometer: soft reset */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL2;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL2__SRST;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		/* Magnetometer: read Device ID */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_WIA;
		g_twi1_packet.addr_length = 1;
		g_twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_2_version = g_twi1_m_data[0];

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_GYRO_03, g_twi1_gyro_1_version, g_twi1_gyro_2_version);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

		/* Magnetometer: 16 bit access and prepare for PROM access */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_PROM_VAL;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* Magnetometer: read correction data for X, Y and Z */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ASAX;
		g_twi1_packet.addr_length = 1;
		g_twi1_packet.length = 3;
		sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_2_asax = g_twi1_m_data[0];
		g_twi1_gyro_2_asay = g_twi1_m_data[1];
		g_twi1_gyro_2_asaz = g_twi1_m_data[2];

		/* Magnetometer: mode change via power-down mode */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_16B_POWER_DOWN;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		/* Magnetometer: mode change for 16bit and run all axis at 8 Hz */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_16B_RUN_8HZ_VAL;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: GYRO set offset values */
		sc = twi1_gyro_gyro_offset_set();
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: ACCEL set offset values */
		sc = twi1_gyro_accel_offset_set();
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: FIFO frequency = 10 Hz */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_SMPLRT_DIV;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = 99;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: GYRO Bandwidth = 5 Hz, Fs = 1 kHz */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_CONFIG;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = 6;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: ACCEL Bandwidth = 5 Hz, Fs = 1 kHz */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ACCEL_CONFIG2;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = 6;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: Wake On Motion interrupt = 0.1 g (1 LSB = 4 mg) */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_WOM_THR;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = 25;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: RESET all internal data paths */
		g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_USER_CTRL;
		g_twi1_packet.addr_length = 1;
		g_twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_USER_CTRL__SIG_COND_RST;  // | TWI1_SLAVE_GYRO_DTA_1_USER_CTRL__FIFO_EN;
		g_twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		g_twi1_gyro_valid = true;

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_GYRO_OK);
		task_twi2_lcd_str(8, 4 * 10, g_prepare_buf);

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_GYRO_04);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
		return;
	} while(false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_GYRO_05, sc);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}

static void init_twi1_baro(void)
{
	status_code_t sc;

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_01, TWI1_SLAVE_BARO_ADDR);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	do {
		/* MS560702BA03-50 Baro: RESET all internal data paths */
		g_twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_RESET;
		g_twi1_packet.addr_length = 1;
		g_twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(3);

		/* MS560702BA03-50 Baro: get version information */
		g_twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
		g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_VERSION;
		g_twi1_packet.addr_length = 1;
		g_twi1_packet.length = 2;
		sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
		if (sc != STATUS_OK) {
			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_02, sc);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
			break;
		}
		g_twi1_baro_version = (((uint16_t)g_twi1_m_data[0] << 8) | (uint16_t)g_twi1_m_data[1]) >> 4;
		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_03, g_twi1_baro_version);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

		/* MS560702BA03-50 Baro: get correction data from the PROM */
		for (int adr = 1; adr < C_TWI1_BARO_C_CNT; ++adr) {
			g_twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
			g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_PROM | (adr << 1);
			g_twi1_packet.addr_length = 1;
			g_twi1_packet.length = 2;
			sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
			if (sc != STATUS_OK) {
				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_04, adr, sc);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
				break;
			}
			g_twi1_baro_c[adr] = (g_twi1_m_data[0] << 8) | g_twi1_m_data[1];
		}

		g_twi1_baro_valid = true;

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_BARO_OK);
		task_twi2_lcd_str(8, 5 * 10, g_prepare_buf);

		len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_05);
		udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
		return;
	} while(false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_BARO_06, sc);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}


/* TWI1 - GSM, Gyro, Baro, Hygro devices */

static void start_twi1_onboard(void)
{
	/* Booting screen with header */
	task_twi2_lcd_header();

	/* Device Hygro SHT31-DIS - I2C address: 0x44 */
	init_twi1_hygro();

	/* Device Gyro MPU-9250 - I2C address: 0x68, 0x0C (or 0x0D or 0x0E or 0x0F) */
	init_twi1_gyro();

	/* Device Baro MS560702BA03-50 - I2C address: 0x76 */
	init_twi1_baro();

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_TWI1_INIT_ONBOARD_01);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

#if 0 /*HERE*/
	/* Calibration of TWI1 devices */
	{
		irqflags_t flags;

		/* Assuming the board is not rotating during calibration of the GYRO */
		calibration_mode(CALIBRATION_MODE_ENUM__GYRO);

		flags = cpu_irq_save();
		if (((-100 < g_twi1_gyro_1_accel_x_mg) && (g_twi1_gyro_1_accel_x_mg <  100)) &&
		    ((-100 < g_twi1_gyro_1_accel_y_mg) && (g_twi1_gyro_1_accel_y_mg <  100)) &&
		    (( 900 < g_twi1_gyro_1_accel_z_mg) && (g_twi1_gyro_1_accel_z_mg < 1100))) {
			/* Do calibrate ACCEL-Z only when board is placed horizontally */
			cpu_irq_restore(flags);
			calibration_mode(CALIBRATION_MODE_ENUM__ACCEL_Z);

		} else {
			cpu_irq_restore(flags);
		}
	}
#endif
}

/* TWI2 - LCD Port */
void start_twi2_lcd(void)
{
	/* Read the version number */
	g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_GET_VER;
	g_twi2_packet.addr_length = 1;
	g_twi2_packet.length = 1;
	twi_master_read(&TWI2_MASTER, &g_twi2_packet);
	g_twi2_lcd_version = g_twi2_m_data[0];

	if (g_twi2_lcd_version >= 0x11) {
		/* Firstly drop backlight */
		twi2_set_ledbl(0, 0);

		/* Select "Smart-LCD draw box" mode
		 * that includes a clear screen     */
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_MODE;
		g_twi2_m_data[0] = 0x10;
		g_twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);

		/* Reset the LCD to bring it up in case of a bad POR */
		twi2_waitUntilReady(true);
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_RESET;
		g_twi2_packet.length = 0;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_ms(50);

		/* LED red */
		twi2_set_leds(0x01);

		/* Set the pixel type to SET pixels */
		twi2_waitUntilReady(false);
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_PIXEL_TYPE;
		g_twi2_m_data[0] = GFX_PIXEL_SET;
		g_twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);

		/* Set optimum contrast voltage */
		twi2_set_bias(g_bias_pm);

		/* Do the first beep for a sequence */
		twi2_set_beep(44, 25);  // 440 Hz, 250 ms
		delay_ms(300);

		/* Ramp up backlight */
		for (uint8_t lum = 0; lum <= 251U; lum += 4) {
			twi2_set_ledbl(0, lum);
			delay_ms(1);
		}

		/* Do the second beep for a sequence */
		twi2_set_beep(88, 25);  // 880 Hz, 250 ms
		delay_ms(300);

		/* Ramp down backlight */
		uint8_t l_my_pwm = g_backlight_mode_pwm & 0xff;
		for (uint8_t lum = 255U; lum >= l_my_pwm; lum -= 4) {
			twi2_set_ledbl(0, lum);
			delay_ms(1);
		}

		/* Set the backlight level exactly */
		twi2_set_ledbl(0, l_my_pwm);
	}
}


void twi_init(void) {
	#ifdef TWI1_MASTER
	TWI1_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;  // SDA1
	TWI1_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;  // SCL1
	#endif

	#ifdef TWI2_MASTER
	TWI2_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;  // SDA2
	TWI2_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;  // SCL2
	#endif
}

void twi_start(void) {
	#ifdef TWI1_SLAVE
	uint8_t i1;

	sysclk_enable_peripheral_clock(&TWI1_SLAVE);
	TWI_SlaveInitializeDriver(&g_twi1_slave, &TWI1_SLAVE, *twi1_slave_process);
	TWI_SlaveInitializeModule(&g_twi1_slave, TWI1_SLAVE_ADDR,	TWI_SLAVE_INTLVL_MED_gc);

	for (i1 = 0; i1 < TWIS_SEND_BUFFER_SIZE; i1++) {
		g_twi1_slave.receivedData[i1] = 0;
	}
	#else
	#ifdef TWI1_MASTER
	sysclk_enable_peripheral_clock(&TWI1_MASTER);
	twi_master_init(&TWI1_MASTER, &g_twi1_options);
	twi_master_enable(&TWI1_MASTER);
	#endif
	#endif

	#ifdef TWI2_SLAVE
	uint8_t i2;

	sysclk_enable_peripheral_clock(&TWI2_SLAVE);
	TWI_SlaveInitializeDriver(&g_twi2_slave, &TWI2_SLAVE, *twi2_slave_process);
	TWI_SlaveInitializeModule(&g_twi2_slave, TWI2_SLAVE_ADDR,	TWI_SLAVE_INTLVL_MED_gc);

	for (i2 = 0; i2 < TWIS_SEND_BUFFER_SIZE; i2++) {
		g_twi2_slave.receivedData[i2] = 0;
	}
	#else
	#ifdef TWI2_MASTER
	sysclk_enable_peripheral_clock(&TWI2_MASTER);
	twi_master_init(&TWI2_MASTER, &g_twi2_options);
	twi_master_enable(&TWI2_MASTER);
	#endif
	#endif

	delay_ms(1000);											// Give Smart-LCD some time being up and ready
	start_twi2_lcd();

	/* Start each TWI channel devices */
	start_twi1_onboard();
}


static bool service_twi1_hygro(bool sync)
{
	/* Real time usage: < 1 ms */

	/* No spare-time handling in use */
	if (!sync) {
		return false;
	}

	/* Our friend Baro takes the bus */
	if (g_twi1_lock) {
		return false;
	}

	/* Read current measurement data */
	g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
	g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_FETCH_DATA_HI;
	g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_FETCH_DATA_LO;
	g_twi1_packet.addr_length = 2;
	g_twi1_packet.length = 5;
	status_code_t sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc == STATUS_OK) {
		g_twi1_hygro_S_T	= ((uint16_t)g_twi1_m_data[0] << 8) | g_twi1_m_data[1];
		g_twi1_hygro_S_RH	= ((uint16_t)g_twi1_m_data[3] << 8) | g_twi1_m_data[4];
	}

	/* Start next measurement - available 15ms later */
	g_twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
	g_twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_ONESHOT_HIPREC_NOCLKSTRETCH_HI;
	g_twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_ONESHOT_HIPREC_NOCLKSTRETCH_LO;
	g_twi1_packet.addr_length = 2;
	g_twi1_packet.length = 0;
	sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
	if (sc == STATUS_OK) {
		return true;
	}
	return false;
}

bool service_twi1_gyro(bool sync)
{
	/* Real time usage: abt. 1 ms */

	/* No spare-time handling in use */
	if (!sync) {
		return false;
	}

	/* Our friend Baro takes the bus */
	if (g_twi1_lock) {
		return false;
	}

	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ACCEL_XOUT_H;		// Starting with this address (big endian)
	g_twi1_packet.addr_length = 1;
	g_twi1_packet.length = 8;											// Auto incrementation
	status_code_t sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc != STATUS_OK) {
		return false;
	}
	g_twi1_gyro_1_accel_x = (int16_t) (((uint16_t)g_twi1_m_data[0] << 8) | g_twi1_m_data[1]);
	g_twi1_gyro_1_accel_y = (int16_t) (((uint16_t)g_twi1_m_data[2] << 8) | g_twi1_m_data[3]);
	g_twi1_gyro_1_accel_z = (int16_t) (((uint16_t)g_twi1_m_data[4] << 8) | g_twi1_m_data[5]);
	g_twi1_gyro_1_temp    = (int16_t) (((uint16_t)g_twi1_m_data[6] << 8) | g_twi1_m_data[7]);

	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_GYRO_XOUT_H;
	g_twi1_packet.addr_length = 1;
	g_twi1_packet.length = 6;
	sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc != STATUS_OK) {
		return false;
	}
	g_twi1_gyro_1_gyro_x = (int16_t) (((uint16_t)g_twi1_m_data[0] << 8) | g_twi1_m_data[1]);
	g_twi1_gyro_1_gyro_y = (int16_t) (((uint16_t)g_twi1_m_data[2] << 8) | g_twi1_m_data[3]);
	g_twi1_gyro_1_gyro_z = (int16_t) (((uint16_t)g_twi1_m_data[4] << 8) | g_twi1_m_data[5]);

	/* Do update GYRO offset registers */
	if (g_twi1_gyro_gyro_offset_set__flag) {
		g_twi1_gyro_gyro_offset_set__flag = false;
		(void) twi1_gyro_gyro_offset_set();
	}

	/* Do update ACCEL offset registers */
	if (g_twi1_gyro_accel_offset_set__flag) {
		g_twi1_gyro_accel_offset_set__flag = false;
		(void) twi1_gyro_accel_offset_set();
	}

	/* Magnetometer: check if new data is available */
	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ST1;
	g_twi1_packet.addr_length = 1;
	g_twi1_packet.length = 1;
	sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc != STATUS_OK) {
		return false;
	}
	if (!(g_twi1_m_data[0] & TWI1_SLAVE_GYRO_DTA_2_ST1__DRDY)) {
		/* Data of Magnetometer AK8963 not ready yet */
		return false;
	}

	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_HX_L;			// Starting with this address (little endian)
	g_twi1_packet.addr_length = 1;
	g_twi1_packet.length = 6;										// Auto incrementation
	sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc != STATUS_OK) {
		return false;
	}
	g_twi1_gyro_2_mag_x = ((int16_t) ((((uint16_t)g_twi1_m_data[1]) << 8) | g_twi1_m_data[0])) + g_twi1_gyro_2_ofsx;
	g_twi1_gyro_2_mag_y = ((int16_t) ((((uint16_t)g_twi1_m_data[3]) << 8) | g_twi1_m_data[2])) + g_twi1_gyro_2_ofsy;
	g_twi1_gyro_2_mag_z = ((int16_t) ((((uint16_t)g_twi1_m_data[5]) << 8) | g_twi1_m_data[4])) + g_twi1_gyro_2_ofsz;

	/* Magnetometer: check for data validity and release cycle */
	g_twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
	g_twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ST2;
	g_twi1_packet.addr_length = 1;
	g_twi1_packet.length = 1;
	sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
	if (sc != STATUS_OK) {
		return false;
	}
	if (g_twi1_m_data[0] & TWI1_SLAVE_GYRO_DTA_2_ST2__HOFL) {
		/* Data of Magnetometer AK8963 overflowed */
		g_twi1_gyro_2_mag_z = g_twi1_gyro_2_mag_y = g_twi1_gyro_2_mag_x = 0;
		//return true;	// Even overflowed data is correct information
	}
	return true;
}

static bool service_twi1_baro(bool sync)
{
	/* Real time usage: abt. 22 ms */

	static uint8_t  s_step = 100;								// FSM: stopped mode
	static uint32_t s_twi1_baro_d1 = 0UL;
	static uint32_t s_twi1_baro_d2 = 0UL;

	/* Spare-time handling is in use: finite state machine (FSM) follows */

	/* Restart a new cycle if ready */
	if (sync && (s_step >= 100)) {
		s_step = 0;
		g_twi1_lock = true;
	}

	switch (s_step) {
		case 0:
			/* Request D1 */
			g_twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
			g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_CONV_D1_4096;
			g_twi1_packet.addr_length = 1;
			g_twi1_packet.length = 0;
			status_code_t sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
			if (sc == STATUS_OK) {
				s_step = 1;
				return false;
			}

			s_step = 200;										// Failed, stay until new sync triggers
			return false;
		break;

		case 21:
			/* Get data */
			g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_ADC_READ;
			g_twi1_packet.length = 3;
			sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
			if (sc == STATUS_OK) {
				s_twi1_baro_d1 = ((uint32_t)g_twi1_m_data[0] << 16) | ((uint32_t)g_twi1_m_data[1] << 8) | g_twi1_m_data[2];

				/* Request D2 */
				g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_CONV_D2_4096;
				g_twi1_packet.length = 0;
				sc = twi_master_write(&TWI1_MASTER, &g_twi1_packet);
				if (sc == STATUS_OK) {
					s_step = 22;
					return false;
				}
			}

			s_step = 211;										// Failed, stay until new sync triggers
			return false;
		break;

		case 43:
			g_twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_ADC_READ;
			g_twi1_packet.length = 3;
			sc = twi_master_read(&TWI1_MASTER, &g_twi1_packet);
			if (sc == STATUS_OK) {
				s_twi1_baro_d2 = ((uint32_t)g_twi1_m_data[0] << 16) | ((uint32_t)g_twi1_m_data[1] << 8) | g_twi1_m_data[2];

				/* Setting the global values */
				{
					irqflags_t flags = cpu_irq_save();
					g_twi1_baro_d1					= s_twi1_baro_d1;
					g_twi1_baro_d2					= s_twi1_baro_d2;
					cpu_irq_restore(flags);
				}

				s_step = 123;									// Success, stay until new sync triggers
				g_twi1_lock = false;
				return true;
			}

			s_step = 223;										// Failed, stay until new sync triggers
			return false;
		break;

		default:
			/* Delay step of 0.5 ms */
			if (s_step < 100) {
				s_step++;

			} else {
				g_twi1_lock = false;
			}
	}
	return false;
}


/* 10ms TWI1 - Gyro device */
void isr_10ms_twi1_onboard(void)
{	/* Service time slot */
	// not in use yet
}

/* 100ms TWI1 - Gyro device */
void isr_100ms_twi1_onboard(void)
{	/* Service time slot */
	cpu_irq_enable();

	if (g_twi1_gyro_valid) {
		if (service_twi1_gyro(true)) {
			sched_push(task_twi1_gyro, SCHED_ENTRY_CB_TYPE__LISTTIME, 0, true, false, false);
		}
	}
}

/* 500ms TWI1 - Baro, Hygro devices */
void isr_500ms_twi1_onboard(void)
{	/* Service time slot */
	cpu_irq_enable();

	if (g_twi1_hygro_valid) {
		if (service_twi1_hygro(true)) {
			sched_push(task_twi1_hygro, SCHED_ENTRY_CB_TYPE__LISTTIME,  70, true, false, false);
		}
	}

	if (g_twi1_baro_valid) {
		service_twi1_baro(true);
	}
}

/* 2560 cycles per second */
void isr_sparetime_twi1_onboard(void)
{	/* Service time slot */
	cpu_irq_enable();

	if (g_twi1_baro_valid) {
		if (service_twi1_baro(false)) {
			/* Every 500ms */
			sched_push(task_twi1_baro, SCHED_ENTRY_CB_TYPE__LISTTIME, 70, true, false, false);
		}
	}
}

static void task_twi1_hygro(void)
{	// Calculations for the presentation layer
	static uint16_t s_twi1_hygro_S_T	= 0UL;
	static uint16_t s_twi1_hygro_S_RH	= 0UL;
	int16_t l_twi1_hygro_T_100, l_twi1_hygro_RH_100;
	uint16_t l_twi1_hygro_S_T, l_twi1_hygro_S_RH;
	bool hasChanged = false;

	/* Getting the global values */
	{
		irqflags_t flags = cpu_irq_save();
		l_twi1_hygro_T_100	= g_twi1_hygro_T_100;	// last value
		l_twi1_hygro_RH_100	= g_twi1_hygro_RH_100;	// last value
		l_twi1_hygro_S_T	= g_twi1_hygro_S_T;
		l_twi1_hygro_S_RH	= g_twi1_hygro_S_RH;
		cpu_irq_restore(flags);
	}

	/* Calculate and present Temp value when a different measurement has arrived */
	int16_t temp_100 = l_twi1_hygro_T_100;
	if (s_twi1_hygro_S_T != l_twi1_hygro_S_T) {
		temp_100 = (int16_t)((((int32_t)l_twi1_hygro_S_T  * 17500) / 0xFFFF) - 4500);

		/* Setting the global value */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_hygro_T_100 = temp_100;
			cpu_irq_restore(flags);
		}

		hasChanged = true;
		s_twi1_hygro_S_T = l_twi1_hygro_S_T;
	}

	/* Calculate and present Hygro value when a different measurement has arrived */
	int16_t rh_100 = l_twi1_hygro_RH_100;
	if (s_twi1_hygro_S_RH != l_twi1_hygro_S_RH) {
		rh_100 = (int16_t)( ((int32_t)l_twi1_hygro_S_RH * 10000) / 0xFFFF);

		/* Setting the global value */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_hygro_RH_100 = rh_100;
			cpu_irq_restore(flags);
		}

		hasChanged = true;
		s_twi1_hygro_S_RH = l_twi1_hygro_S_RH;
	}

	/* Calculate the dew point temperature */
	/* @see https://de.wikipedia.org/wiki/Taupunkt  formula (15) */
	if (hasChanged)
	{
		//const float K1	= 6.112f;
		const float K2		= 17.62f;
		const float K3		= 243.12f;
		const float K2_m_K3 = 4283.7744f;	// = K2 * K3;
		float ln_phi		= log(rh_100 / 10000.f);
		float k2_m_theta	= K2 * (temp_100 / 100.f);
		float k3_p_theta	= K3 + (temp_100 / 100.f);
		float term_z		= k2_m_theta / k3_p_theta + ln_phi;
		float term_n		= K2_m_K3    / k3_p_theta - ln_phi;
		float tau_100		= 0.5f + ((100.f * K3) * term_z) / term_n;

		/* Setting the global value */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_hygro_DP_100 = (int16_t)tau_100;
			cpu_irq_restore(flags);
		}
	}
}

static void task_twi1_gyro(void)
{	// Calculations for the presentation layer
	{
		int16_t l_twi1_gyro_1_accel_x, l_twi1_gyro_1_accel_y, l_twi1_gyro_1_accel_z;
		int16_t l_twi1_gyro_1_accel_factx, l_twi1_gyro_1_accel_facty, l_twi1_gyro_1_accel_factz;

		/* Getting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_gyro_1_accel_x					= g_twi1_gyro_1_accel_x;
			l_twi1_gyro_1_accel_y					= g_twi1_gyro_1_accel_y;
			l_twi1_gyro_1_accel_z					= g_twi1_gyro_1_accel_z;

			l_twi1_gyro_1_accel_factx				= g_twi1_gyro_1_accel_factx;
			l_twi1_gyro_1_accel_facty				= g_twi1_gyro_1_accel_facty;
			l_twi1_gyro_1_accel_factz				= g_twi1_gyro_1_accel_factz;
			cpu_irq_restore(flags);
		}

		int16_t l_twi1_gyro_1_accel_x_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_x, l_twi1_gyro_1_accel_factx);
		int16_t l_twi1_gyro_1_accel_y_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_y, l_twi1_gyro_1_accel_facty);
		int16_t l_twi1_gyro_1_accel_z_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_z, l_twi1_gyro_1_accel_factz);

		/* Setting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_gyro_1_accel_x_mg				= l_twi1_gyro_1_accel_x_mg;
			g_twi1_gyro_1_accel_y_mg				= l_twi1_gyro_1_accel_y_mg;
			g_twi1_gyro_1_accel_z_mg				= l_twi1_gyro_1_accel_z_mg;
			cpu_irq_restore(flags);
		}
	}

	{
		int16_t l_twi1_gyro_1_gyro_x, l_twi1_gyro_1_gyro_y, l_twi1_gyro_1_gyro_z;

		/* Getting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_gyro_1_gyro_x					= g_twi1_gyro_1_gyro_x;
			l_twi1_gyro_1_gyro_y					= g_twi1_gyro_1_gyro_y;
			l_twi1_gyro_1_gyro_z					= g_twi1_gyro_1_gyro_z;
			cpu_irq_restore(flags);
		}

		int32_t l_twi1_gyro_1_gyro_x_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_x);
		int32_t l_twi1_gyro_1_gyro_y_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_y);
		int32_t l_twi1_gyro_1_gyro_z_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_z);

		/* Setting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_gyro_1_gyro_x_mdps				= l_twi1_gyro_1_gyro_x_mdps;
			g_twi1_gyro_1_gyro_y_mdps				= l_twi1_gyro_1_gyro_y_mdps;
			g_twi1_gyro_1_gyro_z_mdps				= l_twi1_gyro_1_gyro_z_mdps;
			cpu_irq_restore(flags);
		}
	}

	{
		int16_t l_twi1_gyro_2_mag_x, l_twi1_gyro_2_mag_y, l_twi1_gyro_2_mag_z;
		int16_t l_twi1_gyro_2_asax, l_twi1_gyro_2_asay, l_twi1_gyro_2_asaz;
		int16_t l_twi1_gyro_2_mag_factx, l_twi1_gyro_2_mag_facty, l_twi1_gyro_2_mag_factz;
		int16_t l_twi1_gyro_1_temp;

		/* Getting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_gyro_2_mag_x						= g_twi1_gyro_2_mag_x;
			l_twi1_gyro_2_mag_y						= g_twi1_gyro_2_mag_y;
			l_twi1_gyro_2_mag_z						= g_twi1_gyro_2_mag_z;

			l_twi1_gyro_2_asax						= g_twi1_gyro_2_asax;
			l_twi1_gyro_2_asay						= g_twi1_gyro_2_asay;
			l_twi1_gyro_2_asaz						= g_twi1_gyro_2_asaz;

			l_twi1_gyro_2_mag_factx					= g_twi1_gyro_2_mag_factx;
			l_twi1_gyro_2_mag_facty					= g_twi1_gyro_2_mag_facty;
			l_twi1_gyro_2_mag_factz					= g_twi1_gyro_2_mag_factz;

			l_twi1_gyro_1_temp						= g_twi1_gyro_1_temp;
			cpu_irq_restore(flags);
		}

		int32_t l_twi1_gyro_2_mag_x_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_x, l_twi1_gyro_2_asax, l_twi1_gyro_2_mag_factx);
		int32_t l_twi1_gyro_2_mag_y_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_y, l_twi1_gyro_2_asay, l_twi1_gyro_2_mag_facty);
		int32_t l_twi1_gyro_2_mag_z_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_z, l_twi1_gyro_2_asaz, l_twi1_gyro_2_mag_factz);
		int16_t	l_twi1_gyro_1_temp_deg_100	= calc_gyro1_temp_raw2C100(l_twi1_gyro_1_temp);

		/* Setting the global values */
		{
			irqflags_t flags = cpu_irq_save();
			g_twi1_gyro_2_mag_x_nT					= l_twi1_gyro_2_mag_x_nT;
			g_twi1_gyro_2_mag_y_nT					= l_twi1_gyro_2_mag_y_nT;
			g_twi1_gyro_2_mag_z_nT					= l_twi1_gyro_2_mag_z_nT;
			g_twi1_gyro_1_temp_deg_100				= l_twi1_gyro_1_temp_deg_100;
			cpu_irq_restore(flags);
		}
	}
}

static void task_twi1_baro(void)
{	// Calculations for the presentation layer
	static uint32_t s_twi1_baro_d1	= 0UL;
	static uint32_t s_twi1_baro_d2	= 0UL;
	static int32_t	s_p				= 0L;
	uint32_t		l_twi1_baro_d1, l_twi1_baro_d2;
	int32_t			l_p_h;

	/* Getting the global values */
	{
		irqflags_t flags = cpu_irq_save();
		l_twi1_baro_d1								= g_twi1_baro_d1;
		l_twi1_baro_d2								= g_twi1_baro_d2;
		cpu_irq_restore(flags);
	}

	/* Calculate and present Baro and Temp values when a different measurement has arrived */
	if ((l_twi1_baro_d1 != s_twi1_baro_d1) || (l_twi1_baro_d2 != s_twi1_baro_d2)) {
		int32_t dT = (int32_t)l_twi1_baro_d2 - ((int32_t)g_twi1_baro_c[5] << 8);
		int32_t temp_p20 = (int32_t)(((int64_t)dT * g_twi1_baro_c[6]) >> 23);
		int32_t temp = temp_p20 + 2000L;
		int64_t off  = ((int64_t)g_twi1_baro_c[2] << 17) + (((int64_t)g_twi1_baro_c[4] * dT) >> 6);
		int64_t sens = ((int64_t)g_twi1_baro_c[1] << 16) + (((int64_t)g_twi1_baro_c[3] * dT) >> 7);

		/* Low temp and very low temp corrections */
		if (temp < 2000L) {
			int32_t t2 = (int32_t)(((int64_t)dT * (int64_t)dT) >> 31);
			int32_t temp_p20_2 = temp_p20 * temp_p20;
			int32_t off2 = (61 * temp_p20_2) >> 4;
			int32_t sens2 = temp_p20_2 << 1;

			if (temp < -1500L) {
				int32_t temp_m15 = temp + 1500L;
				int32_t temp_m15_2 = temp_m15 * temp_m15;
				off2  += 15 * temp_m15_2;
				sens2 +=  8 * temp_m15_2;
			}
			temp -= t2;
			off  -= off2;
			sens -= sens2;
		}
		int32_t l_p = (int32_t)((((l_twi1_baro_d1 * sens) >> 21) - off) >> 15);

		/* Store data and calculate QNH within valid data range, only */
		if ((-3000 < temp) && (temp < 8000) && (30000L < l_p) && (l_p < 120000L)) {
			/* Setting the global values */
			{
				irqflags_t flags = cpu_irq_save();
				g_twi1_baro_temp_100	= temp;
				g_twi1_baro_p_100		= l_p;
				cpu_irq_restore(flags);
			}

			irqflags_t flags = cpu_irq_save();
			int16_t l_qnh_height_m			= g_qnh_height_m;
			int32_t l_twi1_baro_temp_100	= g_twi1_baro_temp_100;
			cpu_irq_restore(flags);

			/* A valid height value (3D navigation) seems to be access able */
			if (s_p != l_p) {
				float a_m_h		= 0.0065f * l_qnh_height_m;
				float Th0		= C_0DEGC_K + (l_twi1_baro_temp_100 / 100.f);
				float term		= 1.f + (a_m_h / Th0);
				l_p_h			= l_p * pow(term, 5.255f);

				/* Setting the global values */
				{
					irqflags_t flags = cpu_irq_save();
					g_qnh_p_h_100 = l_p_h;
					cpu_irq_restore(flags);
				}

				s_p = l_p;
			}
		}
	}
}

/* TWI1 - onboard devices */
// hint: not called anymore, now done by  isr_500ms_twi1_onboard()
#if 0
static void task_twi1_onboard(void)
{
	if (g_twi1_hygro_valid) {
		task_twi1_hygro();
	}

	if (g_twi1_gyro_valid) {
		task_twi1_gyro();
	}

	if (g_twi1_baro_valid) {
		task_twi1_baro();
	}
}
#endif


void task_twi2_lcd_reset(void)
{
	if (twi2_waitUntilReady(true)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_RESET;
		g_twi2_packet.length = 0;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_ms(50);
	}
}

void task_twi2_lcd_cls(void)
{
	if (twi2_waitUntilReady(true)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_CLS;
		g_twi2_packet.length = 0;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void task_twi2_lcd_pos_xy(uint8_t x, uint8_t y)
{
	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_POS_X_Y;
		g_twi2_m_data[0] = x;
		g_twi2_m_data[1] = y;
		g_twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void task_twi2_lcd_str(uint8_t x, uint8_t y, const char* str)
{
	uint8_t slen = strlen(str);
	if (!slen) {
		return;
	}

	if (twi2_waitUntilReady(false)) {
		while (slen) {
			uint8_t this_len = slen;
			if (this_len > TWI2_STR_MAXLEN) {
				this_len = TWI2_STR_MAXLEN;
			}

			/* Chunk of the string */
			{
				task_twi2_lcd_pos_xy(x, y);

				twi2_waitUntilReady(false);
				g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
				g_twi2_m_data[0] = this_len;
				for (uint8_t idx = 1; idx <= this_len; ++idx) {
					g_twi2_m_data[idx] = *(str++);
				}
				g_twi2_packet.length = this_len + 1;
				twi_master_write(&TWI2_MASTER, &g_twi2_packet);
				delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
			}

			x    += this_len * 6;
			slen -= this_len;
		}
	}
}

void task_twi2_lcd_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
	task_twi2_lcd_pos_xy(x1, y1);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_LINE;
		g_twi2_m_data[0] = x2;
		g_twi2_m_data[1] = y2;
		g_twi2_m_data[2] = color;
		g_twi2_packet.length = 3;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void task_twi2_lcd_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled, uint8_t color)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = filled ?  TWI_SMART_LCD_CMD_DRAW_FILLED_RECT : TWI_SMART_LCD_CMD_DRAW_RECT;
		g_twi2_m_data[0] = width;
		g_twi2_m_data[1] = height;
		g_twi2_m_data[2] = color;
		g_twi2_packet.length = 3;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void task_twi2_lcd_circ(uint8_t x, uint8_t y, uint8_t radius, bool filled, uint8_t color)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = filled ?  TWI_SMART_LCD_CMD_DRAW_FILLED_CIRC : TWI_SMART_LCD_CMD_DRAW_CIRC;
		g_twi2_m_data[0] = radius;
		g_twi2_m_data[1] = color;
		g_twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

void task_twi2_lcd_header(void)
{
	if (twi2_waitUntilReady(true)) {
		/* The header line */
		task_twi2_lcd_cls();
		task_twi2_lcd_str(6 * 10, 2, strcpy_P(g_prepare_buf, PM_TWIHEADER_FINDMESAT));
		task_twi2_lcd_str(6 * 30, 2, strcpy_P(g_prepare_buf, PM_TWIHEADER_BY_DF4IAH));

		/* A tiny satellite */
		task_twi2_lcd_circ( 9, 4, 3, true, 1);
		task_twi2_lcd_rect( 1, 2, 6, 4, false, 1);
		task_twi2_lcd_rect(12, 2, 6, 4, false, 1);

		/* Header line separator */
		task_twi2_lcd_line(0, 11, 239, 11, 1);
	}
}


static void task_twi2_lcd_template(void)
{
	uint8_t line;

	if (twi2_waitUntilReady(false)) {
		if (g_adc_enabled) {
			/* Left measurement names */
			line = 2;
			task_twi2_lcd_str(6 *  0, (line++) * 10 -4, strcpy_P(g_prepare_buf, PM_TWIINIT_DATE_TIME));
			task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_TEMP));
			task_twi2_lcd_str(6 *  3, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_UUSB));
			task_twi2_lcd_str(6 *  3, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_UBAT));
			task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_UVCTCXO));
			//task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_UIOADC4));
			//task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_UIOADC5));
			//task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_MP_USILEN));
		}

		line = 9;
		task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_ENV_TEMP));
		task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_ENV_RELH));
		task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_DP_TEMP));
		task_twi2_lcd_str(6 *  0, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_QNH));

		if (g_adc_enabled) {
			/* Left measurement units */
			line = 3;
			task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_C));
			task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
			task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
			task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
			//task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
			//task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
			//task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_V));
		}

		line = 9;
		task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_C));
		task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_P100));
		task_twi2_lcd_str(6 * 16, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_C));
		//task_twi2_lcd_str(6 * 18, (line++) * 10, strcpy_P(g_prepare_buf, PM_TWIINIT_HPA));

		/* Gyro: plot circles */
		{
			const uint8_t plot_gyro_center_x_X	= 150;
			const uint8_t plot_gyro_center_x_Y	= 150 + 30;
			const uint8_t plot_gyro_center_x_Z	= 150 + 60;
			const uint8_t plot_gyro_center_y	= 100;
			const uint8_t plot_gyro_radius		= 14;

			task_twi2_lcd_circ(plot_gyro_center_x_X, plot_gyro_center_y, plot_gyro_radius, false, 1);
			task_twi2_lcd_circ(plot_gyro_center_x_Y, plot_gyro_center_y, plot_gyro_radius, false, 1);
			task_twi2_lcd_circ(plot_gyro_center_x_Z, plot_gyro_center_y, plot_gyro_radius, false, 1);

			task_twi2_lcd_str(plot_gyro_center_x_X - 4, plot_gyro_center_y + plot_gyro_radius + 4, strcpy_P(g_prepare_buf, PM_TWIINIT_GX));
			task_twi2_lcd_str(plot_gyro_center_x_Y - 4, plot_gyro_center_y + plot_gyro_radius + 4, strcpy_P(g_prepare_buf, PM_TWIINIT_GY));
			task_twi2_lcd_str(plot_gyro_center_x_Z - 4, plot_gyro_center_y + plot_gyro_radius + 4, strcpy_P(g_prepare_buf, PM_TWIINIT_GZ));
		}

		/* Magnetic & Accel */
		{
			task_twi2_lcd_str(18 * 6, 72, strcpy_P(g_prepare_buf, PM_TWIINIT_MAGNETICS));
			task_twi2_lcd_str(196, 72, strcpy_P(g_prepare_buf, PM_TWIINIT_ACCEL));
		}
	}
}

void task_twi2_lcd_print_format_P(uint8_t x, uint8_t y, const char* fmt_P)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
		g_twi2_m_data[0] = sprintf_P((char*)&(g_twi2_m_data[1]), fmt_P);
		g_twi2_packet.length = g_twi2_m_data[0] + 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

static void task_twi2_lcd_print_format_c(uint8_t x, uint8_t y, char val)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
		g_twi2_m_data[0] = 1;
		g_twi2_m_data[1] = (uint8_t)val;
		g_twi2_packet.length = g_twi2_m_data[0] + 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

static void task_twi2_lcd_print_format_long_P(uint8_t x, uint8_t y, long val, const char* fmt_P)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
		g_twi2_m_data[0] = sprintf_P((char*)&(g_twi2_m_data[1]), fmt_P, val);
		g_twi2_packet.length = g_twi2_m_data[0] + 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}

static void task_twi2_lcd_print_format_float_P(uint8_t x, uint8_t y, float flt, const char* fmt_P)
{
	task_twi2_lcd_pos_xy(x, y);

	if (twi2_waitUntilReady(false)) {
		g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
		g_twi2_m_data[0] = sprintf_P((char*)&(g_twi2_m_data[1]), fmt_P, flt);
		g_twi2_packet.length = g_twi2_m_data[0] + 1;
		twi_master_write(&TWI2_MASTER, &g_twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}


static void task_twi2_lcd__pll(void)
{
	const uint8_t size_x	= 240U;
	const uint8_t size_y	= 128U;
	const uint8_t width		= 3U;
	const uint8_t pos_y_top	= 13U;
	const uint8_t pos_y_mul	= (size_y - pos_y_top) >> 1;
	const uint8_t pos_y_mid	= pos_y_mul + pos_y_top;

	if (twi2_waitUntilReady(false)) {
		if (g_1pps_printtwi_avail) {
			/* Get timer for phase */
			int16_t l_pll_lo;
			{
				irqflags_t flags = cpu_irq_save();
				l_pll_lo = g_1pps_last_lo - C_TCC1_MEAN_OFFSET;
				cpu_irq_restore(flags);

				g_1pps_printtwi_avail = false;
			}

			/* LED green/red */
			twi2_set_leds(g_1pps_led);

			/* Clear old line */
			task_twi2_lcd_rect(size_x - width, pos_y_top, width, size_y - pos_y_top, true, GFX_PIXEL_CLR);

			/* Draw new line */
			if (l_pll_lo >= 0) {
				/* Positive phase */
				uint8_t len_y = (uint8_t) (((uint32_t) (pos_y_mul + l_pll_lo)) % pos_y_mul);
				task_twi2_lcd_rect(size_x - width, pos_y_mid - len_y, width, len_y, false, GFX_PIXEL_SET);

			} else {
				/* Negative phase */
				uint8_t len_y = (uint8_t) (((uint32_t) (pos_y_mul - l_pll_lo)) % pos_y_mul);
				task_twi2_lcd_rect(size_x - width, pos_y_mid, width, len_y, false, GFX_PIXEL_SET);
			}
		}
	}
}


/* TWI2 - LCD Port */
void task_twi2_lcd__cpu1(uint8_t col_left)
{
	static uint32_t	s_epoch_secs		= 0UL;
	static uint8_t	s_minute			= 255;
	static uint8_t	s_hour				= 255;
	static uint8_t	s_date				= 255;
	static uint8_t	s_month				= 255;
	static uint16_t	s_year				= 0;
	static int16_t	s_adc_temp_deg_100	= 0;
	static int16_t	s_adc_5v0_volt_1000	= 0;

	if (twi2_waitUntilReady(false)) {
		uint32_t l_seconds;
		int16_t l_adc_temp_deg_100;
		int16_t l_adc_5v0_volt_1000;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_seconds		= g_boot_time_ts;
			l_seconds	   += (uint32_t)(g_milliseconds_cnt64 / 1000);
			cpu_irq_restore(flags);
		}

		if ((s_epoch_secs != l_seconds) && (1000000000UL < l_seconds && l_seconds < 2000000000UL)) {
			s_epoch_secs = l_seconds;

			struct calendar_date calDat;
			calendar_timestamp_to_date(l_seconds, &calDat);

			if (calDat.second <= 60) {
				task_twi2_lcd_print_format_long_P(17 * 6,  2 * 10 -4, calDat.second,		PM_FORMAT_02LD);
			}

			if ((s_minute != calDat.minute) && (calDat.minute <= 59)) {
				s_minute = calDat.minute;
				task_twi2_lcd_print_format_long_P(14 * 6,  2 * 10 -4, calDat.minute,		PM_FORMAT_02LD);
			}

			if ((s_hour != calDat.hour) && (calDat.hour <= 23)) {
				s_hour = calDat.hour;
				task_twi2_lcd_print_format_long_P(11 * 6,  2 * 10 -4, calDat.hour,			PM_FORMAT_02LD);
			}

			if ((s_date != calDat.date) && (calDat.date <= 30)) {
				s_date = calDat.date;
				task_twi2_lcd_print_format_long_P( 8 * 6,  2 * 10 -4, calDat.date  + 1,		PM_FORMAT_02LD);
			}

			if ((s_month != calDat.month) && (calDat.month <= 11)) {
				s_month = calDat.month;
				task_twi2_lcd_print_format_long_P( 5 * 6,  2 * 10 -4, calDat.month + 1,		PM_FORMAT_02LD);
			}

			if ((s_year != calDat.year) && (2017 <= calDat.year) && (calDat.year <= 2100)) {
				s_year = calDat.year;
				task_twi2_lcd_print_format_long_P( 0 * 6,  2 * 10 -4, calDat.year,			PM_FORMAT_4LD);
			}

		} else {
			/* Reset static vars */
			s_epoch_secs	= 0UL;
			s_minute		= 255;
			s_hour			= 255;
			s_date			= 255;
			s_month			= 255;
			s_year			= 0;
		}

		if (!g_adc_enabled) {
			return;
		}

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_adc_temp_deg_100	= g_adc_temp_deg_100;
			l_adc_5v0_volt_1000	= g_adc_5v0_volt_1000;
			cpu_irq_restore(flags);
		}

		if (s_adc_temp_deg_100 != l_adc_temp_deg_100) {
			s_adc_temp_deg_100 = l_adc_temp_deg_100;

			/* ADC_TEMP */
			task_twi2_lcd_print_format_float_P(col_left,  3 * 10, l_adc_temp_deg_100 / 100.f, PM_FORMAT_4F1);
		}

		if (s_adc_5v0_volt_1000 != l_adc_5v0_volt_1000) {
			s_adc_5v0_volt_1000 = l_adc_5v0_volt_1000;

			/* ADC_5V0 */
			task_twi2_lcd_print_format_float_P(col_left,  4 * 10, l_adc_5v0_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}
	}
}

void task_twi2_lcd__cpu2(uint8_t col_left)
{
	static int16_t s_adc_vbat_volt_1000 = 0;
	static int16_t s_adc_vctcxo_volt_1000 = 0;

	if (twi2_waitUntilReady(false)) {
		int16_t l_adc_vbat_volt_1000;
		int16_t l_adc_vctcxo_volt_1000;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_adc_vbat_volt_1000		= g_adc_vbat_volt_1000;
			l_adc_vctcxo_volt_1000		= g_adc_vctcxo_volt_1000;
			cpu_irq_restore(flags);
		}

		if (s_adc_vbat_volt_1000 != l_adc_vbat_volt_1000) {
			s_adc_vbat_volt_1000 = l_adc_vbat_volt_1000;

			/* ADC_VBAT */
			task_twi2_lcd_print_format_float_P(col_left,  5 * 10, l_adc_vbat_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}

		if (s_adc_vctcxo_volt_1000 != l_adc_vctcxo_volt_1000) {
			s_adc_vctcxo_volt_1000 = l_adc_vctcxo_volt_1000;

			/* ADC_VCTCXO */
			task_twi2_lcd_print_format_float_P(col_left,  6 * 10, l_adc_vctcxo_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}
	}
}

#if 0
void task_twi2_lcd__cpu3(uint8_t col_left)
{
	static int16_t s_adc_io_adc4_volt_1000 = 0;
	static int16_t s_adc_io_adc5_volt_1000 = 0;

	if (twi2_waitUntilReady(false)) {
		int16_t l_adc_io_adc4_volt_1000;
		int16_t l_adc_io_adc5_volt_1000;

		/* Get up-to-date global data */
		{
			irqflags_t flags			= cpu_irq_save();
			l_adc_io_adc4_volt_1000		= g_adc_io_adc4_volt_1000;
			l_adc_io_adc5_volt_1000		= g_adc_io_adc5_volt_1000;
			cpu_irq_restore(flags);
		}

		if (s_adc_io_adc4_volt_1000 != l_adc_io_adc4_volt_1000) {
			s_adc_io_adc4_volt_1000 = l_adc_io_adc4_volt_1000;

			/* ADC_IO_ADC4 */
			task_twi2_lcd_print_format_float_P(col_left,  7 * 10, l_adc_io_adc4_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}

		if (s_adc_io_adc5_volt_1000 != l_adc_io_adc5_volt_1000) {
			s_adc_io_adc5_volt_1000 = l_adc_io_adc5_volt_1000;

			/* ADC_IO_ADC5 */
			task_twi2_lcd_print_format_float_P(col_left,  8 * 10, l_adc_io_adc5_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}

		#if 0
		static int16_t s_adc_silence_volt_1000 = 0;
		int16_t l_adc_silence_volt_1000;

		{
			irqflags_t flags			= cpu_irq_save();
			l_adc_silence_volt_1000		= g_adc_silence_volt_1000;
			cpu_irq_restore(flags);
		}

		if (s_adc_silence_volt_1000 != l_adc_silence_volt_1000) {
			s_adc_silence_volt_1000 = l_adc_silence_volt_1000;

			/* ADC_SILENCE */
			task_twi2_lcd_print_format_float_P(col_left,  8 * 10, l_adc_silence_volt_1000 / 1000.f, PM_FORMAT_5F3);
		}
		#endif
	}
}
#endif

void task_twi2_lcd__sim1(uint8_t col_left)
{
	static float	s_gns_lat		= 0.f;
	static float	s_gns_lon		= 0.f;
	static float	s_gns_msl		= 0.f;
	static float	s_gns_speed		= 0.f;

	if (twi2_waitUntilReady(false)) {
		float			l_gns_lat;
		float			l_gns_lon;
		float			l_gns_msl_alt_m;
		float			l_gns_speed_kmPh;
		char			l_lat_prefix;
		char			l_lon_prefix;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_gns_lat			= g_gns_lat;
			l_gns_lon			= g_gns_lon;
			l_gns_msl_alt_m		= g_gns_msl_alt_m;
			l_gns_speed_kmPh	= g_gns_speed_kmPh;
			cpu_irq_restore(flags);
		}

		/* Latitude */
		if (s_gns_lat != l_gns_lat) {
			s_gns_lat  = l_gns_lat;
			l_lat_prefix = l_gns_lat >= 0.f ?  'N' : 'S';
			l_gns_lat += 0.000005f;
			task_twi2_lcd_print_format_c(       0 * 6,  7 * 10, l_lat_prefix);
			task_twi2_lcd_print_format_long_P(  2 * 6,  7 * 10, (long) abs(l_gns_lat), PM_FORMAT_02LD);
			task_twi2_lcd_print_format_c(       4 * 6,  7 * 10, '.');
			float f_abs = l_gns_lat >= 0.f ?  l_gns_lat : -l_gns_lat;
			float f_frac = (f_abs - (int)f_abs) * 1e5;
			task_twi2_lcd_print_format_long_P(  5 * 6,  7 * 10, (long) f_frac, PM_FORMAT_05LD);
			task_twi2_lcd_print_format_c(      10 * 6,  7 * 10, 0x7e);		// °
		}

		/* Longitude */
		if (s_gns_lon != l_gns_lon) {
			s_gns_lon  = l_gns_lon;
			l_lon_prefix = l_gns_lon >= 0.f ?  'E' : 'W';
			l_gns_lon += 0.000005f;
			task_twi2_lcd_print_format_c(       0 * 6,  8 * 10, l_lon_prefix);
			task_twi2_lcd_print_format_long_P(  1 * 6,  8 * 10, (long) abs(l_gns_lon), PM_FORMAT_03LD);
			task_twi2_lcd_print_format_c(       4 * 6,  8 * 10, '.');
			float f_abs = l_gns_lon >= 0.f ?  l_gns_lon : -l_gns_lon;
			float f_frac = (f_abs - (int)f_abs) * 1e5;
			task_twi2_lcd_print_format_long_P(  5 * 6,  8 * 10, (long) f_frac, PM_FORMAT_05LD);
			task_twi2_lcd_print_format_c(      10 * 6,  8 * 10, 0x7e);		// °
		}

		/* Height */
		if ((s_gns_msl != l_gns_msl_alt_m) && (-1000.f < l_gns_msl_alt_m) && (l_gns_msl_alt_m < 10000.f)) {
			s_gns_msl  = l_gns_msl_alt_m;
			task_twi2_lcd_print_format_long_P( 11 * 6,  7 * 10, (long)(l_gns_msl_alt_m + 0.5f), PM_FORMAT_4LD);
			task_twi2_lcd_print_format_c(      16 * 6,  7 * 10, 'm');
		}

		/* Speed */
		if ((s_gns_speed != l_gns_speed_kmPh) && (0.f <= l_gns_speed_kmPh) && (l_gns_speed_kmPh < 1000.f)) {
			s_gns_speed  = l_gns_speed_kmPh;
			task_twi2_lcd_print_format_float_P(12 * 6,  8 * 10, l_gns_speed_kmPh + 0.05f, PM_FORMAT_5F1);
			task_twi2_lcd_print_format_P(      18 * 6,  8 * 10, PM_FORMAT_KMPH);
		}
	}
}

void task_twi2_lcd__hygro(uint8_t col_left)
{
	static int16_t s_twi1_hygro_DP_100 = 0;

	if (twi2_waitUntilReady(false)) {
		int16_t l_twi1_hygro_DP_100;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_hygro_DP_100				= g_twi1_hygro_DP_100;
			cpu_irq_restore(flags);
		}

		/* Dew Point temperature */
		if (s_twi1_hygro_DP_100 != l_twi1_hygro_DP_100) {
			s_twi1_hygro_DP_100 = l_twi1_hygro_DP_100;
			task_twi2_lcd_print_format_float_P(col_left, 11 * 10, l_twi1_hygro_DP_100 / 100.f, PM_FORMAT_05F2);
		}
	}
}

void task_twi2_lcd__gyro_gfxmag(void)
{
	const uint8_t plot_intensity_center_x = 115;
	const uint8_t plot_intensity_center_y =  64;
	const uint8_t plot_mag_center_x = 150;
	const uint8_t plot_mag_center_y =  40;
	static int32_t s_twi1_gyro_2_mag_x_nT = 0;
	static int32_t s_twi1_gyro_2_mag_y_nT = 0;
	static int32_t s_twi1_gyro_2_mag_z_nT = 0;
	static float  s_length = 0.f;
	static int8_t s_p1x = 0;
	static int8_t s_p1y = 0;
	static int8_t s_p2x = 0;
	static int8_t s_p2y = 0;
	static int8_t s_p3x = 0;
	static int8_t s_p3y = 0;

	if (twi2_waitUntilReady(false)) {
		/* Mag lines */
		int32_t l_twi1_gyro_2_mag_x_nT;
		int32_t l_twi1_gyro_2_mag_y_nT;
		int32_t l_twi1_gyro_2_mag_z_nT;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_gyro_2_mag_x_nT		= g_twi1_gyro_2_mag_x_nT;
			l_twi1_gyro_2_mag_y_nT		= g_twi1_gyro_2_mag_y_nT;
			l_twi1_gyro_2_mag_z_nT		= g_twi1_gyro_2_mag_z_nT;
			cpu_irq_restore(flags);
		}

		if ((s_twi1_gyro_2_mag_x_nT != l_twi1_gyro_2_mag_x_nT) || (s_twi1_gyro_2_mag_y_nT != l_twi1_gyro_2_mag_y_nT) || (s_twi1_gyro_2_mag_z_nT != l_twi1_gyro_2_mag_z_nT)) {
			s_twi1_gyro_2_mag_x_nT = l_twi1_gyro_2_mag_x_nT;
			s_twi1_gyro_2_mag_y_nT = l_twi1_gyro_2_mag_y_nT;
			s_twi1_gyro_2_mag_z_nT = l_twi1_gyro_2_mag_z_nT;

			/* Removing old lines first */
			task_twi2_lcd_rect(plot_intensity_center_x - 1,	plot_intensity_center_y - s_length / 3000.f, 3, s_length / 3000.f, true, 0);
			task_twi2_lcd_line(plot_mag_center_x,			plot_mag_center_y,			plot_mag_center_x + s_p1x, plot_mag_center_y + s_p1y, 0);
			task_twi2_lcd_line(plot_mag_center_x + s_p1x,	plot_mag_center_y + s_p1y,	plot_mag_center_x + s_p2x, plot_mag_center_y + s_p2y, 0);
			task_twi2_lcd_line(plot_mag_center_x + s_p2x,	plot_mag_center_y + s_p2y,	plot_mag_center_x + s_p3x, plot_mag_center_y + s_p3y, 0);

			/* Draw center point */
			task_twi2_lcd_circ(plot_mag_center_x, plot_mag_center_y, 1, true, 1);

			/* Draw new lines */
			{
				float l_length = pow(pow(l_twi1_gyro_2_mag_x_nT, 2.0) + pow(l_twi1_gyro_2_mag_y_nT, 2.0) + pow(l_twi1_gyro_2_mag_z_nT, 2.0), 0.5);
				if (!l_length) {
					l_length = 1.f;
				}

				float l_twi1_gyro_2_mag_x_norm = l_twi1_gyro_2_mag_x_nT / l_length;
				float l_twi1_gyro_2_mag_y_norm = l_twi1_gyro_2_mag_y_nT / l_length;
				float l_twi1_gyro_2_mag_z_norm = l_twi1_gyro_2_mag_z_nT / l_length;
				uint8_t p1x =       (l_twi1_gyro_2_mag_x_norm * 12.5);
				uint8_t p1y =      -(l_twi1_gyro_2_mag_x_norm * 12.5);
				uint8_t p2x = p1x + (l_twi1_gyro_2_mag_y_norm * 25);
				uint8_t p2y = p1y;
				uint8_t p3x = p2x;
				uint8_t p3y = p2y + (l_twi1_gyro_2_mag_z_norm * 25);

				// Saturation at 100µT
				if (l_length > 100000.f) {
					l_length = 100000.f;
				}

				task_twi2_lcd_circ(plot_intensity_center_x,		plot_intensity_center_y,	2, false, 1);
				task_twi2_lcd_rect(plot_intensity_center_x - 1,	plot_intensity_center_y - l_length / 3000.f, 3, l_length / 3000.f, true, 1);
				task_twi2_lcd_line(plot_mag_center_x,			plot_mag_center_y,			plot_mag_center_x + p1x, plot_mag_center_y + p1y, 1);
				task_twi2_lcd_line(plot_mag_center_x + p1x,		plot_mag_center_y + p1y,	plot_mag_center_x + p2x, plot_mag_center_y + p2y, 1);
				task_twi2_lcd_line(plot_mag_center_x + p2x,		plot_mag_center_y + p2y,	plot_mag_center_x + p3x, plot_mag_center_y + p3y, 1);

				/* Store new set */
				s_length = l_length;
				s_p1x = p1x;
				s_p1y = p1y;
				s_p2x = p2x;
				s_p2y = p2y;
				s_p3x = p3x;
				s_p3y = p3y;
			}
		}
	}
}

void task_twi2_lcd__gyro_gfxaccel(void)
{
	const uint8_t plot_accel_center_x = 210;
	const uint8_t plot_accel_center_y =  40;
	static int16_t s_twi1_gyro_1_accel_x_mg = 0;
	static int16_t s_twi1_gyro_1_accel_y_mg = 0;
	static int16_t s_twi1_gyro_1_accel_z_mg = 0;
	static int8_t s_p1x = 0;
	static int8_t s_p1y = 0;
	static int8_t s_p2x = 0;
	static int8_t s_p2y = 0;
	static int8_t s_p3x = 0;
	static int8_t s_p3y = 0;

	if (twi2_waitUntilReady(false)) {
		/* Accel lines */
		int16_t l_twi1_gyro_1_accel_x_mg;
		int16_t l_twi1_gyro_1_accel_y_mg;
		int16_t l_twi1_gyro_1_accel_z_mg;
		int16_t l_backlight_mode_pwm;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_twi1_gyro_1_accel_x_mg	= g_twi1_gyro_1_accel_x_mg;
			l_twi1_gyro_1_accel_y_mg	= g_twi1_gyro_1_accel_y_mg;
			l_twi1_gyro_1_accel_z_mg	= g_twi1_gyro_1_accel_z_mg;
			l_backlight_mode_pwm		= g_backlight_mode_pwm;
			cpu_irq_restore(flags);
		}

		if ((s_twi1_gyro_1_accel_x_mg != l_twi1_gyro_1_accel_x_mg) || (s_twi1_gyro_1_accel_y_mg != l_twi1_gyro_1_accel_y_mg) || (s_twi1_gyro_1_accel_z_mg != l_twi1_gyro_1_accel_z_mg)) {
			s_twi1_gyro_1_accel_x_mg = l_twi1_gyro_1_accel_x_mg;
			s_twi1_gyro_1_accel_y_mg = l_twi1_gyro_1_accel_y_mg;
			s_twi1_gyro_1_accel_z_mg = l_twi1_gyro_1_accel_z_mg;

			/* Removing old lines first */
			{
				task_twi2_lcd_line(plot_accel_center_x,			plot_accel_center_y,			plot_accel_center_x + s_p1x, plot_accel_center_y + s_p1y, 0);
				task_twi2_lcd_line(plot_accel_center_x + s_p1x,	plot_accel_center_y + s_p1y,	plot_accel_center_x + s_p2x, plot_accel_center_y + s_p2y, 0);
				task_twi2_lcd_line(plot_accel_center_x + s_p2x,	plot_accel_center_y + s_p2y,	plot_accel_center_x + s_p3x, plot_accel_center_y + s_p3y, 0);
			}

			/* Center point */
			task_twi2_lcd_circ(plot_accel_center_x, plot_accel_center_y, 1, true, 1);

			/* Draw new lines */
			{
				uint8_t p1x =      -(l_twi1_gyro_1_accel_y_mg / 80);
				uint8_t p1y =       (l_twi1_gyro_1_accel_y_mg / 80);
				uint8_t p2x = p1x - (l_twi1_gyro_1_accel_x_mg / 40);
				uint8_t p2y = p1y;
				uint8_t p3x = p2x;
				uint8_t p3y = p2y + (l_twi1_gyro_1_accel_z_mg / 40);

				task_twi2_lcd_line(plot_accel_center_x,			plot_accel_center_y,		plot_accel_center_x + p1x, plot_accel_center_y + p1y, 1);
				task_twi2_lcd_line(plot_accel_center_x + p1x,	plot_accel_center_y + p1y,	plot_accel_center_x + p2x, plot_accel_center_y + p2y, 1);
				task_twi2_lcd_line(plot_accel_center_x + p2x,	plot_accel_center_y + p2y,	plot_accel_center_x + p3x, plot_accel_center_y + p3y, 1);

				/* Store new set */
				s_p1x = p1x;
				s_p1y = p1y;
				s_p2x = p2x;
				s_p2y = p2y;
				s_p3x = p3x;
				s_p3y = p3y;
			}

			/* Calculate the luminance (sunshine on the surface) */
			if (l_backlight_mode_pwm == -2) {
				int32_t lum = 1000 + l_twi1_gyro_1_accel_z_mg;
				if (lum < 0) {
					lum = 0;
				} else if (lum > 2000) {
					lum = 2000;
				}

				twi2_set_ledbl(0, (uint8_t)(lum * 255 / 2000));
			}
		}
	}
}

void task_twi2_lcd__gyro_gfxgyro(void)
{
	const uint8_t plot_gyro_center_x_X	= 150;
	const uint8_t plot_gyro_center_x_Y	= 150 + 30;
	const uint8_t plot_gyro_center_x_Z	= 150 + 60;
	const uint8_t plot_gyro_center_y	= 100;
	const uint8_t plot_gyro_radius		= 12;
	static float s_rads_x = 0.f;
	static float s_rads_y = 0.f;
	static float s_rads_z = 0.f;
	static int32_t s_twi1_gyro_1_gyro_x_mdps = 0;
	static int32_t s_twi1_gyro_1_gyro_y_mdps = 0;
	static int32_t s_twi1_gyro_1_gyro_z_mdps = 0;

	if (twi2_waitUntilReady(false)) {
		/* Gyro lines */
		int32_t l_twi1_gyro_1_gyro_x_mdps;
		int32_t l_twi1_gyro_1_gyro_y_mdps;
		int32_t l_twi1_gyro_1_gyro_z_mdps;

		/* Get up-to-date global data */
		{
			irqflags_t flags			= cpu_irq_save();
			l_twi1_gyro_1_gyro_x_mdps	= g_twi1_gyro_1_gyro_x_mdps;
			l_twi1_gyro_1_gyro_y_mdps	= g_twi1_gyro_1_gyro_y_mdps;
			l_twi1_gyro_1_gyro_z_mdps	= g_twi1_gyro_1_gyro_z_mdps;
			cpu_irq_restore(flags);
		}

		if ((s_twi1_gyro_1_gyro_x_mdps != l_twi1_gyro_1_gyro_x_mdps) || (s_twi1_gyro_1_gyro_y_mdps != l_twi1_gyro_1_gyro_y_mdps) || (s_twi1_gyro_1_gyro_z_mdps != l_twi1_gyro_1_gyro_z_mdps)) {
			s_twi1_gyro_1_gyro_x_mdps = l_twi1_gyro_1_gyro_x_mdps;
			s_twi1_gyro_1_gyro_y_mdps = l_twi1_gyro_1_gyro_y_mdps;
			s_twi1_gyro_1_gyro_z_mdps = l_twi1_gyro_1_gyro_z_mdps;

			float rads_x = ((float)l_twi1_gyro_1_gyro_x_mdps * M_PI) / 180000.f;
			float rads_y = ((float)l_twi1_gyro_1_gyro_y_mdps * M_PI) / 180000.f;
			float rads_z = ((float)l_twi1_gyro_1_gyro_z_mdps * M_PI) / 180000.f;

			/* Remove old lines */
			task_twi2_lcd_line(plot_gyro_center_x_X, plot_gyro_center_y, plot_gyro_center_x_X - (int8_t)(plot_gyro_radius * sin(s_rads_x)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(s_rads_x)), 0);
			task_twi2_lcd_line(plot_gyro_center_x_Y, plot_gyro_center_y, plot_gyro_center_x_Y + (int8_t)(plot_gyro_radius * sin(s_rads_y)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(s_rads_y)), 0);
			task_twi2_lcd_line(plot_gyro_center_x_Z, plot_gyro_center_y, plot_gyro_center_x_Z - (int8_t)(plot_gyro_radius * sin(s_rads_z)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(s_rads_z)), 0);

			/* Draw new lines */
			task_twi2_lcd_line(plot_gyro_center_x_X, plot_gyro_center_y, plot_gyro_center_x_X - (int8_t)(plot_gyro_radius * sin(  rads_x)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(  rads_x)), 1);
			task_twi2_lcd_line(plot_gyro_center_x_Y, plot_gyro_center_y, plot_gyro_center_x_Y + (int8_t)(plot_gyro_radius * sin(  rads_y)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(  rads_y)), 1);
			task_twi2_lcd_line(plot_gyro_center_x_Z, plot_gyro_center_y, plot_gyro_center_x_Z - (int8_t)(plot_gyro_radius * sin(  rads_z)), plot_gyro_center_y - (int8_t)(plot_gyro_radius * cos(  rads_z)), 1);

			/* Store new set */
			s_rads_x = rads_x;
			s_rads_y = rads_y;
			s_rads_z = rads_z;
		}

		/* Calculate speed of rotation */
		if (g_pitch_tone_mode == 1) {
			int32_t speed = l_twi1_gyro_1_gyro_x_mdps + l_twi1_gyro_1_gyro_y_mdps + l_twi1_gyro_1_gyro_z_mdps;
			if (speed < 0) {
				speed = -speed;
			}
			if (speed > 400) {
				twi2_set_beep(18 + (uint8_t)(speed / 700), 10);
				yield_ms(125);
			}
		}
	}
}

void task_twi2_lcd__gyro_beepvario(void)
{
	static uint32_t s_twi1_baro_p_100 = 100000UL;

	if (twi2_waitUntilReady(false)) {
		/* Calculate variometer */
		int32_t l_twi1_baro_p_100;

		/* Get up-to-date global data */
		{
			irqflags_t flags			= cpu_irq_save();
			l_twi1_baro_p_100			= g_twi1_baro_p_100;
			cpu_irq_restore(flags);
		}

		/* Calculate variometer tone */
		if (g_pitch_tone_mode == 2) {
			int32_t vario = g_twi1_baro_p_100 - s_twi1_baro_p_100;
			uint32_t pitch = 100 - vario;

			if (pitch < 10) {
				pitch = 10;
			} else if (pitch > 255) {
				pitch = 255;
			}

			twi2_set_beep(pitch, 10);
			yield_ms(125);
		}

		/* Update static value */
		s_twi1_baro_p_100 = l_twi1_baro_p_100;
	}
}

void task_twi2_lcd__gyro(void)
{
	/* Mag lines */
	task_twi2_lcd__gyro_gfxmag();

	/* Accel lines */
	task_twi2_lcd__gyro_gfxaccel();

	/* Gyro lines */
	task_twi2_lcd__gyro_gfxgyro();

	/* Calculate variometer */
	task_twi2_lcd__gyro_beepvario();
}

void task_twi2_lcd__baro(uint8_t col_left)
{
	static int32_t s_twi1_baro_p_h_100 = 0;

	if (twi2_waitUntilReady(false)) {
		int32_t l_twi1_baro_p_h_100;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();

			l_twi1_baro_p_h_100				= g_qnh_p_h_100;
			if (!l_twi1_baro_p_h_100) {
				l_twi1_baro_p_h_100			= g_twi1_baro_p_100;		// As long as no height is available use relative pressure instead
			}
			cpu_irq_restore(flags);
		}

		/* QNH */
		if (s_twi1_baro_p_h_100 != l_twi1_baro_p_h_100) {
			s_twi1_baro_p_h_100 = l_twi1_baro_p_h_100;
			task_twi2_lcd_print_format_float_P(col_left, 12 * 10, l_twi1_baro_p_h_100 / 100.f, PM_FORMAT_07F2);
		}
	}
}

void task_twi2_lcd__environment(uint8_t col_left)
{
	static int16_t s_env_temp_deg_100 = 0;
	static int16_t s_env_hygro_RH_100 = 0;

	if (twi2_waitUntilReady(false)) {
		int16_t l_env_temp_deg_100;
		int16_t l_env_hygro_RH_100;

		/* Get up-to-date global data */
		{
			irqflags_t flags = cpu_irq_save();
			l_env_temp_deg_100				= g_env_temp_deg_100;
			l_env_hygro_RH_100				= g_env_hygro_RH_100;
			cpu_irq_restore(flags);
		}

		/* Environmental Temp */
		if (s_env_temp_deg_100 != l_env_temp_deg_100) {
			s_env_temp_deg_100 = l_env_temp_deg_100;
			task_twi2_lcd_print_format_float_P(col_left,  9 * 10, l_env_temp_deg_100 / 100.f, PM_FORMAT_05F2);
		}

		/* Environmental relative humidity */
		if (s_env_hygro_RH_100 != l_env_hygro_RH_100) {
			s_env_hygro_RH_100 = l_env_hygro_RH_100;
			if ((0 <= l_env_hygro_RH_100) && (l_env_hygro_RH_100 < 10000)) {
				task_twi2_lcd_print_format_float_P(col_left, 10 * 10, l_env_hygro_RH_100 / 100.f, PM_FORMAT_05F2);
			}
		}
	}
}

static void task_twi2_lcd(void)
{
	static uint8_t s_lcd_entry_state =  0;

	if (g_twi2_lcd_version >= 0x11) {
		/* Show current measurement data on the LCD */
		const uint8_t col_left = 6 * 10;

		/* Check if new 1PPS has arrived do resync of state */
		if (g_1pps_twi_new) {
			g_1pps_twi_new = false;
			s_lcd_entry_state = 0;
		}

		/* Update same text line each second when no Gfx data is drawn */
		switch (s_lcd_entry_state) {
			case 0:
				task_twi2_lcd__cpu1(col_left);
				++s_lcd_entry_state;
			break;

			case 1:
				if (g_adc_enabled) {
					task_twi2_lcd__cpu2(col_left);
				}
				++s_lcd_entry_state;
			break;

			case 2:
				task_twi2_lcd__sim1(col_left);
				++s_lcd_entry_state;
			break;

			case 3:
				task_twi2_lcd__environment(col_left);
				task_twi2_lcd__baro(col_left);
				++s_lcd_entry_state;
			break;

			case 4:
				task_twi2_lcd__hygro(col_left);
				s_lcd_entry_state = 0;
				twi2_waitUntilReady(true);
			break;

			default:
				s_lcd_entry_state = 0;
		}

		/* Update Gfx every time */
		task_twi2_lcd__gyro();
		task_twi2_lcd__pll();

		/* Repaint all items when starting and at some interval */
		if (g_twi2_lcd_repaint) {
			g_twi2_lcd_repaint = false;
			task_twi2_lcd_header();
			task_twi2_lcd_template();
		}

	} else if (g_twi2_lcd_version == 0x10) {
		/* Show PWM in % when version is V1.0 and mode==0x20 selected */
		if (twi2_waitUntilReady(true)) {
			g_twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SHOW_TCXO_PWM;
			g_twi2_m_data[0] = 1;
			g_twi2_m_data[1] = 128;
			g_twi2_packet.length = 2;
			twi_master_write(&TWI2_MASTER, &g_twi2_packet);
			delay_us(TWI_SMART_LCD_DEVICE_TCXOPWM_DELAY_MIN_US);
		}
	}
}


void task_twi(void)
{	/* Calculations for the presentation layer and display */
	#if 0
	// now to be called by the scheduler by  isr_500ms_twi1_onboard()

	/* TWI1 - SIM808, Hygro, Gyro, Baro devices */
	task_twi1_onboard();
	#endif

	/* TWI2 - LCD Port */
	if (g_workmode == WORKMODE_RUN) {
		task_twi2_lcd();
	}

	/* LED off */
	twi2_set_leds(0x00);
}
