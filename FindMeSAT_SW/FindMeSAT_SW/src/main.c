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
#include "conf_dac.h"
#include "twi.h"

#include "main.h"


/* GLOBAL section */

uint8_t						runmode								= 0;
bool						usb_cdc_transfers_autorized			= false;

uint8_t						g_twi1_9axis_version				= 0;
uint8_t						g_twi1_baro_version					= 0;
uint8_t						g_twi1_hygro_version				= 0;
uint8_t						g_twi2_lcd_version					= 0;


struct adc_config			g_adc_a_conf						= { 0 };
struct adc_channel_config	g_adcch_vctcxo_5v0_vbat_conf		= { 0 };
struct adc_channel_config	g_adcch_io_adc4_conf				= { 0 };
struct adc_channel_config	g_adcch_io_adc5_conf				= { 0 };
struct adc_channel_config	g_adcch_temp_conf					= { 0 };

int32_t						g_adc_vctcxo_cur					= 0;
int32_t						g_adc_vctcxo_sum					= 0;
uint16_t					g_adc_vctcxo_cnt					= 0;
int32_t						g_adc_5v0_cur						= 0;
int32_t						g_adc_5v0_sum						= 0;
uint16_t					g_adc_5v0_cnt						= 0;
int32_t						g_adc_vbat_cur						= 0;
int32_t						g_adc_vbat_sum						= 0;
uint16_t					g_adc_vbat_cnt						= 0;
int32_t						g_adc_io_adc4_cur					= 0;
int32_t						g_adc_io_adc4_sum					= 0;
uint16_t					g_adc_io_adc4_cnt					= 0;
int32_t						g_adc_io_adc5_cur					= 0;
int32_t						g_adc_io_adc5_sum					= 0;
uint16_t					g_adc_io_adc5_cnt					= 0;
int32_t						g_adc_temp_cur						= 0;
int32_t						g_adc_temp_sum						= 0;
uint16_t					g_adc_temp_cnt						= 0;


struct dac_config			g_dac_io_dac0_conf					= { 0 };
struct dac_config			g_dac_io_dac1_conf					= { 0 };


char						g_prepare_buf[48]					= "";


twi_options_t twi1_options = {
	.speed     = TWI1_SPEED,
//	.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI1_SPEED),
	.speed_reg = TWI_BAUD((BOARD_XOSC_HZ * CONFIG_PLL0_MUL) / 2, TWI1_SPEED),
	.chip      = TWI1_MASTER_ADDR
};

uint8_t twi1_m_data[TWI_DATA_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

twi_package_t twi1_packet = {
	.buffer      = (void *)twi1_m_data,
	.no_wait     = false
};


twi_options_t twi2_options = {
	.speed     = TWI2_SPEED,
//	.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI2_SPEED),
	.speed_reg = TWI_BAUD((BOARD_XOSC_HZ * CONFIG_PLL0_MUL) / 2, TWI2_SPEED),
	.chip      = TWI2_MASTER_ADDR
};

uint8_t twi2_m_data[TWI_DATA_LENGTH] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

twi_package_t twi2_packet = {
	.chip        = TWI2_SLAVE_ADDR,
	.buffer      = (void *)twi2_m_data,
	.no_wait     = false
};

#ifdef TWI1_SLAVE
TWI_Slave_t		twi1_slave;
uint8_t			twi1_recv_data[DATA_LENGTH];
#endif

#ifdef TWI2_SLAVE
TWI_Slave_t		twi2_slave;
uint8_t			twi2_recv_data[DATA_LENGTH];
#endif


/* STATIC section for this module */

static uint16_t dac_io_dac0_buf[DAC_NR_OF_SAMPLES] = {
	32768, 35325, 37784, 40050, 42036, 43666, 44877, 45623,
	45875, 45623, 44877, 43666, 42036, 40050, 37784, 35325,
	32768, 30211, 27752, 25486, 23500, 21870, 20659, 19913,
	19661, 19913, 20659, 21870, 23500, 25486, 27752, 30211,
};

static uint16_t dac_io_dac1_buf[DAC_NR_OF_SAMPLES] = {
	32768, 35325, 37784, 40050, 42036, 43666, 44877, 45623,
	45875, 45623, 44877, 43666, 42036, 40050, 37784, 35325,
	32768, 30211, 27752, 25486, 23500, 21870, 20659, 19913,
	19661, 19913, 20659, 21870, 23500, 25486, 27752, 30211,
};



/* INIT section */

static void evsys_init(void)
{
	sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_EVSYS);

	/* ADC - event channels 0, 1, 2, 3 */
	EVSYS.CH0MUX  = EVSYS_CHMUX_TCC0_CCC_gc;									// TCC0 CC-C goes to EVSYS CH0
	EVSYS.CH0CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH0 no digital filtering
	EVSYS.CH1MUX  = EVSYS_CHMUX_TCC0_CCC_gc;									// TCC0 CC-C goes to EVSYS CH1
	EVSYS.CH1CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH1 no digital filtering
	EVSYS.CH2MUX  = EVSYS_CHMUX_TCC0_CCC_gc;									// TCC0 CC-C goes to EVSYS CH2
	EVSYS.CH2CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH2 no digital filtering
	EVSYS.CH3MUX  = EVSYS_CHMUX_TCC0_CCC_gc;									// TCC0 CC-C goes to EVSYS CH3
	EVSYS.CH3CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH3 no digital filtering

	/* DAC - event 7 */
	EVSYS.CH7MUX = EVSYS_CHMUX_TCE1_OVF_gc;										// TCE1 overflow goes to EVSYS CH7
	EVSYS.CH7CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH7 no digital filtering
}


