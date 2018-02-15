/*
 * externals.h
 *
 * Created: 10.09.2017 19:51:58
 *  Author: DF4IAH
 */ 


#ifndef EXTERNALS_H_
#define EXTERNALS_H_


#include "interpreter.h"
#include "serial_sim.h"
#include "twi_1_2.h"
#include "spi_ax.h"


extern bool							g_adc_enabled;
extern bool							g_dac_enabled;
extern int16_t						g_backlight_mode_pwm;
extern uint8_t						g_bias_pm;
extern uint8_t						g_pitch_tone_mode;
extern bool							g_errorBeep_enable;
extern bool							g_keyBeep_enable;

extern uint64_t						g_milliseconds_cnt64;
extern uint32_t						g_boot_time_ts;

extern uint16_t						g_1pps_last_lo;
extern uint64_t						g_1pps_last_hi;
extern int16_t						g_1pps_last_diff;
extern bool							g_1pps_last_inSpan;
extern bool							g_1pps_last_new;
extern bool							g_1pps_last_adjust;
extern uint16_t						g_1pps_processed_lo;
extern uint64_t						g_1pps_processed_hi;
extern bool							g_1pps_proceeded_avail;
extern uint8_t						g_1pps_processed_outOfSync;
extern int16_t						g_1pps_deviation;
extern bool							g_1pps_printtwi_avail;
extern bool							g_1pps_printusb_avail;
extern uint8_t						g_1pps_phased_cntr;
extern uint8_t						g_1pps_led;
extern bool							g_1pps_twi_new;

extern uint8_t						g_gns_run_status;
extern uint8_t						g_gns_fix_status;
extern float						g_gns_lat;
extern float						g_gns_lon;
extern float						g_gns_msl_alt_m;
extern float						g_gns_speed_kn;
extern float						g_gns_speed_kmPh;
extern float						g_gns_course_deg;
extern uint8_t						g_gns_fix_mode;
extern float						g_gns_dop_h;
extern float						g_gns_dop_p;
extern float						g_gns_dop_v;
extern uint8_t						g_gns_gps_sats_inView;
extern uint8_t						g_gns_gnss_sats_used;
extern uint8_t						g_gns_glonass_sats_inView;
extern uint8_t						g_gns_cPn0_dBHz;
//extern uint16_t					g_gns_hpa_m;
//extern uint16_t					g_gns_vpa_m;

extern uint8_t						g_aprs_mode;
extern char							g_aprs_link_service[C_APRS_LINK_SERVICE_LEN];
extern char							g_aprs_link_user[C_APRS_LINK_USER_LEN];
extern char							g_aprs_link_pwd[C_APRS_LINK_PWD_LEN];
extern uint8_t						g_aprs_ip_proto;
extern char							g_aprs_ip_name[C_APRS_IP_NAME_LEN];
extern uint16_t						g_aprs_ip_port;
extern char							g_aprs_source_callsign[C_APRS_S_LEN];
extern char							g_aprs_source_ssid[C_APRS_SSID_LEN];
extern char							g_aprs_login_user[C_APRS_USER_LEN];
extern char							g_aprs_login_pwd[C_APRS_PWD_LEN];
extern uint64_t						g_aprs_alert_last;
extern APRS_ALERT_FSM_STATE_ENUM_t	g_aprs_alert_fsm_state;
extern APRS_ALERT_REASON_ENUM_t		g_aprs_alert_reason;
extern float						g_aprs_pos_anchor_lat;
extern float						g_aprs_pos_anchor_lon;
extern int32_t						g_aprs_alert_1_gyro_x_mdps;
extern int32_t						g_aprs_alert_1_gyro_y_mdps;
extern int32_t						g_aprs_alert_1_gyro_z_mdps;
extern int16_t						g_aprs_alert_1_accel_x_mg;
extern int16_t						g_aprs_alert_1_accel_y_mg;
extern int16_t						g_aprs_alert_1_accel_z_mg;
extern int32_t						g_aprs_alert_2_mag_x_nT;
extern int32_t						g_aprs_alert_2_mag_y_nT;
extern int32_t						g_aprs_alert_2_mag_z_nT;

extern bool							g_usb_cdc_stdout_enabled;
extern bool							g_usb_cdc_printStatusLines_atxmega;
extern bool							g_usb_cdc_printStatusLines_sim808;
extern bool							g_usb_cdc_printStatusLines_1pps;
extern bool							g_usb_cdc_rx_received;
extern bool							g_usb_cdc_transfers_authorized;
extern bool							g_usb_cdc_access_blocked;
extern WORKMODE_ENUM_t				g_workmode;

