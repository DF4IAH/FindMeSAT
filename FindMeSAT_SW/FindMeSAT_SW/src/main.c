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
#include "dds.h"
#include "twi.h"
#include "interpreter.h"

#include "main.h"


/* GLOBAL section */

bool						g_adc_enabled						= true;
int16_t						g_backlight_mode_pwm				= 50;		// -2: SPECIAL
uint8_t						g_bias_pm							= 22;
bool						g_dac_enabled						= false;
bool						g_errorBeep_enable					= true;
bool						g_keyBeep_enable					= false;
bool						g_usb_cdc_stdout_enabled			= false;
bool						g_usb_cdc_printStatusLines			= false;
bool						g_usb_cdc_rx_received				= false;
bool						g_usb_cdc_transfers_autorized		= false;
bool						g_usb_cdc_access_blocked			= false;
uint8_t						g_pitch_tone_mode					= 1;
WORKMODE_ENUM_t				g_workmode							= WORKMODE_OFF;

bool						g_twi1_gsm_valid					= false;
uint8_t						g_twi1_gsm_version					= 0;

bool						g_twi1_gyro_valid					= false;
uint8_t						g_twi1_gyro_1_version				= 0;
int16_t						g_twi1_gyro_1_temp					= 0;
int16_t						g_twi1_gyro_1_temp_RTofs			= 0;
int16_t						g_twi1_gyro_1_temp_sens				= 413;
int16_t						g_twi1_gyro_1_temp_deg_100			= 0;
int16_t						g_twi1_gyro_1_accel_x				= 0;
int16_t						g_twi1_gyro_1_accel_y				= 0;
int16_t						g_twi1_gyro_1_accel_z				= 0;
int16_t						g_twi1_gyro_1_accel_ofsx			= ((int16_t) (-46672L / 16));	// 16LSB / OFS
int16_t						g_twi1_gyro_1_accel_ofsy			= ((int16_t) (+41120L / 16));	// 16LSB / OFS
int16_t						g_twi1_gyro_1_accel_ofsz			= ((int16_t) (+76672L / 16));	// 16LSB / OFS
int16_t						g_twi1_gyro_1_accel_factx			=  9980;						// X = Xchip * factx / 10000
int16_t						g_twi1_gyro_1_accel_facty			=  9975;						// Y = Ychip * facty / 10000
int16_t						g_twi1_gyro_1_accel_factz			=  9950;						// Z = Zchip * factz / 10000
int16_t						g_twi1_gyro_1_accel_x_mg			= 0;
int16_t						g_twi1_gyro_1_accel_y_mg			= 0;
int16_t						g_twi1_gyro_1_accel_z_mg			= 0;
int16_t						g_twi1_gyro_1_gyro_x				= 0;
int16_t						g_twi1_gyro_1_gyro_y				= 0;
int16_t						g_twi1_gyro_1_gyro_z				= 0;
int16_t						g_twi1_gyro_1_gyro_ofsx				= ( -32 / 4);					//  4LSB / OFS
int16_t						g_twi1_gyro_1_gyro_ofsy				= ( -80 / 4);					//  4LSB / OFS
int16_t						g_twi1_gyro_1_gyro_ofsz				= (+148 / 4);					//  4LSB / OFS
int32_t						g_twi1_gyro_1_gyro_x_mdps			= 0;
int32_t						g_twi1_gyro_1_gyro_y_mdps			= 0;
int32_t						g_twi1_gyro_1_gyro_z_mdps			= 0;
uint8_t						g_twi1_gyro_2_version				= 0;
int8_t						g_twi1_gyro_2_asax					= 0;
int8_t						g_twi1_gyro_2_asay					= 0;
int8_t						g_twi1_gyro_2_asaz					= 0;
int16_t						g_twi1_gyro_2_ofsx					=  +37;							//  1LSB / OFS
int16_t						g_twi1_gyro_2_ofsy					= +156;							//  1LSB / OFS
int16_t						g_twi1_gyro_2_ofsz					= -217;							//  1LSB / OFS
int16_t						g_twi1_gyro_2_mag_x					= 0;
int16_t						g_twi1_gyro_2_mag_y					= 0;
int16_t						g_twi1_gyro_2_mag_z					= 0;
int16_t						g_twi1_gyro_2_mag_factx				=  9250;						// X = Xchip * factx / 10000
int16_t						g_twi1_gyro_2_mag_facty				=  9250;						// Y = Ychip * facty / 10000
int16_t						g_twi1_gyro_2_mag_factz				= 14440;						// Z = Zchip * factz / 10000
int32_t						g_twi1_gyro_2_mag_x_nT				= 0;
int32_t						g_twi1_gyro_2_mag_y_nT				= 0;
int32_t						g_twi1_gyro_2_mag_z_nT				= 0;

