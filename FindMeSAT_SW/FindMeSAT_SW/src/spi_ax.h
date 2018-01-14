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
float spi_ax_calcFrequency_Regs2MHz(uint32_t vco_regval);
void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB);
void spi_ax_doRanging(void);
void spi_vco_select(uint32_t reg_freq);
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
void spi_ax_test_POCSAG_Rx(void);


/*
 * Powersetting - Table  for TXPWRCOEFFB (0x16A)
 *

 Value		 dBm
 ===============
 170.00    -10.0
 172.66    - 9.9
 175.15    - 9.8
 177.50    - 9.7
 179.71    - 9.6
 181.80    - 9.5
 183.79    - 9.4
 185.69    - 9.3
 187.51    - 9.2
 189.28    - 9.1
 191.00    - 9.0
 192.69    - 8.9
 194.37    - 8.8
 196.04    - 8.7
 197.73    - 8.6
 199.45    - 8.5
 201.21    - 8.4
 203.03    - 8.3
 204.93    - 8.2
 206.91    - 8.1
 209.00    - 8.0
 211.20    - 7.9
 213.52    - 7.8
 215.96    - 7.7
 218.50    - 7.6
 221.15    - 7.5
 223.92    - 7.4
 226.78    - 7.3
 229.76    - 7.2
 232.83    - 7.1
 236.00    - 7.0
 239.27    - 6.9
 242.62    - 6.8
 246.04    - 6.7
 249.53    - 6.6
 253.06    - 6.5
 256.62    - 6.4
 260.21    - 6.3
 263.82    - 6.2
 267.41    - 6.1
 271.00    - 6.0
 274.56    - 5.9
 278.10    - 5.8
 281.62    - 5.7
 285.12    - 5.6
 288.61    - 5.5
 292.10    - 5.4
 295.57    - 5.3
 299.05    - 5.2
 302.52    - 5.1
 306.00    - 5.0
 309.49    - 4.9
 312.99    - 4.8
 316.51    - 4.7
 320.05    - 4.6
 323.61    - 4.5
 327.21    - 4.4
 330.84    - 4.3
 334.52    - 4.2
 338.23    - 4.1
 342.00    - 4.0
 345.82    - 3.9
 349.70    - 3.8
 353.63    - 3.7
 357.63    - 3.6
 361.69    - 3.5
 365.81    - 3.4
 370.00    - 3.3
 374.26    - 3.2
 378.59    - 3.1
 383.00    - 3.0
 387.48    - 2.9
 392.04    - 2.8
 396.67    - 2.7
 401.37    - 2.6
 406.15    - 2.5
 410.99    - 2.4
 415.89    - 2.3
 420.87    - 2.2
 425.90    - 2.1
 431.00    - 2.0
 436.15    - 1.9
 441.34    - 1.8
 446.51    - 1.7
 451.66    - 1.6
 456.73    - 1.5
 461.71    - 1.4
 466.56    - 1.3
 471.24    - 1.2
 475.74    - 1.1
 480.00    - 1.0
 484.02    - 0.9
 487.85    - 0.8
 491.55    - 0.7
 495.17    - 0.6
 498.79    - 0.5
 502.47    - 0.4
 506.27    - 0.3
 510.25    - 0.2
 514.47    - 0.1
 519.00      0.0
 523.89    + 0.1
 529.12    + 0.2
 534.66    + 0.3
 540.49    + 0.4
 546.59    + 0.5
 552.91    + 0.6
 559.45    + 0.7
 566.16    + 0.8
 573.02    + 0.9
 580.00    + 1.0
 587.08    + 1.1
 594.27    + 1.2
 601.56    + 1.3
 608.96    + 1.4
 616.48    + 1.5
 624.12    + 1.6
 631.89    + 1.7
 639.78    + 1.8
 647.82    + 1.9
 656.00    + 2.0
 664.33    + 2.1
 672.81    + 2.2
 681.45    + 2.3
 690.26    + 2.4
 699.25    + 2.5
 708.41    + 2.6
 717.76    + 2.7
 727.31    + 2.8
 737.05    + 2.9
 747.00    + 3.0
 757.17    + 3.1
 767.57    + 3.2
 778.22    + 3.3
 789.17    + 3.4
 800.41    + 3.5
 811.98    + 3.6
 823.90    + 3.7
 836.20    + 3.8
 848.89    + 3.9
 862.00    + 4.0
 875.51    + 4.1
 889.25    + 4.2
 903.01    + 4.3
 916.58    + 4.4
 929.74    + 4.5
 942.28    + 4.6
 953.99    + 4.7
 964.65    + 4.8
 974.06    + 4.9
 982.00    + 5.0
 988.36    + 5.1
 993.40    + 5.2
 997.49    + 5.3
 1000.99   + 5.4
 1004.26   + 5.5
 1007.68   + 5.6
 1011.59   + 5.7
 1016.38   + 5.8
 1022.39   + 5.9
 1030.00   + 6.0
 1039.49   + 6.1
 1050.81   + 6.2
 1063.85   + 6.3
 1078.48   + 6.4
 1094.59   + 6.5
 1112.05   + 6.6
 1130.74   + 6.7
 1150.55   + 6.8
 1171.34   + 6.9
 1193.00   + 7.0
 1215.38   + 7.1
 1238.23   + 7.2
 1261.26   + 7.3
 1284.20   + 7.4
 1306.76   + 7.5
 1328.65   + 7.6
 1349.61   + 7.7
 1369.34   + 7.8
 1387.56   + 7.9
 1404.00   + 8.0
 1418.50   + 8.1
 1431.41   + 8.2
 1443.22   + 8.3
 1454.43   + 8.4
 1465.50   + 8.5
 1476.94   + 8.6
 1489.22   + 8.7
 1502.84   + 8.8
 1518.27   + 8.9
 1536.00   + 9.0
 1556.37   + 9.1
 1579.08   + 9.2
 1603.69   + 9.3
 1629.76   + 9.4
 1656.85   + 9.5
 1684.51   + 9.6
 1712.30   + 9.7
 1739.77   + 9.8
 1766.49   + 9.9
 1792.00   +10.0
 1816.03   +10.1
 1838.97   +10.2
 1861.34   +10.3
 1883.70   +10.4
 1906.59   +10.5
 1930.56   +10.6
 1956.14   +10.7
 1983.87   +10.8
 2014.31   +10.9
 2048.00   +11.0
 2085.31   +11.1
 2125.95   +11.2
 2169.46   +11.3
 2215.39   +11.4
 2263.27   +11.5
 2312.65   +11.6
 2363.07   +11.7
 2414.08   +11.8
 2465.21   +11.9
 2516.00   +12.0
 2566.17   +12.1
 2616.09   +12.2
 2666.31   +12.3
 2717.37   +12.4
 2769.82   +12.5
 2824.21   +12.6
 2881.08   +12.7
 2940.96   +12.8
 3004.42   +12.9
 3072.00   +13.0
 3143.98   +13.1
 3219.65   +13.2
 3298.01   +13.3
 3378.10   +13.4
 3458.93   +13.5
 3539.53   +13.6
 3618.92   +13.7
 3696.11   +13.8
 3770.13   +13.9
 3840.00   +14.0
 3904.74   +14.1
 3963.37   +14.2
 4014.91   +14.3
 4058.39   +14.4
 4092.82   +14.5
 4117.22   +14.6
 4130.62   +14.7
 4132.03   +14.8
 4120.49   +14.9
 4095.00   +15.0

 */

#endif /* SPI_H_ */