extern uint8_t						g_usart_gprs_auto_response_state;
extern bool							g_usart1_rx_ready;
extern bool							g_usart1_rx_OK;
extern uint16_t						g_usart1_rx_idx;
extern char							g_usart1_rx_buf[C_USART1_RX_BUF_LEN];
extern uint16_t						g_usart1_tx_len;
extern char							g_usart1_tx_buf[C_USART1_TX_BUF_LEN];

extern bool							g_gsm_enable;
extern bool							g_gsm_aprs_enable;
extern bool							g_gsm_aprs_gprs_connected;
extern bool							g_gsm_aprs_ip_connected;
extern char							g_gsm_cell_lac[C_GSM_CELL_LAC_LEN];
extern char							g_gsm_cell_ci[C_GSM_CELL_CI_LEN];
extern char							g_gsm_login_pwd[C_GSM_PIN_BUF_LEN];
extern uint8_t						g_gsm_ring;

extern bool							g_twi1_gsm_valid;
extern uint8_t						g_twi1_gsm_version;

extern bool							g_twi1_gyro_valid;
extern uint8_t						g_twi1_gyro_1_version;
extern int16_t						g_twi1_gyro_1_temp;
extern int16_t						g_twi1_gyro_1_temp_RTofs;
extern int16_t						g_twi1_gyro_1_temp_sens;
extern int16_t						g_twi1_gyro_1_temp_deg_100;
extern int16_t						g_twi1_gyro_1_accel_x;
extern int16_t						g_twi1_gyro_1_accel_y;
extern int16_t						g_twi1_gyro_1_accel_z;
extern int16_t						g_twi1_gyro_1_accel_ofsx;
extern int16_t						g_twi1_gyro_1_accel_ofsy;
extern int16_t						g_twi1_gyro_1_accel_ofsz;
extern int16_t						g_twi1_gyro_1_accel_factx;
extern int16_t						g_twi1_gyro_1_accel_facty;
extern int16_t						g_twi1_gyro_1_accel_factz;
extern int16_t						g_twi1_gyro_1_accel_x_mg;
extern int16_t						g_twi1_gyro_1_accel_y_mg;
extern int16_t						g_twi1_gyro_1_accel_z_mg;
extern int16_t						g_twi1_gyro_1_gyro_x;
extern int16_t						g_twi1_gyro_1_gyro_y;
extern int16_t						g_twi1_gyro_1_gyro_z;
extern int16_t						g_twi1_gyro_1_gyro_ofsx;
extern int16_t						g_twi1_gyro_1_gyro_ofsy;
extern int16_t						g_twi1_gyro_1_gyro_ofsz;
extern int32_t						g_twi1_gyro_1_gyro_x_mdps;
extern int32_t						g_twi1_gyro_1_gyro_y_mdps;
extern int32_t						g_twi1_gyro_1_gyro_z_mdps;
extern bool							g_twi1_gyro_gyro_offset_set__flag;
extern bool							g_twi1_gyro_accel_offset_set__flag;
extern uint8_t						g_twi1_gyro_2_version;
extern int8_t						g_twi1_gyro_2_asax;
extern int8_t						g_twi1_gyro_2_asay;
extern int8_t						g_twi1_gyro_2_asaz;
extern int16_t						g_twi1_gyro_2_ofsx;
extern int16_t						g_twi1_gyro_2_ofsy;
extern int16_t						g_twi1_gyro_2_ofsz;
extern int16_t						g_twi1_gyro_2_mag_x;
extern int16_t						g_twi1_gyro_2_mag_y;
extern int16_t						g_twi1_gyro_2_mag_z;
extern int16_t						g_twi1_gyro_2_mag_factx;
extern int16_t						g_twi1_gyro_2_mag_facty;
extern int16_t						g_twi1_gyro_2_mag_factz;
extern int32_t						g_twi1_gyro_2_mag_x_nT;
extern int32_t						g_twi1_gyro_2_mag_y_nT;
extern int32_t						g_twi1_gyro_2_mag_z_nT;

extern bool							g_twi1_baro_valid;
extern uint16_t						g_twi1_baro_version;
extern uint16_t						g_twi1_baro_c[C_TWI1_BARO_C_CNT];
extern uint32_t						g_twi1_baro_d1;
extern uint32_t						g_twi1_baro_d2;
extern int32_t						g_twi1_baro_temp_100;
extern int32_t						g_twi1_baro_p_100;

extern bool							g_twi1_hygro_valid;
extern uint16_t						g_twi1_hygro_status;
extern uint16_t						g_twi1_hygro_S_T;
extern uint16_t						g_twi1_hygro_S_RH;
extern int16_t						g_twi1_hygro_T_100;
extern int16_t						g_twi1_hygro_RH_100;
extern int16_t						g_twi1_hygro_DP_100;

extern uint8_t						g_twi2_lcd_version;
extern bool							g_twi2_lcd_repaint;

