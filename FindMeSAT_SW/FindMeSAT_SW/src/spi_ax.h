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


#define C_SPI_AX_BUFFER_LENGTH										512


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
	AX_SET_REGISTERS_MODULATION_NO_CHANGE							= 0x01,
	AX_SET_REGISTERS_MODULATION_FSK									= 0x11,
	AX_SET_REGISTERS_MODULATION_PR1200								= 0x21,
	AX_SET_REGISTERS_MODULATION_ANALOG_FM							= 0x81,
} AX_SET_REGISTERS_MODULATION_t;

typedef enum AX_SET_REGISTERS_VARIANT {
	AX_SET_REGISTERS_VARIANT_NONE									= 0x00,
	AX_SET_REGISTERS_VARIANT_NO_CHANGE								= 0x01,
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


typedef enum AX_FIFO_CMD {
	AX_FIFO_CMD_NOP													= 0x00,
	AX_FIFO_CMD_ASK_COHERENT										= 0x01,
	AX_FIFO_CMD_CLEAR_FIFO_ERROR									= 0x02,
	AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS							= 0x03,
	AX_FIFO_CMD_COMMIT												= 0x04,
	AX_FIFO_CMD_ROLLBACK											= 0x05
} AX_FIFO_CMD_t;

typedef enum AX_FIFO_DATA_CMD {
	AX_FIFO_DATA_CMD_NOP_TX											= 0x00,
	AX_FIFO_DATA_CMD_RSSI_RX										= 0x31,
	AX_FIFO_DATA_CMD_TXCTRL_TX										= 0x3C,
	AX_FIFO_DATA_CMD_FREQOFFS_RX									= 0x52,
	AX_FIFO_DATA_CMD_ANTRSSI2_RX									= 0x55,
	AX_FIFO_DATA_CMD_REPEATDATA_TX									= 0x62,
	AX_FIFO_DATA_CMD_TIMER_RX										= 0x70,
	AX_FIFO_DATA_CMD_RFFREQOFFS_RX									= 0x73,
	AX_FIFO_DATA_CMD_DATARATE_RX									= 0x74,
	AX_FIFO_DATA_CMD_ANTRSSI3_RX									= 0x75,
	AX_FIFO_DATA_CMD_DATA_TX_RX										= 0xE1,
	AX_FIFO_DATA_CMD_TXPWR_TX										= 0xFD,
} AX_FIFO_DATA_CMD_t;

typedef enum AX_FIFO_DATA_FLAGS_TX_BF {
	AX_FIFO_DATA_FLAGS_TX_PKTSTART									= 0x01,
	AX_FIFO_DATA_FLAGS_TX_PKTEND									= 0x02,
	AX_FIFO_DATA_FLAGS_TX_RESIDUE									= 0x04,
	AX_FIFO_DATA_FLAGS_TX_NOCRC										= 0x08,
	AX_FIFO_DATA_FLAGS_TX_RAW										= 0x10,
	AX_FIFO_DATA_FLAGS_TX_UNENC										= 0x20
} AX_FIFO_CMD_DATA_FLAGS_TX_BF_t;

typedef enum AX_FIFO_DATA_FLAGS_RX_BF {
	AX_FIFO_DATA_FLAGS_RX_PKTSTART									= 0x01,
	AX_FIFO_DATA_FLAGS_RX_PKTEND									= 0x02,
	AX_FIFO_DATA_FLAGS_RX_RESIDUE									= 0x04,
	AX_FIFO_DATA_FLAGS_RX_CRCFAIL									= 0x08,
	AX_FIFO_DATA_FLAGS_RX_ADDRFAIL									= 0x10,
	AX_FIFO_DATA_FLAGS_RX_SIZEFAIL									= 0x20,
	AX_FIFO_DATA_FLAGS_RX_ABORT										= 0x40
} AX_FIFO_CMD_DATA_FLAGS_RX_BF_t;


static inline void spi_ax_select_device(void)
{
	/* clear PORT C4 */
	PORTC_OUTCLR = 0x10;
}

static inline void spi_ax_deselect_device(void)
{
	/* set   PORT C4 */
	PORTC_OUTSET = 0x10;
}


bool spi_ax_transport(bool isProgMem, const char* packet);

void spi_ax_sync2Powerdown(void);
void spi_ax_setPwrMode(AX_SET_REGISTERS_POWERMODE_t powerState);
void spi_ax_setRegisters(bool doReset, AX_SET_REGISTERS_MODULATION_t modulation, AX_SET_REGISTERS_VARIANT_t variant, AX_SET_REGISTERS_POWERMODE_t powerState);

uint32_t spi_ax_calcFrequency_Mhz2Regs(float f_mhz);
void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB);
void spi_ax_doRanging(void);
void spi_ax_selectVcoFreq(bool isFreqB);

void spi_ax_initRegisters_FSK(void);
void spi_ax_initRegisters_FSK_Tx(void);
void spi_ax_initRegisters_FSK_Rx(void);

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
void spi_ax_test_Analog_FM_Tx(void);
void spi_ax_test_Analog_FM_Rx(void);
void spi_ax_monitor_levels(void);

void spi_ax_test_PR1200_Tx(void);
void spi_ax_test_PR1200_Tx_FIFO_Flags(uint8_t count);
void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal_AddressField(void);
void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS_AddressField(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS_InformationField(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS(void);


#endif /* SPI_H_ */