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

#include "twi.h"


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


static void twi_waitUntilReady(void)
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

	// Give Smart-LCD some time being up and ready
	delay_s(2);

	/* Start each TWI channel devices */
	start_twi_onboard();
	start_twi_lcd();
}

/* TWI1 - Gyro, Baro, Hygro, SIM808 devices */
void start_twi_onboard()
{

}

/* TWI2 - LCD Port */
void start_twi_lcd()
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

		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_PIXEL_TYPE;
		twi2_m_data[0] = GFX_PIXEL_SET;
		twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
	}
}


/* TWI1 - Gyro, Baro, Hygro, SIM808 devices */
void task_twi_onboard(uint32_t now, uint32_t last)
{

}

/* TWI2 - LCD Port */
void task_twi_lcd(uint32_t now, uint32_t last)
{
	if (g_twi2_lcd_version >= 0x11) {
		static uint8_t ofs = 0;

		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_POS_X_Y;
		twi2_m_data[0] = 16 + ofs;
		twi2_m_data[1] = 16 + ofs;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);

#if 1
		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_LINE;
		twi2_m_data[0] = 150 + ofs;
		twi2_m_data[1] =  60 + ofs;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
#endif

#if 1
		twi_waitUntilReady();
# if 1
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_RECT;
# else
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_FILLED_RECT;
# endif
		twi2_m_data[0] = 30;
		twi2_m_data[1] =  30;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
#endif

#if 1
		twi_waitUntilReady();
# if 1
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_CIRC;
# else
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_DRAW_FILLED_CIRC;
# endif
		twi2_m_data[0] = 20;
		twi2_packet.length = 1;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
#endif

#if 1
		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SET_POS_X_Y;
		twi2_m_data[0] = 116 + ofs;
		twi2_m_data[1] =  16 + ofs;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);

		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_WRITE;
		twi2_m_data[0] = 4;
		twi2_m_data[1] = 'A';
		twi2_m_data[2] = 'B';
		twi2_m_data[3] = 'C';
		twi2_m_data[4] = 'D';
		twi2_packet.length = twi2_m_data[0] + 1;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
#endif

		if (++ofs > 64) {
			ofs = 0;

			twi_waitUntilReady();
			twi2_packet.addr[0] = TWI_SMART_LCD_CMD_CLS;
			twi2_packet.length = 0;
			twi_master_write(&TWI2_MASTER, &twi2_packet);
			delay_us(TWI_SMART_LCD_DEVICE_SIMPLE_DELAY_MIN_US);
		}

	} else if (g_twi2_lcd_version == 0x10) {
#if 1
		/* Show PWM in % when version is V1.0 and mode==0x20 selected */
		twi_waitUntilReady();
		twi2_packet.addr[0] = TWI_SMART_LCD_CMD_SHOW_TCXO_PWM;
		twi2_m_data[0] = 1;
		twi2_m_data[1] = 128;
		twi2_packet.length = 2;
		twi_master_write(&TWI2_MASTER, &twi2_packet);
		delay_us(TWI_SMART_LCD_DEVICE_TCXOPWM_DELAY_MIN_US);
#endif
	}
}