static void tc_init(void)
{
	/* TCC0: VCTCXO PWM signal generation and ADCA CH0 */
	struct pwm_config pwm_vctcxo_cfg;
	pwm_init(&pwm_vctcxo_cfg, PWM_TCC0, PWM_CH_C, 2560);						// Init PWM structure and enable timer
	pwm_start(&pwm_vctcxo_cfg, 45);												// Start PWM. Percentage with 1% granularity is to coarse, use driver access instead
	tc_write_cc_buffer(&TCC0, TC_CCC, (uint16_t) (0.5f + pwm_vctcxo_cfg.period * C_VCTCXO_DEFAULT_VOLTS / C_VCTCXO_PWM_HI_VOLTS));	// Initial value for VCTCXO

	/* TCE1: DAC clock */
	tc_enable(&TCE1);
	tc_set_wgm(&TCE1, TC_WG_NORMAL);											// Internal clock for DAC convertion
	tc_write_period(&TCE1, (sysclk_get_per_hz() / DAC_RATE_OF_CONV) - 1);		// DAC clock 48 kHz for audio play-back
}

static void tc_start(void)
{
	/* ADC clock */
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV1_gc);

//	tc_write_clock_source(&TCC1, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCD0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCD1, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCE0, TC_CLKSEL_DIV1_gc);

	/* DAC clock */
	tc_write_clock_source(&TCE1, TC_CLKSEL_DIV1_gc);							// VCTCXO PWM start, output still is Z-state
																				// ??? Internal clock

//	tc_write_clock_source(&TCF0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCF1, TC_CLKSEL_DIV1_gc);
}


#if 0
static void rtc_start(void)
{
	PORTC_OUTSET	= 0b00100000;
	PORTC_DIRSET	= 0b00100000;

	RTC32_CTRL		= 0;				// RTC32 disabled
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);

	RTC32.PER		= 0x00000003;		// overflowing every 1024 Hz / (PER + 1)
	RTC32.CNT		= 0;				// from the beginning
	RTC32.COMP		= 0xffffffff;		// no compare
	RTC32.INTCTRL	= 0x01;				// enable overflow interrupt of low priority
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);

	RTC32.CTRL		= RTC32_ENABLE_bm;	// RTC32 enabled
	while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm);
}

ISR(RTC32_OVF_vect) {
	PORTC_OUTTGL = 0b00100000;
}
#endif

void cb_rtc_alarm(uint32_t rtc_time)
{	// Alarm call-back with the current time
	// nothing implemented yet...
}

