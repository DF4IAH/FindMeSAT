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

#include <math.h>
#include "twi.h"

#include "main.h"


extern bool				g_adc_enabled;
extern bool				g_dac_enabled;

extern int32_t			g_adc_temp_cur;
extern int16_t			g_adc_vctcxo_volt_1000;
extern int16_t			g_adc_5v0_volt_1000;
extern int16_t			g_adc_vbat_volt_1000;
extern int16_t			g_adc_io_adc4_volt_1000;
extern int16_t			g_adc_io_adc5_volt_1000;
extern int16_t			g_adc_silence_volt_1000;
extern int16_t			g_adc_temp_deg_100;

extern bool				g_twi1_gsm_valid;
extern uint8_t			g_twi1_gsm_version;

extern bool				g_twi1_gyro_valid;
extern uint8_t			g_twi1_gyro_1_version;
extern int16_t			g_twi1_gyro_1_temp;
extern int16_t			g_twi1_gyro_1_temp_RTofs;
extern int16_t			g_twi1_gyro_1_temp_sens;
extern int16_t			g_twi1_gyro_1_temp_deg_100;
extern int16_t			g_twi1_gyro_1_accel_x;
extern int16_t			g_twi1_gyro_1_accel_y;
extern int16_t			g_twi1_gyro_1_accel_z;
extern int16_t			g_twi1_gyro_1_accel_ofsx;
extern int16_t			g_twi1_gyro_1_accel_ofsy;
extern int16_t			g_twi1_gyro_1_accel_ofsz;
extern int16_t			g_twi1_gyro_1_accel_factx;
extern int16_t			g_twi1_gyro_1_accel_facty;
extern int16_t			g_twi1_gyro_1_accel_factz;
extern int16_t			g_twi1_gyro_1_accel_x_mg;
extern int16_t			g_twi1_gyro_1_accel_y_mg;
extern int16_t			g_twi1_gyro_1_accel_z_mg;
extern int16_t			g_twi1_gyro_1_gyro_x;
extern int16_t			g_twi1_gyro_1_gyro_y;
extern int16_t			g_twi1_gyro_1_gyro_z;
extern int16_t			g_twi1_gyro_1_gyro_ofsx;
extern int16_t			g_twi1_gyro_1_gyro_ofsy;
extern int16_t			g_twi1_gyro_1_gyro_ofsz;
extern int32_t			g_twi1_gyro_1_gyro_x_mdps;
extern int32_t			g_twi1_gyro_1_gyro_y_mdps;
extern int32_t			g_twi1_gyro_1_gyro_z_mdps;
extern uint8_t			g_twi1_gyro_2_version;
extern int8_t			g_twi1_gyro_2_asax;
extern int8_t			g_twi1_gyro_2_asay;
extern int8_t			g_twi1_gyro_2_asaz;
extern int16_t			g_twi1_gyro_2_ofsx;
extern int16_t			g_twi1_gyro_2_ofsy;
extern int16_t			g_twi1_gyro_2_ofsz;
extern int16_t			g_twi1_gyro_2_mag_x;
extern int16_t			g_twi1_gyro_2_mag_y;
extern int16_t			g_twi1_gyro_2_mag_z;
extern int16_t			g_twi1_gyro_2_mag_factx;
extern int16_t			g_twi1_gyro_2_mag_facty;
extern int16_t			g_twi1_gyro_2_mag_factz;
extern int32_t			g_twi1_gyro_2_mag_x_nT;
extern int32_t			g_twi1_gyro_2_mag_y_nT;
extern int32_t			g_twi1_gyro_2_mag_z_nT;


extern bool				g_twi1_baro_valid;
extern uint16_t			g_twi1_baro_version;
extern uint16_t			g_twi1_baro_c[C_TWI1_BARO_C_CNT];
extern uint32_t			g_twi1_baro_d1;
extern uint32_t			g_twi1_baro_d2;
extern int32_t			g_twi1_baro_temp_100;
extern int32_t			g_twi1_baro_p_100;

extern bool				g_twi1_hygro_valid;
extern uint8_t			g_twi1_hygro_status;
extern uint16_t			g_twi1_hygro_S_T;
extern uint16_t			g_twi1_hygro_S_RH;
extern int16_t			g_twi1_hygro_T_100;
extern int16_t			g_twi1_hygro_RH_100;

extern uint8_t			g_twi2_lcd_version;

extern twi_options_t	twi1_options;
extern uint8_t			twi1_m_data[TWI_DATA_LENGTH];
extern twi_package_t	twi1_packet;

extern twi_options_t	twi2_options;
extern uint8_t			twi2_m_data[TWI_DATA_LENGTH];
extern twi_package_t	twi2_packet;

#ifdef TWI1_SLAVE
extern TWI_Slave_t		twi1_slave;
extern uint8_t			twi1_recv_data[TWI_DATA_LENGTH];
#endif

#ifdef TWI2_SLAVE
extern TWI_Slave_t		twi2_slave;
extern uint8_t			twi2_recv_data[TWI_DATA_LENGTH];
#endif



#ifdef TWI1_SLAVE
static void twi1_slave_process(void) {
	int i;

	for(i = 0; i < TWI_DATA_LENGTH; i++) {
		twi1_recv_data[i] = twi1_slave.receivedData[i];
	}
}

ISR(TWIE_TWIS_vect) {
	TWI_SlaveInterruptHandler(&twi1_slave);
}
#endif

#ifdef TWI2_SLAVE
static void twi2_slave_process(void) {
	int i;

	for(i = 0; i < TWI_DATA_LENGTH; i++) {
		twi2_recv_data[i] = twi2_slave.receivedData[i];
	}
}

