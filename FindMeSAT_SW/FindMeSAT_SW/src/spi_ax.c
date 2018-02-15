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
#include <ctype.h>

#include "main.h"
//#include "interpreter.h"
//#include "usb.h"

#include "spi_ax.h"


/* Add access to the global vars */
#include "externals.h"



/* BLOCK of DEFINITIONS */

//#	define  AX_RUN_VCO2_APRS_TX			true

//# define	AX_TEST_VCO1_BANDENDS		true
//# define	AX_TEST_VCO1_FSK_TX			true
//# define	AX_TEST_VCO1_FSK_RX			true
//# define	AX_TEST_VCO1_POCSAG_TX		true
//# define	AX_TEST_VCO1_POCSAG_RX		true

//# define	AX_TEST_VCO2_BANDENDS		true
//# define	AX_TEST_VCO2_ANALOG_FM_TX	true
//# define	AX_TEST_VCO2_ANALOG_FM_RX	true
//# define	AX_TEST_VCO2_PR1200_TX		true
//# define	AX_TEST_VCO2_PR1200_RX		true


#if defined(AX_TEST_VCO1_BANDENDS) | defined(AX_TEST_VCO1_FSK_TX)		| defined(AX_TEST_VCO1_FSK_RX)			| defined(AX_TEST_VCO1_FSK_RX)		| defined(AX_TEST_VCO1_POCSAG_RX) |	\
	defined(AX_TEST_VCO2_BANDENDS) | defined(AX_TEST_VCO2_ANALOG_FM_TX)	| defined(AX_TEST_VCO2_ANALOG_FM_RX)	| defined(AX_TEST_VCO2_PR1200_TX)	| defined(AX_TEST_VCO2_PR1200_RX)
# ifndef AX_TEST
# define AX_TEST true
# endif
#endif



/* Forward declarations */
#ifdef AX_TEST
static void s_spi_start_testBox(void);
#endif



inline
static uint8_t s_sel_u8_from_u32(uint32_t in_u32, uint8_t sel)
{
	return 0xff & (in_u32 >> (sel << 3));
}

inline
static void s_spi_ax_select_device(void)
{
	/* clear PORT C4 */
	PORTC_OUTCLR = 0x10;
}

inline
static void s_spi_ax_deselect_device(void)
{
	/* set   PORT C4 */
	PORTC_OUTSET = 0x10;
}

inline
static uint8_t s_strGetHex(const char* str)
{
	unsigned int ret = 0;

	sscanf(str, "%02x", &ret);
	return (uint8_t)ret;
}

inline
static uint8_t s_strGetDec(const char* str, int* o_val)
{
	return (uint8_t) sscanf(str, "%d", o_val);
}


AX_POCSAG_CW2_t ax_pocsag_analyze_msg_tgtFunc_get(const char* msg, uint16_t msgLen)
{
	if (!msg || !msgLen) {
		return AX_POCSAG_CW2_MODE1_TONE;
	}

	for (uint16_t idx = 0; idx < msgLen; idx++) {
		char c = *(msg + idx);

		if (('0' <= c) && (c <= '9')) {
			continue;
		}

		if ((c == ' ') || (c == '-') || (toupper(c) == 'U') || (c == '[') || (c == ']') || (c == '(') || (c == ')')) {
			continue;
		}

		return AX_POCSAG_CW2_MODE3_ALPHANUM;
	}

	return AX_POCSAG_CW2_MODE0_NUMERIC;
}

uint32_t spi_ax_pocsag_calc_checksumParity(uint32_t codeword_in)
{
	const uint32_t inPoly		= 0xfffff800UL & codeword_in;
	const uint32_t genPoly		= 0x769UL;														// generating polynomial   x10 + x9 + x8 + x6 + x5 + x3 + 1
	uint32_t cw_calc			= inPoly;

	// Calculate the BCH check
	for (uint8_t idx = 31; idx >= 11; idx--) {
		if (cw_calc  & (1UL <<  idx)) {
			volatile uint32_t genPolyShifted = genPoly << (idx - 10);
			cw_calc ^= genPolyShifted;
			(void)genPolyShifted;
		}
	}
	cw_calc &= 0x000007feUL;

	/* Data with BCH check */
	cw_calc |= inPoly;

	/* Calculate for Even Parity */
	uint32_t par = cw_calc;
	{
		par ^= par >> 0x01;
		par ^= par >> 0x02;
		par ^= par >> 0x04;
		par ^= par >> 0x08;
		par ^= par >> 0x10;
		par &= 1;
	}

	/* Data with 31:21 BCH and Parity */
	cw_calc	|= par;

	return cw_calc;
}

uint8_t spi_ax_pocsag_getBcd(char c)
{
	uint8_t u8;

	switch (c) {
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
			u8 = c - 0x30;
			break;

		case 0x23:																				// #
		case 0x2a:																				// *
			u8 = 0xa;
			break;

		case 0x55:																				// U
		case 0x75:																				// u
			u8 = 0xb;
			break;

		case 0x20:																				// ' '
			u8 = 0xc;
			break;

		case 0x2d:																				// -
			u8 = 0xd;
			break;

		case 0x5d:																				// ]
		case 0x29:																				// )
			u8 = 0xe;
			break;

		case 0x5b:																				// [
		case 0x28:																				// (
			u8 = 0xf;
			break;

		default:
			u8 = 0xc;																			// ' '
	}
	return u8;
}

uint32_t spi_ax_pocsag_get20Bits(const char* tgtMsg, uint16_t tgtMsgLen, AX_POCSAG_CW2_t tgtFunc, uint16_t msgBitIdx)
{
	uint32_t msgWord = 0UL;
	uint16_t msgByteIdx;
	uint16_t msgByteIdxLast = 65535;
	uint8_t  byteBitPos;
	uint8_t  byte = 0;

	for (uint16_t bitCnt = 20; bitCnt; bitCnt--, msgBitIdx++) {
		if (tgtFunc == AX_POCSAG_CW2_MODE0_NUMERIC) {
			/* Calculate source byte and bit position of byte */
			msgByteIdx	= msgBitIdx >> 2;
			byteBitPos	= msgBitIdx & 0x3;

			/* Get the byte as NUMERIC - NUMERIC_SPACE when message exhausted */
			if (msgByteIdxLast != msgByteIdx) {
				msgByteIdxLast = msgByteIdx;
				byte = (msgByteIdx < tgtMsgLen) ?  spi_ax_pocsag_getBcd(*(tgtMsg + msgByteIdx)) : 0xc;
			}

		} else {
			/* Calculate source byte and bit position of byte */
			msgByteIdx	= msgBitIdx / 7;
			byteBitPos	= msgBitIdx % 7;

			/* Get the byte as ALPHA - NULL when message exhausted */
			if (msgByteIdxLast != msgByteIdx) {
				msgByteIdxLast = msgByteIdx;
				byte = (msgByteIdx < tgtMsgLen) ?  (uint8_t) *(tgtMsg + msgByteIdx) : 0;
			}
		}

		msgWord <<= 1;
		if (byte & (1U << byteBitPos)) {
			msgWord |= 0b1;
		}
	}
	return msgWord;
}

uint16_t spi_ax_pocsag_skyper_RIC2ActivationString(char* outBuf, uint16_t outBufSize, uint32_t RIC)
{
	uint16_t outLen = 0;

	/* Sanity checks */
	if (!outBuf || outBufSize <= g_ax_pocsag_activation_code_len) {
		return 0;
	}

	for (uint16_t codeIdx = 0; codeIdx < g_ax_pocsag_activation_code_len; codeIdx++) {
		outBuf[outLen++] = (((RIC
							>> g_ax_pocsag_activation_code[codeIdx][AX_POCSAG_SKYPER_ACTIVATION_ARY_SHIFT])
							&  g_ax_pocsag_activation_code[codeIdx][AX_POCSAG_SKYPER_ACTIVATION_ARY_MASK])
							+  g_ax_pocsag_activation_code[codeIdx][AX_POCSAG_SKYPER_ACTIVATION_ARY_OFFSET])
							&  0x7f;															// ASCII-7bit
	}
	outBuf[outLen] = 0;

	return outLen;
}

const char					PM_POCSAG_SKYPER_TIME[]					= "%02d%02d%02d   %02d%02d%02d";
PROGMEM_DECLARE(const char, PM_POCSAG_SKYPER_TIME[]);
uint16_t spi_ax_pocsag_skyper_TimeString(char* outBuf, uint16_t outBufSize, struct calendar_date* calDat)
{
	/* Sanity checks */
	if (!outBuf || outBufSize <= 15) {
		return 0;
	}

	/* Put calendar information into Skyper time message form */
	uint16_t outLen = snprintf_P(outBuf, outBufSize, PM_POCSAG_SKYPER_TIME, calDat->hour, calDat->minute, calDat->second, calDat->date + 1, calDat->month + 1, calDat->year % 100);

	return outLen;
}

const char					PM_POCSAG_SKYPER_RUBRIC[]				= "1%c%c";
PROGMEM_DECLARE(const char, PM_POCSAG_SKYPER_RUBRIC[]);
uint16_t spi_ax_pocsag_skyper_RubricString(char* outBuf, uint16_t outBufSize, uint8_t rubricNumber, const char* rubricLabel, uint16_t rubricLabelLen)
{
	/* Sanity checks */
	if (!outBuf || outBufSize < 8) {
		return 0;
	}

	/* Put rubric label into Skyper rubric message form */
	uint16_t outLen = snprintf_P(outBuf, outBufSize, PM_POCSAG_SKYPER_RUBRIC, ' ' + (rubricNumber - 1), ' ' + 10, rubricLabel);

	char* outBuf_ptr = outBuf + outLen;
	for (uint16_t idx = 0; idx < rubricLabelLen; idx++, outLen++) {
		*(outBuf_ptr++) = *(rubricLabel + idx) + 1;
	}
	*(outBuf_ptr++) = 0;

	return outLen;
}

const char					PM_POCSAG_SKYPER_NEWS[]				= "%c%c";
PROGMEM_DECLARE(const char, PM_POCSAG_SKYPER_NEWS[]);
uint16_t spi_ax_pocsag_skyper_NewsString(char* outBuf, uint16_t outBufSize, uint8_t rubricNumber, uint8_t newsNumber, const char* newsString, uint16_t newsStringLen)
{
	/* Sanity checks */
	if (!outBuf || outBufSize < 8) {
		return 0;
	}

	/* Put news into Skyper news message form */
	uint16_t outLen = snprintf_P(outBuf, outBufSize, PM_POCSAG_SKYPER_NEWS, ' ' + (rubricNumber - 1), ' ' + newsNumber);

	char* outBuf_ptr = outBuf + outLen;
	for (uint16_t idx = 0; idx < newsStringLen; idx++, outLen++) {
		*(outBuf_ptr++) = *(newsString + idx) + 1;
	}
	*(outBuf_ptr++) = 0;

	return outLen;
}


bool spi_ax_transport(bool isProgMem, const char* packet)
{
	uint8_t						l_axIdx								= 0;
	uint16_t					l_fmtIdx							= 0;
	SPI_AX_TRPT_STATE_t			l_state								= SPI_AX_TRPT_STATE_DISABLED;
	char						l_msg_buf[C_SPI_AX_BUFFER_LENGTH];
	const char					*l_fmtPtr							= isProgMem ?  l_msg_buf : packet;
	bool						l_isRead							= false;
	uint8_t						l_adr0 = 0, l_adr1 = 0;

	memset(l_msg_buf, 0, sizeof(l_msg_buf));
	memset(g_ax_spi_packet_buffer, 0, sizeof(g_ax_spi_packet_buffer));

	if (isProgMem) {
		snprintf_P(l_msg_buf, sizeof(l_msg_buf), packet);
	}

	do {
		switch (l_state) {
			case SPI_AX_TRPT_STATE_DISABLED:
			{
				if (*(l_fmtPtr + l_fmtIdx++) == '<') {
					l_adr0 = 0; l_adr1 = 0;

					/* Skip mandatory space */
					if (*(l_fmtPtr + l_fmtIdx++) != ' ') {
						l_state = SPI_AX_TRPT_STATE_ERROR;
						break;
					}

					spi_select_device(&SPI_AX, &g_ax_spi_device_conf);

					/* Check if read symbol for status request comes first */
					if (*(l_fmtPtr + l_fmtIdx) == 'R') {
						l_state = SPI_AX_TRPT_STATE_ENA_DATA_READ;
						break;

					} else {
						l_state = SPI_AX_TRPT_STATE_ENA_ADDR;
					}

				} else {
					l_state = SPI_AX_TRPT_STATE_ERROR;
					break;
				}
			}
			break;

			case SPI_AX_TRPT_STATE_ENA_ADDR:
			{
				l_adr0 = s_strGetHex(l_fmtPtr + l_fmtIdx); l_fmtIdx++; l_fmtIdx++;

				/* Build packet */
				g_ax_spi_packet_buffer[l_axIdx++] = l_adr0;

				l_isRead = (l_adr0 & 0x80) ?  false : true;

				/* Long address format */
				if ((l_adr0 & 0x7f) >= 0b01110000) {
					/* Skip mandatory space */
					if (*(l_fmtPtr + l_fmtIdx++) != ' ') {
						l_state = SPI_AX_TRPT_STATE_ERROR;
						break;
					}

					l_adr1 = s_strGetHex(l_fmtPtr + l_fmtIdx); l_fmtIdx++; l_fmtIdx++;

					/* Append to packet */
					g_ax_spi_packet_buffer[l_axIdx++] = l_adr1;
				}

				/* Skip mandatory space */
				if (*(l_fmtPtr + l_fmtIdx++) != ' ') {
					l_state = SPI_AX_TRPT_STATE_ERROR;
					break;
				}

				/* Send read address */
				if (l_isRead) {
					spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, l_axIdx);
					l_axIdx = 0;
					l_state = SPI_AX_TRPT_STATE_ENA_DATA_READ;

				} else {
					l_state = SPI_AX_TRPT_STATE_ENA_DATA_WRITE;
				}
			}
			break;

			case SPI_AX_TRPT_STATE_ENA_DATA_WRITE:
			{
				char c = tolower(*(l_fmtPtr + l_fmtIdx));
				if (c == '>') {
					spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, l_axIdx);
					l_axIdx = 0;
					l_state = SPI_AX_TRPT_STATE_COMPLETE;
					break;

				} else if ((('0' <= c) && (c <= '9')) || (('a' <= c) && (c <= 'f'))) {
					uint8_t data = s_strGetHex(l_fmtPtr + l_fmtIdx); l_fmtIdx++; l_fmtIdx++;

					/* Append to packet */
					g_ax_spi_packet_buffer[l_axIdx++] = data;

					/* Skip mandatory space */
					if (*(l_fmtPtr + l_fmtIdx++) != ' ') {
						l_state = SPI_AX_TRPT_STATE_ERROR;
						break;
					}

				} else {
					/* Proof: trying to read from a write address ? */
					l_state = SPI_AX_TRPT_STATE_ERROR;
					break;
				}
			}
			break;

			case SPI_AX_TRPT_STATE_ENA_DATA_READ:
			{
				int		cnt = 0;
				uint8_t	len;

				/* Skip mandatory read symbol */
				if (*(l_fmtPtr + l_fmtIdx++) != 'R') {
					l_state = SPI_AX_TRPT_STATE_ERROR;
					break;
				}

				len = s_strGetDec(l_fmtPtr + l_fmtIdx, &cnt); l_fmtIdx += ++len;

				/* Execute read */
				spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, cnt);

				/* Expect deselect symbol */
				if (*(l_fmtPtr + l_fmtIdx++) == '>') {
					l_state = SPI_AX_TRPT_STATE_COMPLETE;

				} else {
					l_state = SPI_AX_TRPT_STATE_ERROR;
					break;
				}
			}
			break;

			case SPI_AX_TRPT_STATE_COMPLETE:
			{
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
				l_state = SPI_AX_TRPT_STATE_END;
			}
			break;

			case SPI_AX_TRPT_STATE_ERROR:
			default:
			{
				/* ERROR in format string */
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

				//return false;
				while (true)
					nop();
			}
		}
	} while (l_state != SPI_AX_TRPT_STATE_END);

	return true;
}


void spi_ax_sync2Powerdown(void)
{
	/* SEL line and MISO check */
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
	delay_us(1);
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);

	while (!ioport_get_pin_level(AX_MISO_PIN)) ;

	/* Set RESET */
	spi_ax_transport(false, "< 82 80 >");													// WR address 0x02: PWRMODE - RESET, PWRMODE=Powerdown

	/* Clear RESET and Powerdown */
	spi_ax_transport(false, "< 82 00 >");													// WR address 0x02: PWRMODE - RESET, PWRMODE=Powerdown
}

static void s_spi_ax_xtal_waitReady(void)
{
	/*  Wait until crystal oscillator is ready */
	do {
		spi_ax_transport(false, "< 1d R1 >");												// RD Address 0x1D: XTALSTATUS
	} while (!(g_ax_spi_packet_buffer[0] & 0x01));											// Bit 0: XTALRUN
}

void spi_ax_setPower_dBm(float dBm)
{
	uint16_t pwrReg;
	uint8_t pwrIdx;

	if (dBm > 15.f) {
		dBm = 15.f;

	} else if (dBm < -20.f) {
		dBm = -10.2f;

	} else if (dBm < -10.f) {
		dBm = -10.1f;
	}

	pwrIdx = (uint8_t) ((dBm + 10.2f) * 10.f + 0.5f);
	pwrReg = g_ax_pwr_ary[pwrIdx];

	/* Set power register value */
	g_ax_spi_packet_buffer[0] = 0xF1;															// WR Address 0x16A: TXPWRCOEFFB
	g_ax_spi_packet_buffer[1] = 0x6A;
	g_ax_spi_packet_buffer[2] = (uint8_t) (pwrReg >> 8);
	g_ax_spi_packet_buffer[3] = (uint8_t) (pwrReg  & 0xff);

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 4);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
}

void spi_ax_setPwrMode(AX_SET_REGISTERS_POWERMODE_t powerState)
{
	/* XOEN and REFEN when oscillator active power modes */
	powerState &= 0x0f;
	if (powerState & 0b1100) {
		powerState |= 0x60;
	}

	/* Prepare packet */
	g_ax_spi_packet_buffer[0] = 0x02 | 0x80;														// WR address 0x02: PWRMODE
	g_ax_spi_packet_buffer[1] = (uint8_t)powerState;

	/* Send packet */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
}