static void adc_init(void)
{
	/* Disable input pins */
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
	adcch_read_configuration(&ADC_TEMP, ADC_TEMP_CH,						&g_adcch_temp_conf);

	/* ADC-clock request */
	adc_set_clock_rate(&g_adc_a_conf, 768000UL);												// at LEAST 16x oversampling of 48kHz for audio

	/* Enable internal ADC-A input for temperature measurement */
	adc_enable_internal_input(&g_adc_a_conf,		ADC_INT_TEMPSENSE | ADC_INT_BANDGAP);

	/* Current limitation */
	adc_set_current_limit(&g_adc_a_conf,			ADC_CURRENT_LIMIT_MED);

	/* ADC impedance */
	adc_set_gain_impedance_mode(&g_adc_a_conf,		ADC_GAIN_HIGHIMPEDANCE);

	/* PIN assignment */
	adcch_set_input(&g_adcch_vctcxo_5v0_vbat_conf,	ADCCH_POS_PIN1,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_io_adc4_conf,			ADCCH_POS_PIN4,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_io_adc5_conf,			ADCCH_POS_PIN5,			ADCCH_NEG_NONE, 1);
	adcch_set_input(&g_adcch_temp_conf,				ADCCH_POS_TEMPSENSE,	ADCCH_NEG_NONE, 1);

	/* Convertion and reference */
	adc_set_conversion_parameters(&g_adc_a_conf,	ADC_SIGN_OFF, ADC_RES_12, ADC_REF_AREFA);	// ADC-A: ADC0 (Pin 62 3V0 as reference pin

	/* PIN scan on ADC-channel 0 */
	adcch_set_pin_scan(&g_adcch_vctcxo_5v0_vbat_conf, 0, 2);									// ADC-A: scan between ADC1 .. ADC3

	/* Trigger */
	adc_set_conversion_trigger(&g_adc_a_conf, ADC_TRIG_EVENT_SINGLE, 4, 0);

	/* Interrupt service routine */
	adc_set_callback(&ADCA, cb_adc_a);

	/* Interrupt type */
	adcch_set_interrupt_mode(&g_adcch_vctcxo_5v0_vbat_conf,	ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_io_adc4_conf,			ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_io_adc5_conf,			ADCCH_MODE_COMPLETE);
	adcch_set_interrupt_mode(&g_adcch_temp_conf,			ADCCH_MODE_COMPLETE);

	/* Interrupt enable */
	adcch_enable_interrupt(&g_adcch_vctcxo_5v0_vbat_conf);
	adcch_enable_interrupt(&g_adcch_io_adc4_conf);
	adcch_enable_interrupt(&g_adcch_io_adc5_conf);
	adcch_enable_interrupt(&g_adcch_temp_conf);

	/* Execute the new settings */
	adc_write_configuration(&ADCA,											&g_adc_a_conf);
	adcch_write_configuration(&ADC_VCTCXO_5V0_VBAT, ADC_VCTCXO_5V0_VBAT_CH,	&g_adcch_vctcxo_5v0_vbat_conf);
	adcch_write_configuration(&ADC_IO_ADC4, ADC_IO_ADC4_CH,					&g_adcch_io_adc4_conf);
	adcch_write_configuration(&ADC_IO_ADC5, ADC_IO_ADC5_CH,					&g_adcch_io_adc5_conf);
	adcch_write_configuration(&ADC_TEMP, ADC_TEMP_CH,						&g_adcch_temp_conf);
}

static void adc_start(void)
{
	/* Power up after configurations are being set */
	adc_enable(&ADCA);
}

void cb_adc_a(ADC_t* adc, uint8_t ch_mask, adc_result_t res)
{
	uint8_t scan_ofs_next = (ADCA_CH0_SCAN >> 4);
	int16_t val = res - C_ADC_0V0_DELTA;

	if ((ch_mask & ADC_VCTCXO_5V0_VBAT_CH)) {
		switch (scan_ofs_next) {
			case ADC_CH0_SCAN_5V0:
				g_adc_vctcxo_sum += val;
				if (++g_adc_vctcxo_cnt >= C_ADC_SUM_CNT) {
					g_adc_vctcxo_cur = (g_adc_vctcxo_sum >> C_ADC_SUM_SHIFT);
					g_adc_vctcxo_sum = g_adc_vctcxo_cnt = 0;
				}
			break;

			case ADC_CH0_SCAN_VBAT:
				g_adc_5v0_sum += val;
				if (++g_adc_5v0_cnt >= C_ADC_SUM_CNT) {
					g_adc_5v0_cur = (g_adc_5v0_sum >> C_ADC_SUM_SHIFT);
					g_adc_5v0_sum = g_adc_5v0_cnt = 0;
				}
			break;

			case ADC_CH0_SCAN_VCTCXO:
				g_adc_vbat_sum += val;
				if (++g_adc_vbat_cnt >= C_ADC_SUM_CNT) {
					g_adc_vbat_cur = (g_adc_vbat_sum >> C_ADC_SUM_SHIFT);
					g_adc_vbat_sum = g_adc_vbat_cnt = 0;
				}
			break;
		}

	} else if (ch_mask & ADC_IO_ADC4_CH) {
		g_adc_io_adc4_sum += val;
		if (++g_adc_io_adc4_cnt >= C_ADC_SUM_CNT) {
			g_adc_io_adc4_cur = (g_adc_io_adc4_sum >> C_ADC_SUM_SHIFT);
			g_adc_io_adc4_sum = g_adc_io_adc4_cnt = 0;
		}

	} else if (ch_mask & ADC_IO_ADC5_CH) {
		g_adc_io_adc5_sum += val;
		if (++g_adc_io_adc5_cnt >= C_ADC_SUM_CNT) {
			g_adc_io_adc5_cur = (g_adc_io_adc5_sum >> C_ADC_SUM_SHIFT);
			g_adc_io_adc5_sum = g_adc_io_adc5_cnt = 0;
		}

	} else if (ch_mask & ADC_TEMP_CH) {
		g_adc_temp_sum += val;
		if (++g_adc_temp_cnt >= C_ADC_SUM_CNT) {
			g_adc_temp_cur = (g_adc_temp_sum >> C_ADC_SUM_SHIFT);
			g_adc_temp_sum = g_adc_temp_cnt = 0;
		}
	}
}