ISR(TWIC_TWIS_vect) {
	TWI_SlaveInterruptHandler(&twi2_slave);
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
	return ((1000L * TWI1_SLAVE_GYRO_DTA_1_GYRO_CONFIG__0250DPS) * (int32_t)raw) >> 15;
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

static void twi2_waitUntilReady(void)
{
	status_code_t status;
	uint8_t isBusy;
	uint8_t isValid;

	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_GET_STATE;
	twi2_packet.addr_length = 1;

	//printf("DBG901\r\n");
	/* Wait until not BUSY */
	do {
		twi2_packet.length = 1;
		status = twi_master_read(&TWI2_MASTER, &twi2_packet);
		isValid = twi2_m_data[0] & 0x80;
		isBusy  = twi2_m_data[0] & 0x01;

		if ((status != STATUS_OK) || !isValid) {
			delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
		} else {
			if (isBusy && isValid) {
				delay_us(TWI_SMART_LCD_DEVICE_BUSY_DELAY_MIN_US);
			}
		}
		//printf("DBG902\r\n");
	} while ((status != STATUS_OK) || !isValid || isBusy);
	//printf("DBG909\r\n");
}


#if 0
static void init_twi1_gsm(void)
{
	status_code_t sc;

	/*
	* No documentation for I2C slave mode found, yet.
	*/
	printf("\r\nTWI-onboard: GSM/BT/GPS SIM808 - I2C address: 0x%02X\r\n", TWI1_SLAVE_GSM_ADDR);
	g_twi1_gsm_version = 0;

	twi1_packet.chip = TWI1_SLAVE_GSM_ADDR;
	twi1_packet.addr[0] = TWI1_SLAVE_GSM_REG_1_DEVICE_ID;
	twi1_packet.addr_length = 1;
	twi1_packet.length = 1;
	sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
	if (sc == STATUS_OK) {
		g_twi1_gsm_version = twi1_m_data[0];
		printf("TWI-onboard: GSM/BT/GPS SIM808 -     version: 0x%02X\r\n", g_twi1_gsm_version);
	} else {
		printf("TWI-onboard:  ... device not on board. (sc=%d)\r\n", sc);
	}
}
#endif

static void init_twi1_hygro(void)
{
	status_code_t sc;

	printf("\r\nTWI-onboard: Hygro SHT31-DIS - I2C address: 0x%02X\r\n", TWI1_SLAVE_HYGRO_ADDR);
	g_twi1_hygro_status = 0;

	do {
		twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_BREAK_HI;
		twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_BREAK_LO;
		twi1_packet.addr_length = 2;
		twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			printf("TWI-onboard: Hygro SHT31-DIS -   address NACK / 'break' bad response\r\n");
			break;
		}
		delay_ms(2);

		twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_RESET_HI;
		twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_RESET_LO;
		twi1_packet.addr_length = 2;
		twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(2);

		twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_STATUS_HI;
		twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_STATUS_LO;
		twi1_packet.addr_length = 2;
		twi1_packet.length = 2;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_hygro_status = (twi1_m_data[0] << 8) | twi1_m_data[1];
		printf("TWI-onboard: Hygro SHT31-DIS -   status: 0x%02X\r\n", g_twi1_hygro_status);

		/* Start cyclic measurements with 2 MPS @ high repeatability */
		twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_PERIODIC_2MPS_HIPREC_HI;
		twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_PERIODIC_2MPS_HIPREC_LO;
		twi1_packet.addr_length = 2;
		twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		g_twi1_hygro_valid = true;
		printf("TWI-onboard:  INIT success.\r\n");
		return;
	} while(false);

	printf("TWI-onboard:  ... device not on board. (sc=%d)\r\n", sc);
}

static void init_twi1_gyro(void)
{
	status_code_t sc;

	printf("\r\nTWI-onboard: Gyro MPU-9250 - I2C address: 0x%02X, 0x%02X\r\n", TWI1_SLAVE_GYRO_ADDR_1, TWI1_SLAVE_GYRO_ADDR_2);
	g_twi1_gyro_1_version = 0;
	g_twi1_gyro_2_version = 0;

	do {
		/* MPU-9250 6 axis: RESET */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_PWR_MGMT_1;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_PWR_MGMT_1__HRESET | TWI1_SLAVE_GYRO_DTA_1_PWR_MGMT_1__CLKSEL_VAL;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			printf("TWI-onboard: Gyro MPU-9250   -   'reset 1' bad response\r\n");
			break;
		}
		delay_ms(10);

		/* MPU-9250 6 axis: read Who Am I control value */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_WHOAMI;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_1_version = twi1_m_data[0];

		/* MPU-9250 6 axis: I2C bypass on to access the Magnetometer chip */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_INT_PIN_CFG;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_INT_PIN_CFG__BYPASS_EN;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* Magnetometer: soft reset */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL2;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL2__SRST;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		/* Magnetometer: read Device ID */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_WIA;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_2_version = twi1_m_data[0];
		g_twi1_gyro_valid = true;
		printf("TWI-onboard: Gyro MPU-9250 -     version: 0x%02X, 0x%02X\r\n", g_twi1_gyro_1_version, g_twi1_gyro_2_version);

		/* Magnetometer: 16 bit access and prepare for PROM access */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_PROM_VAL;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* Magnetometer: read correction data for X, Y and Z */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ASAX;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 3;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_2_asax = twi1_m_data[0];
		g_twi1_gyro_2_asay = twi1_m_data[1];
		g_twi1_gyro_2_asaz = twi1_m_data[2];

		/* Magnetometer: mode change via power-down mode */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_16B_POWER_DOWN;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		/* Magnetometer: mode change for 16bit and run all axis at 8 Hz */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_CNTL1;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_2_CNTL1__MODE_16B_RUN_8HZ_VAL;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: set gyro offset values */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_GYRO_XG_OFFSET_H;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = (uint8_t) (g_twi1_gyro_1_gyro_ofsx >> 8);
		twi1_m_data[1] = (uint8_t) (g_twi1_gyro_1_gyro_ofsx & 0xFF);
		twi1_m_data[2] = (uint8_t) (g_twi1_gyro_1_gyro_ofsy >> 8);
		twi1_m_data[3] = (uint8_t) (g_twi1_gyro_1_gyro_ofsy & 0xFF);
		twi1_m_data[4] = (uint8_t) (g_twi1_gyro_1_gyro_ofsz >> 8);
		twi1_m_data[5] = (uint8_t) (g_twi1_gyro_1_gyro_ofsz & 0xFF);
		twi1_packet.length = 6;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: set accel offset values */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_XA_OFFSET_H;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsx & 0x7F80) >> 7);
		twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsx &   0x7F) << 1);
		twi1_packet.length = 2;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_YA_OFFSET_H;
		twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsy & 0x7F80) >> 7);
		twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsy &   0x7F) << 1);
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ZA_OFFSET_H;
		twi1_m_data[0] = (uint8_t) ((g_twi1_gyro_1_accel_ofsz & 0x7F80) >> 7);
		twi1_m_data[1] = (uint8_t) ((g_twi1_gyro_1_accel_ofsz &   0x7F) << 1);
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: FIFO frequency = 10 Hz */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_SMPLRT_DIV;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = 99;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: Bandwidth = 5 Hz, Fs = 1 kHz */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_CONFIG;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = 6;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: Bandwidth = 5 Hz, Fs = 1 kHz */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ACCEL_CONFIG2;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = 6;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: Wake On Motion interrupt = 0.1 g (1 LSB = 4 mg) */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_WOM_THR;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = 25;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}

		/* MPU-9250 6 axis: RESET all internal data paths */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_USER_CTRL;
		twi1_packet.addr_length = 1;
		twi1_m_data[0] = TWI1_SLAVE_GYRO_DTA_1_USER_CTRL__SIG_COND_RST;  // | TWI1_SLAVE_GYRO_DTA_1_USER_CTRL__FIFO_EN;
		twi1_packet.length = 1;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(10);

		printf("TWI-onboard:  INIT success.\r\n");
		return;
	} while(false);

	printf("TWI-onboard:  ... device not on board. (sc=%d)\r\n", sc);
}

