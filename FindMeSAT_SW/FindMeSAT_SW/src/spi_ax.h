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


#define C_AX_PRW_LENGTH												253
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
	AX_SET_REGISTERS_MODULATION_POCSAG								= 0x71,
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
} AX_FIFO_DATA_FLAGS_TX_BF_t;

typedef enum AX_FIFO_DATA_FLAGS_RX_BF {
	AX_FIFO_DATA_FLAGS_RX_PKTSTART									= 0x01,
	AX_FIFO_DATA_FLAGS_RX_PKTEND									= 0x02,
	AX_FIFO_DATA_FLAGS_RX_RESIDUE									= 0x04,
	AX_FIFO_DATA_FLAGS_RX_CRCFAIL									= 0x08,
	AX_FIFO_DATA_FLAGS_RX_ADDRFAIL									= 0x10,
	AX_FIFO_DATA_FLAGS_RX_SIZEFAIL									= 0x20,
	AX_FIFO_DATA_FLAGS_RX_ABORT										= 0x40
} AX_FIFO_DATA_FLAGS_RX_BF_t;


typedef enum AX_FIFO_RX_FSM {
	AX_FIFO_RX_FSM__START											= 0x00,
	AX_FIFO_RX_FSM__STOP											= 0x0f,

	AX_FIFO_RX_FSM_TIMER_1											= 0x11,
	AX_FIFO_RX_FSM_TIMER_2,
	AX_FIFO_RX_FSM_TIMER_3,

	AX_FIFO_RX_FSM_RSSI_1											= 0x21,

	AX_FIFO_RX_FSM_ANTRSSI2_1										= 0x31,
	AX_FIFO_RX_FSM_ANTRSSI2_2,

	AX_FIFO_RX_FSM_ANTRSSI3_1										= 0x41,
	AX_FIFO_RX_FSM_ANTRSSI3_2,
	AX_FIFO_RX_FSM_ANTRSSI3_3,

	AX_FIFO_RX_FSM_RFFREQOFFS_1										= 0x51,
	AX_FIFO_RX_FSM_RFFREQOFFS_2,
	AX_FIFO_RX_FSM_RFFREQOFFS_3,

	AX_FIFO_RX_FSM_FREQOFFS_1										= 0x61,
	AX_FIFO_RX_FSM_FREQOFFS_2,

	AX_FIFO_RX_FSM_DATARATE_1										= 0x71,
	AX_FIFO_RX_FSM_DATARATE_2,
	AX_FIFO_RX_FSM_DATARATE_3,

	AX_FIFO_RX_FSM_DATA_1											= 0x81,
	AX_FIFO_RX_FSM_DATA_2,
	AX_FIFO_RX_FSM_DATA_3,

	AX_FIFO_RX_FSM__FAIL_STATE										= 0xf1,
	AX_FIFO_RX_FSM__FAIL_CMD										= 0xf2,
} AX_FIFO_RX_FSM_t;


typedef enum AX_POCSAG_CODES_ENUM {
	AX_POCSAG_CODES_PREAMBLE										= 0xaaaaaaaaUL,		// LSB   0 1 0 1  0 1 0 1     0 1 0 1  0 1 0 1     0 1 0 1  0 1 0 1     0 1 0 1  0 1 0 1   MSB
	AX_POCSAG_CODES_SYNCWORD										= 0x1ba84b3eUL,		// LSB   0 1 1 1  1 1 0 0     1 1 0 1  0 0 1 0     0 0 0 1  0 1 0 1     1 1 0 1  1 0 0 0   MSB
	AX_POCSAG_CODES_IDLEWORD										= 0xe983915eUL,		// LSB   0 1 1 1  1 0 1 0     1 0 0 0  1 0 0 1     1 1 0 0  0 0 0 1     1 0 0 1  0 1 1 1   MSB
} AX_POCSAG_CODES_t;

typedef enum AX_POCSAG_CW_ENUM {
	AX_POCSAG_CW_IS_ADDR											= 0x00000000UL,
	AX_POCSAG_CW_IS_MSG												= 0x00000001UL,

	AX_POCSAG_CW_MODE0_NUMERIC										= 0x00000000UL,
	AX_POCSAG_CW_MODE1												= 0x00080000UL,
	AX_POCSAG_CW_MODE2												= 0x00100000UL,
	AX_POCSAG_CW_MODE3_ALPHA										= 0x00180000UL,
} AX_POCSAG_CW_t;



#if 0
inline static uint8_t s_sel_u8_from_u32(uint32_t in_u32, uint8_t sel);
inline static void s_spi_ax_select_device(void);
inline static void s_spi_ax_deselect_device(void);
inline static uint8_t s_strGetHex(const char* str);
inline static uint8_t s_strGetDec(const char* str, int* o_val);
#endif

uint32_t spi_ax_POCASG_get3Nbls(const char* buf, int len, uint16_t nblIdx);


bool spi_ax_transport(bool isProgMem, const char* packet);

void spi_ax_sync2Powerdown(void);
void spi_ax_setPower_dBm(float dBm);
void spi_ax_setPwrMode(AX_SET_REGISTERS_POWERMODE_t powerState);
void spi_ax_setRegisters(bool doReset, AX_SET_REGISTERS_MODULATION_t modulation, AX_SET_REGISTERS_VARIANT_t variant, AX_SET_REGISTERS_POWERMODE_t powerState);