void spi_ax_setRegisters(bool doReset, AX_SET_REGISTERS_MODULATION_t modulation, AX_SET_REGISTERS_VARIANT_t variant, AX_SET_REGISTERS_POWERMODE_t powerState)
{
	static AX_SET_REGISTERS_MODULATION_t s_modulation	= AX_SET_REGISTERS_MODULATION_NONE;
	static AX_SET_REGISTERS_VARIANT_t    s_variant		= AX_SET_REGISTERS_VARIANT_NONE;
	static AX_SET_REGISTERS_POWERMODE_t  s_powerState	= AX_SET_REGISTERS_POWERMODE_NONE;

	/* Allow to reset to unknown state */
	if (modulation == AX_SET_REGISTERS_MODULATION_INVALIDATE) {
		s_modulation = AX_SET_REGISTERS_MODULATION_NONE;
	}
	if (variant == AX_SET_REGISTERS_VARIANT_INVALIDATE) {
		s_variant = AX_SET_REGISTERS_VARIANT_NONE;
	}

	/* Sanity check */
	if (powerState == AX_SET_REGISTERS_POWERMODE_NONE) {
		return;
	}

	/* Entering DEEPSLEEP - all register contents are lost */
	if (powerState == AX_SET_REGISTERS_POWERMODE_DEEPSLEEP) {
		s_modulation	= AX_SET_REGISTERS_MODULATION_NONE;
		s_variant		= AX_SET_REGISTERS_VARIANT_NONE;
		s_powerState	= powerState;
		spi_ax_setPwrMode(powerState);
		return;
	}

	if (doReset) {
		s_modulation	= AX_SET_REGISTERS_MODULATION_NONE;
		s_variant		= AX_SET_REGISTERS_VARIANT_NONE;
		s_powerState	= AX_SET_REGISTERS_POWERMODE_POWERDOWN;
		spi_ax_sync2Powerdown();
	}

	/* Copy when NO_CHANGE */
	{
		if ((modulation == AX_SET_REGISTERS_MODULATION_NO_CHANGE)	&& (s_modulation != AX_SET_REGISTERS_MODULATION_NONE)) {
			modulation = s_modulation;
		}

		if ((variant == AX_SET_REGISTERS_VARIANT_NO_CHANGE)			&& (s_variant != AX_SET_REGISTERS_VARIANT_NONE)) {
			variant = s_variant;
		}
	}

	/* No change, leave */
	if ((s_modulation == modulation) && (s_variant == variant) && (s_powerState == powerState)) {
		return;
	}

	/* Register file modifications */
	if (((modulation != AX_SET_REGISTERS_MODULATION_INVALIDATE) && (s_modulation != modulation)) ||
		((variant    != AX_SET_REGISTERS_VARIANT_INVALIDATE)    && (s_variant    != variant   ))) {

		/* Go to POWERDOWN mode for register file modifications */
		if (s_powerState != AX_SET_REGISTERS_POWERMODE_POWERDOWN) {
			s_powerState  = AX_SET_REGISTERS_POWERMODE_POWERDOWN;
			spi_ax_setPwrMode(s_powerState);
		}

		/* Overwrite with modulation settings */
		switch (modulation) {
			case AX_SET_REGISTERS_MODULATION_FSK:
			{
				if (s_modulation != modulation) {
					s_modulation = AX_SET_REGISTERS_MODULATION_FSK;
					s_variant	 = AX_SET_REGISTERS_VARIANT_RX;
					spi_ax_initRegisters_FSK();
					spi_ax_initRegisters_FSK_Rx();
				}

				/* Overwrite with variant settings */
				if (s_variant != variant) {
					s_variant = variant;

					switch (variant) {
						case AX_SET_REGISTERS_VARIANT_TX:
						{
							spi_ax_initRegisters_FSK_Tx();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX:
						{
							spi_ax_initRegisters_FSK_Rx();
						}
						break;

						default:
						{
							/* Nothing to be set */
						}
					}  // switch (variant)
				}  // if (s_variant != variant)
			}  // case AX_SET_REGISTERS_MODULATION_FSK
			break;

			case AX_SET_REGISTERS_MODULATION_PR1200:
			{
				if (s_modulation != modulation) {
					s_modulation = AX_SET_REGISTERS_MODULATION_PR1200;
					s_variant	 = AX_SET_REGISTERS_VARIANT_RX_CONT;
					spi_ax_initRegisters_PR1200();
					spi_ax_initRegisters_PR1200_Rx();
					spi_ax_initRegisters_PR1200_Rx_cont();
				}

				/* Overwrite with variant settings */
				if (s_variant != variant) {
					s_variant = variant;

					switch (variant) {
						case AX_SET_REGISTERS_VARIANT_TX:
						{
							spi_ax_initRegisters_PR1200_Tx();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX_WOR:
						{
							spi_ax_initRegisters_PR1200_Rx();
							spi_ax_initRegisters_PR1200_Rx_WoR();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX_CONT:
						case AX_SET_REGISTERS_VARIANT_RX:
						{
							spi_ax_initRegisters_PR1200_Rx();
							spi_ax_initRegisters_PR1200_Rx_cont();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX_CONT_SINGLEPARAMSET:
						{
							spi_ax_initRegisters_PR1200_Rx();
							spi_ax_initRegisters_PR1200_Rx_cont_SingleParamSet();
						}
						break;

						default:
						{
							/* Nothing to be set */
						}
					}  // switch (variant)
				}  // if (s_variant != variant)
			}  // case AX_SET_REGISTERS_MODULATION_PR1200
			break;

			case AX_SET_REGISTERS_MODULATION_POCSAG:
			{
				if (s_modulation != modulation) {
					s_modulation = AX_SET_REGISTERS_MODULATION_POCSAG;
					s_variant	 = AX_SET_REGISTERS_VARIANT_RX_CONT;
					spi_ax_initRegisters_POCSAG();
					spi_ax_initRegisters_POCSAG_Rx();
					spi_ax_initRegisters_POCSAG_Rx_cont();
				}

				/* Overwrite with variant settings */
				if (s_variant != variant) {
					s_variant = variant;

					switch (variant) {
						case AX_SET_REGISTERS_VARIANT_TX:
						{
							spi_ax_initRegisters_POCSAG_Tx();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX_WOR:
						{
							spi_ax_initRegisters_POCSAG_Rx();
							spi_ax_initRegisters_POCSAG_Rx_WoR();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX_CONT:
						case AX_SET_REGISTERS_VARIANT_RX:
						{
							spi_ax_initRegisters_POCSAG_Rx();
							spi_ax_initRegisters_POCSAG_Rx_cont();
						}
						break;

						#if 0
						case AX_SET_REGISTERS_VARIANT_RX_CONT_SINGLEPARAMSET:
						{
							spi_ax_initRegisters_POCSAG_Rx();
							spi_ax_initRegisters_POCSAG_Rx_cont_SingleParamSet();
						}
						break;
						#endif

						default:
						{
							/* Nothing to be set */
						}
					}  // switch (variant)
				}  // if (s_variant != variant)
			}
			break;

			case AX_SET_REGISTERS_MODULATION_ANALOG_FM:
			{
				if (s_modulation != modulation) {
					s_modulation = AX_SET_REGISTERS_MODULATION_ANALOG_FM;
					s_variant	 = AX_SET_REGISTERS_VARIANT_RX;
					spi_ax_initRegisters_AnlogFM();
					spi_ax_initRegisters_AnlogFM_Rx();
				}

				/* Overwrite with variant settings */
				if (s_variant != variant) {
					s_variant = variant;

					switch (variant) {
						case AX_SET_REGISTERS_VARIANT_TX:
						{
							spi_ax_initRegisters_AnlogFM_Tx();
						}
						break;

						case AX_SET_REGISTERS_VARIANT_RX:
						case AX_SET_REGISTERS_VARIANT_RX_CONT:
						case AX_SET_REGISTERS_VARIANT_RX_WOR:
						case AX_SET_REGISTERS_VARIANT_RX_CONT_SINGLEPARAMSET:
						{
							spi_ax_initRegisters_AnlogFM_Rx();
						}
						break;

						default:
						{
							/* Nothing to be set */
						}
					}  // switch (variant)
				}  // if (s_variant != variant)
			}
			break;

			default:
			{
				/* Nothing to be set */
			}
		}  // switch (modulation)
	}


	/* Finally enter desired powerState */
	switch (powerState) {
		case AX_SET_REGISTERS_POWERMODE_WOR:
		case AX_SET_REGISTERS_POWERMODE_DEEPSLEEP:
		case AX_SET_REGISTERS_POWERMODE_POWERDOWN:
		{
			s_powerState = powerState;
			spi_ax_setPwrMode(powerState);
			return;
		}
		break;

		case AX_SET_REGISTERS_POWERMODE_FIFO:
		case AX_SET_REGISTERS_POWERMODE_STANDBY:
		{
			s_powerState = powerState;
			spi_ax_setPwrMode(s_powerState);
			s_spi_ax_xtal_waitReady();  // T_xtal
			return;
		}
		break;

		case AX_SET_REGISTERS_POWERMODE_SYNTHRX:
		{
			switch (s_powerState) {
				case AX_SET_REGISTERS_POWERMODE_POWERDOWN:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					s_spi_ax_xtal_waitReady();  // T_xtal

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHRX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHRX:
				{
					// unused
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_FULLRX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHRX;
					spi_ax_setPwrMode(s_powerState);
					// ?
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHTX:
				case AX_SET_REGISTERS_POWERMODE_FULLTX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					// ?

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHRX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth
				}
				break;

				default: { }
			}  // switch (s_powerState)
		}
		break;

		case AX_SET_REGISTERS_POWERMODE_FULLRX:
		{
			switch (s_powerState) {
				case AX_SET_REGISTERS_POWERMODE_POWERDOWN:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					s_spi_ax_xtal_waitReady();  // T_xtal

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHRX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth

					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLRX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(5);  // T_rx_rssi
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHRX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLRX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(5);  // T_rx_rssi
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_FULLRX:
				{
					// unused
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHTX:
				case AX_SET_REGISTERS_POWERMODE_FULLTX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					// ?

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHRX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth

					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLRX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(5);  // T_rx_rssi
				}
				break;

				default: { }
			}  // switch (s_powerState)
		}
		break;

		case AX_SET_REGISTERS_POWERMODE_SYNTHTX:
		{
			switch (s_powerState) {
				case AX_SET_REGISTERS_POWERMODE_POWERDOWN:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					s_spi_ax_xtal_waitReady();  // T_xtal

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHTX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHRX:
				case AX_SET_REGISTERS_POWERMODE_FULLRX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					// ?

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHTX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHTX:
				{
					// unused
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_FULLTX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLTX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(1);  // T_tx_on
				}
				break;

				default: { }
			}  // switch (s_powerState)
		}
		break;

		case AX_SET_REGISTERS_POWERMODE_FULLTX:
		{
			switch (s_powerState) {
				case AX_SET_REGISTERS_POWERMODE_POWERDOWN:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					s_spi_ax_xtal_waitReady();  // T_xtal

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHTX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth

					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLTX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(1);  // T_tx_on
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHRX:
				case AX_SET_REGISTERS_POWERMODE_FULLRX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_STANDBY;
					spi_ax_setPwrMode(s_powerState);
					// ?

					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHTX;
					spi_ax_setPwrMode(s_powerState);
					delay_us(40);  // T_synth

					s_powerState = AX_SET_REGISTERS_POWERMODE_FULLTX;
					spi_ax_setPwrMode(s_powerState);
					delay_ms(1);  // T_tx_on
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_SYNTHTX:
				{
					s_powerState = AX_SET_REGISTERS_POWERMODE_SYNTHTX;
					spi_ax_setPwrMode(s_powerState);
					// ?
				}
				break;

				case AX_SET_REGISTERS_POWERMODE_FULLTX:
				{
					// unused
				}
				break;

				default: { }
			}  // switch (s_powerState)
		}
		break;

		default: { }
	}  // switch (powerState)
}


uint32_t spi_ax_calcFrequency_Mhz2Regs(float f_mhz)
{
	const float xtal_hz = 16E+6f;																// XTAL = 16 MHz
	const float reg_per_mhz = (1LL << 24) * 1E+6f / xtal_hz;
	return (uint32_t) (0.5f + f_mhz * reg_per_mhz);
}

float spi_ax_calcFrequency_Regs2MHz(uint32_t vco_regval)
{
	const float xtal_hz = 16E+6f;																// XTAL = 16 MHz
	const float reg_per_mhz = (1LL << 24) * 1E+6f / xtal_hz;
	return vco_regval / reg_per_mhz;
}

void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB)
{
	uint32_t f_reg;

	if (chan > 2) {
		/* ERROR */
		return;
	}
	f_reg = g_ax_spi_freq_chan[chan];

	/* Make range value non-valid again */
	g_ax_spi_range_chan[chan] = C_SPI_AX_RANGE_NOT_SET;

	/* Prepare packet */
	g_ax_spi_packet_buffer[0] = (isFreqB ?  0x3C : 0x34) | 0x80;								// WR Address 0x34 or 0x3C
	g_ax_spi_packet_buffer[1] = (uint8_t) (f_reg >> 24) & 0xff;
	g_ax_spi_packet_buffer[2] = (uint8_t) (f_reg >> 16) & 0xff;
	g_ax_spi_packet_buffer[3] = (uint8_t) (f_reg >>  8) & 0xff;
	g_ax_spi_packet_buffer[4] = (uint8_t) (f_reg      ) & 0xff;

	/* Send packet */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 5);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
}

void spi_ax_doRanging(void)
{
	static uint32_t s_ax_spi_freq_chan[2] = { 0x28, 0x28 };

	if ((0x00 <= g_ax_spi_range_chan[0]) && (g_ax_spi_range_chan[0] <= 0x0f)  &&  (s_ax_spi_freq_chan[0] == g_ax_spi_freq_chan[0])  &&
	    (0x00 <= g_ax_spi_range_chan[1]) && (g_ax_spi_range_chan[1] <= 0x0f)  &&  (s_ax_spi_freq_chan[1] == g_ax_spi_freq_chan[1])) {

		spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

		/* Recall the ranging values for both frequencies FREQA and FREQB */
		for (uint8_t idx = 0; idx < 2; idx++) {
			uint8_t regAdr = idx ?  0x3b : 0x33;												// WR Address 0x33: PLLRANGINGA  or  Address 0x3b: PLLRANGINGB
			uint8_t val		= g_ax_spi_range_chan[idx];

			g_ax_spi_packet_buffer[0] = regAdr | 0x80;
			g_ax_spi_packet_buffer[1] = val  & 0x0f;
			spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
			spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
			spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
		}

	} else {
		/* Do automatic ranging */

		/* POWERMODE STANDBY */
		spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_STANDBY);

		/* MODULATION */
		spi_ax_transport(false, "< 90 08 >");													// WR address 0x10: MODULATION - 08: FSK

		/* Default 100kHz loop BW for ranging */
		/* PLLLOOP */
		spi_ax_transport(false, "< b0 09 >");													// WR address 0x30: PLLLOOP - DIRECT: Bypass External Filter Pin, FLT: Internal Loop Filter x1 --> BW = 100 kHz for I_CP = 68 �A

		/* PLLCPI */
		spi_ax_transport(false, "< b1 08 >");													// WR address 0x31: PLLCPI - PLLCPI: Charge pump current in multiples of 8.5 �A --> I_CP = 68 �A

		/* FSKDEV */
		spi_ax_transport(false, "< f1 61 00 00 00 >");											// WR address 0x161: FSKDEV - off

		for (uint8_t idx = 0; idx < 2; idx++) {
			if ((0x10 <= g_ax_spi_range_chan[idx]) || (s_ax_spi_freq_chan[idx] != g_ax_spi_freq_chan[idx]))
			{
				uint8_t regAdr = idx ?  0x3b : 0x33;											// WR Address 0x33: PLLRANGINGA  or  Address 0x3b: PLLRANGINGB

				/* Switch to VCO1 or VCO2 as needed */
				(void) spi_ax_vco_select(g_ax_spi_freq_chan[idx], true);

				/* Command message */
				g_ax_spi_packet_buffer[0]	= regAdr | 0x80;
				g_ax_spi_packet_buffer[1]	= 0x18;												// Bit 4: RNGSTART, ranging start value = 8
				spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

				/* Wait until ranging has ended */
				do {
					/* Request message */
					g_ax_spi_packet_buffer[0]	= regAdr;
					spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
					spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
					spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
					spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
				} while (g_ax_spi_packet_buffer[0] & _BV(4));									// Bit 4: RNGSTATUS

				/* Ranging value to be stored - when error occurs store that result, also */
				g_ax_spi_range_chan[idx]	= g_ax_spi_packet_buffer[0];
				s_ax_spi_freq_chan[idx]		= g_ax_spi_freq_chan[idx];

				/* When ranging has failed set default ranging value to keep VCO stable */
				if (g_ax_spi_packet_buffer[0] & _BV(5)) {
					/* Command message */
					g_ax_spi_packet_buffer[0]	= regAdr | 0x80;
					g_ax_spi_packet_buffer[1]	= 0x08;											// Ranging default value = 8
					spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
					spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
					spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
				}
			}
		}

		/* VCOI Manual calibration when GPADC13 is attached to the VCO control voltage, available at FILT in external loop filter mode */
		#if defined(AX_GPADC13_ENABLED)
		{
			spi_ax_initRegisters_PR1200_Tx();

			/* MODULATION */
			spi_ax_transport(false, "< 90 08 >");												// WR address 0x10: MODULATION - 08: FSK

			/* FSKDEV */
			spi_ax_transport(false, "< f1 61 00 00 00 >");										// WR address 0x161: FSKDEV - off

			/* PLLLOOP */
			{
				spi_ax_transport(false, "< 30 R1 >");											// RD address 0x38: PLLLOOP - 04: FILTEN
				g_ax_spi_packet_buffer[1] = 0x04 | g_ax_spi_packet_buffer[0];
				g_ax_spi_packet_buffer[0] = 0x30 | 0x80;

				spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);							// Write back with modified value
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
			}

			/* 0xF35 */
			{
				spi_ax_transport(false, "< 7f 35 R1 >");										// RD address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)
				uint8_t new_0xF35 = 0x80 | g_ax_spi_packet_buffer[0];

				if (0x02 & (uint8_t)~new_0xF35) {
					++new_0xF35;
				}

				g_ax_spi_packet_buffer[0] = 0xFF;
				g_ax_spi_packet_buffer[1] = 0x35;
				g_ax_spi_packet_buffer[2] = new_0xF35;

				spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);							// Write back with modified value
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
			}

			spi_ax_transport(false, "< 82 6c >");												// WR Address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=SYNTH_TX

			{
				/* PLLVCOI */
				spi_ax_transport(false, "< 71 80 R1 >");										// RD address 0x180: PLLVCOI
				uint8_t vcoi_save = g_ax_spi_packet_buffer[0];

				uint8_t dropFirstCalIdx = 2;
				for (uint8_t chanIdx = 0; chanIdx < 2; chanIdx++) {
					g_ax_spi_vcoi_chan[chanIdx] = 0;

					if (g_ax_spi_range_chan[chanIdx] & 0x20) {
						continue;
					}

					g_ax_spi_packet_buffer[0] = 0x33 | 0x80;									// WR Address 0x33: PLLRANGINGA
					g_ax_spi_packet_buffer[1] = g_ax_spi_range_chan[0] & 0x0f;

					spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
					spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
					spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

					spi_ax_setFrequency2Regs(chanIdx, chanIdx == 1 ?  true : false);

					do {
						g_ax_spi_vcoi_chan[chanIdx] = s_spi_ax_cal_vcoi();
					} while (--dropFirstCalIdx);
					dropFirstCalIdx = 1;
				}

				/* Revert to initial setting */
				g_ax_spi_packet_buffer[0] = 0x71 | 0x80;										// WR Address 0x180: PLLVCOI
				g_ax_spi_packet_buffer[1] = 0x80;
				g_ax_spi_packet_buffer[2] = vcoi_save;

				spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
			}
		}
		#endif  // VCOI Calibration

		/* POWERMODE POWERDOWN */
		spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_INVALIDATE, AX_SET_REGISTERS_VARIANT_INVALIDATE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);
	}
}

bool spi_ax_vco_select(uint32_t reg_freq, bool force)
{
	float f_Mhz = spi_ax_calcFrequency_Regs2MHz(reg_freq);
	bool curVco2;
	bool modified = false;

	/* Read VCO settings */
	{
		/* PLLVCODIV */
		spi_ax_transport(false, "< 32 R1 >");													// RD address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV
		curVco2 = (g_ax_spi_packet_buffer[0] & 0x20) ?  true : false;
	}

	/* VCO1 ranges abt. 380 MHz ... 540 MHz (RFDIV=1)  and  760 MHz ... 1080 MHz (RFDIV=0) */
	if (f_Mhz >= 760.f) {
		modified = true;

		/* PLLVCODIV - mind you: check 0xF34, also */
		spi_ax_transport(false, "< b2 00 >");													// WR address 0x32: PLLVCODIV - no RFDIV

		/* 0xF34 */
		spi_ax_transport(false, "< ff 34 08 >");												// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise
	}

	else if (f_Mhz >= 380.f) {
		/* Select VCO1 */
		if (curVco2 || force) {
			modified = true;

			/* PLLVCODIV - mind you: check 0xF34, also */
			spi_ax_transport(false, "< b2 04 >");												// WR address 0x32: PLLVCODIV - RFDIV

			/* 0xF34 */
			spi_ax_transport(false, "< ff 34 28 >");											// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise
		}
	}

	else {
		/* Select VCO2 */
		if (!curVco2 || force) {
			modified = true;

			/* PLLVCODIV - mind you: check 0xF34, also */
			spi_ax_transport(false, "< b2 34 >");												// WR address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV

			/* 0xF34 */
			spi_ax_transport(false, "< ff 34 28 >");											// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise
		}
	}

	return modified;
}

bool spi_ax_selectVcoFreq(bool isFreqB)
{
	/* Switch to VCO1 or VCO2 as needed */
	bool modified = spi_ax_vco_select(g_ax_spi_freq_chan[isFreqB ?  1 : 0], false);

	/* PLLLOOP */
	spi_ax_transport(false, "< 30 R1 >");														// RD address 0x30: PLLLOOP
	g_ax_spi_packet_buffer[1] = (isFreqB ?  (g_ax_spi_packet_buffer[0] | 0x80)					// Set   FREQB
										 :  (g_ax_spi_packet_buffer[0] & 0x7f));				// Clear FREQB
	g_ax_spi_packet_buffer[0] = 0x30 | 0x80;													// WR address 0x30: PLLLOOP

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	return modified;
}

void spi_ax_util_FIFO_waitFree(uint8_t neededSpace)
{
	uint16_t fifoFree;

	do {
		/* FIFOFREE */
		spi_ax_transport(false, "< 2c R2 >");											// RD address 0x2C: FIFOFREE
		fifoFree = 0x1ff & (((uint16_t)g_ax_spi_packet_buffer[0] << 8) | g_ax_spi_packet_buffer[1]);
	} while (fifoFree < neededSpace);
}