static void init_twi1_baro(void)
{
	status_code_t sc;

	printf("\r\nTWI-onboard: Baro MS560702BA03-50 - I2C address: 0x%02X\r\n", TWI1_SLAVE_BARO_ADDR);

	do {
		twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_RESET;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 0;
		sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		delay_ms(3);

		twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
		twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_VERSION;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 2;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			printf("TWI-onboard:  BAD reading serial/CRC word. (sc=%d)\r\n", sc);
			break;
		}
		g_twi1_baro_version = (((uint16_t)twi1_m_data[0] << 8) | (uint16_t)twi1_m_data[1]) >> 4;
		printf("TWI-onboard: Baro MS560702BA03-50 -     serial#: %d\r\n", g_twi1_baro_version);

		for (int adr = 1; adr < C_TWI1_BARO_C_CNT; ++adr) {
			twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
			twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_PROM | (adr << 1);
			twi1_packet.addr_length = 1;
			twi1_packet.length = 2;
			sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
			if (sc != STATUS_OK) {
				//printf("TWI-onboard:  BAD reading PROM address %d. (sc=%d)\r\n", adr, sc);
				break;
			}
			g_twi1_baro_c[adr] = (twi1_m_data[0] << 8) | twi1_m_data[1];
		}

		g_twi1_baro_valid = true;
		printf("TWI-onboard:  INIT success.\r\n");
		return;
	} while(false);

	printf("TWI-onboard:  ... device not on board. (sc=%d)\r\n", sc);
}

/* TWI1 - GSM, Gyro, Baro, Hygro devices */
static void start_twi1_onboard(void)
{
	/* Device GSM/BT/GPS SIM808 - I2C address: 0xXX */
	//init_twi1_gsm();

	/* Device Hygro SHT31-DIS - I2C address: 0x44 */
	init_twi1_hygro();

	/* Device Gyro MPU-9250 - I2C address: 0x68, 0x0C (or 0x0D or 0x0E or 0x0F) */
	init_twi1_gyro();

	/* Device Baro MS560702BA03-50 - I2C address: 0x76 */
	init_twi1_baro();

	printf("-----------\r\n\r\n");
}

/* TWI2 - LCD Port */
static void start_twi2_lcd(void)
{
	/* Read the version number */
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_GET_VER;
	twi2_packet.addr_length = 1;
	twi2_packet.length = 1;
	twi_master_read(&TWI2_MASTER, &twi2_packet);
	g_twi2_lcd_version = twi2_m_data[0];

	if (g_twi2_lcd_version >= 0x11) {
		/* Select "Smart-LCD draw box" mode
		 * that includes a clear screen     */
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_MODE;
		twi2_m_data[0] = 0x10;
		twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);

		twi2_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_PIXEL_TYPE;
		twi2_m_data[0] = GFX_PIXEL_SET;
		twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
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
	TWI_SlaveInitializeDriver(&twi1_slave, &TWI1_SLAVE, *twi1_slave_process);
	TWI_SlaveInitializeModule(&twi1_slave, TWI1_SLAVE_ADDR,	TWI_SLAVE_INTLVL_MED_gc);

	for (i1 = 0; i1 < TWIS_SEND_BUFFER_SIZE; i1++) {
		twi1_slave.receivedData[i1] = 0;
	}
	#else
	#ifdef TWI1_MASTER
	sysclk_enable_peripheral_clock(&TWI1_MASTER);
	twi_master_init(&TWI1_MASTER, &twi1_options);
	twi_master_enable(&TWI1_MASTER);
	#endif
	#endif

	#ifdef TWI2_SLAVE
	uint8_t i2;

	sysclk_enable_peripheral_clock(&TWI2_SLAVE);
	TWI_SlaveInitializeDriver(&twi2_slave, &TWI2_SLAVE, *twi2_slave_process);
	TWI_SlaveInitializeModule(&twi2_slave, TWI2_SLAVE_ADDR,	TWI_SLAVE_INTLVL_MED_gc);

	for (i2 = 0; i2 < TWIS_SEND_BUFFER_SIZE; i2++) {
		twi2_slave.receivedData[i2] = 0;
	}
	#else
	#ifdef TWI2_MASTER
	sysclk_enable_peripheral_clock(&TWI2_MASTER);
	twi_master_init(&TWI2_MASTER, &twi2_options);
	twi_master_enable(&TWI2_MASTER);
	#endif
	#endif

	/* Start each TWI channel devices */
	start_twi1_onboard();

	delay_ms(250);											// Give Smart-LCD some time being up and ready
	start_twi2_lcd();
}