uint32_t spi_ax_create_POCSAG_checksumParity(uint32_t codeword_in);
uint32_t spi_ax_calcFrequency_Mhz2Regs(float f_mhz);
float spi_ax_calcFrequency_Regs2MHz(uint32_t vco_regval);
void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB);
void spi_ax_doRanging(void);
bool spi_ax_vco_select(uint32_t reg_freq, bool force);
bool spi_ax_selectVcoFreq(bool isFreqB);

void spi_ax_initRegisters_FSK(void);
void spi_ax_initRegisters_FSK_Tx(void);
void spi_ax_initRegisters_FSK_Rx(void);

void spi_ax_initRegisters_PR1200(void);
void spi_ax_initRegisters_PR1200_Tx(void);
void spi_ax_initRegisters_PR1200_Rx(void);
void spi_ax_initRegisters_PR1200_Rx_WoR(void);
void spi_ax_initRegisters_PR1200_Rx_cont(void);
void spi_ax_initRegisters_PR1200_Rx_cont_SingleParamSet(void);

void spi_ax_initRegisters_POCSAG(void);
void spi_ax_initRegisters_POCSAG_Tx(void);
void spi_ax_initRegisters_POCSAG_Rx(void);
void spi_ax_initRegisters_POCSAG_Rx_WoR(void);
void spi_ax_initRegisters_POCSAG_Rx_cont(void);

void spi_ax_initRegisters_AnlogFM(void);
void spi_ax_initRegisters_AnlogFM_Tx(void);
void spi_ax_initRegisters_AnlogFM_Rx(void);

void spi_init(void);
void spi_start(void);

/* Debugging */
void spi_ax_monitor_levels(void);
void spi_ax_Rx_FIFO(void);

void spi_ax_test_Analog_FM_Tx(void);
void spi_ax_test_Analog_FM_Rx(void);

void spi_ax_test_PR1200_Tx(void);
void spi_ax_test_PR1200_Tx_FIFO_Flags(uint8_t count);
void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal_AddressField(void);
void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS_AddressField(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS_InformationField(void);
void spi_ax_test_PR1200_Tx_FIFO_APRS(void);
void spi_ax_test_PR1200_Rx(void);

void spi_ax_test_POCSAG(void);
void spi_ax_test_POCSAG_Tx(void);
void spi_ax_test_POCSAG_Tx_FIFO_Preamble(void);
void spi_ax_test_POCSAG_Tx_FIFO_Batches(uint32_t tgtAddr, const char* msgBuf, uint8_t len);
void spi_ax_test_POCSAG_Rx(void);


/*
 * Technical details for POCSAG receiption:		@see http://akafunk.faveve.uni-stuttgart.de/pocsag/
 * QRG = 439.9875 MHz (12.5 kHz channel spacing
 * Modulation = FSK with  +/- 4.0 kHz
 * '0' = Higher frequency
 * '1' = Lower  frequency
 *
 * Message:										@see http://www.qsl.net/db0avh/code.html
 *
 * Bit No.:    Function:					Remarks:
 *
 * 1           Codeword explanation			0 = Address, 1 = Message
 * 2-21        Address (2-19)				Mode (20-21) / Message (2-21)
 * 22-31       Checksum
 * 32          Parity (even)
 *
 * Preamble-SW-CWCW-CWCW-CWCW-CWCW-CWCW-CWCW-CWCW-CWCW-SW-CWCW-CWCW-CWCW ...
 *
 * Preamble >= 288 x '01'  equals to 18x 0xAAAAAAAA
 *
 * Checksum:  Each codeword contains 21 information bits. They are the coefficients of a polynomial (x^30 .. X^10). The remaining coefficients of the polynomial are being zeroed.
 *			 That polynomial has to be divided (modulo 2) with the generator polynomial (x^10 + x^9 + x^8 + x^6 + x^5 + x^3 + x^0).
 *			 The checksum bits are defined to be the resulting bits from the remaining coefficients (x^9 .. x^0) after the division.
 *
 * Characters & Numbers:
 *
 *                                          Character table
 *
 *                                          Numeric      Alpha
 *                                                    7   0   0   0 0 1 1 1 1
 *                        Bit                         6   0   0   1 1 0 0 1 1
 *                        4 3 2 1                     5   0   1   0 1 0 1 0 1
 *
 *                        0 0 0 0              0         NUL DLE ' '0 ß P ' p
 *                        0 0 0 1              1         SOH  DC  ! 1 A Q a q
 *                        0 0 1 0              2         STX  DC  " 2 B R b r
 *                        0 0 1 1              3         ETX  DC  # 3 C S c s
 *                        0 1 0 0              4         EOT  DC  $ 4 D T d t
 *                        0 1 0 1              5         ENQ NAK  % 5 E U e u
 *                        0 1 1 0              6         ACK SYN  & 6 F V f v
 *                        0 1 1 1              7         BEL ETB  ` 7 G W g w
 *                        1 0 0 0              8         BS  CAN  ( 8 H X h x
 *                        1 0 0 1              9         HT  EM   ) 9 I Y i y
 *                        1 0 1 0          - SPARE -     LF  SUB  * : J Z j z
 *                        1 0 1 1              U         VT  ESC  + ; K Ä k ä
 *                        1 1 0 0             ' '        FF  FS   , < L Ö l ö
 *                        1 1 0 1              -         CR  GS   - = M Ü m ü
 *                        1 1 1 0              ]         SO  RS   . > N ^ n ß
 *                        1 1 1 1              [         SI  US   / ? O _ O DEL
 */

#endif /* SPI_H_ */