void spi_ax_initRegisters_FSK(void)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 08 >");														// WR address 0x10: MODULATION - 08: FSK

	/* ENCODING */
	spi_ax_transport(false, "< 91 00 >");														// WR address 0x11: ENCODING - normal

	/* FRAMING */
	spi_ax_transport(false, "< 92 00 >");														// WR address 0x12: FRAMING - off

	/* PINFUNCTCXO_EN */
	#if 0
	spi_ax_transport(false, "< a6 05 >");														// WR address 0x26: PINFUNCTCXO_EN - Use TCXO_EN pin as DAC output
	#else
	spi_ax_transport(false, "< a6 00 >");														// WR address 0x26: PINFUNCTCXO_EN - Set to output '0'
	#endif

	/* WAKEUPXOEARLY */
	spi_ax_transport(false, "< ee 01 >");														// WR address 0x6E: WAKEUPXOEARLY - 1 LPOSC cycle wake-up time before receiver is started


	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 00 cd >");													// WR address 0x100: IFFREQ - IFFREQ: 3,128 Hz (f_xtal = 16 MHz)

	/* DECIMATION */
	spi_ax_transport(false, "< f1 02 25 >");													// WR address 0x102: DECIMATION - DECIMATION: 37d, f_BASEBAND = 27,027.03 Hz

	/* RXDATARATE */
	spi_ax_transport(false, "< f1 03 00 b4 2e >");												// WR address 0x103: RXDATARATE - 1,200 bit/s

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET - off

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 07 5f >");												// WR address 0x109: MAXRFOFFSET - FREQOFFSCORR: Correct frequency offset at the first LO if this bit is one, MAXRFOFFSET: +/- 1,799.6 Hz

	/* FSKDMAX */
	spi_ax_transport(false, "< f1 0c 03 f3 >");													// WR address 0x10C: FSKDMAX - FSKDMAX: +1,011d (should be +640d?, seems to be moved by abt. 1.5x up)

	/* FSKDMIN */
	spi_ax_transport(false, "< f1 0e ff 0d >");													// WR address 0x10E: FSKDMIN - FSKDMIN: -243d (should be -640d?, seems to be moved by abt. 1.5x up)

	/* AMPLFILTER */
	spi_ax_transport(false, "< f1 15 00 >");													// WR address 0x115: AMPLFILTER - AMPLFILTER: filter bypassed.

	#if 0
	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 00 >");													// WR address 0x116: FREQUENCYLEAK
	#endif

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 f4 >");													// WR address 0x117: RXPARAMSETS - RXPS0: 0, RXPS1: 1, RXPS2: 3, RXPS3: 3

	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0 - AGCATTACK0: 8, AGCDECAY0: 14

	/* AGCTARGET0 */
	spi_ax_transport(false, "< f1 21 84 >");													// WR address 0x121: AGCTARGET0 - average AGC magnitude = 304

	/* TIMEGAIN0 */
	spi_ax_transport(false, "< f1 24 ba >");													// WR address 0x124: TIMEGAIN0

	/* DRGAIN0  0xB4 */
	spi_ax_transport(false, "< f1 25 b4 >");													// WR address 0x125: DRGAIN0

	/* PHASEGAIN0 */
	spi_ax_transport(false, "< f1 26 c3 >");													// WR address 0x126: PHASEGAIN0

	/* FREQGAINA0 */
	spi_ax_transport(false, "< f1 27 0f >");													// WR address 0x127: FREQGAINA0

	/* FREQGAINB0 */
	spi_ax_transport(false, "< f1 28 1f >");													// WR address 0x128: FREQGAINB0

	/* FREQGAINC0 */
	spi_ax_transport(false, "< f1 29 0a >");													// WR address 0x129: FREQGAINC0

	/* FREQGAIND0 */
	spi_ax_transport(false, "< f1 2a 0a >");													// WR address 0x12A: FREQGAIND0

	/* AMPLGAIN0 */
	spi_ax_transport(false, "< f1 2b 06 >");													// WR address 0x12B: AMPLGAIN0

	/* FREQDEV0 */
	spi_ax_transport(false, "< f1 2c 00 00 >");													// WR address 0x12C: FREQDEV0

	/* BBOFFSRES0 */
	spi_ax_transport(false, "< f1 2f 00 >");													// WR address 0x12F: BBOFFSRES0


	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 e8 >");													// WR address 0x130: AGCGAIN1

	/* AGCTARGET1 */
	spi_ax_transport(false, "< f1 31 84 >");													// WR address 0x131: AGCTARGET1

	/* AGCAHYST1 */
	spi_ax_transport(false, "< f1 32 00 >");													// WR address 0x132: AGCAHYST1

	/* AGCMINMAX1 */
	spi_ax_transport(false, "< f1 33 00 >");													// WR address 0x133: AGCMINMAX1

	/* TIMEGAIN1 */
	spi_ax_transport(false, "< f1 34 b8 >");													// WR address 0x134: TIMEGAIN1

	/* DRGAIN1 */
	spi_ax_transport(false, "< f1 35 b3 >");													// WR address 0x135: DRGAIN1

	/* PHASEGAIN1 */
	spi_ax_transport(false, "< f1 36 c3 >");													// WR address 0x136: PHASEGAIN1

	/* FREQGAINA1 */
	spi_ax_transport(false, "< f1 37 0f >");													// WR address 0x137: FREQGAINA1

	/* FREQGAINB1 */
	spi_ax_transport(false, "< f1 38 1f >");													// WR address 0x138: FREQGAINB1

	/* FREQGAINC1 */
	spi_ax_transport(false, "< f1 39 0a >");													// WR address 0x139: FREQGAINC1

	/* FREQGAIND1 */
	spi_ax_transport(false, "< f1 3a 0a >");													// WR address 0x13A: FREQGAIND1

	/* AMPLGAIN1 */
	spi_ax_transport(false, "< f1 3b 06 >");													// WR address 0x13B: AMPLGAIN1

	/* FREQDEV1 */
	spi_ax_transport(false, "< f1 3c 00 4b >");													// WR address 0x13C: FREQDEV1

	/* FOURFSK1 */
	spi_ax_transport(false, "< f1 3e 16 >");													// WR address 0x13E: FOURFSK1

	/* BBOFFSRES1 */
	spi_ax_transport(false, "< f1 3f 00 >");													// WR address 0x13F: BBOFFSRES1


	/* AGCGAIN3 */
	spi_ax_transport(false, "< f1 50 ff >");													// WR address 0x150: AGCGAIN3

	/* AGCTARGET3 */
	spi_ax_transport(false, "< f1 51 84 >");													// WR address 0x151: AGCTARGET3

	/* AGCAHYST3 */
	spi_ax_transport(false, "< f1 52 00 >");													// WR address 0x152: AGCAHYST3

	/* AGCMINMAX3 */
	spi_ax_transport(false, "< f1 53 00 >");													// WR address 0x153: AGCMINMAX3

	/* TIMEGAIN3 */
	spi_ax_transport(false, "< f1 54 b8 >");													// WR address 0x154: TIMEGAIN3

	/* DRGAIN3 */
	spi_ax_transport(false, "< f1 55 b3 >");													// WR address 0x155: DRGAIN3

	/* PHASEGAIN3 */
	spi_ax_transport(false, "< f1 56 c3 >");													// WR address 0x156: PHASEGAIN3

	/* FREQGAINA3 */
	spi_ax_transport(false, "< f1 57 0f >");													// WR address 0x157: FREQGAINA3

	/* FREQGAINB3 */
	spi_ax_transport(false, "< f1 58 1f >");													// WR address 0x158: FREQGAINB3

	/* FREQGAINC3 */
	spi_ax_transport(false, "< f1 59 0d >");													// WR address 0x159: FREQGAINC3

	/* FREQGAIND3 */
	spi_ax_transport(false, "< f1 5a 0d >");													// WR address 0x15A: FREQGAIND3

	/* AMPLGAIN3 */
	spi_ax_transport(false, "< f1 5b 06 >");													// WR address 0x15B: AMPLGAIN3

	/* FREQDEV3 */
	spi_ax_transport(false, "< f1 5c 00 4b >");													// WR address 0x15C: FREQDEV3

	/* FOURFSK3 */
	spi_ax_transport(false, "< f1 5e 16 >");													// WR address 0x15E: FOURFSK3

	/* BBOFFSRES3 */
	spi_ax_transport(false, "< f1 5f 00 >");													// WR address 0x15F: BBOFFSRES3 - not used


	/* MODCFGF */
	spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF - FREQSHAPE: External Loop Filter (BT = 0.0)

	/* FSKDEV */
	spi_ax_transport(false, "< f1 61 00 02 0c >");												// WR address 0x161: FSKDEV - FSKDEV: 500 Hz @ fxtal = 16 MHz.


	/* MODCFGA */
	spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA - AMPLSHAPE, TXDIFF

	/* TXRATE */
	spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: TXRATE - TXRATE: 1,200 bit/s

	/* TXPWRCOEFFA */
	spi_ax_transport(false, "< f1 68 00 00 >");													// WR address 0x168: TXPWRCOEFFA - no correction

	/* TXPWRCOEFFB */
	spi_ax_transport(false, "< f1 6a 00 00 >");													// WR address 0x16A: TXPWRCOEFFB - off

	/* TXPWRCOEFFC */
	spi_ax_transport(false, "< f1 6c 00 00 >");													// WR address 0x16C: TXPWRCOEFFC - no correction

	/* TXPWRCOEFFD */
	spi_ax_transport(false, "< f1 6e 00 00 >");													// WR address 0x16E: TXPWRCOEFFD - no correction

	/* TXPWRCOEFFE */
	spi_ax_transport(false, "< f1 70 00 00 >");													// WR address 0x170: TXPWRCOEFFE - no correction


	/* PLLVCOI */
	spi_ax_transport(false, "< f1 80 99 >");													// WR address 0x180: PLLVCOI - 10 �A * 153 = 1,53 mA

	/* PLLRNGCLK */
	spi_ax_transport(false, "< f1 83 03 >");													// WR address 0x183: PLLRNGCLK - PLLRNGCLK: 7,812 Hz ? (@see Table 148 in AND9347/D)

	/* BBTUNE */
	spi_ax_transport(false, "< f1 88 0f >");													// WR address 0x188: BBTUNE - BBTUNE: 15 (?)

	/* BBOFFSCAP */
	spi_ax_transport(false, "< f1 89 77 >");													// WR address 0x189: BBOFFSCAP - CAPINTB: 7, CAPINTA: 7


	/* TMGTXBOOST */
	spi_ax_transport(false, "< f2 20 32 >");													// WR address 0x220: TMGTXBOOST - TMGTXBOOSTE = 1, TMGTXBOOSTM = 18 --> 36 �s

	/* TMGTXSETTLE */
	spi_ax_transport(false, "< f2 21 14 >");													// WR address 0x221: TMGTXSETTLE - TMGTXSETTLEE = 0, TMGTXSETTLEM = 20 --> 20 �s

	/* TMGRXBOOST */
	spi_ax_transport(false, "< f2 23 32 >");													// WR address 0x223: TMGRXBOOST - TMGRXBOOSTE = 1, TMGRXBOOSTM = 18 --> 36 �s

	/* TMGRXSETTLE */
	spi_ax_transport(false, "< f2 24 14 >");													// WR address 0x224: TMGRXSETTLE - TMGRXSETTLEE = 0, TMGRXSETTLEM = 20 --> 20 �s

	/* TMGRXOFFSACQ */
	spi_ax_transport(false, "< f2 25 00 >");													// WR address 0x225: TMGRXOFFSACQ - TMGRXOFFSACQE = 0, TMGRXOFFSACQM = 0 --> 0 �s

	/* TMGRXCOARSEAGC */
	spi_ax_transport(false, "< f2 26 73 >");													// WR address 0x226: TMGRXCOARSEAGC - TMGRXCOARSEAGCE = 3, TMGRXCOARSEAGCM = 19 --> 152 �s (Bits)  @see PKTMISCFLAGS.RXAGC

	/* TMGRXRSSI */
	spi_ax_transport(false, "< f2 28 03 >");													// WR address 0x228: TMGRXRSSI - TMGRXAGCE = 0, TMGRXAGCM = 3 --> 3 �s (Bits)  @see PKTMISCFLAGS.RXRSSI

	/* TMGRXPREAMBLE1 */
	//spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1 - TMGRXPREAMBLE1 timeout = none

	/* TMGRXPREAMBLE2 */
	spi_ax_transport(false, "< f2 2a 17 >");													// WR address 0x22A: TMGRXPREAMBLE2 - TMGRXPREAMBLE2 timeout = 7 * 2^1 = 14 bits

	/* TMGRXPREAMBLE3 */
	//spi_ax_transport(false, "< f2 2b 00 >");													// WR address 0x22B: TMGRXPREAMBLE3 - TMGRXPREAMBLE3 timeout = none

	/* RSSIREFERENCE */
	spi_ax_transport(false, "< f2 2c f8 >");													// WR address 0x22C: RSSIREFERENCE - RSSI Offset, this register adds a constant offset to the computed RSSI value. It is used to compensate for board effects.

	/* RSSIABSTHR */
	spi_ax_transport(false, "< f2 2d dd >");													// WR address 0x22D: RSSIABSTHR - RSSIABSTHR > -35 (-64) = -99 dBm (BUSY)

	/* BGNDRSSITHR */
	spi_ax_transport(false, "< f2 2f 00 >");													// WR address 0x22F: BGNDRSSITHR - BGNDRSSITHR: off


	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 00 >");													// WR address 0x231: PKTMISCFLAGS - no BGND RSSI, RXAGC CLK, RXRSSI CLK clock sources: 1 �s


	/* LPOSCCONFIG */
	spi_ax_transport(false, "< f3 10 12 >");													// WR address 0x310: LPOSCCONFIG - LPOSC CALIBF 0x10, LPOSC FAST 0x02, (LPOSC ENA 0x01)

	/* LPOSCKFILT */
	spi_ax_transport(false, "< f3 12 01 5d >");													// WR address 0x312: LPOSCKFILT - LPOSCKFILT <= 1,398 (lower value gives lower jitter; 1/4 th taken: 349)

	/* LPOSCREF */
	spi_ax_transport(false, "< f3 14 61 a8 >");													// WR address 0x314: LPOSCREF - LPOSCREF = 25,000

	/* LPOSCFREQ */
	spi_ax_transport(false, "< f3 16 00 00 >");													// WR address 0x316: LPOSCFREQ - no manual tune, done by automatic


	#if 1
	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE - DACSHIFT = 12 bit

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 83 >");													// WR address 0x332: DACCONFIG - DACPWM, DACINPUT=TRKFREQUENCY
	#endif


	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10: XTALOSC

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11: XTALAMPL
}

void spi_ax_initRegisters_FSK_Tx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP - DIRECT 0x08, no FILTEN, FLT 02: Internal Loop Filter x2 (BW = 200 kHz for ICP = 272 mA)

	/* PLLLOOPBOOST */
	spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)
}

void spi_ax_initRegisters_FSK_Rx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP - DIRECT 0x08, no FILTEN, FLT 02: Internal Loop Filter x2 (BW = 200 kHz for ICP = 272 mA)

	/* PLLLOOPBOOST */
	spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)
}

void spi_ax_init_FSK_Tx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* FSK */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(433.9250);						// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x09
		/*
		Radiometrix TXL2/RXL2 - 16kbps bi-phase FSK
		433.925MHz - CHAN0
		433.285MHz - CHAN1
		433.605MHz - CHAN2
		434.245MHz - CHAN3
		434.565MHz - CHAN4
		*/

		/* POCSAG */
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.9875);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_FULLTX);

	#if 1
		/* Set VCO-PLL to FREQA - 433.9250 MHz */
		(void) spi_ax_selectVcoFreq(false);
	#else
		/* Set VCO-PLL to FREQB - 439.9875 MHz */
		(void) spi_ax_selectVcoFreq(true);
	#endif

	/* Set power level */
	spi_ax_setPower_dBm(-20);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}

void spi_ax_init_FSK_Rx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* FSK */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(433.9250);						// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x09
		/*
		Radiometrix TXL2/RXL2 - 16kbps bi-phase FSK
		433.925MHz - CHAN0
		433.285MHz - CHAN1
		433.605MHz - CHAN2
		434.245MHz - CHAN3
		434.565MHz - CHAN4
		*/

		/* POCSAG */
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.9875);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_RX_WOR, AX_SET_REGISTERS_POWERMODE_WOR);

	#if 1
		/* Set VCO-PLL to FREQA - 433.9250 MHz */
		(void) spi_ax_selectVcoFreq(false);
	#else
		/* Set VCO-PLL to FREQB - 439.9875 MHz */
		(void) spi_ax_selectVcoFreq(true);
	#endif

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}