static void isr_twi1_hygro(uint32_t now, bool sync)
{
	if (!sync) {
		return;
	}

	/* Read cyclic measurement data */
	twi1_packet.chip = TWI1_SLAVE_HYGRO_ADDR;
	twi1_packet.addr[0] = TWI1_SLAVE_HYGRO_REG_FETCH_DATA_HI;
	twi1_packet.addr[1] = TWI1_SLAVE_HYGRO_REG_FETCH_DATA_LO;
	twi1_packet.addr_length = 2;
	twi1_packet.length = 5;
	status_code_t sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
	if (sc == STATUS_OK) {
		g_twi1_hygro_S_T	= ((uint16_t)twi1_m_data[0] << 8) | twi1_m_data[1];
		g_twi1_hygro_S_RH	= ((uint16_t)twi1_m_data[3] << 8) | twi1_m_data[4];
	}
}

static void isr_twi1_gyro(uint32_t now, bool sync)
{
	if (!sync) {
		return;
	}

	do {
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_ACCEL_XOUT_H;		// Big endian
		twi1_packet.addr_length = 1;
		twi1_packet.length = 8;
		status_code_t sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_1_accel_x = ((uint16_t)twi1_m_data[0] << 8) | twi1_m_data[1];
		g_twi1_gyro_1_accel_y = ((uint16_t)twi1_m_data[2] << 8) | twi1_m_data[3];
		g_twi1_gyro_1_accel_z = ((uint16_t)twi1_m_data[4] << 8) | twi1_m_data[5];
		g_twi1_gyro_1_temp    = ((uint16_t)twi1_m_data[6] << 8) | twi1_m_data[7];

		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_1;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_1_GYRO_XOUT_H;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 6;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_1_gyro_x = ((uint16_t)twi1_m_data[0] << 8) | twi1_m_data[1];
		g_twi1_gyro_1_gyro_y = ((uint16_t)twi1_m_data[2] << 8) | twi1_m_data[3];
		g_twi1_gyro_1_gyro_z = ((uint16_t)twi1_m_data[4] << 8) | twi1_m_data[5];

		/* Magnetometer: check if new data is available */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ST1;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		if (!(twi1_m_data[0] & TWI1_SLAVE_GYRO_DTA_2_ST1__DRDY)) {
			/* Data of Magnetometer AK8963 not ready yet */
			break;
		}

		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_HX_L;			// Little endian
		twi1_packet.addr_length = 1;
		twi1_packet.length = 6;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		g_twi1_gyro_2_mag_x = ((int16_t) ((((uint16_t)twi1_m_data[1]) << 8) | twi1_m_data[0])) + g_twi1_gyro_2_ofsx;
		g_twi1_gyro_2_mag_y = ((int16_t) ((((uint16_t)twi1_m_data[3]) << 8) | twi1_m_data[2])) + g_twi1_gyro_2_ofsy;
		g_twi1_gyro_2_mag_z = ((int16_t) ((((uint16_t)twi1_m_data[5]) << 8) | twi1_m_data[4])) + g_twi1_gyro_2_ofsz;

		/* Magnetometer: check for data validity and release cycle */
		twi1_packet.chip = TWI1_SLAVE_GYRO_ADDR_2;
		twi1_packet.addr[0] = TWI1_SLAVE_GYRO_REG_2_ST2;
		twi1_packet.addr_length = 1;
		twi1_packet.length = 1;
		sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
		if (sc != STATUS_OK) {
			break;
		}
		if (twi1_m_data[0] & TWI1_SLAVE_GYRO_DTA_2_ST2__HOFL) {
			/* Data of Magnetometer AK8963 overflowed */
			g_twi1_gyro_2_mag_z = g_twi1_gyro_2_mag_y = g_twi1_gyro_2_mag_x = 0;
			break;
		}
	} while (false);
}

static void isr_twi1_baro(uint32_t now, bool sync)
{
	static uint8_t  s_step = 100;
	static uint32_t s_twi1_baro_d1 = 0UL;
	static uint32_t s_twi1_baro_d2 = 0UL;

	/* Restart a new cycle if ready */
	if (sync && (s_step >= 100)) {
		s_step = 0;
	}

	switch (s_step) {
		case 0:
			/* Request D1 */
			twi1_packet.chip = TWI1_SLAVE_BARO_ADDR;
			twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_CONV_D1_4096;
			twi1_packet.addr_length = 1;
			twi1_packet.length = 0;
			status_code_t sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
			if (sc == STATUS_OK) {
				s_step = 1;
				return;
			}

			s_step = 200;										// Failed, stay until new sync triggers
			return;
		break;

		case 21:
			/* Get data */
			twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_ADC_READ;
			twi1_packet.length = 3;
			sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
			if (sc == STATUS_OK) {
				s_twi1_baro_d1 = ((uint32_t)twi1_m_data[0] << 16) | ((uint32_t)twi1_m_data[1] << 8) | twi1_m_data[2];

				/* Request D2 */
				twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_CONV_D2_4096;
				twi1_packet.length = 0;
				sc = twi_master_write(&TWI1_MASTER, &twi1_packet);
				if (sc == STATUS_OK) {
					s_step = 22;
					return;
				}
			}

			s_step = 211;										// Failed, stay until new sync triggers
			return;
		break;

		case 43:
			twi1_packet.addr[0] = TWI1_SLAVE_BARO_REG_ADC_READ;
			twi1_packet.length = 3;
			sc = twi_master_read(&TWI1_MASTER, &twi1_packet);
			if (sc == STATUS_OK) {
				s_twi1_baro_d2 = ((uint32_t)twi1_m_data[0] << 16) | ((uint32_t)twi1_m_data[1] << 8) | twi1_m_data[2];

				irqflags_t flags = cpu_irq_save();
				g_twi1_baro_d1 = s_twi1_baro_d1;
				g_twi1_baro_d2 = s_twi1_baro_d2;
				cpu_irq_restore(flags);

				s_step = 123;									// Success, stay until new sync triggers
				return;
			}

			s_step = 223;										// Failed, stay until new sync triggers
			return;
		break;

		default:
			/* Delay step of 0.5 ms */
			if (s_step < 100) {
				s_step++;
			}
	}
}