static void dac_init(void)
{
#if 0
    dac_read_configuration(&DAC_IO_DAC0, &g_dac_io_dac0_conf);
    dac_read_configuration(&DAC_IO_DAC1, &g_dac_io_dac1_conf);

    dac_set_conversion_parameters(&g_dac_io_dac0_conf, DAC_REF_BANDGAP, DAC_ADJ_LEFT);
    dac_set_conversion_parameters(&g_dac_io_dac1_conf, DAC_REF_BANDGAP, DAC_ADJ_LEFT);

    dac_set_active_channel(&g_dac_io_dac0_conf, DAC_IO_DAC0_CHANNEL, 0);
    dac_set_active_channel(&g_dac_io_dac1_conf, DAC_IO_DAC1_CHANNEL, 0);

    dac_set_conversion_trigger(&g_dac_io_dac0_conf, DAC_IO_DAC0_CHANNEL, 6);
    dac_set_conversion_trigger(&g_dac_io_dac1_conf, DAC_IO_DAC1_CHANNEL, 6);

    #ifdef XMEGA_DAC_VERSION_1
    dac_set_conversion_interval(&g_dac_io_dac0_conf, 2);
    dac_set_conversion_interval(&g_dac_io_dac1_conf, 2);
    #endif

    dac_write_configuration(&DAC_IO_DAC0, &g_dac_io_dac0_conf);
    dac_write_configuration(&DAC_IO_DAC1, &g_dac_io_dac1_conf);
#endif
}

static void dac_start(void)
{
	//dac_enable(&DACB);
}

static void usb_init(void)
{
	udc_start();

#if 1
	stdio_usb_init();	// Init and enable stdio_usb
	stdio_usb_enable();
#endif
}

void usb_callback_suspend_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Disable hardware component to reduce power consumption
	// Reduce power consumption in suspend mode (max. 2.5mA on VBUS)

}

void usb_callback_resume_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Re-enable hardware component

}

void usb_callback_remotewakeup_enable(void)
{
	/* USB wake-up remote host feature */

	// Enable application wakeup events (e.g. enable GPIO interrupt)

}

void usb_callback_remotewakeup_disable(void)
{
	/* USB wake-up remote host feature */

	// Disable application wakeup events (e.g. disable GPIO interrupt)

}

void usb_send_wakeup_event(void)
{
	/* USB wake-up remote host feature */

	udc_remotewakeup();
}

bool usb_callback_cdc_enable(void)
{
	/* USB CDC feature for serial communication */

	usb_cdc_transfers_autorized = true;
	return true;
}

void usb_callback_cdc_disable(void)
{
	/* USB CDC feature for serial communication */

	usb_cdc_transfers_autorized = false;
}

void usb_callback_config(uint8_t port, usb_cdc_line_coding_t * cfg)
{

}

void usb_callback_cdc_set_dtr(uint8_t port, bool b_enable)
{

}

void usb_callback_cdc_set_rts(uint8_t port, bool b_enable)
{

}

void usb_callback_rx_notify(uint8_t port)
{

}

void usb_callback_tx_empty_notify(uint8_t port)
{

}



/* RUNNING section */

static void task_dac(void)
{
	static int idx_dacX = 0;

#if 0
	if (dac_channel_is_ready(&DAC_IO_DAC0, DAC_CH0 | DAC_CH1)) {
		dac_set_channel_value(&DAC_IO_DAC0, DAC_IO_DAC0_CHANNEL, dac_io_dac0_buf[idx_dacX]);
		dac_set_channel_value(&DAC_IO_DAC1, DAC_IO_DAC1_CHANNEL, dac_io_dac1_buf[idx_dacX]);
		
		idx_dacX++;
		idx_dacX %= DAC_NR_OF_SAMPLES;
	}
#endif
}