bool						g_twi1_baro_valid					= false;
uint16_t					g_twi1_baro_version					= 0;
uint16_t					g_twi1_baro_c[C_TWI1_BARO_C_CNT]	= { 0 };
uint32_t					g_twi1_baro_d1						= 0UL;
uint32_t					g_twi1_baro_d2						= 0UL;
int32_t						g_twi1_baro_temp_100				= 0L;
int32_t						g_twi1_baro_p_100					= 0L;

bool						g_twi1_hygro_valid					= false;
uint8_t						g_twi1_hygro_status					= 0;
uint16_t					g_twi1_hygro_S_T					= 0;
uint16_t					g_twi1_hygro_S_RH					= 0;
int16_t						g_twi1_hygro_T_100					= 0;
int16_t						g_twi1_hygro_RH_100					= 0;

uint8_t						g_twi2_lcd_version					= 0;
bool						g_twi2_lcd_repaint					= false;


struct adc_config			g_adc_a_conf						= { 0 };
struct adc_channel_config	g_adcch_vctcxo_5v0_vbat_conf		= { 0 };
struct adc_channel_config	g_adcch_io_adc4_conf				= { 0 };
struct adc_channel_config	g_adcch_io_adc5_conf				= { 0 };
struct adc_channel_config	g_adcch_silence_conf				= { 0 };

struct adc_config			g_adc_b_conf						= { 0 };
struct adc_channel_config	g_adcch_temp_conf					= { 0 };

uint32_t					g_adc_vctcxo_cur					= 0;
uint32_t					g_adc_vctcxo_sum					= 0;
uint16_t					g_adc_vctcxo_cnt					= 0;
uint32_t					g_adc_5v0_cur						= 0;
uint32_t					g_adc_5v0_sum						= 0;
uint16_t					g_adc_5v0_cnt						= 0;
uint32_t					g_adc_vbat_cur						= 0;
uint32_t					g_adc_vbat_sum						= 0;
uint16_t					g_adc_vbat_cnt						= 0;
uint32_t					g_adc_io_adc4_cur					= 0;
uint32_t					g_adc_io_adc4_sum					= 0;
uint16_t					g_adc_io_adc4_cnt					= 0;
uint32_t					g_adc_io_adc5_cur					= 0;
uint32_t					g_adc_io_adc5_sum					= 0;
uint16_t					g_adc_io_adc5_cnt					= 0;
uint32_t					g_adc_silence_cur					= 0;
uint32_t					g_adc_silence_sum					= 0;
uint16_t					g_adc_silence_cnt					= 0;
uint32_t					g_adc_temp_cur						= 0;
uint32_t					g_adc_temp_sum						= 0;
uint16_t					g_adc_temp_cnt						= 0;
int16_t						g_adc_vctcxo_volt_1000				= 0;
int16_t						g_adc_5v0_volt_1000					= 0;
int16_t						g_adc_vbat_volt_1000				= 0;
int16_t						g_adc_io_adc4_volt_1000				= 0;
int16_t						g_adc_io_adc5_volt_1000				= 0;
int16_t						g_adc_silence_volt_1000				= 0;
int16_t						g_adc_temp_deg_100					= 0;

char						g_prepare_buf[C_TX_BUF_SIZE]		= "";

uint8_t						g_sched_lock						= 0;
bool						g_sched_yield						= false;
sched_entry_t				g_sched_data[C_SCH_SLOT_CNT]		= { 0 };
uint8_t						g_sched_sort[C_SCH_SLOT_CNT]		= { 0 };


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
	.no_wait     = true
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
	.no_wait     = true
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

static void isr_10ms(uint32_t now);
static void isr_500ms(uint32_t now);
static void isr_sparetime(uint32_t now);

static void dma_init(void);
static void dma_start(void);
static void isr_dma_dac_ch0_A(enum dma_channel_status status);
static void isr_dma_dac_ch0_B(enum dma_channel_status status);

static void task_dac(uint32_t now);
static void task_usb(uint32_t now);


/* UTILS section */

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