/* 10ms TWI1 - Gyro device */
void isr_10ms_twi1_onboard(uint32_t now)
{	/* Service time slot */
}

/* 500ms TWI1 - Baro, Hygro devices */
void isr_500ms_twi1_onboard(uint32_t now)
{	/* Service time slot */
	if (g_twi1_hygro_valid) {
		isr_twi1_hygro(now, true);
	}

	if (g_twi1_gyro_valid) {
		isr_twi1_gyro(now, true);
	}

	if (g_twi1_baro_valid) {
		isr_twi1_baro(now, true);
	}
}

/* 2560 cycles per second */
void isr_sparetime_twi1_onboard(uint32_t now)
{
	if (g_twi1_hygro_valid) {
		isr_twi1_hygro(now, false);
	}

	if (g_twi1_gyro_valid) {
		isr_twi1_gyro(now, false);
	}

	if (g_twi1_baro_valid) {
		isr_twi1_baro(now, false);
	}
}


static void task_twi1_hygro(uint32_t now)
{	// Calculations for the presentation layer
	static uint16_t s_twi1_hygro_S_T	= 0UL;
	static uint16_t s_twi1_hygro_S_RH	= 0UL;

	irqflags_t flags = cpu_irq_save();
	uint16_t l_twi1_hygro_S_T	= g_twi1_hygro_S_T;
	uint16_t l_twi1_hygro_S_RH	= g_twi1_hygro_S_RH;
	cpu_irq_restore(flags);

	/* Calculate and present Temp value when a different measurement has arrived */
	if (l_twi1_hygro_S_T != s_twi1_hygro_S_T) {
		int16_t temp_100 = (int16_t)((((int32_t)l_twi1_hygro_S_T  * 17500) / 0xFFFF) - 4500);

		flags = cpu_irq_save();
		g_twi1_hygro_T_100 = temp_100;
		cpu_irq_restore(flags);
	}

	/* Calculate and present Hygro value when a different measurement has arrived */
	if (l_twi1_hygro_S_RH != s_twi1_hygro_S_RH) {
		int16_t rh_100 = (int16_t)( ((int32_t)l_twi1_hygro_S_RH * 10000) / 0xFFFF);

		flags = cpu_irq_save();
		g_twi1_hygro_RH_100 = rh_100;
		cpu_irq_restore(flags);
	}
}

static void task_twi1_gyro(uint32_t now)
{	// Calculations for the presentation layer
	irqflags_t flags = cpu_irq_save();
	int16_t	l_twi1_gyro_1_accel_x	= g_twi1_gyro_1_accel_x;
	int16_t	l_twi1_gyro_1_accel_y	= g_twi1_gyro_1_accel_y;
	int16_t	l_twi1_gyro_1_accel_z	= g_twi1_gyro_1_accel_z;
	int16_t l_twi1_gyro_1_gyro_x	= g_twi1_gyro_1_gyro_x;
	int16_t l_twi1_gyro_1_gyro_y	= g_twi1_gyro_1_gyro_y;
	int16_t l_twi1_gyro_1_gyro_z	= g_twi1_gyro_1_gyro_z;
	int16_t	l_twi1_gyro_1_temp		= g_twi1_gyro_1_temp;
	int16_t l_twi1_gyro_2_mag_x		= g_twi1_gyro_2_mag_x;
	int16_t l_twi1_gyro_2_mag_y		= g_twi1_gyro_2_mag_y;
	int16_t l_twi1_gyro_2_mag_z		= g_twi1_gyro_2_mag_z;
	cpu_irq_restore(flags);

	int16_t l_twi1_gyro_1_accel_x_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_x, g_twi1_gyro_1_accel_factx);
	int16_t l_twi1_gyro_1_accel_y_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_y, g_twi1_gyro_1_accel_facty);
	int16_t l_twi1_gyro_1_accel_z_mg	= calc_gyro1_accel_raw2mg(l_twi1_gyro_1_accel_z, g_twi1_gyro_1_accel_factz);
	int32_t l_twi1_gyro_1_gyro_x_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_x);
	int32_t l_twi1_gyro_1_gyro_y_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_y);
	int32_t l_twi1_gyro_1_gyro_z_mdps	= calc_gyro1_gyro_raw2mdps(l_twi1_gyro_1_gyro_z);
	int32_t l_twi1_gyro_2_mag_x_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_x, g_twi1_gyro_2_asax, g_twi1_gyro_2_mag_factx);
	int32_t l_twi1_gyro_2_mag_y_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_y, g_twi1_gyro_2_asay, g_twi1_gyro_2_mag_facty);
	int32_t l_twi1_gyro_2_mag_z_nT		= calc_gyro2_correct_mag_2_nT(l_twi1_gyro_2_mag_z, g_twi1_gyro_2_asaz, g_twi1_gyro_2_mag_factz);
	int16_t	l_twi1_gyro_1_temp_deg_100	= calc_gyro1_temp_raw2C100(l_twi1_gyro_1_temp);

	flags = cpu_irq_save();
	g_twi1_gyro_1_accel_x_mg	= l_twi1_gyro_1_accel_x_mg;
	g_twi1_gyro_1_accel_y_mg	= l_twi1_gyro_1_accel_y_mg;
	g_twi1_gyro_1_accel_z_mg	= l_twi1_gyro_1_accel_z_mg;
	g_twi1_gyro_1_gyro_x_mdps	= l_twi1_gyro_1_gyro_x_mdps;
	g_twi1_gyro_1_gyro_y_mdps	= l_twi1_gyro_1_gyro_y_mdps;
	g_twi1_gyro_1_gyro_z_mdps	= l_twi1_gyro_1_gyro_z_mdps;
	g_twi1_gyro_2_mag_x_nT		= l_twi1_gyro_2_mag_x_nT;
	g_twi1_gyro_2_mag_y_nT		= l_twi1_gyro_2_mag_y_nT;
	g_twi1_gyro_2_mag_z_nT		= l_twi1_gyro_2_mag_z_nT;
	g_twi1_gyro_1_temp_deg_100	= l_twi1_gyro_1_temp_deg_100;
	cpu_irq_restore(flags);
}