static void task_adc(uint32_t now, uint32_t last)
{
	static uint32_t adc_last = 0;
	int32_t l_adc_vctcxo_cur,
			 l_adc_5v0_cur,
			 l_adc_vbat_cur,
			 l_adc_io_adc4_cur,
			 l_adc_io_adc5_cur,
			 l_adc_temp_cur;
	float	 l_temp;
	uint16_t l_temp_i;
	uint8_t	 l_temp_f;

	if ((now - adc_last) >= 512) {
		adc_last = now;

		irqflags_t flags = cpu_irq_save();
		l_adc_vctcxo_cur = g_adc_vctcxo_cur;
		l_adc_5v0_cur = g_adc_5v0_cur;
		l_adc_vbat_cur = g_adc_vbat_cur;
		l_adc_io_adc4_cur = g_adc_io_adc4_cur;
		l_adc_io_adc5_cur = g_adc_io_adc5_cur;
		l_adc_temp_cur = g_adc_temp_cur;
		cpu_irq_restore(flags);

		l_temp = (((l_adc_temp_cur / ((float)C_ADC_STEPS)) * C_VCC_3V0_AREF_VOLTS) / C_TEMPSENSE_MULT) - C_0DEGC_K;
		l_temp_i = (uint16_t)l_temp;
		l_temp_f = (uint8_t)(10 * (l_temp - l_temp_i));

		printf("time = %5ld: vctcxo=%04ld, 5v0=%04ld, vbat=%04ld, adc4=%04ld, adc5=%04ld, temp=%04ld = %d.%dC\r\n",
			now >> 10, l_adc_vctcxo_cur, l_adc_5v0_cur, l_adc_vbat_cur, l_adc_io_adc4_cur, l_adc_io_adc5_cur, l_adc_temp_cur, l_temp_i, l_temp_f);
	}
}

static void task_usb(void)
{
	if (usb_cdc_transfers_autorized) {
#if 0
		// Dedicated handling

		//if () {
		//	task_usb_cdc();
		//}

		// Waits and gets a value on CDC line
		// int udi_cdc_getc(void);

		// Reads a RAM buffer on CDC line
		// iram_size_t udi_cdc_read_buf(int* buf, iram_size_t size);

		// Puts a byte on CDC line
		// int udi_cdc_putc(int value);

		// Writes a RAM buffer on CDC line
		// iram_size_t udi_cdc_write_buf(const int* buf, iram_size_t size);	}

		// Handling ...
#else
		// Already done by stdio redirection to the USB CDC device:

		// stdio_usb_init();
		// stdio_usb_enable();
#endif
	}
}

static void task_twi(uint32_t now, uint32_t last)
{
	/* TWI1 - Gyro, Baro, Hygro, SIM808 devices */
	task_twi_onboard(now, last);

	/* TWI2 - LCD Port */
	task_twi_lcd(now, last);
}

static void task(void)
{
	static uint32_t last = 0;
	uint32_t now = rtc_get_time();

	/* TASK when woken up */
	task_dac();
	task_adc(now, last);

	/* Handling the USB connection */
	task_usb();

	/* Handle TWI1 and TWI2 communications */
	task_twi(now, last);

#if 0
	/* DEBUGGING USB */
	uint32_t now_sec = now >> 10;
	if ((last >> 10) != now_sec) {
		printf("%c\r\nFindMeSAT V1 @USB: RTC32 = %06ld sec\r\n", 0x0c, now_sec);
	}
#endif

	last = now;
}

void halt(void)
{
	/* MAIN Loop Shutdown */

	runmode = 0;
}


int main(void)
{
	uint8_t retcode = 0;

	/* Init of interrupt system */
	irq_initialize_vectors();
	pmic_init();
	pmic_set_scheduling(PMIC_SCH_FIXED_PRIORITY);

	sysclk_init();		// Clock configuration set-up

	sleepmgr_init();	// Unlocks all sleep mode levels

	rtc_init();
	rtc_set_callback(cb_rtc_alarm);

	evsys_init();		// Event system
	tc_init();			// Timers
	adc_init();			// ADC
	dac_init();			// DAC
	twi_init();			// I2C / TWI

	board_init();		// Activates all in/out pins not already handled above - transitions from Z to dedicated states

	/* All interrupt sources & PMIC are prepared until here - IRQ activation follows */
	cpu_irq_enable();

	/* Start of sub-modules */
	tc_start();			// All clocks and PWM timers start here

	/* Init of USB system */
	usb_init();			// USB device stack start function to enable stack and start USB
	twi_start();		// Start TWI
	adc_start();		// Start AD convertions
	dac_start();		// Start DA convertions

	/* The application code */
	runmode = 1;
    while (runmode) {
		task();
		sleepmgr_enter_sleep();
    }

	cpu_irq_disable();
	sleepmgr_enter_sleep();

	return retcode;
}