void spi_ax_initRegisters_PR1200(void)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 0a >");														// WR address 0x10: MODULATION - 0A: AFSK

	/* ENCODING */
	spi_ax_transport(false, "< 91 03 >");														// WR address 0x11: ENCODING -  NRZI = ENC DIFF 0x02  &  ENC INV 0x01

	/* FRAMING */
	/* When FRMMODE: "Raw, Soft Bits (0b001 << 1)" is selected, do address 0xF72 also */
	spi_ax_transport(false, "< 92 14 >");														// WR address 0x12: FRAMING - CRCMODE: CCITT (16 bit) 0x10 (right for AX.25), FRMMODE: HDLC [1] 0x04			--> with automatic ending CRC, Flag and 16x '1's, 2xSPACE, off
	//spi_ax_transport(false, "< 92 10 >");														// WR address 0x12: FRAMING - CRCMODE: CCITT (16 bit) 0x10 (right for AX.25), FRMMODE: Raw 0x00					--> no automatic ending Flag, 2xSPACE, off
	//spi_ax_transport(false, "< 92 12 >");														// WR address 0x12: FRAMING - CRCMODE: CCITT (16 bit) 0x10 (right for AX.25), FRMMODE: Raw, Soft Bits 0x02		--> no automatic ending Flag, 2xSPACE, off
	//spi_ax_transport(false, "< 92 16 >");														// WR address 0x12: FRAMING - CRCMODE: CCITT (16 bit) 0x10 (right for AX.25), FRMMODE: Raw, Pattern Match 0x06	--> no automatic ending Flag, 2xSPACE, off
	//spi_ax_transport(false, "< 92 04 >");														// WR address 0x12: FRAMING - CRCMODE: none 0x00, FRMMODE: HDLC [1] 0x04
	//spi_ax_transport(false, "< 92 24 >");														// WR address 0x12: FRAMING - CRCMODE: CRC-16 0x20 (wrong for AX.25), FRMMODE: HDLC [1] 0x04

	/* CRCINIT */
	spi_ax_transport(false, "< 94 ff ff ff ff >");												// WR address 0x14: CRCINIT


	/*WAKEUPTIMER */
	spi_ax_transport(false, "< 68 R2 >");														// RD address 0x68: WAKEUPTIMER
	uint16_t wutNext = ((uint16_t)g_ax_spi_packet_buffer[0] << 8) | g_ax_spi_packet_buffer[1];
	wutNext += 1024;

	g_ax_spi_packet_buffer[0] = 0x6A | 0x80;													// WR address 0x6A: WAKEUP - 100 ms later
	g_ax_spi_packet_buffer[1] = wutNext >> 8;
	g_ax_spi_packet_buffer[2] = wutNext & 0xff;
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* WAKEUPFREQ */
	spi_ax_transport(false, "< ec 04 00 >");													// WR address 0x6C: WAKEUPFREQ - every 100 ms = 1024d


	/* PINFUNCTCXO_EN */
	#if 0
	spi_ax_transport(false, "< a6 05 >");														// WR address 0x26: PINFUNCTCXO_EN - Use TCXO_EN pin as DAC output
	#else
	spi_ax_transport(false, "< a6 00 >");														// WR address 0x26: PINFUNCTCXO_EN - Set to output '0'
	#endif

	/* WAKEUPXOEARLY */
	spi_ax_transport(false, "< ee 01 >");														// WR address 0x6E: WAKEUPXOEARLY - 1 LPOSC cycle wake-up time before receiver is started


	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 00 cd >");													// WR address 0x100: IFFREQ - IFFREQ: 3,128 Hz (f_xtal = 16 MHz)

	/* DECIMATION */
	spi_ax_transport(false, "< f1 02 25 >");													// WR address 0x102: DECIMATION - DECIMATION: 37d, f_BASEBAND = 27,027.03 Hz

	/* RXDATARATE */
	spi_ax_transport(false, "< f1 03 00 b4 2e >");												// WR address 0x103: RXDATARATE - 1,200 bit/s

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET - off

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 04 ea >");												// WR address 0x109: MAXRFOFFSET - FREQOFFSCORR: Correct frequency offset at the first LO 0x80, MAXRFOFFSET: +/- 1,200 Hz

	/* FSKDMAX */
	spi_ax_transport(false, "< f1 0c 03 f3 >");													// WR address 0x10C: FSKDMAX - FSKDMAX: +1,011d (should be +640d?, seems to be moved by abt. 1.5x up)

	/* FSKDMIN */
	spi_ax_transport(false, "< f1 0e ff 0d >");													// WR address 0x10E: FSKDMIN - FSKDMIN: -243d (should be -640d?, seems to be moved by abt. 1.5x up)

	/* AMPLFILTER */
	spi_ax_transport(false, "< f1 15 00 >");													// WR address 0x115: AMPLFILTER - AMPLFILTER: filter bypassed.

	#if 0
	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 00 >");													// WR address 0x116: FREQUENCYLEAK
	#endif

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 f4 >");													// WR address 0x117: RXPARAMSETS - RXPS0: 0, RXPS1: 1, RXPS2: 3, RXPS3: 3

	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0 - AGCATTACK0: 8, AGCDECAY0: 14

	/* AGCTARGET0 */
	spi_ax_transport(false, "< f1 21 84 >");													// WR address 0x121: AGCTARGET0 - average AGC magnitude = 304

	/* TIMEGAIN0 */
	spi_ax_transport(false, "< f1 24 ba >");													// WR address 0x124: TIMEGAIN0

	/* DRGAIN0  0xB4 */
	spi_ax_transport(false, "< f1 25 b4 >");													// WR address 0x125: DRGAIN0

	/* PHASEGAIN0 */
	spi_ax_transport(false, "< f1 26 c3 >");													// WR address 0x126: PHASEGAIN0

	/* FREQGAINA0 */
	spi_ax_transport(false, "< f1 27 0f >");													// WR address 0x127: FREQGAINA0

	/* FREQGAINB0 */
	spi_ax_transport(false, "< f1 28 1f >");													// WR address 0x128: FREQGAINB0

	/* FREQGAINC0 */
	spi_ax_transport(false, "< f1 29 0a >");													// WR address 0x129: FREQGAINC0

	/* FREQGAIND0 */
	spi_ax_transport(false, "< f1 2a 0a >");													// WR address 0x12A: FREQGAIND0

	/* AMPLGAIN0 */
	spi_ax_transport(false, "< f1 2b 06 >");													// WR address 0x12B: AMPLGAIN0

	/* FREQDEV0 */
	spi_ax_transport(false, "< f1 2c 00 00 >");													// WR address 0x12C: FREQDEV0

	/* BBOFFSRES0 */
	spi_ax_transport(false, "< f1 2f 00 >");													// WR address 0x12F: BBOFFSRES0

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 e8 >");													// WR address 0x130: AGCGAIN1

	/* AGCTARGET1 */
	spi_ax_transport(false, "< f1 31 84 >");													// WR address 0x131: AGCTARGET1

	/* AGCAHYST1 */
	spi_ax_transport(false, "< f1 32 00 >");													// WR address 0x132: AGCAHYST1

	/* AGCMINMAX1 */
	spi_ax_transport(false, "< f1 33 00 >");													// WR address 0x133: AGCMINMAX1

	/* TIMEGAIN1 */
	spi_ax_transport(false, "< f1 34 b8 >");													// WR address 0x134: TIMEGAIN1

	/* DRGAIN1 */
	spi_ax_transport(false, "< f1 35 b3 >");													// WR address 0x135: DRGAIN1

	/* PHASEGAIN1 */
	spi_ax_transport(false, "< f1 36 c3 >");													// WR address 0x136: PHASEGAIN1

	/* FREQGAINA1 */
	spi_ax_transport(false, "< f1 37 0f >");													// WR address 0x137: FREQGAINA1

	/* FREQGAINB1 */
	spi_ax_transport(false, "< f1 38 1f >");													// WR address 0x138: FREQGAINB1

	/* FREQGAINC1 */
	spi_ax_transport(false, "< f1 39 0a >");													// WR address 0x139: FREQGAINC1

	/* FREQGAIND1 */
	spi_ax_transport(false, "< f1 3a 0a >");													// WR address 0x13A: FREQGAIND1

	/* AMPLGAIN1 */
	spi_ax_transport(false, "< f1 3b 06 >");													// WR address 0x13B: AMPLGAIN1

	/* FREQDEV1 */
	spi_ax_transport(false, "< f1 3c 00 4b >");													// WR address 0x13C: FREQDEV1

	/* FOURFSK1 */
	spi_ax_transport(false, "< f1 3e 16 >");													// WR address 0x13E: FOURFSK1

	/* BBOFFSRES1 */
	spi_ax_transport(false, "< f1 3f 00 >");													// WR address 0x13F: BBOFFSRES1


	/* AGCGAIN3 */
	spi_ax_transport(false, "< f1 50 ff >");													// WR address 0x150: AGCGAIN3

	/* AGCTARGET3 */
	spi_ax_transport(false, "< f1 51 84 >");													// WR address 0x151: AGCTARGET3

	/* AGCAHYST3 */
	spi_ax_transport(false, "< f1 52 00 >");													// WR address 0x152: AGCAHYST3

	/* AGCMINMAX3 */
	spi_ax_transport(false, "< f1 53 00 >");													// WR address 0x153: AGCMINMAX3

	/* TIMEGAIN3 */
	spi_ax_transport(false, "< f1 54 b8 >");													// WR address 0x154: TIMEGAIN3

	/* DRGAIN3 */
	spi_ax_transport(false, "< f1 55 b3 >");													// WR address 0x155: DRGAIN3

	/* PHASEGAIN3 */
	spi_ax_transport(false, "< f1 56 c3 >");													// WR address 0x156: PHASEGAIN3

	/* FREQGAINA3 */
	spi_ax_transport(false, "< f1 57 0f >");													// WR address 0x157: FREQGAINA3

	/* FREQGAINB3 */
	spi_ax_transport(false, "< f1 58 1f >");													// WR address 0x158: FREQGAINB3

	/* FREQGAINC3 */
	spi_ax_transport(false, "< f1 59 0d >");													// WR address 0x159: FREQGAINC3

	/* FREQGAIND3 */
	spi_ax_transport(false, "< f1 5a 0d >");													// WR address 0x15A: FREQGAIND3

	/* AMPLGAIN3 */
	spi_ax_transport(false, "< f1 5b 06 >");													// WR address 0x15B: AMPLGAIN3

	/* FREQDEV3 */
	spi_ax_transport(false, "< f1 5c 00 4b >");													// WR address 0x15C: FREQDEV3

	/* FOURFSK3 */
	spi_ax_transport(false, "< f1 5e 16 >");													// WR address 0x15E: FOURFSK3

	/* BBOFFSRES3 */
	spi_ax_transport(false, "< f1 5f 00 >");													// WR address 0x15F: BBOFFSRES3 - not used

	/* MODCFGF */
	spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF - FREQSHAPE: External Loop Filter (BT = 0.0)

	/* FSKDEV */
	spi_ax_transport(false, "< f1 61 00 02 0c >");												// WR address 0x161: FSKDEV - FSKDEV: 500 Hz @ fxtal = 16 MHz.


	/* MODCFGA */
	spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA - AMPLSHAPE, TXDIFF

	/* TXRATE */
	spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: TXRATE - TXRATE: 1,200 bit/s

	/* TXPWRCOEFFA */
	spi_ax_transport(false, "< f1 68 00 00 >");													// WR address 0x168: TXPWRCOEFFA - no correction

	/* TXPWRCOEFFB */
	spi_ax_transport(false, "< f1 6a 00 00 >");													// WR address 0x16A: TXPWRCOEFFB - off

	/* TXPWRCOEFFC */
	spi_ax_transport(false, "< f1 6c 00 00 >");													// WR address 0x16C: TXPWRCOEFFC - no correction

	/* TXPWRCOEFFD */
	spi_ax_transport(false, "< f1 6e 00 00 >");													// WR address 0x16E: TXPWRCOEFFD - no correction

	/* TXPWRCOEFFE */
	spi_ax_transport(false, "< f1 70 00 00 >");													// WR address 0x170: TXPWRCOEFFE - no correction


	/* PLLVCOI */
	spi_ax_transport(false, "< f1 80 99 >");													// WR address 0x180: PLLVCOI - 10 �A * 153 = 1,53 mA

	/* PLLRNGCLK */
	spi_ax_transport(false, "< f1 83 03 >");													// WR address 0x183: PLLRNGCLK - PLLRNGCLK: 7,812 Hz ? (@see Table 148 in AND9347/D)

	/* BBTUNE */
	spi_ax_transport(false, "< f1 88 0f >");													// WR address 0x188: BBTUNE - BBTUNE: 15 (?)

	/* BBOFFSCAP */
	spi_ax_transport(false, "< f1 89 77 >");													// WR address 0x189: BBOFFSCAP - CAPINTB: 7, CAPINTA: 7


	/* PKTADDRCFG */
	spi_ax_transport(false, "< f2 00 20 >");													// WR address 0x200: PKTADDRCFG - !MSB_FIRST, !CRC_SKIP_FIRST, FEC SYNC DIS = 0x20, ADDR POS = 0x00

	/* PKTLENCFG */
	spi_ax_transport(false, "< f2 01 00 >");													// WR address 0x201: PKTLENCFG - none

	/* PKTLENOFFSET */
	spi_ax_transport(false, "< f2 02 08 >");													// WR address 0x202: PKTLENOFFSET - receiver_length = Length-Field +8 bytes, in case Length-Field is sent. (Remarks by DF4IAH: reason unknown)

	/* PKTMAXLEN */
	spi_ax_transport(false, "< f2 03 ff >");													// WR address 0x203: PKTMAXLEN - PKTMAXLEN = 255


	/* MATCH0PAT */
	spi_ax_transport(false, "< f2 10 aa cc aa cc >");											// WR address 0x210: MATCH0PAT - not in use
	//spi_ax_transport(false, "< f2 10 7e 00 00 00 >");											// WR address 0x210: MATCH0PAT - flag = 0x7E

	/* MATCH0LEN */
	spi_ax_transport(false, "< f2 14 00 >");													// WR address 0x214: MATCH0LEN - not in use
	//spi_ax_transport(false, "< f2 14 87 >");													// WR address 0x214: MATCH0LEN - MATCH0LEN = 8, MATCH0RAW: Select whether Match Unit 0 operates on decoded (after Manchester, Descrambler etc.) (if 0), or on raw received bits (if 1)

	/* MATCH0MIN */
	spi_ax_transport(false, "< f2 15 00 >");													// WR address 0x215: MATCH0MIN - not in use

	/* MATCH0MAX */
	spi_ax_transport(false, "< f2 16 1f >");													// WR address 0x216: MATCH0MAX - not in use
	//spi_ax_transport(false, "< f2 16 07 >");													// WR address 0x216: MATCH0MAX - MATCH0MAX = 7

	/* MATCH1PAT */
	spi_ax_transport(false, "< f2 18 7e 7e >");													// WR address 0x218: MATCH1PAT - MATCH1PAT = 0x7E 0x7E

	/* MATCH1LEN */
	spi_ax_transport(false, "< f2 1c 8a >");													// WR address 0x21C: MATCH1LEN - MATCH1LEN = 11, MATCH1RAW

	/* MATCH1MAX */
	spi_ax_transport(false, "< f2 1e 0a >");													// WR address 0x21E: MATCH1MAX - MATCH1MAX = 10


	/* TMGTXBOOST */
	spi_ax_transport(false, "< f2 20 32 >");													// WR address 0x220: TMGTXBOOST - TMGTXBOOSTE = 1, TMGTXBOOSTM = 18 --> 36 �s

	/* TMGTXSETTLE */
	spi_ax_transport(false, "< f2 21 14 >");													// WR address 0x221: TMGTXSETTLE - TMGTXSETTLEE = 0, TMGTXSETTLEM = 20 --> 20 �s

	/* TMGRXBOOST */
	spi_ax_transport(false, "< f2 23 32 >");													// WR address 0x223: TMGRXBOOST - TMGRXBOOSTE = 1, TMGRXBOOSTM = 18 --> 36 �s

	/* TMGRXSETTLE */
	spi_ax_transport(false, "< f2 24 14 >");													// WR address 0x224: TMGRXSETTLE - TMGRXSETTLEE = 0, TMGRXSETTLEM = 20 --> 20 �s

	/* TMGRXOFFSACQ */
	spi_ax_transport(false, "< f2 25 00 >");													// WR address 0x225: TMGRXOFFSACQ - TMGRXOFFSACQE = 0, TMGRXOFFSACQM = 0 --> 0 �s

	/* TMGRXCOARSEAGC */
	spi_ax_transport(false, "< f2 26 73 >");													// WR address 0x226: TMGRXCOARSEAGC - TMGRXCOARSEAGCE = 3, TMGRXCOARSEAGCM = 19 --> 152 �s (Bits)  @see PKTMISCFLAGS.RXAGC

	/* TMGRXRSSI */
	//spi_ax_transport(false, "< f2 28 02 >");													// WR address 0x228: TMGRXRSSI - TMGRXRSSIE = 0, TMGRXRSSIM = 2 --> 2 �s (Bits)  @see PKTMISCFLAGS.RXRSSI (for datarates < 2 kbit possible)
	spi_ax_transport(false, "< f2 28 03 >");													// WR address 0x228: TMGRXRSSI - TMGRXRSSIE = 0, TMGRXRSSIM = 3 --> 3 �s (Bits)  @see PKTMISCFLAGS.RXRSSI (fall-back strategy)

	/* TMGRXPREAMBLE1 */
	//spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1 - TMGRXPREAMBLE1 timeout = none

	/* TMGRXPREAMBLE2 */
	spi_ax_transport(false, "< f2 2a 17 >");													// WR address 0x22A: TMGRXPREAMBLE2 - TMGRXPREAMBLE2 timeout = 7 * 2^1 = 14 bits

	/* TMGRXPREAMBLE3 */
	//spi_ax_transport(false, "< f2 2b 00 >");													// WR address 0x22B: TMGRXPREAMBLE3 - TMGRXPREAMBLE3 timeout = none

	/* RSSIREFERENCE */
	spi_ax_transport(false, "< f2 2c f8 >");													// WR address 0x22C: RSSIREFERENCE - RSSI Offset, this register adds a constant offset to the computed RSSI value. It is used to compensate for board effects.

	/* RSSIABSTHR */
	spi_ax_transport(false, "< f2 2d dd >");													// WR address 0x22D: RSSIABSTHR - RSSIABSTHR >  -36 (-64 offset binary) = -100 dBm (BUSY)
	//spi_ax_transport(false, "< f2 2d 85 >");	// testing function of BGNDRSSITHR

	/* BGNDRSSITHR */
	//spi_ax_transport(false, "< f2 2f 00 >");													// WR address 0x22F: BGNDRSSITHR - off
	//spi_ax_transport(false, "< f2 2f 01 >");													// WR address 0x22F: BGNDRSSITHR - BGNDRSSITHR: 1 dB above the BGNDRSSI
	spi_ax_transport(false, "< f2 2f 30 >");	// testing


	#if 0
	/* PKTMISCFLAGS - handeld by  PR1200_Rx_WoR() and PR1200_Rx_cont() */
	spi_ax_transport(false, "< f2 31 00 >");													// WR address 0x231: PKTMISCFLAGS - no BGND RSSI !0x04, RXAGC CLK, RXRSSI CLK clock sources: 1 �s
	#endif

	/* LPOSCCONFIG */
	spi_ax_transport(false, "< f3 10 13 >");													// WR address 0x310: LPOSCCONFIG - LPOSC CALIBF 0x10, LPOSC FAST 0x02, (LPOSC ENA 0x01)

	/* LPOSCKFILT */
	spi_ax_transport(false, "< f3 12 01 5d >");													// WR address 0x312: LPOSCKFILT - LPOSCKFILT <= 1,398 (lower value gives lower jitter; 1/4 th taken: 349)

	/* LPOSCREF */
	spi_ax_transport(false, "< f3 14 61 a8 >");													// WR address 0x314: LPOSCREF - LPOSCREF = 25,000

	/* LPOSCFREQ */
	spi_ax_transport(false, "< f3 16 00 00 >");													// WR address 0x316: LPOSCFREQ - no manual tune, done by automatic


	/* PKTCHUNKSIZE */
	spi_ax_transport(false, "< f2 30 0d >");													// WR address 0x230: PKTCHUNKSIZE - PKTCHUNKSIZE: 240 bytes

	/* PKTSTOREFLAGS */
	spi_ax_transport(false, "< f2 32 17 >");													// WR address 0x232: PKTSTOREFLAGS - ST RSSI, ST RFOFFS, ST FOFFS, ST TIMER

	/* PKTACCEPTFLAGS */
	spi_ax_transport(false, "< f2 33 28 >");													// WR address 0x233: PKTACCEPTFLAGS - ACCPT LRGP 0x20, not ACCPT SZF !0x10, ACCPT ADDRF 0x08, not ACCPT CRCF !0x04, not ACCPT ABRT !0x02, not ACCPT RESIDUE !0x01
	//spi_ax_transport(false, "< f2 33 2c >");	// Test


	#if 1
	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE - DACSHIFT = 12 bit

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 83 >");													// WR address 0x332: DACCONFIG - DACPWM, DACINPUT=TRKFREQUENCY
	#endif


	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10: XTALOSC

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11: XTALAMPL
}

void spi_ax_initRegisters_PR1200_Tx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP - DIRECT 0x08, no FILTEN, FLT 02: Internal Loop Filter x2 (BW = 200 kHz for ICP = 272 mA)

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

	/* PLLVCODIV - mind you: check 0xF34, also */
	spi_ax_transport(false, "< b2 34 >");														// WR address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV

	/* PLLLOOPBOOST */
	spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)


	/* AFSKSPACE - TX */
	spi_ax_transport(false, "< f1 10 00 24 >");													// WR address 0x110: AFSKSPACE - AFSKSPACE: 2,200 Hz = 36d

	/* AFSKMARK - TX */
	spi_ax_transport(false, "< f1 12 00 14 >");													// WR address 0x112: AFSKMARK - AFSKMARK: 1,200 Hz = 20d

	#if 0
	/* AFSKCTRL - RX (& TX?) */
	spi_ax_transport(false, "< f1 14 06 >");													// WR address 0x114: AFSKCTRL - AFSKSHIFT: 6
	#endif


	/* MODCFGF */
	spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF - FREQSHAPE: External Loop Filter (BT = 0.0)

	/* FSKDEV - AFSK */
	spi_ax_transport(false, "< f1 61 00 08 cc >");												// WR address 0x161: FSKDEV - FSKDEV: +/-2,500 Hz @ fxtal = 16 MHz = 2,252d
	//spi_ax_transport(false, "< f1 61 00 06 af >");											// WR address 0x161: FSKDEV - FSKDEV: +/-1,900 Hz @ fxtal = 16 MHz = 1,711d
	//spi_ax_transport(false, "< f1 61 00 06 28 >");											// WR address 0x161: FSKDEV - FSKDEV: +/-1,750 Hz @ fxtal = 16 MHz = 1,576d
	//spi_ax_transport(false, "< f1 61 00 05 47 >");											// WR address 0x161: FSKDEV - FSKDEV: +/-1,500 Hz @ fxtal = 16 MHz = 1,351d
	//spi_ax_transport(false, "< f1 61 00 04 66 >");											// WR address 0x161: FSKDEV - FSKDEV: +/-1,250 Hz @ fxtal = 16 MHz = 1,126d
	//spi_ax_transport(false, "< f1 61 00 03 85 >");											// WR address 0x161: FSKDEV - FSKDEV: +/-1,000 Hz @ fxtal = 16 MHz =   901d

	/* MODCFGA */
	spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA - AMPLSHAPE, TXDIFF

	/* TXRATE */
	spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: TXRATE - TXRATE: 1,200 bit/s


	/* XTALCAP */
	spi_ax_transport(false, "< f1 84 0c >");													// WR address 0x184: XTALCAP - DF4IAH: adjusted from 0x08 to 0x0c


	/* 0xF00 */
	spi_ax_transport(false, "< ff 00 0f >");													// WR address 0xF00 (RX/TX) - Set to 0x0F

	/* 0xF0C */
	spi_ax_transport(false, "< ff 0c 00 >");													// WR address 0xF0C (RX/TX) - Keep the default 0x00

	/* 0xF0D = REF */
	spi_ax_transport(false, "< ff 0d 03 >");													// WR address 0xF0D: REF (RX/TX) - Set to 0x03

	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10 (RX/TX) - Set to 0x04 if a TCXO is used. If a crystal is used, set to 0x0D if the reference frequency (crystal or TCXO) is more than 43 MHz, or to 0x03 otherwise

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11 (RX/TX) - Set to 0x07 if a crystal is connected to CLK16P/CLK16N, or 0x00 if a TCXO is used

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX)

	/* 0xF1C */
	spi_ax_transport(false, "< ff 1c 07 >");													// WR address 0xF1C (RX/TX) - Set to 0x07

	/* 0xF34 */
	spi_ax_transport(false, "< ff 34 28 >");													// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise

	/* 0xF35 */
	spi_ax_transport(false, "< ff 35 10 >");													// WR address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)

	/* 0xF44 */
	spi_ax_transport(false, "< ff 44 25 >");													// WR address 0xF44 (RX/TX)
}

