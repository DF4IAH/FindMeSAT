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


bool spi_ax_transport(bool isProgMem, const char* packet)
{
	uint8_t						l_axIdx								= 0;
	uint16_t					l_fmtIdx							= 0;
	SPI_AX_TRPT_STATE_ENUM_t	l_state								= SPI_AX_TRPT_STATE_DISABLED;
	char						l_msg_buf[C_SPI_AX_BUFFER_LENGTH]	= { 0 };
	const char					*l_fmtPtr							= isProgMem ?  l_msg_buf : packet;
	bool						l_isRead							= false;
	uint8_t						l_adr0 = 0, l_adr1 = 0;

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

uint32_t spi_ax_calcFrequency_Mhz2Regs(float f_mhz)
{
	const float xtal_hz = 16E+6f;																// XTAL = 16 MHz
	const float reg_per_mhz = (1LL << 24) * 1E+6f / xtal_hz;
	return (uint32_t) (0.5f + f_mhz * reg_per_mhz);
}

void spi_ax_setFrequency2Regs(uint8_t chan, bool isFreqB)
{
	uint32_t f_reg;
	uint8_t l_packet[5];

	if (chan > 2) {
		/* ERROR */
		return;
	}
	f_reg = g_ax_spi_freq_chan[chan];

	/* Prepare packet */
	l_packet[0] = isFreqB ?  0xBC : 0xB4;														// WR Address 0x34 or 0x3C
	l_packet[1] = (uint8_t) (f_reg >> 24) & 0xff;
	l_packet[2] = (uint8_t) (f_reg >> 16) & 0xff;
	l_packet[3] = (uint8_t) (f_reg >>  8) & 0xff;
	l_packet[4] = (uint8_t) (f_reg      ) & 0xff;

	/* Send packet */
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	spi_write_packet(&SPI_AX, l_packet, sizeof(l_packet));
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
}

void spi_ax_initRegisters_PR1200(void)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 08 >");														// WR address 0x10: MODULATION - 08: FSK

	/* ENCODING */
	spi_ax_transport(false, "< 91 00 >");														// WR address 0x11: ENCODING

	/* FRAMING */
	spi_ax_transport(false, "< 91 26 >");														// WR address 0x12: FRAMING

	/* PINFUNCTCXO_EN */
	spi_ax_transport(false, "< a6 05 >");														// WR address 0x26: PINFUNCTCXO_EN - Use TCXO_EN pin as DAC output

	/* WAKEUPXOEARLY */
	spi_ax_transport(false, "< ee 01 >");														// WR address 0x6E: WAKEUPXOEARLY

	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 00 cd >");													// WR address 0x100: IFFREQ

	/* DECIMATION */
	spi_ax_transport(false, "< f1 02 25 >");													// WR address 0x102: DECIMATION

	/* RXDATARATE */
	spi_ax_transport(false, "< f1 03 00 b4 2e >");												// WR address 0x103: RXDATARATE

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 07 5f >");												// WR address 0x109: MAXRFOFFSET

	/* FSKDMAX */
	spi_ax_transport(false, "< f1 0c 03 f3 >");													// WR address 0x10C: FSKDMAX

	/* FSKDMIN */
	spi_ax_transport(false, "< f1 0e ff 0d >");													// WR address 0x10E: FSKDMIN

	/* AMPLFILTER */
	spi_ax_transport(false, "< f1 15 00 >");													// WR address 0x115: AMPLFILTER

	#if 0
	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 00 >");													// WR address 0x116: FREQUENCYLEAK
	#endif

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 f4 >");													// WR address 0x117: RXPARAMSETS

	/* AGCGAIN0 */
	spi_ax_transport(false, "< f1 20 e8 >");													// WR address 0x120: AGCGAIN0

	/* AGCTARGET0 */
	spi_ax_transport(false, "< f1 21 84 >");													// WR address 0x121: AGCTARGET0

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
	spi_ax_transport(false, "< f1 50 84 >");													// WR address 0x151: AGCTARGET3

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
	spi_ax_transport(false, "< f1 5f 00 >");													// WR address 0x15F: BBOFFSRES3

	/* MODCFGF */
	spi_ax_transport(false, "< f1 60 00 >");													// WR address 0x160: MODCFGF

	/* FSKDEV */
	spi_ax_transport(false, "< f1 61 00 02 0c >");												// WR address 0x161: FSKDEV

	/* MODCFGA */
	spi_ax_transport(false, "< f1 64 05 >");													// WR address 0x164: MODCFGA

	/* TXRATE  0x00 0x04 0xEA */
	spi_ax_transport(false, "< f1 65 00 04 ea >");												// WR address 0x165: FSKDEV

	/* TXPWRCOEFFB */
	spi_ax_transport(false, "< f1 6a 00 aa >");													// WR address 0x16A: TXPWRCOEFFB

	/* PLLVCOI */
	spi_ax_transport(false, "< f1 80 99 >");													// WR address 0x180: PLLVCOI

	/* PLLRNGCLK */
	spi_ax_transport(false, "< f1 83 03 >");													// WR address 0x183: PLLRNGCLK

	/* BBTUNE */
	spi_ax_transport(false, "< f1 88 0f >");													// WR address 0x188: BBTUNE

	/* BBOFFSCAP */
	spi_ax_transport(false, "< f1 89 77 >");													// WR address 0x189: BBOFFSCAP

	/* PKTADDRCFG */
	spi_ax_transport(false, "< f2 00 00 >");													// WR address 0x200: PKTADDRCFG

	/* PKTLENCFG */
	spi_ax_transport(false, "< f2 01 00 >");													// WR address 0x201: PKTLENCFG

	/* PKTMAXLEN */
	spi_ax_transport(false, "< f2 03 f0 >");													// WR address 0x203: PKTMAXLEN

	/* MATCH0PAT */
	spi_ax_transport(false, "< f2 10 7e 00 00 00 >");											// WR address 0x210: MATCH0PAT

	/* MATCH0LEN */
	spi_ax_transport(false, "< f2 14 87 >");													// WR address 0x214: MATCH0LEN

	/* MATCH0MAX */
	spi_ax_transport(false, "< f2 16 07 >");													// WR address 0x216: MATCH0MAX

	/* MATCH1PAT */
	spi_ax_transport(false, "< f2 18 7e 7e >");													// WR address 0x218: MATCH1PAT

	/* MATCH1LEN */
	spi_ax_transport(false, "< f2 1c 8a >");													// WR address 0x21C: MATCH1LEN

	/* MATCH1MAX */
	spi_ax_transport(false, "< f2 1e 0a >");													// WR address 0x21E: MATCH1MAX

	/* TMGTXBOOST */
	spi_ax_transport(false, "< f2 20 32 >");													// WR address 0x220: TMGTXBOOST

	/* TMGTXSETTLE */
	spi_ax_transport(false, "< f2 21 14 >");													// WR address 0x221: TMGTXSETTLE

	/* TMGRXBOOST */
	spi_ax_transport(false, "< f2 23 32 >");													// WR address 0x223: TMGRXBOOST

	/* TMGRXSETTLE */
	spi_ax_transport(false, "< f2 24 14 >");													// WR address 0x224: TMGRXSETTLE

	/* TMGRXOFFSACQ */
	spi_ax_transport(false, "< f2 25 00 >");													// WR address 0x225: TMGRXOFFSACQ

	/* TMGRXCOARSEAGC */
	spi_ax_transport(false, "< f2 26 73 >");													// WR address 0x226: TMGRXCOARSEAGC

	/* TMGRXRSSI */
	spi_ax_transport(false, "< f2 28 03 >");													// WR address 0x228: TMGRXRSSI

	/* TMGRXPREAMBLE2 */
	spi_ax_transport(false, "< f2 2a 12 >");													// WR address 0x22A: TMGRXPREAMBLE2

	/* RSSIREFERENCE */
	spi_ax_transport(false, "< f2 2c f8 >");													// WR address 0x22C: RSSIREFERENCE - RSSI Offset, this register adds a constant offset to the computed RSSI value. It is used to compensate for board effects.

	/* RSSIABSTHR */
	spi_ax_transport(false, "< f2 2d dd >");													// WR address 0x22D: RSSIABSTHR

	/* BGNDRSSITHR */
	spi_ax_transport(false, "< f2 2f 00 >");													// WR address 0x22F: BGNDRSSITHR

	/* PKTCHUNKSIZE */
	spi_ax_transport(false, "< f2 30 0d >");													// WR address 0x230: PKTCHUNKSIZE

	/* PKTACCEPTFLAGS */
	spi_ax_transport(false, "< f2 33 20 >");													// WR address 0x233: PKTACCEPTFLAGS

	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 02 >");													// WR address 0x332: DACCONFIG

	/* 0xF10 - XTALOSC*/
	spi_ax_transport(false, "< ff 10 03 >");													// WR address 0xF10: XTALOSC

	/* 0xF11 - XTALAMPL */
	spi_ax_transport(false, "< ff 11 07 >");													// WR address 0xF11: XTALAMPL
}