static void task_twi1_baro(uint32_t now)
{	// Calculations for the presentation layer
	static uint32_t s_twi1_baro_d1 = 0UL;
	static uint32_t s_twi1_baro_d2 = 0UL;

	irqflags_t flags = cpu_irq_save();
	uint32_t l_twi1_baro_d1 = g_twi1_baro_d1;
	uint32_t l_twi1_baro_d2 = g_twi1_baro_d2;
	cpu_irq_restore(flags);

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
		int32_t p = (int32_t)((((l_twi1_baro_d1 * sens) >> 21) - off) >> 15);

		flags = cpu_irq_save();
		g_twi1_baro_temp_100 = temp;
		g_twi1_baro_p_100    = p;
		cpu_irq_restore(flags);
	}
}

/* TWI1 - onboard devices */
void task_twi1_onboard(uint32_t now)
{
	irqflags_t flags = cpu_irq_save();
	bool l_twi1_hygro_valid	= g_twi1_hygro_valid;
	bool l_twi1_gyro_valid	= g_twi1_gyro_valid;
	bool l_twi1_baro_valid	= g_twi1_baro_valid;
	cpu_irq_restore(flags);

	if (l_twi1_hygro_valid) {
		task_twi1_hygro(now);
	}

	if (l_twi1_gyro_valid) {
		task_twi1_gyro(now);
	}

	if (l_twi1_baro_valid) {
		task_twi1_baro(now);
	}
}


