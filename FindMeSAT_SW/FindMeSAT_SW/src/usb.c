/**
 * \file
 *
 * \brief FindMeSAT
 * usb.c
 *
 * Created: 08.08.2017 21:49:49
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

#include "main.h"
#include "interpreter.h"
#include "twi.h"

#include "usb.h"


/* Add access to the global vars */
#include "externals.h"


/* Functions */

static bool udi_write_tx_char(int chr, bool stripControl)
{
	if (stripControl) {
		/* Drop control character and report putc() success */
		if ((chr < 0x20) || (chr >= 0x80)) {
			return true;
		}
	}

	if (!g_usb_cdc_access_blocked) {  // atomic operation
		return udi_cdc_putc(chr);
	}

	return true;	// write to nowhere
}

uint8_t udi_write_tx_buf(const char* buf, uint8_t len, bool stripControl)
{
	uint8_t ret = 0;
	uint8_t cnt = 0;

	if (!g_usb_cdc_transfers_authorized) {
		return 0;
	}

	/* Write each character - avoiding to use the block write function */
	while (ret < len) {
		if (!udi_cdc_is_tx_ready()) {
			++cnt;
			yield_ms(C_USB_LINE_DELAY_MS);

			} else if (!udi_write_tx_char(*(buf + ret), stripControl)) {
			++cnt;
			yield_ms(C_USB_LINE_DELAY_MS);

			} else {
			cnt = 0;
			++ret;
		}

		if (cnt > 200) {
			/* Device blocks, get rid of it */
			g_usb_cdc_transfers_authorized = false;
			return ret;
		}
	}
	return ret;
}


const char					PM_USBINIT_HEADER_1[]				= "\r\n\r\n\r\n";
const char					PM_USBINIT_HEADER_2[]				= "%c\r\n===============================\r\n";
const char					PM_USBINIT_HEADER_3[]				= "FindMeSAT - USB logging started\r\n";
const char					PM_USBINIT_HEADER_4[]				= "===============================\r\n\r\n";
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_1[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_2[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_3[]);
PROGMEM_DECLARE(const char, PM_USBINIT_HEADER_4[]);

void usb_init(void)
{
	stdio_usb_init();	// Init and enable stdio_usb
	if (g_usb_cdc_stdout_enabled) {
		stdio_usb_enable();
	}
	yield_ms(1000);

	int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_1);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	yield_ms(500);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_2, 0x0c);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	yield_ms(250);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_3);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
	yield_ms(250);

	len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_USBINIT_HEADER_4);
	udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
}

void usb_callback_suspend_action(void)
{
	/* USB BUS powered: suspend / resume operations */

	// Disable hardware component to reduce power consumption
	// Reduce power consumption in suspend mode (max. 2.5mA on VBUS)

	g_workmode = WORKMODE_SUSPENDED;
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
	g_workmode = WORKMODE_RUN;
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
	g_usb_cdc_transfers_authorized = true;
	return true;
}