void spi_ax_initRegisters_PR1200_Tx(void)
{
	/* PLLLOOP */
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

	#if defined(AX_VCO_INTERNAL)
	/* PLLVCODIV */
	spi_ax_transport(false, "< b2 04 >");														// WR address 0x32: PLLVCODIV - RFDIV
	#else
	/* PLLVCODIV */
	spi_ax_transport(false, "< b2 34 >");														// WR address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV
	#endif

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
	spi_ax_transport(false, "< b0 0a >");														// WR address 0x30: PLLLOOP

	/* PLLCPI */
	spi_ax_transport(false, "< b1 10 >");														// WR address 0x31: PLLCPI

#if defined(AX_VCO_INTERNAL)
	/* PLLVCODIV */
	spi_ax_transport(false, "< b2 04 >");														// WR address 0x32: PLLVCODIV - RFDIV
#else
	/* PLLVCODIV */
	spi_ax_transport(false, "< b2 34 >");														// WR address 0x32: PLLVCODIV - VCO2INT, VCOSEL, RFDIV
#endif

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
	spi_ax_transport(false, "< ff 72 00 >");													// WR address 0xF72 (RX) - Set to 0x06 if the framing mode is set to “Raw, Soft Bits” (register FRAMING), or to 0x00 otherwise
}

