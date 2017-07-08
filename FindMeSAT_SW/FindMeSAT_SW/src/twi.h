/*
 * \file
 *
 * \brief FindMeSAT
 * twi.h
 *
 * Created: 07.06.2017 22:48:48
 * Author : DF4IAH
 */


#ifndef TWI_H_
#define TWI_H_

#include <asf.h>


/* Smart-LCD address and command-set */

// I2C address of the Smart-LCD device
#define TWI_SMART_LCD_ADDR									0x22

// unique commands of the Smart-LCD device for all modes
#define TWI_SMART_LCD_CMD_NOOP								0x00
#define TWI_SMART_LCD_CMD_GET_VER							0x01
#define TWI_SMART_LCD_CMD_SET_MODE							0x02
#define TWI_SMART_LCD_CMD_GET_STATE							0x03

// mode 0x10 commands (Smart-LCD draw box)
#define TWI_SMART_LCD_CMD_CLS								0x10
#define TWI_SMART_LCD_CMD_SET_PIXEL_TYPE					0x14
#define TWI_SMART_LCD_CMD_SET_POS_X_Y						0x20
#define TWI_SMART_LCD_CMD_WRITE								0x30
#define TWI_SMART_LCD_CMD_DRAW_LINE							0x32
#define TWI_SMART_LCD_CMD_DRAW_RECT							0x34
#define TWI_SMART_LCD_CMD_DRAW_FILLED_RECT					0x36
#define TWI_SMART_LCD_CMD_DRAW_CIRC							0x38
#define TWI_SMART_LCD_CMD_DRAW_FILLED_CIRC					0x3A

// mode 0x20 commands (10 MHz-Ref-Osc)
#define TWI_SMART_LCD_CMD_SHOW_CLK_STATE					0x80
#define TWI_SMART_LCD_CMD_SHOW_YEAR_MON_DAY					0x81
#define TWI_SMART_LCD_CMD_SHOW_HR_MIN_SEC					0x82
#define TWI_SMART_LCD_CMD_SHOW_PPB							0x83

#define TWI_SMART_LCD_CMD_SHOW_TCXO_PWM						0x84
#define TWI_SMART_LCD_CMD_SHOW_TCXO_VC						0x85

#define TWI_SMART_LCD_CMD_SHOW_SATS							0x88
#define TWI_SMART_LCD_CMD_SHOW_DOP							0x89
#define TWI_SMART_LCD_CMD_SHOW_POS_STATE					0x8A
#define TWI_SMART_LCD_CMD_SHOW_POS_LAT						0x8B
#define TWI_SMART_LCD_CMD_SHOW_POS_LON						0x8C
#define TWI_SMART_LCD_CMD_SHOW_POS_HEIGHT					0x8D

// Delay definitions
#define TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US			   1
#define TWI_SMART_LCD_DEVICE_BUSY_DELAY_MIN_US				 100
#define TWI_SMART_LCD_DEVICE_TCXOPWM_DELAY_MIN_US			1000


#define TWI1_MASTER											TWIE
//#define TWI1_SLAVE										TWIE
#define TWI1_MASTER_PORT									PORTE
#define TWI1_MASTER_ADDR									0x10
#define TWI1_SLAVE_ADDR										0x12
#define TWI1_SPEED											400000

//#define TWI1_SLAVE_GSM_ADDR								0x00
//#define TWI1_SLAVE_GSM_REG_1_DEVICE_ID					0x00

#define TWI1_SLAVE_GYRO_ADDR_1								0x68
#define TWI1_SLAVE_GYRO_REG_1_RESET							0x6B
#define TWI1_SLAVE_GYRO_DTA_1_RESET							0x81
#define TWI1_SLAVE_GYRO_REG_1_WHOAMI						0x75
#define TWI1_SLAVE_GYRO_ADDR_2								0x0C
#define TWI1_SLAVE_GYRO_REG_2_RESET							0x0A
#define TWI1_SLAVE_GYRO_DTA_2_RESET							0x1F
#define TWI1_SLAVE_GYRO_REG_2_DEVICE_ID						0x00

#define TWI1_SLAVE_BARO_ADDR								0x76
#define TWI1_SLAVE_BARO_REG_RESET							0x1E
#define TWI1_SLAVE_BARO_REG_VERSION							0x57
#define TWI1_SLAVE_BARO_REG_PROM							0xA0
#define TWI1_SLAVE_BARO_REG_CONV_D1_256						0x40
#define TWI1_SLAVE_BARO_REG_CONV_D1_512						0x42
#define TWI1_SLAVE_BARO_REG_CONV_D1_1024					0x44
#define TWI1_SLAVE_BARO_REG_CONV_D1_2048					0x46
#define TWI1_SLAVE_BARO_REG_CONV_D1_4096					0x48
#define TWI1_SLAVE_BARO_REG_CONV_D2_256						0x50
#define TWI1_SLAVE_BARO_REG_CONV_D2_512						0x52
#define TWI1_SLAVE_BARO_REG_CONV_D2_1024					0x54
#define TWI1_SLAVE_BARO_REG_CONV_D2_2048					0x56
#define TWI1_SLAVE_BARO_REG_CONV_D2_4096					0x58
#define TWI1_SLAVE_BARO_REG_ADC_READ						0x00

#define TWI1_SLAVE_HYGRO_ADDR								0x44
#define TWI1_SLAVE_HYGRO_REG_RESET_HI						0x30
#define TWI1_SLAVE_HYGRO_REG_RESET_LO						0xA2
#define TWI1_SLAVE_HYGRO_REG_STATUS_HI						0xF3
#define TWI1_SLAVE_HYGRO_REG_STATUS_LO						0x2D


#define TWI2_MASTER											TWIC
//#define TWI2_SLAVE										TWIC
#define TWI2_MASTER_PORT									PORTC
#define TWI2_MASTER_ADDR									0x20
#define TWI2_SLAVE_ADDR										TWI_SMART_LCD_ADDR
#define TWI2_SPEED											400000

#define TWI_DATA_LENGTH										TWIS_SEND_BUFFER_SIZE



void twi_init(void);
void twi_start(void);

void service_10ms_twi1_onboard(uint32_t now);
void service_500ms_twi1_onboard(uint32_t now);

void task_twi1_onboard(uint32_t now);
void task_twi2_lcd(uint32_t now);


#endif /* TWI_H_ */
