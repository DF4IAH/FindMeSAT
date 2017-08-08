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


#include "main.h"
#include "interpreter.h"

#include "serial.h"


extern bool				g_adc_enabled;
extern bool				g_dac_enabled;
extern int16_t			g_backlight_mode_pwm;
extern uint8_t			g_bias_pm;
extern uint8_t			g_pitch_tone_mode;
extern bool				g_errorBeep_enable;
extern bool				g_keyBeep_enable;

extern bool				g_usb_cdc_stdout_enabled;
extern bool				g_usb_cdc_printStatusLines;
extern bool				g_usb_cdc_rx_received;
extern bool				g_usb_cdc_transfers_authorized;
extern bool				g_usb_cdc_access_blocked;
extern WORKMODE_ENUM_t	g_workmode;

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

extern uint8_t			g_twi1_lock;
extern bool				g_twi1_hygro_valid;
extern uint8_t			g_twi1_hygro_status;
extern uint16_t			g_twi1_hygro_S_T;
extern uint16_t			g_twi1_hygro_S_RH;
extern int16_t			g_twi1_hygro_T_100;
extern int16_t			g_twi1_hygro_RH_100;

extern int16_t			g_adc_vctcxo_volt_1000;
extern int16_t			g_adc_5v0_volt_1000;
extern int16_t			g_adc_vbat_volt_1000;
extern int16_t			g_adc_io_adc4_volt_1000;
extern int16_t			g_adc_io_adc5_volt_1000;
extern int16_t			g_adc_silence_volt_1000;
extern int16_t			g_adc_temp_deg_100;

extern uint8_t			g_interpreter_lock;

extern char				g_prepare_buf[C_TX_BUF_SIZE];


/* Set up serial connection to the SIM808 */
void serial_init(void)
{
	
	
}

/* USB device stack start function to enable stack and start USB */
void serial_start(void)
{
	
	
}