extern bool							g_ax_enable;
extern bool							g_ax_aprs_enable;
extern bool							g_ax_pocsag_enable;
extern struct spi_device			g_ax_spi_device_conf;
extern uint8_t						g_ax_spi_packet_buffer[C_SPI_AX_BUFFER_LENGTH];
extern uint32_t						g_ax_spi_freq_chan[2];
extern uint8_t						g_ax_spi_range_chan[2];
extern uint8_t						g_ax_spi_vcoi_chan[2];

extern int32_t						g_xo_mode_pwm;

extern struct adc_config			g_adc_a_conf;
extern struct adc_channel_config	g_adcch_vctcxo_5v0_vbat_conf;
extern struct adc_channel_config	g_adcch_io_adc4_conf;
extern struct adc_channel_config	g_adcch_io_adc5_conf;
extern struct adc_channel_config	g_adcch_silence_conf;

extern struct adc_config			g_adc_b_conf;
extern struct adc_channel_config	g_adcch_temp_conf;

extern uint32_t						g_adc_vctcxo_cur;
extern uint32_t						g_adc_vctcxo_sum;
extern uint16_t						g_adc_vctcxo_cnt;
extern uint32_t						g_adc_5v0_cur;
extern uint32_t						g_adc_5v0_sum;
extern uint16_t						g_adc_5v0_cnt;
extern uint32_t						g_adc_vbat_cur;
extern uint32_t						g_adc_vbat_sum;
extern uint16_t						g_adc_vbat_cnt;
extern uint32_t						g_adc_io_adc4_cur;
extern uint32_t						g_adc_io_adc4_sum;
extern uint16_t						g_adc_io_adc4_cnt;
extern uint32_t						g_adc_io_adc5_cur;
extern uint32_t						g_adc_io_adc5_sum;
extern uint16_t						g_adc_io_adc5_cnt;
extern uint32_t						g_adc_silence_cur;
extern uint32_t						g_adc_silence_sum;
extern uint16_t						g_adc_silence_cnt;
extern uint32_t						g_adc_temp_cur;
extern uint32_t						g_adc_temp_sum;
extern uint16_t						g_adc_temp_cnt;
extern int16_t						g_adc_vctcxo_volt_1000;
extern int16_t						g_adc_5v0_volt_1000;
extern int16_t						g_adc_vbat_volt_1000;
extern int16_t						g_adc_io_adc4_volt_1000;
extern int16_t						g_adc_io_adc5_volt_1000;
extern int16_t						g_adc_silence_volt_1000;
extern int16_t						g_adc_temp_deg_100;

extern int16_t						g_env_temp_delta_100;
extern int16_t						g_env_temp_deg_100;
extern int16_t						g_env_hygro_RH_100;

extern bool							g_qnh_is_auto;
extern int16_t						g_qnh_height_m;
extern int32_t						g_qnh_p_h_100;

extern fifo_desc_t					g_fifo_sched_desc;
extern uint32_t						g_fifo_sched_buffer[FIFO_SCHED_BUFFER_LENGTH];

extern struct pwm_config			g_pwm_vctcxo_cfg;
extern struct pwm_config			g_pwm_ctr_pll_cfg;

extern uint8_t						g_sched_lock;
extern uint8_t						g_interpreter_lock;
extern uint8_t						g_twi1_lock;
extern bool							g_sched_yield;
extern bool							g_sched_pop_again;
extern sched_entry_t				g_sched_data[C_SCH_SLOT_CNT];
extern uint8_t						g_sched_sort[C_SCH_SLOT_CNT];

extern char							g_prepare_buf[C_TX_BUF_SIZE];

extern bool							g_ax_pocsag_chime_enable;
extern uint32_t						g_ax_pocsag_individual_ric;

extern const uint16_t				g_ax_pwr_ary[C_AX_PRW_LENGTH];
extern uint8_t						g_ax_pocsag_news_idx;
extern uint8_t						g_ax_pocsag_activation_code_len;
extern const uint8_t				g_ax_pocsag_activation_code[][3];


/* TWI communications  */

extern twi_options_t				g_twi1_options;
extern uint8_t						g_twi1_m_data[TWI_DATA_LENGTH];
extern twi_package_t				g_twi1_packet;

extern twi_options_t				g_twi2_options;
extern uint8_t						g_twi2_m_data[TWI_DATA_LENGTH];
extern twi_package_t				g_twi2_packet;

#ifdef TWI1_SLAVE
extern TWI_Slave_t					g_twi1_slave;
extern uint8_t						g_twi1_recv_data[DATA_LENGTH];
#endif

#ifdef TWI2_SLAVE
extern TWI_Slave_t					g_twi2_slave;
extern uint8_t						g_twi2_recv_data[DATA_LENGTH];
#endif


#endif /* EXTERNALS_H_ */
