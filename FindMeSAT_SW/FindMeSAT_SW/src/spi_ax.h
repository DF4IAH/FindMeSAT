/*
 * \file
 *
 * \brief FindMeSAT
 * spi.h
 *
 * Created: 27.12.2017 18:37:04
 * Author : DF4IAH
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <asf.h>



#if 0
# define USART_SPI_AX												USARTC1
#else
# define SPI_AX														SPIC
#endif


#define C_SPI_AX_BUFFER_LENGTH										256


typedef enum SPI_AX_TRPT_STATE {
	SPI_AX_TRPT_STATE_DISABLED										= 0x00,
	SPI_AX_TRPT_STATE_ENA_ADDR,
	SPI_AX_TRPT_STATE_ENA_DATA_WRITE,
	SPI_AX_TRPT_STATE_ENA_DATA_READ,
	SPI_AX_TRPT_STATE_COMPLETE,

	SPI_AX_TRPT_STATE_END											= 0x7f,
	SPI_AX_TRPT_STATE_ERROR											= 0xff,
} SPI_AX_TRPT_STATE_t;


typedef enum AX_SET_REGISTERS_MODULATION {
	AX_SET_REGISTERS_MODULATION_NONE								= 0x00,
	AX_SET_REGISTERS_MODULATION_DEFAULT								= 0x11,
	AX_SET_REGISTERS_MODULATION_PR1200								= 0x21,
	AX_SET_REGISTERS_MODULATION_ANALOG_FM							= 0x81,
} AX_SET_REGISTERS_MODULATION_t;

typedef enum AX_SET_REGISTERS_VARIANT {
	AX_SET_REGISTERS_VARIANT_NONE									= 0x00,
	AX_SET_REGISTERS_VARIANT_TX										= 0x11,
	AX_SET_REGISTERS_VARIANT_RX										= 0x21,
	AX_SET_REGISTERS_VARIANT_RX_WOR,
	AX_SET_REGISTERS_VARIANT_RX_CONT,
	AX_SET_REGISTERS_VARIANT_RX_CONT_SINGLEPARAMSET,
} AX_SET_REGISTERS_VARIANT_t;

typedef enum AX_SET_REGISTERS_POWERMODE {
	AX_SET_REGISTERS_POWERMODE_POWERDOWN							= 0x00,
	AX_SET_REGISTERS_POWERMODE_DEEPSLEEP							= 0x01,
	AX_SET_REGISTERS_POWERMODE_STANDBY								= 0x05,
	AX_SET_REGISTERS_POWERMODE_FIFO									= 0x06,
	AX_SET_REGISTERS_POWERMODE_SYNTHRX								= 0x08,
	AX_SET_REGISTERS_POWERMODE_FULLRX								= 0x09,
	AX_SET_REGISTERS_POWERMODE_WOR									= 0x0B,
	AX_SET_REGISTERS_POWERMODE_SYNTHTX								= 0x0C,
	AX_SET_REGISTERS_POWERMODE_FULLTX								= 0x0D,
} AX_SET_REGISTERS_POWERMODE_t;



bool spi_ax_transport(bool isProgMem, const char* packet);

void spi_ax_sync2Powerdown(void);
void spi_ax_setPwrMode(AX_SET_REGISTERS_POWERMODE_t powerState);
void spi_ax_setRegisters(bool doReset, AX_SET_REGISTERS_MODULATION_t modulation, AX_SET_REGISTERS_VARIANT_t variant, AX_SET_REGISTERS_POWERMODE_t powerState);

uint32_t spi_ax_calcFrequency_Mhz2Regs(float f_mhz);
void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB);
void spi_ax_doRanging(void);
void spi_ax_selectVcoFreq(bool isFreqB);

void spi_ax_initRegisters_Default(void);

void spi_ax_initRegisters_PR1200(void);
void spi_ax_initRegisters_PR1200_Tx(void);
void spi_ax_initRegisters_PR1200_Rx(void);
void spi_ax_initRegisters_PR1200_Rx_WoR(void);
void spi_ax_initRegisters_PR1200_Rx_cont(void);
void spi_ax_initRegisters_PR1200_Rx_cont_SingleParamSet(void);

void spi_ax_initRegisters_AnlogFM(void);
void spi_ax_initRegisters_AnlogFM_Tx(void);
void spi_ax_initRegisters_AnlogFM_Rx(void);

void spi_init(void);
void spi_start(void);

/* Debugging */
void spi_ax_test_Analog_FM_TX(void);
void spi_ax_test_Analog_FM_RX(void);
void spi_ax_monitor_levels(void);

void spi_ax_test_PR1200_TX(void);


#endif /* SPI_H_ */