void spi_ax_initRegisters_PR1200_Rx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP - DIRECT 0x08, no FILTEN, FLT 02: Internal Loop Filter x2 (BW = 200 kHz for ICP = 272 mA)

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

	/* PLLVCODIV - mind you: check 0xF34, also */
	spi_ax_transport(false, "< b2 34 >");														// WR address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV

	/* PLLLOOPBOOST */
	spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)


	/* AFSKSPACE - RX */
	spi_ax_transport(false, "< f1 10 01 4d >");													// WR address 0x110: AFSKSPACE - AFSKSPACE: 2,200 Hz

	/* AFSKMARK - RX */
	spi_ax_transport(false, "< f1 12 00 b6 >");													// WR address 0x112: AFSKMARK - AFSKMARK: 1,200 Hz

	/* AFSKCTRL */
	spi_ax_transport(false, "< f1 14 06 >");													// WR address 0x114: AFSKCTRL - AFSKSHIFT: 6

	/* XTALCAP */
	spi_ax_transport(false, "< f1 84 0c >");													// WR address 0x184: XTALCAP - DF4IAH: adjusted from 0x08 to 0x0c

	/* 0xF00 */
	spi_ax_transport(false, "< ff 00 0f >");													// WR address 0xF00 (RX/TX) - Set to 0x0F

	/* 0xF0C */
	spi_ax_transport(false, "< ff 0c 00 >");													// WR address 0xF0C (RX/TX) - Keep the default 0x00

	/* 0xF0D = REF */
	spi_ax_transport(false, "< ff 0d 03 >");													// WR address 0xF0D: REF (RX/TX) - Set to 0x03

	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10 (RX/TX) - Set to 0x04 if a TCXO is used. If a crystal is used, set to 0x0D if the reference frequency (crystal or TCXO) is more than 43 MHz, or to 0x03 otherwise

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11 (RX/TX) - Set to 0x07 if a crystal is connected to CLK16P/CLK16N, or 0x00 if a TCXO is used

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX)

	/* 0xF1C */
	spi_ax_transport(false, "< ff 1c 07 >");													// WR address 0xF1C (RX/TX) - Set to 0x07

	/* 0xF21 */
	spi_ax_transport(false, "< ff 21 68 >");													// WR address 0xF21 (RX)

	/* 0xF22 */
	spi_ax_transport(false, "< ff 22 ff >");													// WR address 0xF22 (RX)

	/* 0xF23 */
	spi_ax_transport(false, "< ff 23 84 >");													// WR address 0xF23 (RX)

	/* 0xF26 */
	spi_ax_transport(false, "< ff 26 98 >");													// WR address 0xF26 (RX)

	/* 0xF34 */
	spi_ax_transport(false, "< ff 34 28 >");													// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise

	/* 0xF35 */
	spi_ax_transport(false, "< ff 35 10 >");													// WR address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)

	/* 0xF44 */
	spi_ax_transport(false, "< ff 44 25 >");													// WR address 0xF44 (RX/TX)

	/* 0xF72 */
	spi_ax_transport(false, "< ff 72 00 >");													// WR address 0xF72 (RX) - Set to 0x06 if the framing mode is set to �Raw, Soft Bits� (register FRAMING), or to 0x00 otherwise
}

void spi_ax_initRegisters_PR1200_Rx_WoR(void)
{
	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 83 >");													// WR address 0x120: AGCGAIN0 - AGCDECAY0 = 8  (f_3db=621 Hz), AGCATTACK0 = 3  (f_3db=18,660 Hz)

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 83 >");													// WR address 0x130: AGCGAIN1 - AGCDECAY1 = 8  (f_3db=621 Hz), AGCATTACK1 = 3  (f_3db=18,660 Hz)


	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 54 >");													// WR address 0x227: TMGRXAGC - TMGRXAGCE = 5, TMGRXAGCM = 4 --> 128 �s (Bits)  @see PKTMISCFLAGS.RXRSSI

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 34 >");													// WR address 0x229: TMGRXPREAMBLE1 - TMGRXPREAMBLEE = 3, TMGRXPREAMBLEM = 4 --> 32 Bits

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 05 >");													// WR address 0x231: PKTMISCFLAGS - BGND RSSI 0x04, !RXAGC CLK, RXRSSI CLK clock sources: Bit clock
}

void spi_ax_initRegisters_PR1200_Rx_cont(void)
{
	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0 - AGCDECAY0 = 14  (f_3db=10 Hz), AGCATTACK0 = 8  (f_3db=621 Hz)

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 e8 >");													// WR address 0x130: AGCGAIN1 - AGCDECAY1 = 14  (f_3db=10 Hz), AGCATTACK1 = 8  (f_3db=621 Hz)


	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 00 >");													// WR address 0x227: TMGRXAGC - not used

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1 - not used

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 00 >");													// WR address 0x231: PKTMISCFLAGS - no BGND RSSI !0x04, RXAGC CLK, RXRSSI CLK clock sources: 1 �s
}

void spi_ax_initRegisters_PR1200_Rx_cont_SingleParamSet(void)
{
	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 ff >");													// WR address 0x117: RXPARAMSETS

	/* FREQDEV3 */
	spi_ax_transport(false, "< f1 5c 00 >");													// WR address 0x15C: FREQDEV3

	/* AGCGAIN3 */
	spi_ax_transport(false, "< f1 50 e8 >");													// WR address 0x150: AGCGAIN3
}

void spi_ax_init_PR1200_Tx(void)
{
	/* Syncing and sending reset command, then setting the packet radio values for transmission */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* APRS  */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.8000);						// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05

		/* Burst-Aussendungen fuer Steuerungszwecke */
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9250);						// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_PR1200, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_FULLTX);

	#if 1
		/* Set VCO-PLL to FREQA - 144.800 MHz */
		(void) spi_ax_selectVcoFreq(false);
	#else
		/* Set VCO-PLL to FREQB - 144.925 MHz */
		(void) spi_ax_selectVcoFreq(true);
	#endif

	/* Set power level */
	spi_ax_setPower_dBm(-20);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}

void spi_ax_run_PR1200_Tx_FIFO_APRS(const char addrAry[][C_PR1200_CALL_LENGTH], const uint8_t* ssidAry, uint8_t addrCnt, const char* aprsMsg, uint8_t aprsMsgLen)
{
	/* Enter an APRS UI frame */

	/* 1 - Flags */
	spi_ax_util_PR1200_Tx_FIFO_Flags(50);														// 333 ms

	/* 2 - Address field, Control and PID */
	spi_ax_util_PR1200_Tx_FIFO_AddressField(addrAry, ssidAry, addrCnt);

	/* 3 - Information field - APRS data */
	spi_ax_util_PR1200_Tx_FIFO_InformationField(aprsMsg, aprsMsgLen);
}

void spi_ax_util_PR1200_Tx_FIFO_Flags(uint8_t count)
{
	uint16_t idx = 0;

	/* Wait until enough space for next batch is available */
	spi_ax_util_FIFO_waitFree(5 + count);

	g_ax_spi_packet_buffer[idx++] = 0xA9;														// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_REPEATDATA_TX;

	/* Setting RAW to one causes the DATA to bypass the framing mode, but still pass through the encoder */
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_RAW | AX_FIFO_DATA_FLAGS_TX_NOCRC | AX_FIFO_DATA_FLAGS_TX_PKTSTART;

	g_ax_spi_packet_buffer[idx++] = count;
	g_ax_spi_packet_buffer[idx++] = 0b01111110;													// The AX25 'Flag'
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* FIFO do a COMMIT */
	spi_ax_transport(false, "< a8 04 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
}

void spi_ax_util_PR1200_Tx_FIFO_AddressField(const char addrAry[][C_PR1200_CALL_LENGTH], const uint8_t* ssidAry, uint8_t addrCnt)
{
	uint16_t idx = 0;

	/* Sanity check */
	if (addrCnt > 4) {
		return;
	}

	/* Wait until enough space for next batch is available */
	spi_ax_util_FIFO_waitFree(4 + addrCnt * 7 + 2);

	g_ax_spi_packet_buffer[idx++] = 0xA9;														// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_DATA_TX_RX;
	g_ax_spi_packet_buffer[idx++] = 0;															// Dummy entry for now
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_PKTSTART;								// FIFO flag byte

	for (uint8_t addrIdx = 0; addrIdx < addrCnt; addrIdx++) {
		const char* addrStr = &addrAry[addrIdx][0];
		uint8_t ssid = 0x0f & ssidAry[addrIdx];
		uint8_t strLen = strnlen(addrStr, C_PR1200_CALL_LENGTH);

		for (uint8_t addrStrIdx = 0; addrStrIdx < C_PR1200_CALL_LENGTH; addrStrIdx++) {
			uint8_t c = addrStrIdx < strLen ?  toupper((char)*(addrStr + addrStrIdx)) : ' ';
			g_ax_spi_packet_buffer[idx++] = (c << 1)	| 0;									// Address: dest. string
		}

		uint8_t pf = (addrIdx == 1) ?  1 : 0;
		uint8_t addrEnd = (addrIdx == addrCnt - 1) ?  1 : 0;
		uint8_t val = (pf << 7) | (0b11 << 5) | (ssid << 1)	| addrEnd;							// Address: dest. SSID
		g_ax_spi_packet_buffer[idx++] = val;
	}

	g_ax_spi_packet_buffer[idx++] = (0b0 << 4) |  0b11;											// Control: UI frame with no Poll bit set
	g_ax_spi_packet_buffer[idx++] = 0xf0;														// PID

	/* Set length for FIFO DATA command */
	g_ax_spi_packet_buffer[    2] = idx - 3;													// Length

	/* FIFO data enter */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* FIFO do a COMMIT */
	spi_ax_transport(false, "< a8 04 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
}

void spi_ax_util_PR1200_Tx_FIFO_InformationField(const char* aprsMsg, uint8_t aprsMsgLen)
{
	uint16_t idx = 0;

	/* Sanity checks */
	if (!aprsMsg || (aprsMsgLen > 80)) {
		return;
	}

	/* Wait until enough space for next batch is available */
	spi_ax_util_FIFO_waitFree(4 + aprsMsgLen);

	g_ax_spi_packet_buffer[idx++] = 0xA9;														// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_DATA_TX_RX;
	g_ax_spi_packet_buffer[idx++] = 0;															// Dummy entry for now
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_PKTEND;								// FIFO flag byte

	for (uint8_t msgStrIdx = 0; msgStrIdx < aprsMsgLen; msgStrIdx++) {
		g_ax_spi_packet_buffer[idx++] = *(aprsMsg + msgStrIdx);									// Info: APRS data
	}

	/* Set length for FIFO DATA command */
	g_ax_spi_packet_buffer[    2] = idx - 3;													// Length

	/* FIFO data enter */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* FIFO do a COMMIT */
	spi_ax_transport(false, "< a8 04 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
}

void spi_ax_init_PR1200_Rx(void)
{
	/* Syncing and sending reset command, then setting the packet radio values for transmission */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* APRS  */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.8000);						// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05

		/* Burst-Aussendungen fuer Steuerungszwecke */
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9250);						// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_PR1200, AX_SET_REGISTERS_VARIANT_RX_WOR, AX_SET_REGISTERS_POWERMODE_WOR);

	#if 1
		/* Set VCO-PLL to FREQA - 144.800 MHz */
		(void) spi_ax_selectVcoFreq(false);
	#else
		/* Set VCO-PLL to FREQB - 144.925 MHz */
		(void) spi_ax_selectVcoFreq(true);
	#endif

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}


void spi_ax_initRegisters_POCSAG(void)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 08 >");														// WR address 0x10: MODULATION - 08: FSK

	/* ENCODING */
	spi_ax_transport(false, "< 91 01 >");														// WR address 0x11: ENCODING -  ENC INV 0x01

	/* FRAMING */
	/* When FRMMODE: "Raw, Soft Bits (0b001 << 1)" is selected, do address 0xF72 also */
	spi_ax_transport(false, "< 92 06 >");														// WR address 0x12: FRAMING - CRCMODE: none, FRMMODE: Raw, Pattern Match 0x06

	#if 0
	/* CRCINIT */
	spi_ax_transport(false, "< 94 ff ff ff ff >");												// WR address 0x14: CRCINIT
	#endif


	/* PINFUNCSYSCLK */
	spi_ax_transport(false, "< a1 82 >");														// WR address 0x21: PINFUNCSYSCLK - Set to 'Z'

	/* PINFUNCDCLK */
	spi_ax_transport(false, "< a2 82 >");														// WR address 0x22: PINFUNCDCLK - Set to 'Z'

	/* PINFUNCDATA */
	spi_ax_transport(false, "< a3 82 >");														// WR address 0x23: PINFUNCDATA - Set to 'Z'

	/* PINFUNCIRQ */
	spi_ax_transport(false, "< a4 03 >");														// WR address 0x24: PINFUNCIRQ - Set to Interrupt request

	/* PINFUNCANTSEL */
	spi_ax_transport(false, "< a5 05 >");														// WR address 0x25: PINFUNCANTSEL - Set to DAC output

	/* PINFUNCPWRAMP / PINFUNCTCXO_EN */
	spi_ax_transport(false, "< a6 02 >");														// WR address 0x26: PINFUNCTCXO_EN - Set to 'Z'


	// --

	/*WAKEUPTIMER */
	spi_ax_transport(false, "< 68 R2 >");														// RD address 0x68: WAKEUPTIMER
	uint16_t wutNext = ((uint16_t)g_ax_spi_packet_buffer[0] << 8) | g_ax_spi_packet_buffer[1];
	wutNext += 1024;

	g_ax_spi_packet_buffer[0] = 0x6A | 0x80;													// WR address 0x6A: WAKEUP - 100 ms later
	g_ax_spi_packet_buffer[1] = wutNext >> 8;
	g_ax_spi_packet_buffer[2] = wutNext & 0xff;
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* WAKEUPFREQ */
	spi_ax_transport(false, "< ec 04 00 >");													// WR address 0x6C: WAKEUPFREQ - every 100 ms = 1024d

	// --

	/* WAKEUPXOEARLY */
	spi_ax_transport(false, "< ee 01 >");														// WR address 0x6E: WAKEUPXOEARLY - 1 LPOSC cycle wake-up time before receiver is started


	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 02 66 >");													// WR address 0x100: IFFREQ - IFFREQ: 9,369 Hz (f_xtal = 16 MHz)

	/* DECIMATION */
	spi_ax_transport(false, "< f1 02 19 >");													// WR address 0x102: DECIMATION - DECIMATION: 37d, f_BASEBAND = 27,027.03 Hz

	/* RXDATARATE */
	spi_ax_transport(false, "< f1 03 01 0a aa >");												// WR address 0x103: RXDATARATE - 1,200 bit/s

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET - off

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 08 31 >");												// WR address 0x109: MAXRFOFFSET - FREQOFFSCORR: Correct frequency offset at the first LO 0x80, MAXRFOFFSET: +/- 1,200 Hz

	/* FSKDMAX */
	spi_ax_transport(false, "< f1 0c 06 55 >");													// WR address 0x10C: FSKDMAX - FSKDMAX: +1,011d (should be +640d?, seems to be moved by abt. 1.5x up)

	/* FSKDMIN */
	spi_ax_transport(false, "< f1 0e f9 ab >");													// WR address 0x10E: FSKDMIN - FSKDMIN: -243d (should be -640d?, seems to be moved by abt. 1.5x up)

	/* AMPLFILTER */
	spi_ax_transport(false, "< f1 15 00 >");													// WR address 0x115: AMPLFILTER - AMPLFILTER: filter bypassed.

	#if 0
	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 00 >");													// WR address 0x116: FREQUENCYLEAK
	#endif

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 f4 >");													// WR address 0x117: RXPARAMSETS - RXPS0: 0, RXPS1: 1, RXPS2: 3, RXPS3: 3

	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0 - AGCATTACK0: 8, AGCDECAY0: 14

	/* AGCTARGET0 */
	spi_ax_transport(false, "< f1 21 84 >");													// WR address 0x121: AGCTARGET0 - average AGC magnitude = 304

	/* TIMEGAIN0 */
	spi_ax_transport(false, "< f1 24 8b >");													// WR address 0x124: TIMEGAIN0

	/* DRGAIN0  0xB4 */
	spi_ax_transport(false, "< f1 25 85 >");													// WR address 0x125: DRGAIN0

	/* PHASEGAIN0 */
	spi_ax_transport(false, "< f1 26 c3 >");													// WR address 0x126: PHASEGAIN0

	/* FREQGAINA0 */
	spi_ax_transport(false, "< f1 27 0f >");													// WR address 0x127: FREQGAINA0

	/* FREQGAINB0 */
	spi_ax_transport(false, "< f1 28 1f >");													// WR address 0x128: FREQGAINB0

	/* FREQGAINC0 */
	spi_ax_transport(false, "< f1 29 0a >");													// WR address 0x129: FREQGAINC0

	/* FREQGAIND0 */
	spi_ax_transport(false, "< f1 2a 0a >");													// WR address 0x12A: FREQGAIND0

	/* AMPLGAIN0 */
	spi_ax_transport(false, "< f1 2b 06 >");													// WR address 0x12B: AMPLGAIN0

	/* FREQDEV0 */
	spi_ax_transport(false, "< f1 2c 00 00 >");													// WR address 0x12C: FREQDEV0

	/* BBOFFSRES0 */
	spi_ax_transport(false, "< f1 2f 00 >");													// WR address 0x12F: BBOFFSRES0

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 e8 >");													// WR address 0x130: AGCGAIN1

	/* AGCTARGET1 */
	spi_ax_transport(false, "< f1 31 84 >");													// WR address 0x131: AGCTARGET1

	/* AGCAHYST1 */
	spi_ax_transport(false, "< f1 32 00 >");													// WR address 0x132: AGCAHYST1

	/* AGCMINMAX1 */
	spi_ax_transport(false, "< f1 33 00 >");													// WR address 0x133: AGCMINMAX1

	/* TIMEGAIN1 */
	spi_ax_transport(false, "< f1 34 89 >");													// WR address 0x134: TIMEGAIN1

	/* DRGAIN1 */
	spi_ax_transport(false, "< f1 35 84 >");													// WR address 0x135: DRGAIN1

	/* PHASEGAIN1 */
	spi_ax_transport(false, "< f1 36 c3 >");													// WR address 0x136: PHASEGAIN1

	/* FREQGAINA1 */
	spi_ax_transport(false, "< f1 37 0f >");													// WR address 0x137: FREQGAINA1

	/* FREQGAINB1 */
	spi_ax_transport(false, "< f1 38 1f >");													// WR address 0x138: FREQGAINB1

	/* FREQGAINC1 */
	spi_ax_transport(false, "< f1 39 0a >");													// WR address 0x139: FREQGAINC1

	/* FREQGAIND1 */
	spi_ax_transport(false, "< f1 3a 0a >");													// WR address 0x13A: FREQGAIND1

	/* AMPLGAIN1 */
	spi_ax_transport(false, "< f1 3b 06 >");													// WR address 0x13B: AMPLGAIN1

	/* FREQDEV1 */
	spi_ax_transport(false, "< f1 3c 02 55 >");													// WR address 0x13C: FREQDEV1

	/* FOURFSK1 */
	spi_ax_transport(false, "< f1 3e 16 >");													// WR address 0x13E: FOURFSK1

	/* BBOFFSRES1 */
	spi_ax_transport(false, "< f1 3f 00 >");													// WR address 0x13F: BBOFFSRES1


	/* AGCGAIN3 */
	spi_ax_transport(false, "< f1 50 ff >");													// WR address 0x150: AGCGAIN3

	/* AGCTARGET3 */
	spi_ax_transport(false, "< f1 51 84 >");													// WR address 0x151: AGCTARGET3

	/* AGCAHYST3 */
	spi_ax_transport(false, "< f1 52 00 >");													// WR address 0x152: AGCAHYST3

	/* AGCMINMAX3 */
	spi_ax_transport(false, "< f1 53 00 >");													// WR address 0x153: AGCMINMAX3

	/* TIMEGAIN3 */
	spi_ax_transport(false, "< f1 54 88 >");													// WR address 0x154: TIMEGAIN3

	/* DRGAIN3 */
	spi_ax_transport(false, "< f1 55 83 >");													// WR address 0x155: DRGAIN3

	/* PHASEGAIN3 */
	spi_ax_transport(false, "< f1 56 c3 >");													// WR address 0x156: PHASEGAIN3

	/* FREQGAINA3 */
	spi_ax_transport(false, "< f1 57 0f >");													// WR address 0x157: FREQGAINA3

	/* FREQGAINB3 */
	spi_ax_transport(false, "< f1 58 1f >");													// WR address 0x158: FREQGAINB3

	/* FREQGAINC3 */
	spi_ax_transport(false, "< f1 59 0d >");													// WR address 0x159: FREQGAINC3

	/* FREQGAIND3 */
	spi_ax_transport(false, "< f1 5a 0d >");													// WR address 0x15A: FREQGAIND3

	/* AMPLGAIN3 */
	spi_ax_transport(false, "< f1 5b 06 >");													// WR address 0x15B: AMPLGAIN3

	/* FREQDEV3 */
	spi_ax_transport(false, "< f1 5c 02 55 >");													// WR address 0x15C: FREQDEV3

	/* FOURFSK3 */
	spi_ax_transport(false, "< f1 5e 16 >");													// WR address 0x15E: FOURFSK3

	/* BBOFFSRES3 */
	spi_ax_transport(false, "< f1 5f 00 >");													// WR address 0x15F: BBOFFSRES3 - not used

	/* MODCFGF */
	spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF - FREQSHAPE: External Loop Filter (BT = 0.0)

	/* FSKDEV */
	spi_ax_transport(false, "< f1 61 00 10 62 >");												// WR address 0x161: FSKDEV - FSKDEV: +/-2,500 Hz @ fxtal = 16 MHz = 2,252d


	/* MODCFGA */
	spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA - AMPLSHAPE, TXDIFF

	/* TXRATE */
	spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: TXRATE - TXRATE: 1,200 bit/s

	/* TXPWRCOEFFA */
	spi_ax_transport(false, "< f1 68 00 00 >");													// WR address 0x168: TXPWRCOEFFA - no correction

	/* TXPWRCOEFFB */
	spi_ax_transport(false, "< f1 6a 00 aa >");													// WR address 0x16A: TXPWRCOEFFB - -10 dBm

	/* TXPWRCOEFFC */
	spi_ax_transport(false, "< f1 6c 00 00 >");													// WR address 0x16C: TXPWRCOEFFC - no correction

	/* TXPWRCOEFFD */
	spi_ax_transport(false, "< f1 6e 00 00 >");													// WR address 0x16E: TXPWRCOEFFD - no correction

	/* TXPWRCOEFFE */
	spi_ax_transport(false, "< f1 70 00 00 >");													// WR address 0x170: TXPWRCOEFFE - no correction


	/* PLLVCOI */
	spi_ax_transport(false, "< f1 80 99 >");													// WR address 0x180: PLLVCOI - 10 �A * 153 = 1,53 mA

	/* PLLRNGCLK */
	spi_ax_transport(false, "< f1 83 03 >");													// WR address 0x183: PLLRNGCLK - PLLRNGCLK: 7,812 Hz ? (@see Table 148 in AND9347/D)

	/* BBTUNE */
	spi_ax_transport(false, "< f1 88 0f >");													// WR address 0x188: BBTUNE - BBTUNE: 15 (?)

	/* BBOFFSCAP */
	spi_ax_transport(false, "< f1 89 77 >");													// WR address 0x189: BBOFFSCAP - CAPINTB: 7, CAPINTA: 7


	/* PKTADDRCFG */
	spi_ax_transport(false, "< f2 00 80 >");													// WR address 0x200: PKTADDRCFG - MSB_FIRST, !CRC_SKIP_FIRST, FEC SYNC DIS = 0x20, ADDR POS = 0x00

	/* PKTLENCFG */
	spi_ax_transport(false, "< f2 01 00 >");													// WR address 0x201: PKTLENCFG - none

	/* PKTLENOFFSET */
	spi_ax_transport(false, "< f2 02 08 >");													// WR address 0x202: PKTLENOFFSET - receiver_length = Length-Field +8 bytes, in case Length-Field is sent. (Remarks by DF4IAH: reason unknown)

	/* PKTMAXLEN */
	spi_ax_transport(false, "< f2 03 40 >");													// WR address 0x203: PKTMAXLEN - PKTMAXLEN = 64


	/* MATCH0PAT */
	spi_ax_transport(false, "< f2 10 1b a8 4b 3e >");											// WR address 0x210: MATCH0PAT - POCSAG SYNC word (inverted by AX5243)

	/* MATCH0LEN */
	spi_ax_transport(false, "< f2 14 1f >");													// WR address 0x214: MATCH0LEN

	/* MATCH0MIN */
	spi_ax_transport(false, "< f2 15 00 >");													// WR address 0x215: MATCH0MIN - not in use

	/* MATCH0MAX */
	spi_ax_transport(false, "< f2 16 1f >");													// WR address 0x216: MATCH0MAX

	/* MATCH1PAT */
	spi_ax_transport(false, "< f2 18 aa aa >");													// WR address 0x218: MATCH1PAT - MATCH1PAT = 0xAA 0xAA

	/* MATCH1LEN */
	spi_ax_transport(false, "< f2 1c 0a >");													// WR address 0x21C: MATCH1LEN - not MATCH1RAW, MATCH1LEN = 11

	/* MATCH1MAX */
	spi_ax_transport(false, "< f2 1e 0a >");													// WR address 0x21E: MATCH1MAX - MATCH1MAX = 10


	/* TMGTXBOOST */
	spi_ax_transport(false, "< f2 20 32 >");													// WR address 0x220: TMGTXBOOST - TMGTXBOOSTE = 1, TMGTXBOOSTM = 18 --> 36 �s

	/* TMGTXSETTLE */
	spi_ax_transport(false, "< f2 21 14 >");													// WR address 0x221: TMGTXSETTLE - TMGTXSETTLEE = 0, TMGTXSETTLEM = 20 --> 20 �s

	/* TMGRXBOOST */
	spi_ax_transport(false, "< f2 23 32 >");													// WR address 0x223: TMGRXBOOST - TMGRXBOOSTE = 1, TMGRXBOOSTM = 18 --> 36 �s

	/* TMGRXSETTLE */
	spi_ax_transport(false, "< f2 24 14 >");													// WR address 0x224: TMGRXSETTLE - TMGRXSETTLEE = 0, TMGRXSETTLEM = 20 --> 20 �s

	/* TMGRXOFFSACQ */
	spi_ax_transport(false, "< f2 25 00 >");													// WR address 0x225: TMGRXOFFSACQ - TMGRXOFFSACQE = 0, TMGRXOFFSACQM = 0 --> 0 �s

	/* TMGRXCOARSEAGC */
	spi_ax_transport(false, "< f2 26 73 >");													// WR address 0x226: TMGRXCOARSEAGC - TMGRXCOARSEAGCE = 3, TMGRXCOARSEAGCM = 19 --> 152 �s (Bits)  @see PKTMISCFLAGS.RXAGC

	/* TMGRXRSSI */
	spi_ax_transport(false, "< f2 28 03 >");													// WR address 0x228: TMGRXRSSI - TMGRXRSSIE = 0, TMGRXRSSIM = 3 --> 3 �s (Bits)  @see PKTMISCFLAGS.RXRSSI (fall-back strategy)

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1 - TMGRXPREAMBLE1 timeout = none

	/* TMGRXPREAMBLE2 */
	spi_ax_transport(false, "< f2 2a 35 >");													// WR address 0x22A: TMGRXPREAMBLE2 - TMGRXPREAMBLE2 timeout = 7 * 2^1 = 14 bits

	/* TMGRXPREAMBLE3 */
	spi_ax_transport(false, "< f2 2b 00 >");													// WR address 0x22B: TMGRXPREAMBLE3 - TMGRXPREAMBLE3 timeout = none

	/* RSSIREFERENCE */
	spi_ax_transport(false, "< f2 2c f6 >");													// WR address 0x22C: RSSIREFERENCE - RSSI Offset, this register adds a constant offset to the computed RSSI value. It is used to compensate for board effects.

	/* RSSIABSTHR */
	spi_ax_transport(false, "< f2 2d dd >");													// WR address 0x22D: RSSIABSTHR - RSSIABSTHR >  -36 (-64 offset binary) = -100 dBm (BUSY)

	/* BGNDRSSITHR */
	spi_ax_transport(false, "< f2 2f 00 >");													// WR address 0x22F: BGNDRSSITHR - off


	/* PKTCHUNKSIZE */
	spi_ax_transport(false, "< f2 30 f0 >");													// WR address 0x230: PKTCHUNKSIZE - PKTCHUNKSIZE: 240 bytes   Try this
	//spi_ax_transport(false, "< f2 30 0d >");													// WR address 0x230: PKTCHUNKSIZE - PKTCHUNKSIZE:  13 bytes   ???

	/* PKTSTOREFLAGS */
	spi_ax_transport(false, "< f2 32 17 >");													// WR address 0x232: PKTSTOREFLAGS - ST RSSI, ST RFOFFS, ST FOFFS, ST TIMER

	/* PKTACCEPTFLAGS */
	spi_ax_transport(false, "< f2 33 20 >");													// WR address 0x233: PKTACCEPTFLAGS - ACCPT LRGP 0x20, not ACCPT SZF !0x10, ACCPT ADDRF 0x08, not ACCPT CRCF !0x04, not ACCPT ABRT !0x02, not ACCPT RESIDUE !0x01
	//spi_ax_transport(false, "< f2 33 2c >");	// Test


	/* LPOSCCONFIG */
	spi_ax_transport(false, "< f3 10 13 >");													// WR address 0x310: LPOSCCONFIG - LPOSC CALIBF 0x10, LPOSC FAST 0x02, (LPOSC ENA 0x01)

	/* LPOSCKFILT */
	spi_ax_transport(false, "< f3 12 01 5d >");													// WR address 0x312: LPOSCKFILT - LPOSCKFILT <= 1,398 (lower value gives lower jitter; 1/4 th taken: 349)

	/* LPOSCREF */
	spi_ax_transport(false, "< f3 14 61 a8 >");													// WR address 0x314: LPOSCREF - LPOSCREF = 25,000

	/* LPOSCFREQ */
	spi_ax_transport(false, "< f3 16 00 00 >");													// WR address 0x316: LPOSCFREQ - no manual tune, done by automatic


	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE - DACSHIFT = 12 bit

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 81 >");													// WR address 0x332: DACCONFIG


	/* 0xF0D = REF */
	spi_ax_transport(false, "< ff 0d 03 >");													// WR address 0xF0D: REF (RX/TX) - Set to 0x03

	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10 (RX/TX) - Set to 0x04 if a TCXO is used. If a crystal is used, set to 0x0D if the reference frequency (crystal or TCXO) is more than 43 MHz, or to 0x03 otherwise

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11 (RX/TX) - Set to 0x07 if a crystal is connected to CLK16P/CLK16N, or 0x00 if a TCXO is used

	/* 0xF1C */
	spi_ax_transport(false, "< ff 1c 07 >");													// WR address 0xF1C (RX/TX) - Set to 0x07
}

