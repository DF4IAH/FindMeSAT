/*
 * \file
 *
 * \brief FindMeSAT
 * spi.c
 *
 * Created: 27.12.2017 18:36:40
 * Author : DF4IAH
 */ 

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

//#include <math.h>

#include "main.h"
//#include "interpreter.h"
//#include "usb.h"

#include "spi_ax.h"


/* Add access to the global vars */
#include "externals.h"


void spi_init(void) {
	/* Set init level */
	ioport_set_pin_mode(AX_SEL,			IOPORT_INIT_HIGH);										// SEL  inactive
	ioport_set_pin_mode(AX_MOSI,		IOPORT_INIT_HIGH);										// MOSI inactive
	ioport_set_pin_mode(AX_CLK,			IOPORT_INIT_HIGH);										// CLK  inactive

	/* Set input pulling resistors */
	ioport_set_pin_mode(AX_IRQ,			IOPORT_MODE_PULLUP);
	ioport_set_pin_mode(AX_MOSI,		IOPORT_MODE_PULLUP);

	/* Set port directions */
	ioport_set_pin_dir(AX_IRQ,			IOPORT_DIR_INPUT);
	ioport_set_pin_dir(AX_SEL,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(AX_MOSI,			IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(AX_MISO,			IOPORT_DIR_INPUT);
	ioport_set_pin_dir(AX_CLK,			IOPORT_DIR_OUTPUT);
}

void spi_start(void) {
	g_ax_spi_device_conf.id = AX_SEL;

	spi_master_init(&SPI_AX);
	spi_master_setup_device(&SPI_AX, &g_ax_spi_device_conf, SPI_MODE_0, 10000, 0);				// 10 kHz
	spi_enable(&SPI_AX);


	/* TEST CODE */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x00;
	g_ax_spi_packet_buffer[1]	= 0x00;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);										// Address 0x00: silicon revision
	spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x80 | 0x01;
	g_ax_spi_packet_buffer[1]	= 0x96;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);										// Address 0x01: scratch register R/W - set
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x01;
	g_ax_spi_packet_buffer[1]	= 0x00;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);										// Address 0x01: scratch register R/W - check
	spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	while (true)
		nop();
}
