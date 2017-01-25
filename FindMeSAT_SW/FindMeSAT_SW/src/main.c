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
	
	sysclk_init();
	irq_initialize_vectors();
	cpu_irq_enable();
	board_init();
	sleepmgr_init();
		
	/* USB device stack start function to enable stack and start USB */
	udc_start();

	/* Insert application code here, after the board has been initialized. */
    /* Replace with your application code */
	runmode = (uint8_t) 1;
    while (runmode) {
		task();
		sleepmgr_enter_sleep();
    }

	cpu_irq_disable();
	sleepmgr_enter_sleep();
	
	return retcode;
}