void spi_ax_initRegisters_POCSAG_Tx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0b >");														// WR address 0x30: PLLLOOP

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

	/* PLLVCODIV - mind you: check 0xF34, also */
	spi_ax_transport(false, "< b2 24 >");														// WR address 0x32: PLLVCODIV - VCO2INT, RFDIV

	/* PLLLOOPBOOST */
	//spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)


	/* MODCFGF */
	//spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF - FREQSHAPE: External Loop Filter (BT = 0.0)

	/* FSKDEV */
	//spi_ax_transport(false, "< f1 61 00 10 62 >");												// WR address 0x161: FSKDEV - FSKDEV: +/-4,000 Hz @ fxtal = 16 MHz

	/* MODCFGA */
	//spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA - AMPLSHAPE, TXDIFF

	/* TXRATE */
	//spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: TXRATE - TXRATE: 1,200 bit/s


	/* XTALCAP */
	spi_ax_transport(false, "< f1 84 0c >");													// WR address 0x184: XTALCAP


	/* 0xF00 */
	spi_ax_transport(false, "< ff 00 0f >");													// WR address 0xF00:

	/* 0xF0C */
	//spi_ax_transport(false, "< ff 0c 00 >");													// WR address 0xF0C (RX/TX) - Keep the default 0x00

	/* 0xF0D = REF */
	//spi_ax_transport(false, "< ff 0d 03 >");													// WR address 0xF0D: REF (RX/TX) - Set to 0x03

	/* 0xF10 - XTALOSC*/
	//spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10 (RX/TX) - Set to 0x04 if a TCXO is used. If a crystal is used, set to 0x0D if the reference frequency (crystal or TCXO) is more than 43 MHz, or to 0x03 otherwise

	/* 0xF11 - XTALAMPL */
	//spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11 (RX/TX) - Set to 0x07 if a crystal is connected to CLK16P/CLK16N, or 0x00 if a TCXO is used

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX)

	/* 0xF1C */
	//spi_ax_transport(false, "< ff 1c 07 >");													// WR address 0xF1C (RX/TX) - Set to 0x07

	/* 0xF34 */
	//spi_ax_transport(false, "< ff 34 28 >");													// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise

	/* 0xF35 */
	//spi_ax_transport(false, "< ff 35 10 >");													// WR address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)

	/* 0xF44 */
	//spi_ax_transport(false, "< ff 44 25 >");													// WR address 0xF44 (RX/TX)

	/* 0xF21 */
	//spi_ax_transport(false, "< ff 21 68 >");													// WR address 0xF21 (RX)

	/* 0xF22 */
	//spi_ax_transport(false, "< ff 22 ff >");													// WR address 0xF22 (RX)

	/* 0xF23 */
	//spi_ax_transport(false, "< ff 23 84 >");													// WR address 0xF23 (RX)

	/* 0xF26 */
	//spi_ax_transport(false, "< ff 26 98 >");													// WR address 0xF26 (RX)

	/* 0xF34 */
	//spi_ax_transport(false, "< ff 34 28 >");													// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise

	/* 0xF35 */
	//spi_ax_transport(false, "< ff 35 10 >");													// WR address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)

	/* 0xF44 */
	//spi_ax_transport(false, "< ff 44 25 >");													// WR address 0xF44 (RX/TX)
}

void spi_ax_initRegisters_POCSAG_Rx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0b >");														// WR address 0x30: PLLLOOP - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

	/* PLLVCODIV - mind you: check 0xF34, also */
	spi_ax_transport(false, "< b2 24 >");														// WR address 0x32: PLLVCODIV - VCO2INT, RFDIV

	/* PLLLOOPBOOST */
	//spi_ax_transport(false, "< b8 0b >");														// WR address 0x38: PLLLOOPBOOST - DIRECT 0x08, no FILTEN, FLT 03: Internal Loop Filter x5 (BW = 500 kHz for ICP = 1.7 mA)


	/* XTALCAP */
	spi_ax_transport(false, "< f1 84 0c >");													// WR address 0x184: XTALCAP


	/* 0xF00 */
	spi_ax_transport(false, "< ff 00 0f >");													// WR address 0xF00:

	/* 0xF0C */
	//spi_ax_transport(false, "< ff 0c 00 >");													// WR address 0xF0C (RX/TX) - Keep the default 0x00

	/* 0xF0D = REF */
	//spi_ax_transport(false, "< ff 0d 03 >");													// WR address 0xF0D: REF (RX/TX) - Set to 0x03

	/* 0xF10 - XTALOSC*/
	//spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10 (RX/TX) - Set to 0x04 if a TCXO is used. If a crystal is used, set to 0x0D if the reference frequency (crystal or TCXO) is more than 43 MHz, or to 0x03 otherwise

	/* 0xF11 - XTALAMPL */
	//spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11 (RX/TX) - Set to 0x07 if a crystal is connected to CLK16P/CLK16N, or 0x00 if a TCXO is used

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 02 >");													// WR address 0xF18 (RX/TX) - Differs between RX and TX

	/* 0xF1C */
	//spi_ax_transport(false, "< ff 1c 07 >");													// WR address 0xF1C (RX/TX) - Set to 0x07

	/* 0xF21 */
	//spi_ax_transport(false, "< ff 21 68 >");													// WR address 0xF21 (RX)

	/* 0xF22 */
	//spi_ax_transport(false, "< ff 22 ff >");													// WR address 0xF22 (RX)

	/* 0xF23 */
	//spi_ax_transport(false, "< ff 23 84 >");													// WR address 0xF23 (RX)

	/* 0xF26 */
	//spi_ax_transport(false, "< ff 26 98 >");													// WR address 0xF26 (RX)

	/* 0xF34 */
	//spi_ax_transport(false, "< ff 34 28 >");													// WR address 0xF34 (RX/TX) - Set to 0x28 if RFDIV in register PLLVCODIV is set, or to 0x08 otherwise

	/* 0xF35 */
	//spi_ax_transport(false, "< ff 35 10 >");													// WR address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)

	/* 0xF44 */
	//spi_ax_transport(false, "< ff 44 25 >");													// WR address 0xF44 (RX/TX)

	/* 0xF72 */
	//spi_ax_transport(false, "< ff 72 00 >");													// WR address 0xF72 (RX) - Set to 0x06 if the framing mode is set to �Raw, Soft Bits� (register FRAMING), or to 0x00 otherwise
}

void spi_ax_initRegisters_POCSAG_Rx_WoR(void)
{
	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 83 >");													// WR address 0x120: AGCGAIN0 - AGCDECAY0 = 14  (f_3db=10 Hz), AGCATTACK0 = 8  (f_3db=621 Hz)

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 83 >");													// WR address 0x130: AGCGAIN1 - AGCDECAY1 = 14  (f_3db=10 Hz), AGCATTACK1 = 8  (f_3db=621 Hz)


	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 54 >");													// WR address 0x227: TMGRXAGC

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 18 >");													// WR address 0x229: TMGRXPREAMBLE1

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 05 >");													// WR address 0x231: PKTMISCFLAGS
}

void spi_ax_initRegisters_POCSAG_Rx_cont(void)
{
	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0 - AGCDECAY0 = 14  (f_3db=10 Hz), AGCATTACK0 = 8  (f_3db=621 Hz)

	/* AGCGAIN1 */
	spi_ax_transport(false, "< f1 30 e8 >");													// WR address 0x130: AGCGAIN1 - AGCDECAY1 = 14  (f_3db=10 Hz), AGCATTACK1 = 8  (f_3db=621 Hz)


	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 00 >");													// WR address 0x227: TMGRXAGC - not used

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1 - not used

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 00 >");													// WR address 0x231: PKTMISCFLAGS - no BGND RSSI !0x04, RXAGC CLK, RXRSSI CLK clock sources: 1 �s
}

void spi_ax_init_POCSAG_Tx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* POCSAG */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.9250);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x05
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.9875);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_POCSAG, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_FULLTX);

	/* Switch to VCO-B - set VCO-PLL to FREQB - 439.9875 MHz */
	(void) spi_ax_selectVcoFreq(true);

	/* Set power level */
	spi_ax_setPower_dBm(-20);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}

int8_t spi_ax_run_POCSAG_Tx_FIFO_Msg(uint32_t pocsagTgtRIC, AX_POCSAG_CW2_t pocsagTgtFunc, const char* pocsagTgtMsg, uint8_t pocsagTgtMsgLen)
{
	/* Enter a POCSAG message */

	/* 1 - Flags */
	spi_ax_util_POCSAG_Tx_FIFO_Preamble();														// 576 bits of 1/0 pattern

	/* 2 - Target RIC, message to be sent */
	return spi_ax_util_POCSAG_Tx_FIFO_Batches(pocsagTgtRIC, pocsagTgtFunc, pocsagTgtMsg, pocsagTgtMsgLen);
}

