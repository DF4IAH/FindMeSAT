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

#include "main.h"


static uint8_t		runmode								= (uint8_t) 0;  // global runmode
static bool			my_flag_autorize_cdc_transfert		= false;



/* USB stack feature */

void usb_init(void)
{
	udc_start();
}


/* USB BUS powered: suspend / resume operations */

void usb_callback_suspend_action(void)
{
	// Disable hardware component to reduce power consumption
	// Reduce power consumption in suspend mode (max. 2.5mA on VBUS)
	
}

void usb_callback_resume_action(void)
{
	// Re-enable hardware component
	
}


/* USB wake-up remote host feature */

void usb_callback_remotewakeup_enable(void)
{
	// Enable application wakeup events (e.g. enable GPIO interrupt)
	
}

void usb_callback_remotewakeup_disable(void)
{
	// Disable application wakeup events (e.g. disable GPIO interrupt)
	
}

void usb_send_wakeup_event(void)
{
	udc_remotewakeup();
}


/* USB CDC feature for serial communication */

bool usb_callback_cdc_enable(void)
{
	my_flag_autorize_cdc_transfert = true;
	return true;
}

void usb_callback_cdc_disable(void)
{
	my_flag_autorize_cdc_transfert = false;
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



/* TASK when woken up */

void task(void)
{
	// handling the USB connection
	if (my_flag_autorize_cdc_transfert) {
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
	
		// handling ...
	}
}


/* MAIN Loop Shutdown */

void halt(void)
{
	runmode = (uint8_t) 0;
}



/* main() function implementation */

int main(void)
{
	uint8_t retcode = 0;
	struct pwm_config pwm_vctcxo_cfg;
	
	pmic_init();
	sysclk_init();


	/* VCTCXO PWM signal generation */
#if 1
	pwm_init(&pwm_vctcxo_cfg, PWM_TCC0, PWM_CH_D, 500);							// Init pwm structure and enable timer
	pwm_start(&pwm_vctcxo_cfg, 45);												// Start PWM. Percentage with 1% granularity is to coarse, use driver access instead
	tc_write_cc_buffer(&TCC0, TC_CCD, (uint16_t) (0.5f + 65536 * 1.5f/3.3f));	// Initial value for VCTCXO @ 1.5 V
#else
	tc_enable(&TCC0);
	tc_set_wgm(&TCC0, TC_WG_SS);												// 16-bit counter with PWM signal generation
	tc_write_period(&TCC0, 60000);												// Clock @ 30 MHz gives 500 iterations / sec for the VCTCXO pull-voltage low pass filter
	tc_write_cc_buffer(&TCC0, TC_CCD, (uint16_t) (65536 * 1.5f/3.3f));			// Initial value for VCTCXO @ 1.5 V
	tc_enable_cc_channels(&TCC0, TC_CCDEN);										// Enable PWM to pin alternate selection for TCC0D
#endif

	/* all interrupt sources prepared for enabling interrupt, here */
	irq_initialize_vectors();
	cpu_irq_enable();


	/* all timer starts, here */
	tc_write_clock_source(&TCC0, TC_TC0_CLKSEL_DIV1_gc);			// VCTCXO PWM start, output still is Z-state
	
	/* activates all in/out pins - transitions from Z to dedicated states */
	board_init();
	
	/* unlocks all sleep mode levels */
	sleepmgr_init();
	
	/* USB device stack start function to enable stack and start USB */
	udc_start();


	/* The application code */
	runmode = (uint8_t) 1;
    while (runmode) {
		task();
		sleepmgr_enter_sleep();
    }
	
	cpu_irq_disable();
	sleepmgr_enter_sleep();
	
	return retcode;
}