void usb_callback_cdc_disable(void)
{
	/* USB CDC feature for serial communication */
	g_usb_cdc_transfers_authorized = false;
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


static void usb_rx_process(uint32_t thisTime)
{
	char cdc_rx_buf[4];

	/* Drop data connection is not authorized (yet) */
	if (!g_usb_cdc_transfers_authorized) {
		return;
	}

	/* Single thread only */
	if (!sched_getLock(&g_interpreter_lock)) {
		return;
	}

	/* Get command lines from the USB host */
	iram_size_t cdc_rx_len = udi_cdc_get_nb_received_data();
	cdc_rx_len = min(cdc_rx_len, sizeof(cdc_rx_buf));
	while (cdc_rx_len) {
		if (g_keyBeep_enable) {
			twi2_set_beep(176, 1);  // Click sound
			yield_ms(10);
		}

		/* Read the data block */
		udi_cdc_read_no_polling(cdc_rx_buf, cdc_rx_len);

		/* Echo back when not monitoring information are enabled */
		if (!g_usb_cdc_printStatusLines_atxmega) {
			udi_write_tx_buf(cdc_rx_buf, cdc_rx_len, true);
		}

		/* Call the interpreter */
		interpreter_doProcess(cdc_rx_buf, cdc_rx_len);

		/* Check for more available data */
		cdc_rx_len = udi_cdc_get_nb_received_data();
	}

	/* Release this lock */
	sched_freeLock(&g_interpreter_lock);
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
const char					PM_INFO_PART_PLL1[]					= "PLL: time=%8ld.%03ld + %03d/30E+6 sec, ";
const char					PM_INFO_PART_PLL2[]					= "1pps_deviation=%+10f, ";
const char					PM_INFO_PART_PLL3[]					= "XO_PWM=%05ldd : 0x%02x\r\n\r\n";

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
PROGMEM_DECLARE(const char, PM_INFO_PART_PLL1[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_PLL2[]);
PROGMEM_DECLARE(const char, PM_INFO_PART_PLL3[]);

void task_usb(uint32_t now)
{
	/* Monitoring at the USB serial terminal */
	if (g_usb_cdc_transfers_authorized) {
		static uint32_t usb_last = 0UL;

		/* Get command lines from the USB host */
		if (g_usb_cdc_rx_received) {
			g_usb_cdc_rx_received = false;
			usb_rx_process(now);
		}

		/* Status of the PLL unit */
		if (g_usb_cdc_printStatusLines_sim808 && g_1pps_printusb_avail) {
			uint16_t l_pll_lo;
			uint64_t l_pll_hi;
			float l_1pps_deviation;
			uint32_t l_xo_mode_pwm;
			{
				irqflags_t flags = cpu_irq_save();
				l_pll_lo				= g_1pps_last_lo;
				l_pll_hi				= g_1pps_last_hi;
				l_1pps_deviation		= g_1pps_deviation;
				l_xo_mode_pwm			= g_xo_mode_pwm;
				cpu_irq_restore(flags);

				g_1pps_printusb_avail	= false;
			}

			int len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_PLL1, (uint32_t)(l_pll_hi / 1000U), (uint32_t) (l_pll_hi % 1000), l_pll_lo);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_PLL2, l_1pps_deviation);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);

			len = snprintf_P(g_prepare_buf, sizeof(g_prepare_buf), PM_INFO_PART_PLL3, (l_xo_mode_pwm & C_XO_VAL_INT_MASK) >> C_XO_VAL_INT_SHIFT, l_xo_mode_pwm & C_XO_VAL_FRAC_MASK);
			udi_write_tx_buf(g_prepare_buf, min(len, sizeof(g_prepare_buf)), false);
		}

		/* Status output when requested */
		if (g_usb_cdc_printStatusLines_atxmega) {
			if (((now - usb_last) >= 512) || (now < usb_last)) {
				int16_t l_adc_vctcxo_volt_1000;
				int16_t l_adc_5v0_volt_1000;
				int16_t l_adc_vbat_volt_1000;
				int16_t l_adc_io_adc4_volt_1000;
				int16_t l_adc_io_adc5_volt_1000;
				int16_t l_adc_silence_volt_1000;
				int16_t l_adc_temp_deg_100;
				int32_t l_twi1_baro_temp_100;
				int32_t l_twi1_baro_p_100;
				int16_t l_twi1_hygro_T_100;
				int16_t l_twi1_hygro_RH_100;
				int16_t	l_twi1_gyro_1_accel_x;
				int16_t	l_twi1_gyro_1_accel_y;
				int16_t	l_twi1_gyro_1_accel_z;
				int16_t	l_twi1_gyro_1_accel_x_mg;
				int16_t	l_twi1_gyro_1_accel_y_mg;
				int16_t	l_twi1_gyro_1_accel_z_mg;
				int16_t l_twi1_gyro_1_gyro_x;
				int16_t l_twi1_gyro_1_gyro_y;
				int16_t l_twi1_gyro_1_gyro_z;
				int32_t	l_twi1_gyro_1_gyro_x_mdps;
				int32_t	l_twi1_gyro_1_gyro_y_mdps;
				int32_t	l_twi1_gyro_1_gyro_z_mdps;
				int16_t	l_twi1_gyro_1_temp;
				int16_t	l_twi1_gyro_1_temp_deg_100;
				int16_t l_twi1_gyro_2_mag_x;
				int16_t l_twi1_gyro_2_mag_y;
				int16_t l_twi1_gyro_2_mag_z;
				int32_t	l_twi1_gyro_2_mag_x_nT;
				int32_t	l_twi1_gyro_2_mag_y_nT;
				int32_t	l_twi1_gyro_2_mag_z_nT;

				/* Getting a copy of the values */
				{
					irqflags_t flags			= cpu_irq_save();
					l_adc_vctcxo_volt_1000		= g_adc_vctcxo_volt_1000;
					l_adc_5v0_volt_1000			= g_adc_5v0_volt_1000;
					l_adc_vbat_volt_1000		= g_adc_vbat_volt_1000;
					l_adc_io_adc4_volt_1000		= g_adc_io_adc4_volt_1000;
					l_adc_io_adc5_volt_1000		= g_adc_io_adc5_volt_1000;
					l_adc_silence_volt_1000		= g_adc_silence_volt_1000;
					l_adc_temp_deg_100			= g_adc_temp_deg_100;
					l_twi1_baro_temp_100		= g_twi1_baro_temp_100;
					l_twi1_baro_p_100			= g_twi1_baro_p_100;
					l_twi1_hygro_T_100			= g_twi1_hygro_T_100;
					l_twi1_hygro_RH_100			= g_twi1_hygro_RH_100;
					l_twi1_gyro_1_accel_x		= g_twi1_gyro_1_accel_x;
					l_twi1_gyro_1_accel_y		= g_twi1_gyro_1_accel_y;
					l_twi1_gyro_1_accel_z		= g_twi1_gyro_1_accel_z;
					l_twi1_gyro_1_accel_x_mg	= g_twi1_gyro_1_accel_x_mg;
					l_twi1_gyro_1_accel_y_mg	= g_twi1_gyro_1_accel_y_mg;
					l_twi1_gyro_1_accel_z_mg	= g_twi1_gyro_1_accel_z_mg;
					l_twi1_gyro_1_gyro_x		= g_twi1_gyro_1_gyro_x;
					l_twi1_gyro_1_gyro_y		= g_twi1_gyro_1_gyro_y;
					l_twi1_gyro_1_gyro_z		= g_twi1_gyro_1_gyro_z;
					l_twi1_gyro_1_gyro_x_mdps	= g_twi1_gyro_1_gyro_x_mdps;
					l_twi1_gyro_1_gyro_y_mdps	= g_twi1_gyro_1_gyro_y_mdps;
					l_twi1_gyro_1_gyro_z_mdps	= g_twi1_gyro_1_gyro_z_mdps;
					l_twi1_gyro_1_temp			= g_twi1_gyro_1_temp;
					l_twi1_gyro_1_temp_deg_100	= g_twi1_gyro_1_temp_deg_100;
					l_twi1_gyro_2_mag_x			= g_twi1_gyro_2_mag_x;
					l_twi1_gyro_2_mag_y			= g_twi1_gyro_2_mag_y;
					l_twi1_gyro_2_mag_z			= g_twi1_gyro_2_mag_z;
					l_twi1_gyro_2_mag_x_nT		= g_twi1_gyro_2_mag_x_nT;
					l_twi1_gyro_2_mag_y_nT		= g_twi1_gyro_2_mag_y_nT;
					l_twi1_gyro_2_mag_z_nT		= g_twi1_gyro_2_mag_z_nT;
					cpu_irq_restore(flags);
				}

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