void spi_ax_util_POCSAG_Tx_FIFO_Preamble(void)
{
	uint16_t idx = 0;

	/* Wait until enough space for next batch is available */
	spi_ax_util_FIFO_waitFree(4 + 18 * 4);

	g_ax_spi_packet_buffer[idx++] = 0xA9;														// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_REPEATDATA_TX;
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_PKTSTART;								// FIFO flag byte
	g_ax_spi_packet_buffer[idx++] = 18 * 4;														// PREAMBLE length: 576 bits = 18 words
	g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(AX_POCSAG_CODES_PREAMBLE, 0);				// 1/0 pattern

	/* FIFO data enter */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	#if 0
	/* FIFO do a COMMIT */
	spi_ax_transport(false, "< a8 04 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
	#endif
}

int8_t spi_ax_util_POCSAG_Tx_FIFO_Batches(uint32_t tgtRIC, AX_POCSAG_CW2_t tgtFunc, const char* tgtMsg, uint8_t tgtMsgLen)
{
	//const uint32_t tgtAddrHi	= calc_BitReverse(18, tgtRIC >> 3);
	uint32_t tgtAddrHi	= tgtRIC >> 3;
	uint8_t  tgtAddrLo	= tgtRIC & 0x7;
	uint16_t msgBitIdx	= 0U;
	uint8_t  batchIdx	= 0U;
	bool     inMsg		= false;
	bool     msgDone	= false;

	/* Sanity checks */
	{
		if (!tgtRIC) {
			return -1;
		}

		switch (tgtFunc) {
			case AX_POCSAG_CW2_MODE0_NUMERIC:
				if (!tgtMsg || !tgtMsgLen || (tgtMsgLen > 40)) {
					return -2;
				}
			break;

			case AX_POCSAG_CW2_MODE1_TONE:
				{
					/* Any data is silently ignored */
				}
			break;

			case AX_POCSAG_CW2_MODE2_ACTIVATION:
				if (!tgtMsg || !tgtMsgLen || (tgtMsgLen > 10)) {								// TODO: fix me - correct maximum size, here.
					return -2;
				}
			break;

			case AX_POCSAG_CW2_MODE3_ALPHANUM:
				if (!tgtMsg || !tgtMsgLen || ((tgtMsgLen > 80) && (tgtRIC != AX_POCSAG_SKYPER_RIC_NEWS))) {
					return -2;
				}
			break;

			default:
				return -3;
		}
	}

	/* Process message as much batches it needs - each batch is enqueued into the transmitter FIFO */
	do {
		uint8_t idx	= 0;

		/* Wait until enough space for next batch is available */
		spi_ax_util_FIFO_waitFree(4 + 4 + 8 * (4 + 4));											// FIFO_cmd + SYNCWORD + 8 batches * (2 words)

		g_ax_spi_packet_buffer[idx++] = 0xA9;													// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
		g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_DATA_TX_RX;
		g_ax_spi_packet_buffer[idx++] = 0;														// Dummy entry for now
		g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_PKTSTART | AX_FIFO_DATA_FLAGS_TX_NOCRC;	// FIFO flag byte

		/* SYNC */
		g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(AX_POCSAG_CODES_SYNCWORD, 3);
		g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(AX_POCSAG_CODES_SYNCWORD, 2);
		g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(AX_POCSAG_CODES_SYNCWORD, 1);
		g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(AX_POCSAG_CODES_SYNCWORD, 0);

		/* Frames */
		for (uint8_t frIdx = 0; frIdx < 8; frIdx++) {
			for (uint8_t cwIdx = 0; cwIdx < 2; cwIdx++) {
				uint32_t pad;

				/* WORD */
				if (!inMsg && !msgDone) {
					if (frIdx < tgtAddrLo) {
						pad = AX_POCSAG_CODES_IDLEWORD;

					} else {
						uint32_t addrCW;

						inMsg	= true;
						addrCW  = tgtAddrHi			<< 13;
						addrCW |= (uint32_t)tgtFunc	<< 11;
						pad		= spi_ax_pocsag_calc_checksumParity(addrCW);

						/* No message content when TONE is sent */
						if (tgtFunc == AX_POCSAG_CW2_MODE1_TONE) {
							msgDone = true;
						}
					}

				} else if (inMsg && !msgDone) {
					uint32_t msgCW;

					msgCW		= spi_ax_pocsag_get20Bits(tgtMsg, tgtMsgLen, tgtFunc, msgBitIdx) << 11;
					msgCW	   |= 0x80000000UL;
					msgBitIdx  += 20;
					pad			= spi_ax_pocsag_calc_checksumParity(msgCW);

					/* Check for message end */
					switch (tgtFunc) {
						case AX_POCSAG_CW2_MODE0_NUMERIC:
							if (tgtMsgLen < (msgBitIdx >> 2)) {
								msgDone = true;
							}
						break;

						#if 0																	// TODO: to be implemented
						case AX_POCSAG_CW2_MODE2_ACTIVATION:
						break;
						#endif

						case AX_POCSAG_CW2_MODE3_ALPHANUM:
							if (tgtMsgLen < (msgBitIdx / 7)) {
								msgDone = true;
							}
						break;

						default:
							msgDone = true;
					}

				} else if (msgDone) {
					inMsg	= false;
					pad		= AX_POCSAG_CODES_IDLEWORD;

					/* Break transmission after data + IDLEWORD has been sent - leave the loops */
					cwIdx = 2;
					frIdx = 8;
				}

				g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(pad, 3);
				g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(pad, 2);
				g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(pad, 1);
				g_ax_spi_packet_buffer[idx++] = s_sel_u8_from_u32(pad, 0);
			}  // for (cwIdx)
		}  // for (frIdx)

		/* Set length for FIFO DATA command */
		g_ax_spi_packet_buffer[    2] = idx - 3;												// Length

		/* FIFO data enter */
		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

		/* FIFO do a COMMIT */
		spi_ax_transport(false, "< a8 04 >");													// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT

		batchIdx++;
	} while (!msgDone || inMsg);

	return 0;
}

void spi_ax_init_POCSAG_Rx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* FSK */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(433.9250);						// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x09
		/*
		Radiometrix TXL2/RXL2 - 16kbps bi-phase FSK
		433.925MHz - CHAN0
		433.285MHz - CHAN1
		433.605MHz - CHAN2
		434.245MHz - CHAN3
		434.565MHz - CHAN4
		*/

		/* POCSAG */
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.9875);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_RX_WOR, AX_SET_REGISTERS_POWERMODE_WOR);

	/* Switch to VCO-B - set VCO-PLL to FREQB - 439.9875 MHz */
	(void) spi_ax_selectVcoFreq(true);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}


void spi_ax_initRegisters_AnlogFM(void)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 0b >");														// WR address 0x10: MODULATION - 0B: Analog FM
}

void spi_ax_initRegisters_AnlogFM_Tx(void)
{
	/* PINFUNCDATA */
	spi_ax_transport(false, "< a3 04 >");														// WR address 0x23: PINFUNCDATA - DATA Input/Output Modem Data: enables continuous TX operation, rather than powering up the PA only if there is committed FIFO data. This is similar to wire mode, except that no data is read from the pin in FM mode.

	/* PINFUNCTCXO_EN */
	spi_ax_transport(false, "< a6 00 >");														// WR address 0x26: PINFUNCTCXO_EN - Output '0'

	/* FSKDEV */
	spi_ax_transport(false, "< f1 61 00 c0 07 >");												// WR address 0x161: FSKDEV - GPADC13, enable sign extension and offset (=midcode) subtraction, fdeviation = � 65 kHz [max / min ADC value gives fdeviation = � fxtal / 2^(AX5043_FSKDEV0[2:0]+1), allowed values are 0..7

	/* GPADCCTRL */
	spi_ax_transport(false, "< f3 00 06 >");													// WR address 0x300: GPADCCTRL - continuous sampling of GPADC13

	/* GPADCPERIOD */
	spi_ax_transport(false, "< f3 01 07 >");													// WR address 0x301: GPADCPERIOD - Fs = fxtal/32/GPADCPERIOD  5 gives 100 kHz @ fxtal = 16 MHz. This determines the sampling rate, TXRATE has no meaning in FM mode.

	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 00 >");													// WR address 0x330: DACVALUE - off

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 80 >");													// WR address 0x332: DACCONFIG - DACPWM, output DACVALUE

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX) - ? (is set to 0x06, explicit named for using Analog FM)
}

void spi_ax_initRegisters_AnlogFM_Rx(void)
{
	/* PINFUNCTCXO_EN */
	spi_ax_transport(false, "< a6 05 >");														// WR address 0x26: PINFUNCTCXO_EN - Use TCXO_EN pin as DAC output

	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 06 66 >");													// WR address 0x100: IFFREQ - 25 kHz (f_xtal = 16 MHz)

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET - off

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 cc cc >");												// WR address 0x109: MAXRFOFFSET - track at LO1, max 50 kHz @ f_xtal = 16 MHz

	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 04 >");													// WR address 0x116: FREQUENCYLEAK - FREQUENCYGAINB0 + 2, prevents the demodulator AFC loop from tracking static frequency offsets

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 00 >");													// WR address 0x117: RXPARAMSETS - only use receiver parameter set 0

	/* TIMEGAIN0 */
	spi_ax_transport(false, "< f1 24 00 >");													// WR address 0x124: TIMEGAIN0 - disable bit timing recovery, which would only add jitter

	/* DRGAIN0 */
	spi_ax_transport(false, "< f1 25 00 >");													// WR address 0x125: DRGAIN0 - off

	/* FREQGAINA0 */
	spi_ax_transport(false, "< f1 27 a0 >");													// WR address 0x127: FREQGAINA0 - off

	/* FREQGAINB0 */
	spi_ax_transport(false, "< f1 28 02 >");													// WR address 0x128: FREQGAINB0 - bandwidth of �inner� AFC loop used for FM demodulation. f_3dB = 0.115*BR.	This is the fastest setting available

	/* FREQGAINC0 */
	spi_ax_transport(false, "< f1 29 1f >");													// WR address 0x129: FREQGAINC0 - off

	/* FREQGAIND0 */
	spi_ax_transport(false, "< f1 2a 08 >");													// WR address 0x12A: FREQGAIND0 - bandwidth of �outer� AFC loop (tracking frequency mismatch), 78 Hz @ BR = 100 kbps, f_xtal = 16 MHz

	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE - DACSHIFT = 12 bit. This gives maximum volume, downshifting further gives smaller volume

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 83 >");													// WR address 0x332: DACCONFIG - DACPWM, output TRKFREQ (= demodulated signal) on DAC

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX) - ? (is set to 0x06, explicit named for using Analog FM)
}

void spi_ax_init_AnalogFM_Tx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* Burst-Aussendungen fuer Steuerungszwecke - lower and upper frequencies */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.9245);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9255);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_FULLTX);

	(void) spi_ax_selectVcoFreq(false);

	/* Set power level */
	spi_ax_setPower_dBm(-20);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");													// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}

void spi_ax_init_AnalogFM_Rx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* Burst-Aussendungen fuer Steuerungszwecke - lower and upper frequencies */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.9245);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9255);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_RX_CONT, AX_SET_REGISTERS_POWERMODE_FULLRX);

	(void) spi_ax_selectVcoFreq(false);

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS
}


#if defined(AX_GPADC13_ENABLED)
static void spi_ax_adcCtrlSet(uint8_t val)
{
	g_ax_spi_packet_buffer[0] = 0xF3;															// WR Address 0x300: GPADCCTRL
	g_ax_spi_packet_buffer[1] = 0x30;
	g_ax_spi_packet_buffer[2] = val;

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
}

static uint8_t spi_ax_adcCtrlGet(void)
{
	/* GPADCCTRL */
	spi_ax_transport(false, "< 73 30 R1 >");													// RD address 0x300: GPADCCTRL
	return g_ax_spi_packet_buffer[0];
}

static uint16_t spi_ax_adcValGet(void)
{
	/* GPADC13VALUE */
	spi_ax_transport(false, "< 73 08 R2 >");													// RD address 0x308: GPADC13VALUE

	uint16_t val = g_ax_spi_packet_buffer[0] & 0x03;
	val <<= 8;
	val  |= g_ax_spi_packet_buffer[1];
	return val;
}

inline
static void spi_ax_adcConvertionWait(void)
{
	do { }  while (spi_ax_adcCtrlGet() & 0x80);													// GPADCCTRL - BUSY Conversion ongoing when 1
}

static uint16_t spi_axr_getVcoTuneVoltage(void)
{
    int16_t tuneVoltage = 0;

	/* Prepare the ADC */
    uint8_t cnt = 64;
    do {
		spi_ax_adcCtrlSet(0x84);																// GPADCCTRL - BUSY When writing 1, a single conversion is started - GPADC13 Enable Sampling GPADC1?GPADC3
        spi_ax_adcConvertionWait();
    } while (--cnt);

    cnt = 32;
    do {
		spi_ax_adcCtrlSet(0x84);																// GPADCCTRL - BUSY When writing 1, a single conversion is started - GPADC13 Enable Sampling GPADC1?GPADC3
        spi_ax_adcConvertionWait();
		tuneVoltage += spi_ax_adcValGet();
    } while (--cnt);

    return tuneVoltage;
}

/* REMARKS: the AX5243 has got an automatic VCOI adjustment built in - in contrast this function does the setting manually with the help of the GPADC13 */
static uint8_t s_spi_ax_cal_vcoi(void)
{
    uint8_t vcoiRet = 0;
    uint16_t vmin = 0xffff;
    uint16_t vmax = 0x0000;

    for (uint8_t vcoiCurrentIdx = 0x40; vcoiCurrentIdx; ) {
        --vcoiCurrentIdx;
        uint8_t vcoiNew = 0x80 | vcoiCurrentIdx;												// VCOIE - Enable manual VCOI

		/* PLLVCOI */
		g_ax_spi_packet_buffer[0] = 0xf1;														// WR address 0x180: PLLVCOI
		g_ax_spi_packet_buffer[1] = 0x80;
		g_ax_spi_packet_buffer[2] = vcoiNew;

		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);									// Write back with modified value
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

        /* Clear STICKY LOCK */
		spi_ax_transport(false, "< 33 R1 >");													// RD Address 0x33: PLLRANGINGA

        uint16_t curTuneVoltage = spi_axr_getVcoTuneVoltage();

        /* Clear STICKY LOCK again */
		spi_ax_transport(false, "< 33 R1 >");													// RD Address 0x33: PLLRANGINGA

        //global_var[vcoiCurrentIdx] = curTuneVoltage;											// For debugging VCO_TuneVoltage = f(VCOI)
        if (curTuneVoltage > vmax) {
            vmax = curTuneVoltage;
		}

        if (curTuneVoltage < vmin) {
            vmin = curTuneVoltage;

            /* Check whether the PLL is still locked */
			spi_ax_transport(false, "< 33 R1 >");												// RD Address 0x33: PLLRANGINGA
			uint8_t curPllrangingA = g_ax_spi_packet_buffer[0];

			/* Both flags STICKY LOCK and PLL LOCK are set */
            if (!(0xc0 & ~curPllrangingA)) {
                vcoiRet = vcoiCurrentIdx;
			}
        }
    }

	/* Security checks */
    if (!vcoiRet || (vmax >= 0xFF00) || (vmin < 0x0100) || ((vmax - vmin) < 0x6000)) {
        return 0;
	}
    return vcoiRet;
}
#endif


void spi_init(void) {
	/* Set init level */
	ioport_set_pin_mode(AX_SEL,	 (uint8_t) (IOPORT_INIT_HIGH | IOPORT_SRL_ENABLED));			// SEL  inactive
	ioport_set_pin_mode(AX_MOSI, (uint8_t) (IOPORT_INIT_HIGH | IOPORT_SRL_ENABLED));			// MOSI inactive
	ioport_set_pin_mode(AX_CLK,	 (uint8_t) (IOPORT_INIT_HIGH | IOPORT_SRL_ENABLED));			// CLK  inactive

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
	spi_master_setup_device(&SPI_AX, &g_ax_spi_device_conf, SPI_MODE_0, 10000000, 0);			// max. 10 MHz (100 ns) when AX in POWERDOWN mode
	spi_enable(&SPI_AX);

	#if 0
	/* Frequency settings */
	{
		#if defined(AX_RUN_VCO2_APRS_TX)
			/* Default setting for the application: APRS */
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_PR1200, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* APRS  */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.8000);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05

			/* Burst-Aussendungen fuer Steuerungszwecke */
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9250);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05


		#elif defined(AX_TEST_VCO1_BANDENDS)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* VCO A/B settings */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(400.0000);					// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x0e
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(525.0000);					// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x02

		#elif defined(AX_TEST_VCO1_FSK_TX) | defined(AX_TEST_VCO1_FSK_RX) | defined(AX_TEST_VCO1_POCSAG_TX) | defined(AX_TEST_VCO1_POCSAG_RX)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* FSK */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(433.9250);					// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x09
			/*
			Radiometrix TXL2/RXL2 - 16kbps bi-phase FSK
			433.925MHz - CHAN0
			433.285MHz - CHAN1
			433.605MHz - CHAN2
			434.245MHz - CHAN3
			434.565MHz - CHAN4
			*/

			/* POCSAG */
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.9875);					// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		#elif defined(AX_TEST_VCO2_BANDENDS)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_FSK, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* VCO A/B settings */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(137.0000);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x0e
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(149.0000);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x00

		#elif defined(AX_TEST_VCO2_ANALOG_FM_RX)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* APRS */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.8000);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05

			/* DB0ZH */
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(145.6250);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x04

		#elif defined(AX_TEST_VCO2_ANALOG_FM_TX)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* Burst-Aussendungen fuer Steuerungszwecke - lower and upper frequencies */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.9245);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9255);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		#elif defined(AX_TEST_VCO2_PR1200_TX) | defined(AX_TEST_VCO2_PR1200_RX)
			/* Syncing and sending reset command, then setting the default values */
			spi_ax_setRegisters(true, AX_SET_REGISTERS_MODULATION_PR1200, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

			/* APRS  */
			g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.8000);					// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05

			/* Burst-Aussendungen fuer Steuerungszwecke */
			g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(144.9250);					// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x05

		#else
			#error "A FREQA / FREQB pair has to be set."
		#endif

		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Auto ranging and storing */
		spi_ax_doRanging();

		#ifndef AX_RUN_VCO2_APRS_TX
			#if 0
				while (true) { nop(); }
			#endif
		#endif
	}

	/* TEST BOX */
	s_spi_start_testBox();
#endif
}


/* Debugging */

#ifdef AX_TEST
static void s_spi_start_testBox(void)
{
	//  AX_TEST_ANALOG_FM_TX
	#if defined(AX_TEST_VCO2_ANALOG_FM_TX)
	spi_ax_test_Analog_FM_Tx();

	// AX_TEST_ANALOG_FM_RX
	#elif defined(AX_TEST_VCO2_ANALOG_FM_RX)
	spi_ax_test_Analog_FM_Rx();


	// AX_TEST_PR1200_TX
	#elif defined(AX_TEST_VCO2_PR1200_TX)
	spi_ax_test_PR1200_Tx();

	// AX_TEST_PR1200_RX
	#elif defined(AX_TEST_VCO2_PR1200_RX)
	spi_ax_test_PR1200_Rx();


	// AX_TEST_POCSAG_TX
	#elif defined(AX_TEST_VCO1_POCSAG_TX)
	spi_ax_test_POCSAG_Tx();

	// AX_TEST_POCSAG_RX
	#elif defined(AX_TEST_VCO1_POCSAG_RX)
	spi_ax_test_POCSAG_Rx();
	#endif
}
#endif

void spi_ax_monitor_levels(void)
{
	volatile uint8_t curRssi = 0;
	volatile uint8_t curBgndRssi = 0;
	volatile uint8_t curAgcCounter = 0;
	volatile uint16_t curTrkAmpl = 0;
	volatile uint32_t curTrkRfFreq = 0;
	volatile uint16_t curTrkFreq = 0;

	while (true) {
		/* RSSI, BGNDRSSI */
		spi_ax_transport(false, "< 40 R2 >");												// RD Address 0x40: RSSI, BGNDRSSI
		curRssi			= g_ax_spi_packet_buffer[0];
		curBgndRssi		= g_ax_spi_packet_buffer[1];

		/* AGCCOUNTER */
		spi_ax_transport(false, "< 43 R1 >");												// RD Address 0x43: AGCCOUNTER
		curAgcCounter	= g_ax_spi_packet_buffer[0];

		/* TRKAMPL */
		spi_ax_transport(false, "< 48 R2 >");												// RD Address 0x48: TRKAMPL
		curTrkAmpl		= ((uint16_t)g_ax_spi_packet_buffer[0] << 8) | g_ax_spi_packet_buffer[1];

		/* TRKRFFREQ */
		spi_ax_transport(false, "< 4D R3 >");												// RD Address 0x4D: TRKRFFREQ
		curTrkRfFreq	= (((uint32_t)g_ax_spi_packet_buffer[0] & 0x0f) << 24) | ((uint32_t)g_ax_spi_packet_buffer[1] << 8) | g_ax_spi_packet_buffer[2];

		/* TRKFREQ */
		spi_ax_transport(false, "< 50 R2 >");												// RD Address 0x50: TRKFREQ
		curTrkFreq		= ((uint16_t)g_ax_spi_packet_buffer[0] << 8) | g_ax_spi_packet_buffer[1];

		/* View debugger auto variables*/
		nop();

		(void) curRssi;
		(void) curBgndRssi;
		(void) curAgcCounter;
		(void) curTrkAmpl;
		(void) curTrkRfFreq;
		(void) curTrkFreq;
	}
}