void spi_ax_initRegisters_PR1200_Rx_WoR(void)
{
	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 1c >");													// WR address 0x227: TMGRXAGC

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 19 >");													// WR address 0x229: TMGRXPREAMBLE1

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 03 >");													// WR address 0x231: PKTMISCFLAGS
}

void spi_ax_initRegisters_PR1200_Rx_cont(void)
{
	/* TMGRXAGC */
	spi_ax_transport(false, "< f2 27 00 >");													// WR address 0x227: TMGRXAGC

	/* TMGRXPREAMBLE1 */
	spi_ax_transport(false, "< f2 29 00 >");													// WR address 0x229: TMGRXPREAMBLE1

	/* PKTMISCFLAGS */
	spi_ax_transport(false, "< f2 31 00 >");													// WR address 0x231: PKTMISCFLAGS
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

void spi_ax_initRegisters_AnlogFM(bool isFull)
{
	/* MODULATION */
	spi_ax_transport(false, "< 90 08 >");														// WR address 0x10: MODULATION - 08: FSK

	/* PINFUNCTCXO_EN */
	spi_ax_transport(false, "< a6 05 >");														// WR address 0x26: PINFUNCTCXO_EN - Use TCXO_EN pin as DAC output

	/* IFFREQ */
	spi_ax_transport(false, "< f1 00 00 cd >");													// WR address 0x100: IFFREQ

	/* MAXDROFFSET */
	spi_ax_transport(false, "< f1 06 00 00 00 >");												// WR address 0x106: MAXDROFFSET - off

	/* MAXRFOFFSET */
	spi_ax_transport(false, "< f1 09 80 07 5f >");												// WR address 0x109: MAXRFOFFSET - track at LO1, max 50 kHz @ f_xtal = 16 MHz

	/* FREQUENCYLEAK */
	spi_ax_transport(false, "< f1 16 04 >");													// WR address 0x116: FREQUENCYLEAK - FREQUENCYGAINB0 + 2, prevents the demodulator AFC loop from tracking static frequency offsets

	/* RXPARAMSETS */
	spi_ax_transport(false, "< f1 17 00 >");													// WR address 0x117: RXPARAMSETS

	/* TIMEGAIN0 */
	spi_ax_transport(false, "< f1 24 ba >");													// WR address 0x124: TIMEGAIN0

	/* DRGAIN0 */
	spi_ax_transport(false, "< f1 25 b4 >");													// WR address 0x125: DRGAIN0

	/* FREQGAINA0 */
	spi_ax_transport(false, "< f1 27 0f >");													// WR address 0x127: FREQGAINA0

	/* FREQGAINB0 */
	spi_ax_transport(false, "< f1 28 02 >");													// WR address 0x128: FREQGAINB0 - bandwidth of “inner” AFC loop

	/* FREQGAINC0 */
	spi_ax_transport(false, "< f1 29 1f >");													// WR address 0x129: FREQGAINC0

	/* FREQGAIND0 */
	spi_ax_transport(false, "< f1 2a 08 >");													// WR address 0x12A: FREQGAIND0 - bandwidth of “outer” AFC loop (tracking frequency mismatch)

	/* TIMEGAIN1 */
	spi_ax_transport(false, "< f1 34 b8 >");													// WR address 0x134: TIMEGAIN1

	/* DRGAIN1 */
	spi_ax_transport(false, "< f1 35 b3 >");													// WR address 0x135: DRGAIN1

	/* DACCONFIG */
	spi_ax_transport(false, "< f3 32 03 >");													// WR address 0x332: DACCONFIG - output TRKFREQUENCY (= demodulated signal) on DAC

	/* DACVALUE */
	spi_ax_transport(false, "< f3 30 00 0c >");													// WR address 0x330: DACVALUE - DACSHIFT = 12 bit. This gives maximum volume, downshifting further gives smaller volume

	/* 0xF18 */
	spi_ax_transport(false, "< ff 18 06 >");													// WR address 0xF18 (RX/TX) - ? (is set to 0x06, explicit named for using Analog FM)


	if (isFull) {
		/* PWRMODE */
		spi_ax_transport(false, "< 82 09 >");													// WR address 0x02: PWRMODE - FULL RX
	}
}

#if defined(AX_GPADC13_ENABLED)
static void spi_ax_adcCtrlSet(uint8_t val)
{
	g_ax_spi_packet_buffer[0] = 0xF3;															// WR Address 0x300: GPADCCTRL
	g_ax_spi_packet_buffer[1] = 0x30;
	g_ax_spi_packet_buffer[2] = val;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);
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
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);									// Write back with modified value

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


	/*
	The following list explains the typical programming flow.
	Preparation:

	1. Reset the Chip. Set SEL to high for at least 1?s,
	then low. Wait until MISO goes high. Set, and then
	clear, the RST bit of register PWRMODE.

	2. Set the PWRMODE register to POWERDOWN.
	*/
	{
		/* SEL line and MISO check */
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
		delay_us(1);
		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);

		while (!ioport_get_pin_level(AX_MISO_PIN))
			;

		/* Set RESET */
		spi_ax_transport(false, "< 82 80 >");													// WR address 0x02: PWRMODE - RESET, PWRMODE=Powerdown

		/* Clear RESET and Powerdown */
		spi_ax_transport(false, "< 82 00 >");													// WR address 0x02: PWRMODE - RESET, PWRMODE=Powerdown
	}

	/*
	3. Program parameters. It is recommended that
	suitable parameters are calculated using the
	AX_RadioLab tool available from ON Semiconductors (former Axsem).
	*/
	{
		/* Setting default values */
		spi_ax_initRegisters_PR1200();
		spi_ax_initRegisters_PR1200_Rx();

		#if defined(AX_VCO_INTERNAL)
		/* VCO A/B settings */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(433.800);							// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x09
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(439.025);							// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x09

		//g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(400.000);						// VCO1 (internal without ext. L) with RFDIV --> VCORA = 0x0e
		//g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(525.000);						// VCO1 (internal without ext. L) with RFDIV --> VCORB = 0x02

		#else
		/* VCO A/B settings */
		g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(144.800);							// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x05
		g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(145.625);							// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x04

		//g_ax_spi_freq_chan[0] = spi_ax_calcFrequency_Mhz2Regs(137.000);						// VCO2 (internal with    ext. L) with RFDIV --> VCORA = 0x0e
		//g_ax_spi_freq_chan[1] = spi_ax_calcFrequency_Mhz2Regs(149.000);						// VCO2 (internal with    ext. L) with RFDIV --> VCORB = 0x01
		#endif

		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);
	}

	/*
	4. Perform auto-ranging, to ensure the correct VCO
	range setting.
	The chip is now ready for transmit and receive operations
	*/
	{
		/* Default 100kHz loop BW for ranging */
		/* PLLLOOP, PLLCPI */
		spi_ax_transport(false, "< b0 09 08 >");												// WR address 0x30: PLLLOOP -
																								// WR address 0x31: PLLCPI -

		//  Set PWRMODE to STANDBY, Enable TCXO if used
		spi_ax_transport(false, "< 82 65 >");													// WR Address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=STANDBY Crystal Oscillator enabled
		//spi_ax_transport(false, "< f0 02 65 >");												// WR Address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=STANDBY Crystal Oscillator enabled (by long address access)

		//  Wait until crystal oscillator is ready
		do {
			spi_ax_transport(false, "< 1d R1 >");												// RD Address 0x1D: XTALSTATUS
		} while (!(g_ax_spi_packet_buffer[0] & 0x01));											// Bit 0: XTALRUN

		spi_ax_transport(false, "< R2 >");


		/* MODULATION */
		spi_ax_transport(false, "< 90 08 >");													// WR address 0x10: MODULATION - 08: FSK

		/* FSKDEV */
		spi_ax_transport(false, "< f1 61 00 00 00 >");											// WR address 0x161: FSKDEV - off


		//	Auto-ranging starts at the VCOR
		//	(register PLLRANGINGA or PLLRANGINGB) setting;
		//	if you already know the approximately correct synthesizer
		//	VCO range, you should set VCORA/VCORB to this value
		//	prior to starting auto-ranging; this can speed up the ranging
		//	process considerably. If you have no prior knowledge about
		//	the correct range, set VCORA/VCORB to 8. Starting with
		//	VCORA/VCORB < 6 should be avoided, as the initial
		//	synthesizer frequency can exceed the maximum frequency
		//	specification.

		// VCORA to 0x08
		spi_ax_transport(false, "< b3 18 >");													// WR Address 0x33: PLLRANGINGA - Bit 4: RNGSTART, ranging start value = 8

		//	Hardware clears the RNG START bit automatically as
		//	soon as the ranging is finished; the device may be
		//	programmed to deliver an interrupt on resetting of the RNG
		//	START bit.
		//	Waiting until auto-ranging terminates can be performed
		//	by either polling the register PLLRANGINGA or
		//	PLLRANGINGB for RNG START to go low, or by enabling
		//	the IRQMPLLRNGDONE interrupt in register
		//	IRQMASK1.
		do {
			spi_ax_transport(false, "< 33 R1 >");												// RD Address 0x33: PLLRANGINGA
		} while (g_ax_spi_packet_buffer[0] & _BV(4));											// Bit 4: RNGSTART
		g_ax_spi_range_chan[0] = g_ax_spi_packet_buffer[0];

		if (g_ax_spi_packet_buffer[0] & _BV(5)) {
			/* Ranging error - reset to center */
			spi_ax_transport(false, "< b3 08 >");
		}

		// VCORB to 0x08
		spi_ax_transport(false, "< bb 18 >");													// WR Address 0x3B: PLLRANGINGB - Bit 4: RNGSTART, ranging start value = 8

		do {
			spi_ax_transport(false, "< 3b R1 >");												// RD Address 0x3B: PLLRANGINGB
		} while (g_ax_spi_packet_buffer[0] & _BV(4));											// Bit 4: RNGSTART
		g_ax_spi_range_chan[1] = g_ax_spi_packet_buffer[0];

		if (g_ax_spi_packet_buffer[0] & _BV(5)) {
			/* Ranging error - reset to center */
			spi_ax_transport(false, "< bb 08 >");
		}


		/* VCOI Manual calibration when GPADC13 is attached to the VCO control voltage, available at FILT in external loop filter mode */
		#if defined(AX_GPADC13_ENABLED)

		spi_ax_initRegistersTx();

		/* MODULATION */
		spi_ax_transport(false, "< 90 08 >");													// WR address 0x10: MODULATION - 08: FSK

		/* FSKDEV */
		spi_ax_transport(false, "< f1 61 00 00 00 >");											// WR address 0x161: FSKDEV - off

		/* FSKDEV */
		spi_ax_transport(false, "< 30 R1 >");													// RD address 0x38: PLLLOOP - 04: FILTEN
		g_ax_spi_packet_buffer[1] = 0x04 | g_ax_spi_packet_buffer[0];
		g_ax_spi_packet_buffer[0] = 0x30 | 0x80;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);									// Write back with modified value

		/* 0xF35 */
        {
			spi_ax_transport(false, "< 7f 35 R1 >");											// RD address 0xF35 (RX/TX) - Set to 0x10 for reference frequencies (crystal or TCXO) less than 24.8 MHz (fXTALDIV = 1), or to 0x11 otherwise (fXTALDIV = 2)
            uint8_t new_0xF35 = 0x80 | g_ax_spi_packet_buffer[0];

            if (0x02 & (uint8_t)~new_0xF35) {
                ++new_0xF35;
			}

			g_ax_spi_packet_buffer[0] = 0xFF;
			g_ax_spi_packet_buffer[1] = 0x35;
            g_ax_spi_packet_buffer[2] = new_0xF35;
			spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 3);								// Write back with modified value
        }

		spi_ax_transport(false, "< 82 6c >");													// WR Address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=SYNTH_TX

		{
			/* PLLVCOI */
			spi_ax_transport(false, "< 71 80 R1 >");											// RD address 0x180: PLLVCOI
			uint8_t vcoi_save = g_ax_spi_packet_buffer[0];

            uint8_t dropFirstCalIdx = 2;
            for (uint8_t chanIdx = 0; chanIdx < 2; chanIdx++) {
				g_ax_spi_vcoi_chan[chanIdx] = 0;

                if (g_ax_spi_range_chan[chanIdx] & 0x20) {
                    continue;
				}

				g_ax_spi_packet_buffer[0] = 0x33 | 0x80;										// WR Address 0x33: PLLRANGINGA
				g_ax_spi_packet_buffer[1] = g_ax_spi_range_chan[0] & 0x0f;
				spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);

				spi_ax_setFrequency2Regs(chanIdx, chanIdx == 1 ?  true : false);

                do {
					g_ax_spi_vcoi_chan[chanIdx] = s_spi_ax_cal_vcoi();
                } while (--dropFirstCalIdx);
                dropFirstCalIdx = 1;
            }

			/* Revert to initial setting */
			g_ax_spi_packet_buffer[0] = 0x71 | 0x80;											// WR Address 0x180: PLLVCOI
			g_ax_spi_packet_buffer[1] = 0x80;
			g_ax_spi_packet_buffer[2] = vcoi_save;
			spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
		}

		/* Restore settings and set range vars */

		spi_ax_transport(false, "< 82 00 >");													// WR Address 0x02: PWRMODE - PWRMODE=POWERDOWN

		spi_ax_initRegisters();
		spi_ax_initRegistersRx();

		spi_ax_setFrequency2Regs(0, false);
		spi_ax_setFrequency2Regs(1, true);

		uint8_t val = g_ax_spi_range_chan[0];
		if (val & 0x20) {
			val = 0x08;
		}
		g_ax_spi_packet_buffer[0] = 0x33 | 0x80;												// WR Address 0x33: PLLRANGINGA
		g_ax_spi_packet_buffer[1] = val & 0x0f;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);

		val = g_ax_spi_range_chan[1];
		if (val & 0x20) {
			val = 0x08;
		}
		g_ax_spi_packet_buffer[0] = 0x3B | 0x80;												// WR Address 0x3B: PLLRANGINGB
		g_ax_spi_packet_buffer[1] = val & 0x0f;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
		#endif  // VCOI Calibration


		/* PWRMODE=SYNTHRX */
		spi_ax_transport(false, "< 82 68 >");													// WR address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=SYNTHRX
		delay_us(40);

		/* PWRMODE=FULLRX */
		spi_ax_transport(false, "< 82 69 >");													// WR address 0x02: PWRMODE - XOEN, REFEN, PWRMODE=FULLRX

		/* Set VCO-PLL to FREQA */
		/* PLLLOOP */
		spi_ax_transport(false, "< 30 R1 >");													// WR address 0x30: PLLLOOP - not FREQB
		g_ax_spi_packet_buffer[1] = g_ax_spi_packet_buffer[0] & 0x7f;
		g_ax_spi_packet_buffer[0] = 0xB0;

		/* BGNDRSSI */
		spi_ax_transport(false, "< C1 0C >");													// WR Address 0x41: BGNDRSSI

		/* RSSIABSTHR */
		spi_ax_transport(false, "< F2 2D A0 >");												// WR Address 0x22D: RSSIABSTHR

		volatile uint8_t curRssi = 0;
		volatile int16_t curRssiCalc = 0;
		volatile uint8_t curBgndRssi = 0;
		volatile uint8_t curAgcCounter = 0;
		volatile uint16_t curTrkAmpl = 0;
		volatile uint32_t curTrkRfFreq = 0;
		volatile uint16_t curTrkFreq = 0;
		while (true) {
			/* RSSI, BGNDRSSI */
			spi_ax_transport(false, "< 40 R2 >");												// RD Address 0x40: RSSI, BGNDRSSI
			curRssi			= g_ax_spi_packet_buffer[0];
			curRssiCalc		= (int16_t)(int8_t)curRssi - 64;
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

			nop();
			(void)  curRssi;
			(void)  curRssiCalc;
			(void)  curBgndRssi;
			(void)  curAgcCounter;
			(void)  curTrkAmpl;
			(void)  curTrkRfFreq;
			(void)  curTrkFreq;
		}
	}


	/* TEST CODES */