static void task_twi2_lcd_cls(void)
{
	twi2_waitUntilReady();
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_CLS;
	twi2_packet.length = 0;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_pos_xy(uint8_t x, uint8_t y)
{
	twi2_waitUntilReady();
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_POS_X_Y;
	twi2_m_data[0] = x;
	twi2_m_data[1] = y;
	twi2_packet.length = 2;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_str(uint8_t x, uint8_t y, const char* str)
{
	uint8_t slen = strlen(str);
	if (!slen) {
		return;
	}

	while (slen) {
		uint8_t this_len = slen;
		if (this_len > TWI2_STR_MAXLEN) {
			this_len = TWI2_STR_MAXLEN;
		}

		/* Chunk of the string */
		{
			task_twi2_lcd_pos_xy(x, y);

			twi2_waitUntilReady();
			twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
			twi2_m_data[0] = this_len;
			for (uint8_t idx = 1; idx <= this_len; ++idx) {
				twi2_m_data[idx] = *(str++);
			}
			twi2_packet.length = this_len + 1;
			twi_master_write(&TWI2_MASTER, &twi2_packet);
			delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
		}

		x    += this_len * 6;
		slen -= this_len;
	}
}

static void task_twi2_lcd_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
	task_twi2_lcd_pos_xy(x1, y1);

	twi2_waitUntilReady();
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_LINE;
	twi2_m_data[0] = x2;
	twi2_m_data[1] = y2;
	twi2_m_data[2] = color;
	twi2_packet.length = 3;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled, uint8_t color)
{
	task_twi2_lcd_pos_xy(x, y);

	twi2_waitUntilReady();
	twi2_packet.addr[0] = filled ?  TWI_SMART_LCD_CMD_DRAW_FILLED_RECT : TWI_SMART_LCD_CMD_DRAW_RECT;
	twi2_m_data[0] = width;
	twi2_m_data[1] = height;
	twi2_m_data[2] = color;
	twi2_packet.length = 3;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_circ(uint8_t x, uint8_t y, uint8_t radius, bool filled, uint8_t color)
{
	task_twi2_lcd_pos_xy(x, y);

	twi2_waitUntilReady();
	twi2_packet.addr[0] = filled ?  TWI_SMART_LCD_CMD_DRAW_FILLED_CIRC : TWI_SMART_LCD_CMD_DRAW_CIRC;
	twi2_m_data[0] = radius;
	twi2_m_data[1] = color;
	twi2_packet.length = 2;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_header(void)
{
	uint8_t line;

	/* The header line */
	task_twi2_lcd_cls();
	task_twi2_lcd_str(6 * 10, 2, "FindMeSAT");
	task_twi2_lcd_str(6 * 30, 2, "by DF4IAH");

	/* A tiny satellite */
	task_twi2_lcd_circ( 9, 4, 3, true, 1);
	task_twi2_lcd_rect( 1, 2, 6, 4, false, 1);
	task_twi2_lcd_rect(12, 2, 6, 4, false, 1);

	/* Header line separator */
	task_twi2_lcd_line(0, 11, 239, 11, 1);

	if (g_adc_enabled) {
		/* Left measurement names */
		line = 2;
		task_twi2_lcd_str(6 *  0, (line++) * 10, "mP Temp =");
		task_twi2_lcd_str(6 *  3, (line++) * 10,    "Vusb =");
		task_twi2_lcd_str(6 *  3, (line++) * 10,    "Vbat =");
		task_twi2_lcd_str(6 *  0, (line++) * 10, "Vvctcxo =");
		task_twi2_lcd_str(6 *  0, (line++) * 10, "Vioadc4 =");
		task_twi2_lcd_str(6 *  0, (line++) * 10, "Vioadc5 =");
		//task_twi2_lcd_str(6 *  0, (line++) * 10, "Vsilen. =");
		line++;
	} else {
		line = 9;
	}

	task_twi2_lcd_str(6 *  0, (line++) * 10, "Ba_Temp =");
	task_twi2_lcd_str(6 *  0, (line++) * 10, "Ba_Pres =");
	task_twi2_lcd_str(6 *  0, (line++) * 10, "Hy_Temp =");
	task_twi2_lcd_str(6 *  0, (line++) * 10, "Hy_RelH =");

	if (g_adc_enabled) {
		/* Left measurement units */
		line = 2;
		task_twi2_lcd_str(6 * 16, (line++) * 10, "C");
		task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		//task_twi2_lcd_str(6 * 16, (line++) * 10, "V");
		line++;
	} else {
		line = 9;
	}

	task_twi2_lcd_str(6 * 16, (line++) * 10, "C");
	task_twi2_lcd_str(6 * 18, (line++) * 10, "hPa");
	task_twi2_lcd_str(6 * 16, (line++) * 10, "C");
	task_twi2_lcd_str(6 * 16, (line++) * 10, "%");

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

		task_twi2_lcd_str(plot_gyro_center_x_X - 4, plot_gyro_center_y + plot_gyro_radius + 4, "Gx");
		task_twi2_lcd_str(plot_gyro_center_x_Y - 4, plot_gyro_center_y + plot_gyro_radius + 4, "Gy");
		task_twi2_lcd_str(plot_gyro_center_x_Z - 4, plot_gyro_center_y + plot_gyro_radius + 4, "Gz");
	}

	/* Magnetic & Accel */
	{
		task_twi2_lcd_str(113, 72, "Magnetics");
		task_twi2_lcd_str(196, 72, "Accel.");
	}
}

static void task_twi2_lcd_print_format_uint16(uint8_t x, uint8_t y, int16_t adc_i, int16_t adc_f, const char* fmt)
{
	task_twi2_lcd_pos_xy(x, y);

	twi2_waitUntilReady();
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
	twi2_m_data[0] = sprintf((char*)&(twi2_m_data[1]), fmt, adc_i, adc_f);
	twi2_packet.length = twi2_m_data[0] + 1;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

static void task_twi2_lcd_print_format_uint32(uint8_t x, uint8_t y, int32_t adc_i, int32_t adc_f, const char* fmt)
{
	task_twi2_lcd_pos_xy(x, y);

	twi2_waitUntilReady();
	twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
	twi2_m_data[0] = sprintf((char*)&(twi2_m_data[1]), fmt, adc_i, adc_f);
	twi2_packet.length = twi2_m_data[0] + 1;
	twi_master_write(&TWI2_MASTER, &twi2_packet);
	delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
}

/* TWI2 - LCD Port */
void task_twi2_lcd(uint32_t now)
{
	static uint16_t s_lcd_entry_cnt =  0U;
	static uint32_t s_lcd_last		= 0UL;

	if (g_twi2_lcd_version >= 0x11) {
		//static uint8_t s_ofs = 0;

		/* Show current measurement data on the LCD */
		if (((now - s_lcd_last) >= 512) || (now < s_lcd_last)) {
			const uint8_t col_left = 6 * 10;
			uint8_t line = 2;

			s_lcd_last = now;

			/* Get up-to-date global data */
			irqflags_t flags = cpu_irq_save();
			int16_t l_adc_vctcxo_volt_1000		= g_adc_vctcxo_volt_1000;
			int16_t l_adc_5v0_volt_1000			= g_adc_5v0_volt_1000;
			int16_t l_adc_vbat_volt_1000		= g_adc_vbat_volt_1000;
			int16_t l_adc_temp_deg_100			= g_adc_temp_deg_100;
			int16_t l_adc_io_adc4_volt_1000		= g_adc_io_adc4_volt_1000;
			int16_t l_adc_io_adc5_volt_1000		= g_adc_io_adc5_volt_1000;
			//int16_t l_adc_silence_volt_1000	= g_adc_silence_volt_1000;
			int32_t l_twi1_baro_temp_100		= g_twi1_baro_temp_100;
			int32_t l_twi1_baro_p_100			= g_twi1_baro_p_100;
			int16_t l_twi1_hygro_T_100			= g_twi1_hygro_T_100;
			int16_t l_twi1_hygro_RH_100			= g_twi1_hygro_RH_100;
			int16_t l_twi1_gyro_1_accel_x_mg	= g_twi1_gyro_1_accel_x_mg;
			int16_t l_twi1_gyro_1_accel_y_mg	= g_twi1_gyro_1_accel_y_mg;
			int16_t l_twi1_gyro_1_accel_z_mg	= g_twi1_gyro_1_accel_z_mg;
			int32_t l_twi1_gyro_1_gyro_x_mdps	= g_twi1_gyro_1_gyro_x_mdps;
			int32_t l_twi1_gyro_1_gyro_y_mdps	= g_twi1_gyro_1_gyro_y_mdps;
			int32_t l_twi1_gyro_1_gyro_z_mdps	= g_twi1_gyro_1_gyro_z_mdps;
			int32_t l_twi1_gyro_2_mag_x_nT		= g_twi1_gyro_2_mag_x_nT;
			int32_t l_twi1_gyro_2_mag_y_nT		= g_twi1_gyro_2_mag_y_nT;
			int32_t l_twi1_gyro_2_mag_z_nT		= g_twi1_gyro_2_mag_z_nT;
			cpu_irq_restore(flags);

			/* Repaint all items when starting and at some interval */
			if (!(s_lcd_entry_cnt++)) {
				task_twi2_lcd_header();
			#if 1
			} else if (s_lcd_entry_cnt >= (300 * 2)) {  // Repaint after five minutes
				s_lcd_entry_cnt = 0;
			#endif
			}

			/* Gfx update twice a second */
			{
				/* Mag lines */
				{
					const uint8_t plot_intensity_center_x = 115;
					const uint8_t plot_intensity_center_y =  64;
					const uint8_t plot_mag_center_x = 150;
					const uint8_t plot_mag_center_y =  40;
					static float  s_length = 0.f;
					static int8_t s_p1x = 0;
					static int8_t s_p1y = 0;
					static int8_t s_p2x = 0;
					static int8_t s_p2y = 0;
					static int8_t s_p3x = 0;
					static int8_t s_p3y = 0;

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

				/* Accel lines */
				{
					const uint8_t plot_accel_center_x = 210;
					const uint8_t plot_accel_center_y =  40;
					static int8_t s_p1x = 0;
					static int8_t s_p1y = 0;
					static int8_t s_p2x = 0;
					static int8_t s_p2y = 0;
					static int8_t s_p3x = 0;
					static int8_t s_p3y = 0;

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
				}

				/* Gyro lines */
				{
					const uint8_t plot_gyro_center_x_X	= 150;
					const uint8_t plot_gyro_center_x_Y	= 150 + 30;
					const uint8_t plot_gyro_center_x_Z	= 150 + 60;
					const uint8_t plot_gyro_center_y	= 100;
					const uint8_t plot_gyro_radius		= 12;
					static float s_rads_x = 0.f;
					static float s_rads_y = 0.f;
					static float s_rads_z = 0.f;
					float rads_x = (l_twi1_gyro_1_gyro_x_mdps * M_PI) / 180000.f;
					float rads_y = (l_twi1_gyro_1_gyro_y_mdps * M_PI) / 180000.f;
					float rads_z = (l_twi1_gyro_1_gyro_z_mdps * M_PI) / 180000.f;

					/* Remove old lines */
					task_twi2_lcd_line(plot_gyro_center_x_X, plot_gyro_center_y, plot_gyro_center_x_X - plot_gyro_radius * sin(s_rads_x), plot_gyro_center_y - plot_gyro_radius * cos(s_rads_x), 0);
					task_twi2_lcd_line(plot_gyro_center_x_Y, plot_gyro_center_y, plot_gyro_center_x_Y + plot_gyro_radius * sin(s_rads_y), plot_gyro_center_y - plot_gyro_radius * cos(s_rads_y), 0);
					task_twi2_lcd_line(plot_gyro_center_x_Z, plot_gyro_center_y, plot_gyro_center_x_Z - plot_gyro_radius * sin(s_rads_z), plot_gyro_center_y - plot_gyro_radius * cos(s_rads_z), 0);

					/* Draw new lines */
					task_twi2_lcd_line(plot_gyro_center_x_X, plot_gyro_center_y, plot_gyro_center_x_X - plot_gyro_radius * sin(rads_x), plot_gyro_center_y - plot_gyro_radius * cos(rads_x), 1);
					task_twi2_lcd_line(plot_gyro_center_x_Y, plot_gyro_center_y, plot_gyro_center_x_Y + plot_gyro_radius * sin(rads_y), plot_gyro_center_y - plot_gyro_radius * cos(rads_y), 1);
					task_twi2_lcd_line(plot_gyro_center_x_Z, plot_gyro_center_y, plot_gyro_center_x_Z - plot_gyro_radius * sin(rads_z), plot_gyro_center_y - plot_gyro_radius * cos(rads_z), 1);

					/* Store new set */
					s_rads_x = rads_x;
					s_rads_y = rads_y;
					s_rads_z = rads_z;
				}
			}

			/* Text update each second */
			if (!(s_lcd_entry_cnt % 2)) {
				if (g_adc_enabled) {
					/* ADC_TEMP */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_temp_deg_100 / 100,      (l_adc_temp_deg_100 / 10) % 10,  "%02d.%01d");

					/* ADC_5V0 */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_5v0_volt_1000 / 1000,     l_adc_5v0_volt_1000 % 1000,     "%1d.%03d");

					/* ADC_VBAT */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_vbat_volt_1000 / 1000,    l_adc_vbat_volt_1000 % 1000,    "%1d.%03d");

					/* ADC_VCTCXO */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_vctcxo_volt_1000 / 1000,  l_adc_vctcxo_volt_1000 % 1000,  "%1d.%03d");

					/* ADC_IO_ADC4 */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_io_adc4_volt_1000 / 1000, l_adc_io_adc4_volt_1000 % 1000, "%1d.%03d");

					/* ADC_IO_ADC5 */
					task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_io_adc5_volt_1000 / 1000, l_adc_io_adc5_volt_1000 % 1000, "%1d.%03d");

					/* ADC_SILENCE */
					//task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_adc_silence_volt_1000 / 1000, l_adc_silence_volt_1000 % 1000, "%1d.%03d");
					line++;
				} else {
					line = 9;
				}

				/* Baro_Temp */
				task_twi2_lcd_print_format_uint32(col_left, (line++) * 10, l_twi1_baro_temp_100 / 100,     l_twi1_baro_temp_100 % 100,     "%02ld.%02ld");

				/* Baro_P */
				task_twi2_lcd_print_format_uint32(col_left, (line++) * 10, l_twi1_baro_p_100 / 100,        l_twi1_baro_p_100 % 100,        "%04ld.%02ld");

				/* Hygro_Temp */
				task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_twi1_hygro_T_100 / 100,       l_twi1_hygro_T_100 % 100,       "%02d.%02d");

				/* Hygro_RH */
				task_twi2_lcd_print_format_uint16(col_left, (line++) * 10, l_twi1_hygro_RH_100 / 100,      l_twi1_hygro_RH_100 % 100,      "%02d.%02d");
			}
		}

	} else if (g_twi2_lcd_version == 0x10) {
		/* Show PWM in % when version is V1.0 and mode==0x20 selected */
		twi2_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SHOW_TCXO_PWM;
		twi2_m_data[0] = 1;
		twi2_m_data[1] = 128;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_TCXOPWM_DELAY_MIN_US);
	}
}