void adc_app_enable(bool enable)
{
	irqflags_t flags = cpu_irq_save();
	bool l_adc_enabled = g_adc_enabled;
	cpu_irq_restore(flags);

	if (l_adc_enabled != enable) {
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

void backlight_mode_pwm(int16_t mode_pwm)
{
	uint8_t l_pwm = mode_pwm & 0xff;

	irqflags_t flags = cpu_irq_save();
	g_backlight_mode_pwm = mode_pwm;
	cpu_irq_restore(flags);

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

void dac_app_enable(bool enable)
{
	irqflags_t flags = cpu_irq_save();
	bool l_dac_enabled = g_dac_enabled;
	cpu_irq_restore(flags);

	if (l_dac_enabled != enable) {
		if (enable) {
			flags = cpu_irq_save();
			dds0_freq_mHz	= 2000000UL;		// 2 kHz
			dds0_reg		= 0UL;				// Sine
			dds1_freq_mHz	= 4000010UL;		// 4 kHz
			dds1_reg		= 0x40000000UL;		// Cosine
			cpu_irq_restore(flags);

			dac_init();
			tc_start();
			dac_start();

		} else {
			dac_stop();
		}

		/* atomic */
		{
			g_dac_enabled = enable;
		}
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

	/* Calculate new increment values */
	task_dac(rtc_get_time());
}

void errorBeep_enable(bool enable)
{
	/* atomic */
	{
		g_errorBeep_enable = enable;
	}
}

void printStatusLines_enable(bool enable)
{
	/* atomic */
	{
		g_usb_cdc_printStatusLines = enable;
	}
}

void keyBeep_enable(bool enable)
{
	/* atomic */
	{
		g_keyBeep_enable = enable;
	}
}

void pitchTone_mode(uint8_t mode)
{
	/* atomic */
	{
		g_pitch_tone_mode = mode;
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


/* Simple scheduler concept */

bool sched_getLock(volatile uint8_t* lockVar)
{
	bool status = false;
	irqflags_t flags;

	flags = cpu_irq_save();

	++*lockVar;
	if (*lockVar == 1) {														// No use before
		status = true;
	} else {
		--(*lockVar);
	}

	cpu_irq_restore(flags);
	return status;
}

void sched_freeLock(volatile uint8_t* lockVar)
{
	irqflags_t flags = cpu_irq_save();
	*lockVar = 0;
	cpu_irq_restore(flags);
}

void sched_push(sched_callback cb, uint32_t wakeTime, bool isDelay)
{
	const uint32_t pushTime = rtc_get_time();

	if (isDelay) {
		/* Sanity checks */
		if (wakeTime < 2) {
			wakeTime = 2;														// Min value to use to work properly
		} else if (wakeTime > 30000U) {
			wakeTime = ((30000U << 10) / 1000);									// Max value 30 sec
		} else {
			wakeTime = ((wakeTime << 10) / 1000);								// Time correction for 1024 ticks per second
		}

		/* Absolute time ticks */
		wakeTime += pushTime;
	}

	/* Lock access to the scheduler entries */
	while (!sched_getLock(&g_sched_lock))
		;

	/* Get next free slot */
	bool dataEntryStored = false;
	uint8_t slot = 0;
	for (int idx = 0; idx < C_SCH_SLOT_CNT; ++idx) {
		if (!(g_sched_data[idx].occupied)) {
			g_sched_data[idx].occupied = true;
			g_sched_data[idx].callback = cb;
			g_sched_data[idx].wakeTime = wakeTime;
			slot = idx + 1;
			dataEntryStored = true;
			break;
		}
	}

	if (!dataEntryStored) {
		return;  // should not happen
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
	uint32_t alarmTime = g_sched_data[g_sched_sort[0] - 1].wakeTime;

	/* Release lock */
	sched_freeLock(&g_sched_lock);

	/* Set next time to wake up */
	uint32_t checkLimit = rtc_get_time() + 2;
	if (alarmTime < checkLimit) {
		alarmTime = checkLimit;
	}
	rtc_set_alarm(alarmTime);
}

void sched_pop(uint32_t wakeNow)
{
	uint8_t idx = g_sched_sort[0];
	if (!idx) {
		return;
	}
	if (!(g_sched_data[idx - 1].occupied)) {
		return;
	}

	/* Process each entry until now  */
	uint32_t alarmTime = g_sched_data[idx - 1].wakeTime;
	while (alarmTime <= wakeNow) {
		/* Get callback */
		sched_callback cb = g_sched_data[idx - 1].callback;

		/* Free entry */
		g_sched_data[idx - 1].occupied = false;

		/* Move all items down by one */
		for (int mvidx = 0; mvidx < (C_SCH_SLOT_CNT - 1); ++mvidx) {
			g_sched_sort[mvidx] = g_sched_sort[mvidx + 1];
		}

		/* Call the CB function */
		if (cb) {
			cb(alarmTime);
		}

		/* Get the next alarm time */
		{
			idx = g_sched_sort[0];
			if (!idx) {
				return;
			}
			if (!(g_sched_data[idx - 1].occupied)) {
				return;
			}
			alarmTime = g_sched_data[idx - 1].wakeTime;
		}
	}

	/* Set next time to wake up */
	rtc_set_alarm(alarmTime);
}

void yield_ms(uint16_t ms)
{
	bool l_sched_yield;
	irqflags_t flags;

	sched_push(yield_ms_cb, ms, true);

	flags = cpu_irq_save();
	g_sched_yield = l_sched_yield = true;
	cpu_irq_restore(flags);

	/* Continued sleep until our callback is done */
	do {
		/* Enter sleep mode */
		sleepmgr_enter_sleep();

		/* Woke up for any reason */
		flags = cpu_irq_save();
		l_sched_yield = g_sched_yield;
		cpu_irq_restore(flags);
	} while (l_sched_yield);
}

void yield_ms_cb(uint32_t listTime)
{
	irqflags_t flags;

	/* Time delay completed */
	flags = cpu_irq_save();
	g_sched_yield = false;
	cpu_irq_restore(flags);
}


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

	/* DAC - event 4 */
	EVSYS.CH4MUX  = EVSYS_CHMUX_TCE1_OVF_gc;									// TCE1 overflow goes to EVSYS CH4
	EVSYS.CH4CTRL = EVSYS_DIGFILT_1SAMPLE_gc;									// EVSYS CH4 no digital filtering
}


static void tc_init(void)
{
	/* TCC0: VCTCXO PWM signal generation and ADCA & ADCB */
	struct pwm_config pwm_vctcxo_cfg;
	pwm_init(&pwm_vctcxo_cfg, PWM_TCC0, PWM_CH_C, 2048);						// Init PWM structure and enable timer - running with 2048 Hz --> 2 Hz averaged data
	pwm_start(&pwm_vctcxo_cfg, 45);												// Start PWM here. Percentage with 1% granularity is to coarse, use driver access instead
	tc_write_cc_buffer(&TCC0, TC_CCC, (uint16_t) (0.5f + pwm_vctcxo_cfg.period * C_VCTCXO_DEFAULT_VOLTS / C_VCTCXO_PWM_HI_VOLTS));	// Initial value for VCTCXO

	/* TCE1: DAC clock */
	tc_enable(&TCE1);
	tc_set_wgm(&TCE1, TC_WG_NORMAL);											// Internal clock for DAC convertion
	tc_write_period(&TCE1, (sysclk_get_per_hz() / DAC_RATE_OF_CONV) - 1);		// DAC clock of 100 kHz for DDS (Direct Digital Synthesis)
}

static void tc_start(void)
{
	/* ADC clock */
	tc_write_clock_source(&TCC0, TC_CLKSEL_DIV1_gc);							// VCTCXO PWM start, output still is Z-state
	tc_set_overflow_interrupt_callback(&TCC0, isr_tcc0_ovfl);
	tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_LO);

//	tc_write_clock_source(&TCC1, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCD0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCD1, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCE0, TC_CLKSEL_DIV1_gc);

	/* DAC clock */
	tc_write_clock_source(&TCE1, TC_CLKSEL_DIV1_gc);

//	tc_write_clock_source(&TCF0, TC_CLKSEL_DIV1_gc);
//	tc_write_clock_source(&TCF1, TC_CLKSEL_DIV1_gc);
}

void isr_tcc0_ovfl(void)
{	// This ISR is called 2048 per second
	static uint32_t	last_10ms  = 0UL;
	static uint32_t	last_500ms = 0UL;

	/* Time downscaling */
	uint32_t now = rtc_get_time();

	/* Clear IF bit to allow interrupt enabled section */
	TCC0_INTFLAGS = TC0_OVFIF_bm;

	if (g_workmode == WORKMODE_RUN) {
		/* Group, which needs to be called about 100x per second */
		if (((now - last_10ms) >= 10) || (now < last_10ms)) {
			last_10ms = now;
			isr_10ms(now);
			return;
		}

		/* Group, which needs to be called about 2x per second */
		if (((now - last_500ms) >= 512) || (now < last_500ms)) {
			last_500ms = now;
			isr_500ms(now);
			return;
		}

		isr_sparetime(now);
	}
}

static void isr_10ms(uint32_t now)
{
	isr_10ms_twi1_onboard(now);
}

static void isr_500ms(uint32_t now)
{
	isr_500ms_twi1_onboard(now);

	/* Kick RTC32 */
	{
		uint32_t kickTime = rtc_get_time() + 2;
		rtc_set_alarm(kickTime);
	}
}

static void isr_sparetime(uint32_t now)
{
	isr_sparetime_twi1_onboard(now);
}


static void rtc_start(void)
{
	rtc_set_callback(isr_rtc_alarm);
}

void isr_rtc_alarm(uint32_t rtc_time)
{	// Alarm call-back with the current time
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
	memset(&dmach_dma0_conf, 0, sizeof(dmach_dma0_conf));	// DACB channel 0 - linked with dma1
	memset(&dmach_dma1_conf, 0, sizeof(dmach_dma1_conf));	// DACB channel 1 - linked with dma0

	dma_channel_set_burst_length(&dmach_dma0_conf, DMA_CH_BURSTLEN_4BYTE_gc);
	dma_channel_set_burst_length(&dmach_dma1_conf, DMA_CH_BURSTLEN_4BYTE_gc);

	dma_channel_set_transfer_count(&dmach_dma0_conf, DAC_NR_OF_SAMPLES * sizeof(dma_dac_buf_t));
	dma_channel_set_transfer_count(&dmach_dma1_conf, DAC_NR_OF_SAMPLES * sizeof(dma_dac_buf_t));

	dma_channel_set_src_reload_mode(&dmach_dma0_conf, DMA_CH_SRCRELOAD_TRANSACTION_gc);
	dma_channel_set_src_dir_mode(&dmach_dma0_conf, DMA_CH_SRCDIR_INC_gc);
	dma_channel_set_source_address(&dmach_dma0_conf, (uint16_t)(uintptr_t) &dac_io_dac0_buf[0][0]);
	dma_channel_set_dest_reload_mode(&dmach_dma0_conf, DMA_CH_DESTRELOAD_BURST_gc);
	dma_channel_set_dest_dir_mode(&dmach_dma0_conf, DMA_CH_DESTDIR_INC_gc);
	dma_channel_set_destination_address(&dmach_dma0_conf, (uint16_t)(uintptr_t) &DACB_CH0DATA);		// Access to CH0 and CH1

	dma_channel_set_src_reload_mode(&dmach_dma1_conf, DMA_CH_SRCRELOAD_TRANSACTION_gc);
	dma_channel_set_src_dir_mode(&dmach_dma1_conf, DMA_CH_SRCDIR_INC_gc);
	dma_channel_set_source_address(&dmach_dma1_conf, (uint16_t)(uintptr_t) &dac_io_dac0_buf[1][0]);
	dma_channel_set_dest_reload_mode(&dmach_dma1_conf, DMA_CH_DESTRELOAD_BURST_gc);
	dma_channel_set_dest_dir_mode(&dmach_dma1_conf, DMA_CH_DESTDIR_INC_gc);
	dma_channel_set_destination_address(&dmach_dma1_conf, (uint16_t)(uintptr_t) &DACB_CH0DATA);		// Access to CH0 and CH1

	dma_channel_set_trigger_source(&dmach_dma0_conf, DMA_CH_TRIGSRC_DACB_CH0_gc);
	dma_channel_set_single_shot(&dmach_dma0_conf);

	dma_channel_set_trigger_source(&dmach_dma1_conf, DMA_CH_TRIGSRC_DACB_CH0_gc);
	dma_channel_set_single_shot(&dmach_dma1_conf);

	task_dac(rtc_get_time());																		// Calculate DDS increments
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


const char					PM_USBINIT_HEADER_01[]				= "\r\n\r\n\r\n";
const char					PM_USBINIT_HEADER_02[]				= "%c\r\n===============================\r\n";
const char					PM_USBINIT_HEADER_03[]				= "FindMeSAT - USB logging started\r\n";
const char					PM_USBINIT_HEADER_04[]				= "===============================\r\n\r\n";
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_01[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_02[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_03[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_04[]);

static void usb_init(void)
{
	stdio_usb_init();	// Init and enable stdio_usb
	if (g_usb_cdc_stdout_enabled) {
		stdio_usb_enable();
	}
	yield_ms(750);

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_01);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_02, 0x0c);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_03);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_04);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

	yield_ms(250);
}

void usb_callback_suspend_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Disable hardware component to reduce power consumption
	// Reduce power consumption in suspend mode (max. 2.5mA on VBUS)
#if 0
#endif

	irqflags_t flags = cpu_irq_save();
	g_workmode = WORKMODE_SUSPENDED;
	cpu_irq_restore(flags);
}

void usb_callback_resume_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Re-enable hardware component
#if 0
	cpu_irq_disable();

	evsys_init();		// Event system
	tc_init();			// Timers
	if (g_adc_enabled) {
		adc_init();		// ADC
	}
	if (g_dac_enabled) {
		dac_init();		// DAC
	}
	twi_init();			// I2C / TWI

	/* All interrupt sources & PMIC are prepared until here - IRQ activation follows */
	cpu_irq_enable();

	/* Start of sub-modules */
	tc_start();			// All clocks and PWM timers start here
	if (g_dac_enabled) {
		dac_start();	// Start DA convertions
	}
	if (g_adc_enabled) {
		adc_start();	// Start AD convertions
	}

	/* Start TWI channels */
	twi_start();		// Start TWI
#endif

	/* The application code */
	irqflags_t flags = cpu_irq_save();
	g_workmode = WORKMODE_RUN;
	cpu_irq_restore(flags);
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

	g_usb_cdc_transfers_autorized = true;
	return true;
}

void usb_callback_cdc_disable(void)
{
	/* USB CDC feature for serial communication */

	g_usb_cdc_transfers_autorized = false;
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
	g_usb_cdc_rx_received = true;
}

void usb_callback_tx_empty_notify(uint8_t port)
{
	g_usb_cdc_access_blocked = false;
}



/* The LOOP section */

static void task_dac(uint32_t now)
{	/* Calculation of the DDS increments */
	static uint32_t s_dds0_freq_mHz = 0UL;
	static uint32_t s_dds1_freq_mHz = 0UL;
	uint32_t l_dds0_freq_mHz, l_dds1_freq_mHz;

	irqflags_t flags = cpu_irq_save();
	l_dds0_freq_mHz = dds0_freq_mHz;
	l_dds1_freq_mHz = dds1_freq_mHz;
	cpu_irq_restore(flags);

	if ((l_dds0_freq_mHz != s_dds0_freq_mHz) || (l_dds1_freq_mHz != s_dds1_freq_mHz)) {
		/* DDS increment calculation */
		uint32_t l_dds0_inc = (uint32_t) (((uint64_t)dds0_freq_mHz * UINT32_MAX) / (DAC_RATE_OF_CONV * 1000UL));
		uint32_t l_dds1_inc = (uint32_t) (((uint64_t)dds1_freq_mHz * UINT32_MAX) / (DAC_RATE_OF_CONV * 1000UL));
		s_dds0_freq_mHz = l_dds0_freq_mHz;
		s_dds1_freq_mHz = l_dds1_freq_mHz;

		flags = cpu_irq_save();
		dds0_inc = l_dds0_inc;
		dds1_inc = l_dds1_inc;
		cpu_irq_restore(flags);
	}
}

static void task_adc(uint32_t now)
{	/* Calculations of the ADC values for the presentation layer */
	static uint32_t adc_last = 0;

	if ((now - adc_last) >= 512 || (now < adc_last)) {
		adc_last = now;

		irqflags_t flags = cpu_irq_save();
		uint32_t l_adc_vctcxo_cur	= g_adc_vctcxo_cur;
		uint32_t l_adc_5v0_cur		= g_adc_5v0_cur;
		uint32_t l_adc_vbat_cur		= g_adc_vbat_cur;
		uint32_t l_adc_io_adc4_cur	= g_adc_io_adc4_cur;
		uint32_t l_adc_io_adc5_cur	= g_adc_io_adc5_cur;
		uint32_t l_adc_silence_cur	= g_adc_silence_cur;
		uint32_t l_adc_temp_cur		= g_adc_temp_cur;
		cpu_irq_restore(flags);

		int16_t l_adc_vctcxo_volt_1000	= (int16_t) (((( 1000UL * l_adc_vctcxo_cur  * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS))  - 1000 * C_VCTCXO_DELTA_VOLTS);
		int16_t l_adc_5v0_volt_1000		= (int16_t) (((  1000UL * l_adc_5v0_cur     * C_VCC_3V0_AREF_VOLTS * C_VCC_5V0_MULT  ) / C_ADC_STEPS));
		int16_t l_adc_vbat_volt_1000	= (int16_t) (((  1000UL * l_adc_vbat_cur    * C_VCC_3V0_AREF_VOLTS * C_VCC_VBAT_MULT ) / C_ADC_STEPS));
		int16_t l_adc_io_adc4_volt_1000	= (int16_t) (((  1000UL * l_adc_io_adc4_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_io_adc5_volt_1000	= (int16_t) (((  1000UL * l_adc_io_adc5_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_silence_volt_1000	= (int16_t) (((  1000UL * l_adc_silence_cur * C_VCC_3V0_AREF_VOLTS                   ) / C_ADC_STEPS));
		int16_t l_adc_temp_deg_100		= (int16_t) ((((  100UL * l_adc_temp_cur                           * C_TEMPSENSE_MULT) / C_ADC_STEPS))  -  100 * C_0DEGC_K);

		flags = cpu_irq_save();
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

static void task_twi(uint32_t now)
{	/* Calculations for the presentation layer and display */

#if 0
	// now to be called by the scheduler by  isr_500ms_twi1_onboard()

	/* TWI1 - SIM808, Hygro, Gyro, Baro devices */
	task_twi1_onboard(now);
#endif

	/* TWI2 - LCD Port */
	yield_ms(100);
	task_twi2_lcd(now);
}


const char					PM_INFO_PART_L1P1A[]				= "Time = %06ld: Uvco=%4d mV, U5v=%4d mV, Ubat=%4d mV, ";
const char					PM_INFO_PART_L1P1B[]				= "Uadc4=%4d mV, Uadc5=%4d mV, Usil=%4d mV, ";
const char					PM_INFO_PART_L1P1C[]				= "mP_Temp=%+06.2fC\t \t";
const char					PM_INFO_PART_L1P2[]					= "Baro_Temp=%+06.2fC, Baro_P=%7.2fhPa\t \t";
const char					PM_INFO_PART_L1P3[]					= "Hygro_Temp=%+06.2fC, Hygro_RelH=%05.2f%%\r\n";
const char					PM_INFO_PART_L2P1A[]				= "\tAx=%+05.3fg (%+06d), Ay=%+05.3fg (%+06d), ";
const char					PM_INFO_PART_L2P1B[]				= "Az=%+05.3fg (%+06d)\t \t";
const char					PM_INFO_PART_L2P2A[]				= "Gx=%+07.2fdps (%+06d), Gy=%+07.2fdps (%+06d), ";
const char					PM_INFO_PART_L2P2B[]				= "Gz=%+07.2fdps (%06d)\t \t";
const char					PM_INFO_PART_L2P3A[]				= "Mx=%+07.3fuT (%+06d), My=%+07.3fuT (%+06d), ";
const char					PM_INFO_PART_L2P3B[]				= "Mz=%+07.3fuT (%+06d)\t \t";
const char					PM_INFO_PART_L2P4[]					= "Gyro_Temp=%+06.2fC (%+06d)\r\n\r\n";

PROGMEM_DECLARE(const char, PM_INFO_PART_L1P1A[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L1P1B[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L1P1C[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L1P2[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L1P3[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P1A[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P1B[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P2A[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P2B[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P3A[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P3B[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_L2P4[]);

static void task_usb(uint32_t now)
{
	/* Monitoring at the USB serial terminal */
	if (g_usb_cdc_transfers_autorized) {
		static uint32_t usb_last = 0UL;

		/* each of them are atomic */
		bool l_usb_cdc_rx_received		= g_usb_cdc_rx_received;
		bool l_usb_cdc_printStatusLines	= g_usb_cdc_printStatusLines;

		/* Get command lines from the USB host */
		if (l_usb_cdc_rx_received) {
			iram_size_t cdc_rx_len = udi_cdc_get_nb_received_data();
			if (cdc_rx_len) {
				char cdc_rx_buf[cdc_rx_len];

				if (g_keyBeep_enable) {
					twi2_set_beep(176, 1);  // Click sound
					yield_ms(5);
				}

				udi_cdc_read_no_polling(cdc_rx_buf, cdc_rx_len);

				/* Echo back when not monitoring information are enabled */
				if (!l_usb_cdc_printStatusLines) {
					udi_write_tx_buf(cdc_rx_buf, cdc_rx_len, true);
				}

				/* Call the interpreter */
				interpreter_doProcess(cdc_rx_buf, cdc_rx_len);

				/* Check for more available data */
				cdc_rx_len = udi_cdc_get_nb_received_data();
			}

			/* atomic */
			{
				g_usb_cdc_rx_received = false;
			}
		}

		/* Status output when requested */
		if (g_usb_cdc_printStatusLines) {
			if (((now - usb_last) >= 512) || (now < usb_last)) {
				irqflags_t flags = cpu_irq_save();
				int16_t l_adc_vctcxo_volt_1000		= g_adc_vctcxo_volt_1000;
				int16_t l_adc_5v0_volt_1000			= g_adc_5v0_volt_1000;
				int16_t l_adc_vbat_volt_1000		= g_adc_vbat_volt_1000;
				int16_t l_adc_io_adc4_volt_1000		= g_adc_io_adc4_volt_1000;
				int16_t l_adc_io_adc5_volt_1000		= g_adc_io_adc5_volt_1000;
				int16_t l_adc_silence_volt_1000		= g_adc_silence_volt_1000;
				int16_t l_adc_temp_deg_100			= g_adc_temp_deg_100;
				int32_t l_twi1_baro_temp_100		= g_twi1_baro_temp_100;
				int32_t l_twi1_baro_p_100			= g_twi1_baro_p_100;
				int16_t l_twi1_hygro_T_100			= g_twi1_hygro_T_100;
				int16_t l_twi1_hygro_RH_100			= g_twi1_hygro_RH_100;
				int16_t	l_twi1_gyro_1_accel_x		= g_twi1_gyro_1_accel_x;
				int16_t	l_twi1_gyro_1_accel_y		= g_twi1_gyro_1_accel_y;
				int16_t	l_twi1_gyro_1_accel_z		= g_twi1_gyro_1_accel_z;
				int16_t	l_twi1_gyro_1_accel_x_mg	= g_twi1_gyro_1_accel_x_mg;
				int16_t	l_twi1_gyro_1_accel_y_mg	= g_twi1_gyro_1_accel_y_mg;
				int16_t	l_twi1_gyro_1_accel_z_mg	= g_twi1_gyro_1_accel_z_mg;
				int16_t l_twi1_gyro_1_gyro_x		= g_twi1_gyro_1_gyro_x;
				int16_t l_twi1_gyro_1_gyro_y		= g_twi1_gyro_1_gyro_y;
				int16_t l_twi1_gyro_1_gyro_z		= g_twi1_gyro_1_gyro_z;
				int32_t	l_twi1_gyro_1_gyro_x_mdps	= g_twi1_gyro_1_gyro_x_mdps;
				int32_t	l_twi1_gyro_1_gyro_y_mdps	= g_twi1_gyro_1_gyro_y_mdps;
				int32_t	l_twi1_gyro_1_gyro_z_mdps	= g_twi1_gyro_1_gyro_z_mdps;
				int16_t	l_twi1_gyro_1_temp			= g_twi1_gyro_1_temp;
				int16_t	l_twi1_gyro_1_temp_deg_100	= g_twi1_gyro_1_temp_deg_100;
				int16_t l_twi1_gyro_2_mag_x			= g_twi1_gyro_2_mag_x;
				int16_t l_twi1_gyro_2_mag_y			= g_twi1_gyro_2_mag_y;
				int16_t l_twi1_gyro_2_mag_z			= g_twi1_gyro_2_mag_z;
				int32_t	l_twi1_gyro_2_mag_x_nT		= g_twi1_gyro_2_mag_x_nT;
				int32_t	l_twi1_gyro_2_mag_y_nT		= g_twi1_gyro_2_mag_y_nT;
				int32_t	l_twi1_gyro_2_mag_z_nT		= g_twi1_gyro_2_mag_z_nT;
				cpu_irq_restore(flags);

				int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L1P1A,
				now >> 10,
				l_adc_vctcxo_volt_1000, l_adc_5v0_volt_1000, l_adc_vbat_volt_1000);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L1P1B,
				l_adc_io_adc4_volt_1000, l_adc_io_adc5_volt_1000, l_adc_silence_volt_1000);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L1P1C,
				l_adc_temp_deg_100 / 100.f);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L1P2,
				l_twi1_baro_temp_100 / 100.f, l_twi1_baro_p_100 / 100.f);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L1P3,
				l_twi1_hygro_T_100 / 100.f, l_twi1_hygro_RH_100 / 100.f);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);


				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P1A,
				l_twi1_gyro_1_accel_x_mg / 1000.f, l_twi1_gyro_1_accel_x,
				l_twi1_gyro_1_accel_y_mg / 1000.f, l_twi1_gyro_1_accel_y);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P1B,
				l_twi1_gyro_1_accel_z_mg / 1000.f, l_twi1_gyro_1_accel_z);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P2A,
				l_twi1_gyro_1_gyro_x_mdps / 1000.f, l_twi1_gyro_1_gyro_x,
				l_twi1_gyro_1_gyro_y_mdps / 1000.f, l_twi1_gyro_1_gyro_y);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P2B,
				l_twi1_gyro_1_gyro_z_mdps / 1000.f, l_twi1_gyro_1_gyro_z);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P3A,
				l_twi1_gyro_2_mag_x_nT / 1000.f, l_twi1_gyro_2_mag_x,
				l_twi1_gyro_2_mag_y_nT / 1000.f, l_twi1_gyro_2_mag_y);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P3B,
				l_twi1_gyro_2_mag_z_nT / 1000.f, l_twi1_gyro_2_mag_z);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_L2P4,
				l_twi1_gyro_1_temp_deg_100 / 100.f, l_twi1_gyro_1_temp);
				udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

				/* Store last time of status line */
				usb_last = now;
			}
		}
	}
}

static void task(void)
{
	irqflags_t flags = cpu_irq_save();
	WORKMODE_ENUM_t l_workmode = g_workmode;
	cpu_irq_restore(flags);

	if (l_workmode == WORKMODE_RUN) {
		uint32_t now = rtc_get_time();

		/* TASK when woken up and all ISRs are done */
		task_dac(now);
		task_adc(now);
		task_twi(now);										// Handle TWI1 and TWI2 communications
		task_usb(now);										// Handling the USB connection
	}
}


int main(void)
{
	uint8_t retcode = 0;

	/* Init of interrupt system */
	g_workmode = WORKMODE_INIT;
	irq_initialize_vectors();
	pmic_init();
	pmic_set_scheduling(PMIC_SCH_FIXED_PRIORITY);

	sysclk_init();		// Clock configuration set-up

	sleepmgr_init();	// Unlocks all sleep mode levels

	rtc_init();
	rtc_start();

	evsys_init();		// Event system
	tc_init();			// Timers
	if (g_adc_enabled) {
		adc_init();		// ADC
	}
	if (g_dac_enabled) {
		dac_init();		// DAC
	}
	twi_init();			// I2C / TWI

	board_init();		// Activates all in/out pins not already handled above - transitions from Z to dedicated states

	nvm_init(INT_FLASH);

	/* All interrupt sources & PMIC are prepared until here - IRQ activation follows */
	cpu_irq_enable();

	/* Start of sub-modules */
	tc_start();			// All clocks and PWM timers start here
	if (g_dac_enabled) {
		dac_start();	// Start DA convertions
	}
	if (g_adc_enabled) {
		adc_start();	// Start AD convertions
	}

	/* Init of USB system */
	usb_init();			// USB device stack start function to enable stack and start USB

	/* Start TWI channels */
	twi_start();		// Start TWI

	/* Show help page of command set */
	printHelp();

	/* Show green LED */
	//twi2_set_leds(0x02);

	/* The application code */
	irqflags_t flags = cpu_irq_save();
	WORKMODE_ENUM_t l_workmode = g_workmode = WORKMODE_RUN;
	cpu_irq_restore(flags);

    while (l_workmode) {
		task();

		twi2_set_leds(0x01);
		yield_ms(0);
		twi2_set_leds(0x00);

		flags = cpu_irq_save();
		l_workmode = g_workmode;
		cpu_irq_restore(flags);
    }

	twi2_set_leds(0x00);  // LEDs: off

	cpu_irq_disable();
	sleepmgr_enter_sleep();

	return retcode;
}