#if defined(AX_FM_WIDE_RX)
	/* Settings for analog FM mode */
	spi_ax_setModulationFM(false);
	barrier();

#elif defined(AX_TEST01)
	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x00;
	g_ax_spi_packet_buffer[1]	= 0x00;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);										// Address 0x00: silicon revision
	spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x80 | 0x01;
	g_ax_spi_packet_buffer[1]	= 0x55;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);										// Address 0x01: scratch register R/W - set
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

	spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
	g_ax_spi_packet_buffer[0]	= 0x01;
	g_ax_spi_packet_buffer[1]	= 0x00;
	spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);										// Address 0x01: scratch register R/W - check
	spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
	spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);
	nop();
#endif




	while (true) {
		/* Read AGCCOUNTER */
		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
		g_ax_spi_packet_buffer[0]	= 0x43;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
		memset(g_ax_spi_packet_buffer, 0, sizeof(g_ax_spi_packet_buffer));
		spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

		/* Read RSSI */
		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
		g_ax_spi_packet_buffer[0]	= 0x40;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
		memset(g_ax_spi_packet_buffer, 0, sizeof(g_ax_spi_packet_buffer));
		spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

		/* Read TRK_AMPLITUDE */
		spi_select_device(&SPI_AX, &g_ax_spi_device_conf);
		g_ax_spi_packet_buffer[0]	= 0x48;
		spi_write_packet(&SPI_AX, g_ax_spi_packet_buffer, 1);
		memset(g_ax_spi_packet_buffer, 0, sizeof(g_ax_spi_packet_buffer));
		spi_read_packet(&SPI_AX, g_ax_spi_packet_buffer, 2);
		spi_deselect_device(&SPI_AX, &g_ax_spi_device_conf);

		nop();
	}
}