void spi_ax_Rx_FIFO(void)
{
	uint8_t				l_curRssi						= 0;
	uint8_t				l_curBgndRssi					= 0;
	uint8_t				l_agcCounter					= 0;
	uint8_t				l_fifo_stat						= 0;
	uint16_t			l_fifo_count					= 0;
	AX_FIFO_RX_FSM_t	l_fifo_state					= AX_FIFO_RX_FSM__START;
	AX_FIFO_DATA_CMD_t	l_fifo_cmd						= AX_FIFO_DATA_CMD_NOP_TX;
	uint32_t			l_fifo_lastPacket_time			= 0UL;
	uint8_t				l_fifo_lastPacket_rssi			= 0U;
	uint8_t				l_fifo_lastPacket_rssi_ant2		= 0U;
	uint8_t				l_fifo_lastPacket_bgndnoise		= 0U;
	uint32_t			l_fifo_lastPacket_frfreqoffset	= 0UL;
	uint16_t			l_fifo_lastPacket_freqoffset	= 0U;
	uint32_t			l_fifo_lastPacket_datarate		= 0UL;
	uint8_t				l_fifo_lastPacket_dataLen		= 0U;
	uint8_t				l_fifo_lastPacket_dataStatus	= 0U;
	uint8_t				l_fifo_lastPacket_dataMsg[C_SPI_AX_BUFFER_LENGTH];

	/* Receive loop */
	do {
		/* Wait for a packet */
		do {
			/* RSSI, BGNDRSSI */
			spi_ax_transport(false, "< 40 R4 >");												// RD Address 0x40: RSSI, BGNDRSSI
			l_curRssi			= g_ax_spi_packet_buffer[0];
			l_curBgndRssi		= g_ax_spi_packet_buffer[1];
			l_agcCounter		= g_ax_spi_packet_buffer[3];

			#if 1
			/* IRQREQUEST0 */
			spi_ax_transport(false, "< 0d R1 >");												// RD address 0x0d: IRQREQUEST0

			#else
			/* FIFOSTAT - does not work in WoR mode */
			spi_ax_transport(false, "< 28 R1 >");												// RD address 0x28: FIFOSTAT
			#endif
		} while (!(g_ax_spi_packet_buffer[0] & 0x01));

		l_fifo_stat = g_ax_spi_packet_buffer[0];

		/* FIFOCOUNT */
		spi_ax_transport(false, "< 2a R2 >");													// RD address 0x28: FIFOCOUNT
		l_fifo_count = ((uint16_t)(g_ax_spi_packet_buffer[0]) << 8) | g_ax_spi_packet_buffer[1];

		/* Reset FSM to START state */
		l_fifo_state = AX_FIFO_RX_FSM__START;

		/* Pull messages from the FIFO */
		for (uint16_t idx = 0; idx < l_fifo_count; idx++) {
			/* FIFODATA */
			spi_ax_transport(false, "< 29 R1 >");												// RD address 0x29: FIFODATA

			switch (l_fifo_state) {
				case AX_FIFO_RX_FSM__START:
				{
					l_fifo_cmd = (AX_FIFO_DATA_CMD_t)g_ax_spi_packet_buffer[0];
					switch (l_fifo_cmd) {
						case AX_FIFO_DATA_CMD_TIMER_RX:
						l_fifo_state = AX_FIFO_RX_FSM_TIMER_1;
						break;

						case AX_FIFO_DATA_CMD_RSSI_RX:
						l_fifo_state = AX_FIFO_RX_FSM_RSSI_1;
						break;

						case AX_FIFO_DATA_CMD_ANTRSSI2_RX:
						l_fifo_state = AX_FIFO_RX_FSM_ANTRSSI2_1;
						break;

						case AX_FIFO_DATA_CMD_ANTRSSI3_RX:
						l_fifo_state = AX_FIFO_RX_FSM_ANTRSSI3_1;
						break;

						case AX_FIFO_DATA_CMD_RFFREQOFFS_RX:
						l_fifo_state = AX_FIFO_RX_FSM_RFFREQOFFS_1;
						break;

						case AX_FIFO_DATA_CMD_FREQOFFS_RX:
						l_fifo_state = AX_FIFO_RX_FSM_FREQOFFS_1;
						break;

						case AX_FIFO_DATA_CMD_DATARATE_RX:
						l_fifo_state = AX_FIFO_RX_FSM_DATARATE_1;
						break;

						case AX_FIFO_DATA_CMD_DATA_TX_RX:
						l_fifo_state = AX_FIFO_RX_FSM_DATA_1;
						break;

						default:
						{
							l_fifo_state = AX_FIFO_RX_FSM__FAIL_CMD;
						}
					}
				}
				break;


				case AX_FIFO_RX_FSM_TIMER_1:
				l_fifo_lastPacket_time  = (uint32_t)g_ax_spi_packet_buffer[0] << 16;
				l_fifo_state = AX_FIFO_RX_FSM_TIMER_2;
				break;

				case AX_FIFO_RX_FSM_TIMER_2:
				l_fifo_lastPacket_time |= (uint32_t)g_ax_spi_packet_buffer[0] <<  8;
				l_fifo_state = AX_FIFO_RX_FSM_TIMER_3;
				break;

				case AX_FIFO_RX_FSM_TIMER_3:
				l_fifo_lastPacket_time |= (uint32_t)g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_RSSI_1:
				l_fifo_lastPacket_rssi = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_ANTRSSI2_1:
				l_fifo_lastPacket_rssi = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM_ANTRSSI2_2;
				break;

				case AX_FIFO_RX_FSM_ANTRSSI2_2:
				l_fifo_lastPacket_bgndnoise = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_ANTRSSI3_1:
				l_fifo_lastPacket_rssi = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM_ANTRSSI3_2;
				break;

				case AX_FIFO_RX_FSM_ANTRSSI3_2:
				l_fifo_lastPacket_rssi_ant2 = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM_ANTRSSI3_3;
				break;

				case AX_FIFO_RX_FSM_ANTRSSI3_3:
				l_fifo_lastPacket_bgndnoise = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_RFFREQOFFS_1:
				l_fifo_lastPacket_frfreqoffset  = (uint32_t)g_ax_spi_packet_buffer[0] << 16;
				l_fifo_state = AX_FIFO_RX_FSM_RFFREQOFFS_2;
				break;

				case AX_FIFO_RX_FSM_RFFREQOFFS_2:
				l_fifo_lastPacket_frfreqoffset |= (uint32_t)g_ax_spi_packet_buffer[0] <<  8;
				l_fifo_state = AX_FIFO_RX_FSM_RFFREQOFFS_3;
				break;

				case AX_FIFO_RX_FSM_RFFREQOFFS_3:
				l_fifo_lastPacket_frfreqoffset |= (uint32_t)g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_FREQOFFS_1:
				l_fifo_lastPacket_freqoffset  = (uint32_t)g_ax_spi_packet_buffer[0] <<  8;
				l_fifo_state = AX_FIFO_RX_FSM_FREQOFFS_2;
				break;

				case AX_FIFO_RX_FSM_FREQOFFS_2:
				l_fifo_lastPacket_freqoffset |= (uint32_t)g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_DATARATE_1:
				l_fifo_lastPacket_datarate  = (uint32_t)g_ax_spi_packet_buffer[0] << 16;
				l_fifo_state = AX_FIFO_RX_FSM_DATARATE_2;
				break;

				case AX_FIFO_RX_FSM_DATARATE_2:
				l_fifo_lastPacket_datarate |= (uint32_t)g_ax_spi_packet_buffer[0] <<  8;
				l_fifo_state = AX_FIFO_RX_FSM_DATARATE_3;
				break;

				case AX_FIFO_RX_FSM_DATARATE_3:
				l_fifo_lastPacket_datarate |= (uint32_t)g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;


				case AX_FIFO_RX_FSM_DATA_1:
				l_fifo_lastPacket_dataLen = g_ax_spi_packet_buffer[0];
				l_fifo_state = AX_FIFO_RX_FSM_DATA_2;
				break;

				case AX_FIFO_RX_FSM_DATA_2:
				l_fifo_lastPacket_dataStatus = g_ax_spi_packet_buffer[0];
				l_fifo_lastPacket_dataLen--;
				l_fifo_state = AX_FIFO_RX_FSM_DATA_3;
				break;

				case AX_FIFO_RX_FSM_DATA_3:
				g_ax_spi_packet_buffer[0] = 0x29;											// RD address 0x29: FIFODATA

				spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
				spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, l_fifo_lastPacket_dataLen);
				spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

				memcpy(l_fifo_lastPacket_dataMsg, g_ax_spi_packet_buffer, l_fifo_lastPacket_dataLen);
				memset(l_fifo_lastPacket_dataMsg + l_fifo_lastPacket_dataLen, 0, sizeof(l_fifo_lastPacket_dataMsg) - l_fifo_lastPacket_dataLen);

				idx += l_fifo_lastPacket_dataLen;
				l_fifo_state = AX_FIFO_RX_FSM__START;
				break;

				default:
				{
					l_fifo_state = AX_FIFO_RX_FSM__FAIL_STATE;
				}
			}
		}


		(void) l_fifo_stat;

		(void) l_curRssi;
		(void) l_curBgndRssi;
		(void) l_agcCounter;

		(void) l_fifo_lastPacket_time;
		(void) l_fifo_lastPacket_rssi;
		(void) l_fifo_lastPacket_rssi_ant2;
		(void) l_fifo_lastPacket_bgndnoise;
		(void) l_fifo_lastPacket_frfreqoffset;
		(void) l_fifo_lastPacket_freqoffset;
		(void) l_fifo_lastPacket_datarate;
		(void) l_fifo_lastPacket_dataLen;
		(void) l_fifo_lastPacket_dataStatus;
		(void) l_fifo_lastPacket_dataMsg;

		nop();
	} while (true);
}


void spi_ax_test_Analog_FM_Tx(void)
{
	bool toggle = false;

	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();

		#if 0
		/* Set VCO-PLL to FREQB */
		(void) spi_ax_selectVcoFreq(true);
		#else
		/* Set VCO-PLL to FREQA */
		(void) spi_ax_selectVcoFreq(false);
		#endif
	}

	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_FULLTX);

	/* Set power level */
	spi_ax_setPower_dBm(-20);


	for (uint16_t cnt = 0x2000; cnt; cnt--) {
		(void) spi_ax_selectVcoFreq(toggle);													// Frequency is alternated  802x per second with the Debug code - 2106x per second with the Release code!
		toggle = !toggle;
	}

	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);
	nop();
}

void spi_ax_test_Analog_FM_Rx(void)
{
	/* Syncing and sending reset command, then setting the default values */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_ANALOG_FM, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		/* FREQA <-- chan[0], FREQB <-- chan[1] */
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();

		#if 0
		/* Set VCO-PLL to FREQB */
		(void) spi_ax_selectVcoFreq(true);
		#else
		/* Set VCO-PLL to FREQA */
		(void) spi_ax_selectVcoFreq(false);
		#endif
	}

	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_FULLRX);

	/* Monitor loop inside */
	spi_ax_monitor_levels();
}


/* PR1200 TX */
void spi_ax_test_PR1200_Tx(void)
{ /* TEST: transmitting some packet */
	//uint8_t cmd = 0x59;
	//volatile uint8_t tmr01[3];
	//volatile uint8_t tmr02[3];

	spi_ax_init_PR1200_Tx();


	/* TIMER */
	/*
	// Convenient operation
	spi_ax_transport(false, "< 59 R3 >");
	memcpy(tmr01, g_ax_spi_packet_buffer, 3);

	// Manual operation
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, &cmd, 1);															// Read TIMER data
	spi_read_packet(&SPI_AX, tmr01, 3);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	// Direct enable line handling
	s_spi_ax_select_device();																		// clear PORT_C4
	spi_write_packet(&SPI_AX, &cmd, 1);															// Read TIMER data
	spi_read_packet(&SPI_AX, tmr01, 3);
	s_spi_ax_deselect_device();																	// set   PORT_C4


	spi_ax_transport(false, "< 59 R3 >");
	memcpy(tmr02, g_ax_spi_packet_buffer, 3);
	// Debug-code: 176 �s

	// Manual operation
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, &cmd, 1);															// Read TIMER data
	spi_read_packet(&SPI_AX, tmr02, 3);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
	// Debug-code: 17 �s

	/ Direct enable line handling
	s_spi_ax_select_device();																		// clear PORT_C4
	spi_write_packet(&SPI_AX, &cmd, 1);															// Read TIMER data
	spi_read_packet(&SPI_AX, tmr02, 3);
	s_spi_ax_deselect_device();																	// set   PORT_C4
	// Debug-code: 8 �s
	*/

	char addrAry[][6]	= { "APXFMS", "DF4IAH", "WIDE1", "WIDE2" };
	uint8_t ssidAry[]	= { 0, 8, 1, 2 };
	char aprsMsg[]		= "!4928.39N/00836.88Ej OP: Uli, QTH: Ladenburg, LOC: JN49hl.";

	for (uint16_t count = 10; count; count--) {
		do {
			/* FIFOSTAT */
			spi_ax_transport(false, "< 28 R1 >");
		} while (!(g_ax_spi_packet_buffer[0] & 0x01));

		#if 0
		/* Enter minimal AX.25 Level-2 packet */
		spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal();
		delay_ms(750);

		#else
		/* Enter an APRS UI frame */
		spi_ax_run_PR1200_Tx_FIFO_APRS(addrAry, ssidAry, 4, aprsMsg, strlen(aprsMsg));
		delay_ms(1000);
		//delay_ms(7500);
		#endif

	}

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	do {
		/* RADIOSTATE */
		spi_ax_transport(false, "< 1c R1 >");													// RD Address 0x1C: RADIOSTATE - IDLE
	} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);

	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	while (true) {
		nop();
	}
}

void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal_AddressField(void)
{
	uint16_t idx = 0;

	g_ax_spi_packet_buffer[idx++] = 0xA9;													// WR address 0x29: FIFODATA  (SPI AX address keeps constant)
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_CMD_DATA_TX_RX;
	g_ax_spi_packet_buffer[idx++] = 0;														// Dummy entry for now
	g_ax_spi_packet_buffer[idx++] = AX_FIFO_DATA_FLAGS_TX_PKTSTART | AX_FIFO_DATA_FLAGS_TX_PKTEND;
	g_ax_spi_packet_buffer[idx++] = ('D' << 1)	| 0;										// Address: dest.       [A 1]
	g_ax_spi_packet_buffer[idx++] = ('F' << 1)	| 0;										// Address: dest.       [A 2]
	g_ax_spi_packet_buffer[idx++] = ('4' << 1)	| 0;										// Address: dest.       [A 3]
	g_ax_spi_packet_buffer[idx++] = ('I' << 1)	| 0;										// Address: dest.       [A 4]
	g_ax_spi_packet_buffer[idx++] = ('A' << 1)	| 0;										// Address: dest.       [A 5]
	g_ax_spi_packet_buffer[idx++] = ('H' << 1)	| 0;										// Address: dest.       [A 6]
	g_ax_spi_packet_buffer[idx++] = ((0b0 << 7) | (0b11 << 5) | (0x2 << 1)	| 0);			// Address: dest.  SSID [A 7]

	g_ax_spi_packet_buffer[idx++] = ('D' << 1)	| 0;										// Address: source      [A 8]
	g_ax_spi_packet_buffer[idx++] = ('F' << 1)	| 0;										// Address: source      [A 9]
	g_ax_spi_packet_buffer[idx++] = ('4' << 1)	| 0;										// Address: source      [A10]
	g_ax_spi_packet_buffer[idx++] = ('I' << 1)	| 0;										// Address: source      [A11]
	g_ax_spi_packet_buffer[idx++] = ('A' << 1)	| 0;										// Address: source      [A12]
	g_ax_spi_packet_buffer[idx++] = ('H' << 1)	| 0;										// Address: source      [A13]
	g_ax_spi_packet_buffer[idx++] = ((0b1 << 7) | (0b11 << 5) | (0x1 << 1)	| 1);			// Address: source SSID [A14]

	g_ax_spi_packet_buffer[idx++] = ((0b0 << 4) |  0b11);									// Control: UI frame with no Poll bit set
	g_ax_spi_packet_buffer[idx++] =  0xf0;													// PID: No layer 3 protocol implemented

	g_ax_spi_packet_buffer[idx++] =  ':';
	g_ax_spi_packet_buffer[idx++] =  '-';
	g_ax_spi_packet_buffer[idx++] =  ')';
	g_ax_spi_packet_buffer[idx++] =  '\r';

	/* Set length for FIFO DATA command */
	g_ax_spi_packet_buffer[    2] = idx - 3;												// Length

	/* FIFO data enter */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, idx);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	/* FIFO do a COMMIT */
	spi_ax_transport(false, "< a8 04 >");													// WR address 0x28: FIFOCMD - AX_FIFO_CMD_COMMIT
}

void spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal()
{
	/* Enter an APRS UI frame */

	/* 1 - Flags */
	spi_ax_util_PR1200_Tx_FIFO_Flags(35);													// Minimal

	/* 2 - Address field, Control and PID */
	spi_ax_test_PR1200_Tx_FIFO_Lev2_minimal_AddressField();
}


/* PR1200 RX */
void spi_ax_test_PR1200_Rx(void)
{
	/* Syncing and sending reset command, then setting the packet radio values for receiving */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_PR1200, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();

		#if 0
		/* Set VCO-PLL to FREQB */
		(void) spi_ax_selectVcoFreq(true);
		#else
		/* Set VCO-PLL to FREQA */
		(void) spi_ax_selectVcoFreq(false);
		#endif
	}

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	#if 0
	/* Enable the receiver */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_RX_WOR, AX_SET_REGISTERS_POWERMODE_WOR);

	#else
	/* Enable the receiver */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_RX_CONT, AX_SET_REGISTERS_POWERMODE_FULLRX);
	#endif

	/* Receive loop */
	spi_ax_Rx_FIFO();
}


/* POCSAG TX */
void spi_ax_test_POCSAG_Tx(void)
{ /* TEST: transmitting some packet */
	/* Syncing and sending reset command, then setting the packet radio values for transmission */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_POCSAG, AX_SET_REGISTERS_VARIANT_TX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();

		/* Switch to VCO-B */
		(void) spi_ax_selectVcoFreq(true);
	}

	/* Enabling the transmitter */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_FULLTX);

	/* Set power level */
	spi_ax_setPower_dBm(-20);


	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	for (uint16_t count = 3; count; count--) {
		do {
			/* FIFOSTAT */
			spi_ax_transport(false, "< 28 R1 >");
		} while (!(g_ax_spi_packet_buffer[0] & 0x01));

		/* Enter POCSAG preamble */
		spi_ax_util_POCSAG_Tx_FIFO_Preamble();

		/* Enter POCSAG batches with message to destination RIC */
		int32_t targetRIC	= 2030000UL;
		const char msgBuf[]	= "DF4IAH: This is a demonstration message to my  RIC 2030000  using 80 characters.";
		(void) spi_ax_util_POCSAG_Tx_FIFO_Batches(targetRIC, AX_POCSAG_CW2_MODE3_ALPHANUM, msgBuf, strlen(msgBuf));

		delay_ms(2000);
		//delay_ms(7500);
	}

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	do {
		/* RADIOSTATE */
		spi_ax_transport(false, "< 1c R1 >");													// RD Address 0x1C: RADIOSTATE - IDLE
	} while ((g_ax_spi_packet_buffer[0] & 0x0f) != 0);

	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_NO_CHANGE, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	while (true) {
		nop();
	}
}

/* POCSAG RX */
void spi_ax_test_POCSAG_Rx(void)
{
	/* Syncing and sending reset command, then setting the packet radio values for receiving */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_POCSAG, AX_SET_REGISTERS_VARIANT_RX, AX_SET_REGISTERS_POWERMODE_POWERDOWN);

	/* Frequency settings */
	{
		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		/* Recall ranging values */
		spi_ax_doRanging();

		/* Set VCO-PLL to FREQB */
		(void) spi_ax_selectVcoFreq(true);
	}

	/* FIFOCMD / FIFOSTAT */
	spi_ax_transport(false, "< a8 03 >");														// WR address 0x28: FIFOCMD - AX_FIFO_CMD_CLEAR_FIFO_DATA_AND_FLAGS

	#if 0
	/* Enable the receiver */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_RX_WOR, AX_SET_REGISTERS_POWERMODE_WOR);

	#else
	/* Enable the receiver */
	spi_ax_setRegisters(false, AX_SET_REGISTERS_MODULATION_NO_CHANGE, AX_SET_REGISTERS_VARIANT_RX_CONT, AX_SET_REGISTERS_POWERMODE_FULLRX);
	#endif

	/* Receive loop */
	spi_ax_Rx_FIFO();
}
