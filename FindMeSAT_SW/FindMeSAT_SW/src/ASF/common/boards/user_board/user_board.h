/**
 * \file
 *
 * \brief FindMeSAT board header file.
 *
 * This file contains definitions and services related to the features of the
 * FindMeSAT board developped by Ulrich Habel, DF4IAH.
 *
 * To use this board define BOARD=USER_BOARD.
 *
 * Copyright (c) 2017 Ulrich Habel (DF4IAH). All rights reserved.
 *
 */
#ifndef USER_BOARD_H_
#define USER_BOARD_H_

#include <compiler.h>
//#include <conf_board.h>

#define MCU_SOC_NAME        "ATxmega256A3BU"
#define BOARD_NAME          "FindMeSAT"
/**
 * \defgroup findmesat_group FindMeSAT board
 * @{
 */

/**
 * \defgroup findmesat_feature_group Feature definitions
 * @{
 */

/**
 * \name VCTCXO
 *
 * The VCTCXO is a osciallator with an frequency of nearly 20 MHz.
 * That clock can be fine tuned by a pulling voltage resulting from
 * an RC low-pass circuit feeded by a PWM signal.
 */
//@{
#define XO_VC_PWM_GPIO                  IOPORT_CREATE_PIN(PORTC, 2)
#define XO_VC_PWM                       XO_VC_PWM_GPIO
//@}

/**
 * \name GSM
 *
 * The SIM808 is a GMS/BT/GPS device.
 */
//@{
#define GSM_RF_SYNC_GPIO                IOPORT_CREATE_PIN(PORTA, 6)
#define GSM_NETLIGHT_GPIO               IOPORT_CREATE_PIN(PORTA, 7)
#define GSM_PCM_SYNC_GPIO               IOPORT_CREATE_PIN(PORTD, 0)
#define GSM_PCM_CLK_DRV_GPIO            IOPORT_CREATE_PIN(PORTD, 1)
#define GSM_PCM_OUT_GPIO                IOPORT_CREATE_PIN(PORTD, 2)
#define GSM_PCM_IN_DRV_GPIO             IOPORT_CREATE_PIN(PORTD, 3)
#define POWERED_GSM_GPIO                IOPORT_CREATE_PIN(PORTE, 3)
#define PWRKEY_GSM_GPIO                 IOPORT_CREATE_PIN(PORTE, 4)
#define RESET_GSM_GPIO                  IOPORT_CREATE_PIN(PORTE, 5)
#define DTR1_GPIO                       IOPORT_CREATE_PIN(PORTF, 0)
#define GSM_DTR_GPIO                    IOPORT_CREATE_PIN(PORTF, 1)
#define RXD1_GPIO                       IOPORT_CREATE_PIN(PORTF, 2)
#define GSM_RXD_GPIO                    IOPORT_CREATE_PIN(PORTF, 3)
#define RI1_GPIO                        IOPORT_CREATE_PIN(PORTF, 4)
#define RTS1_GPIO                       IOPORT_CREATE_PIN(PORTF, 6)
#define GSM_RTS_GPIO                    IOPORT_CREATE_PIN(PORTF, 7)
#define GPS_1PPS_GPIO                   IOPORT_CREATE_PIN(PORTR, 0)

#define GSM_RF_SYNC                     GSM_RF_SYNC_GPIO
#define GSM_NETLIGHT                    GSM_NETLIGHT_GPIO
#define GSM_PCM_SYNC                    GSM_PCM_SYNC_GPIO
#define GSM_PCM_CLK_DRV                 GSM_PCM_CLK_DRV_GPIO
#define GSM_PCM_OUT                     GSM_PCM_OUT_GPIO
#define GSM_PCM_IN_DRV                  GSM_PCM_IN_DRV_GPIO
#define POWERED_GSM                     POWERED_GSM_GPIO
#define PWRKEY_GSM                      PWRKEY_GSM_GPIO
#define RESET_GSM                       RESET_GSM_GPIO
#define DTR1                            DTR1_GPIO
#define GSM_DTR                         GSM_DTR_GPIO
#define RXD1                            RXD1_GPIO
#define GSM_RXD                         GSM_RXD_GPIO
#define RI1                             RI1_GPIO
#define RTS1                            RTS1_GPIO
#define GSM_RTS                         GSM_RTS_GPIO
#define RESET                           RESET_GSM
//@}


/**
 * \name USB
 *
 * USB lines for controlling and communication with the USB port.
 */
//@{
#define USB_ID_GPIO                     IOPORT_CREATE_PIN(PORTB, 1)
#define USB_RESET_DRV_GPIO              IOPORT_CREATE_PIN(PORTD, 5)
#define USB_D_N_GPIO                    IOPORT_CREATE_PIN(PORTD, 6)
#define USB_D_P_GPIO                    IOPORT_CREATE_PIN(PORTD, 7)

#if 0
#define USB_ID                          USB_ID_GPIO
#define USB_RESET_DRV                   USB_RESET_DRV_GPIO
#define USB_D_N                         USB_D_N_GPIO
#define USB_D_P                         USB_D_P_GPIO
#endif
//@}


/**
 * \name I2C
 *
 * I2C bus lines of bus #1 and #2.
 */
//@{
#define SDA1_GPIO                       IOPORT_CREATE_PIN(PORTE, 0)
#define SCL1_GPIO                       IOPORT_CREATE_PIN(PORTE, 1)
#define SDA2_GPIO                       IOPORT_CREATE_PIN(PORTC, 0)
#define SCL2_GPIO                       IOPORT_CREATE_PIN(PORTC, 1)
#define I2C2_INT_GPIO                   IOPORT_CREATE_PIN(PORTB, 0)

#if 0
#define SDA1                            SDA1_GPIO
#define SCL1                            SCL1_GPIO
#define SDA2                            SDA2_GPIO
#define SCL2                            SCL2_GPIO
#define I2C2_INT                        I2C2_INT_GPIO
#endif
//@}


/**
 * \name AX5243 VHF transceiver
 *
 * SPI bus lines of the AX5243 device.
 */
//@{
#define AX_IRQ_GPIO                     IOPORT_CREATE_PIN(PORTC, 3)
#define AX_SEL_GPIO                     IOPORT_CREATE_PIN(PORTC, 4)
#define AX_MOSI_GPIO                    IOPORT_CREATE_PIN(PORTC, 5)
#define AX_MISO_GPIO                    IOPORT_CREATE_PIN(PORTC, 6)
#define AX_CLK_GPIO                     IOPORT_CREATE_PIN(PORTC, 7)

#define AX_IRQ                          AX_IRQ_GPIO
#define AX_SEL                          AX_SEL_GPIO
#define AX_MOSI                         AX_MOSI_GPIO
#define AX_MISO                         AX_MISO_GPIO
#define AX_CLK                          AX_CLK_GPIO
//@}


/**
 * \name JTAG
 *
 * JTAG lines.
 */
//@{
#define TMS_GPIO                        IOPORT_CREATE_PIN(PORTB, 4)
#define TDI_GPIO                        IOPORT_CREATE_PIN(PORTB, 5)
#define TCK_GPIO                        IOPORT_CREATE_PIN(PORTB, 6)
#define TDO_GPIO                        IOPORT_CREATE_PIN(PORTB, 7)

#if 0
#define TMS                             TMS_GPIO
#define TDI                             TDI_GPIO
#define TCK                             TCK_GPIO
#define TDO                             TDO_GPIO
#endif
//@}


/**
 * \name ADC
 *
 * ADC lines for the ADC part to see the outside world.
 */
//@{
#define AD_3V0_ADC_MODULE               ADCA
#define AD_3V0_ADC_INPUT                ADCCH_POS_PIN0
#define AD_3V0_SIGNAL_PIN               IOPORT_CREATE_PIN(PORTA, 0)

#define AD_VCTCXO_ADC_MODULE            ADCA
#define AD_VCTCXO_ADC_INPUT             ADCCH_POS_PIN1
#define AD_VCTCXO_SIGNAL_PIN            IOPORT_CREATE_PIN(PORTA, 1)

#define AD_5V0_ADC_MODULE               ADCA
#define AD_5V0_ADC_INPUT                ADCCH_POS_PIN2
#define AD_5V0_SIGNAL_PIN               IOPORT_CREATE_PIN(PORTA, 2)

#define AD_VBAT_ADC_MODULE              ADCA
#define AD_VBAT_ADC_INPUT               ADCCH_POS_PIN3
#define AD_VBAT_SIGNAL_PIN              IOPORT_CREATE_PIN(PORTA, 3)
//@}


/**
 * \name I/O
 *
 * I/O lines of the I/O header block.
 */
//@{
#define ADC4_ADC_MODULE                 ADCA
#define ADC4_ADC_INPUT                  ADCCH_POS_PIN4
#define ADC4_SIGNAL_PIN                 IOPORT_CREATE_PIN(PORTA, 4)

#define ADC5_ADC_MODULE                 ADCA
#define ADC5_ADC_INPUT                  ADCCH_POS_PIN5
#define ADC5_SIGNAL_PIN                 IOPORT_CREATE_PIN(PORTA, 5)

#define DAC0_ADC_MODULE                 ADCB
#define DAC0_ADC_INPUT                  ADCCH_POS_PIN2
#define DAC0_SIGNAL_PIN                 IOPORT_CREATE_PIN(PORTB, 2)

#define DAC1_ADC_MODULE                 ADCB
#define DAC1_ADC_INPUT                  ADCCH_POS_PIN3
#define DAC1_SIGNAL_PIN                 IOPORT_CREATE_PIN(PORTB, 3)

#define OC1A_GPIO                       IOPORT_CREATE_PIN(PORTD, 4)

#if 0
#define OC1A                            OC1A_GPIO
#endif
//@}


// External oscillator settings.
// Uncomment and set correct values if external oscillator is used.

// External oscillator frequency
#define BOARD_XOSC_HZ          20000000

// External oscillator type.
//!< External clock signal
#define BOARD_XOSC_TYPE        XOSC_TYPE_EXTERNAL
//!< 32.768 kHz resonator on TOSC
//#define BOARD_XOSC_TYPE        XOSC_TYPE_32KHZ
//!< 0.4 to 16 MHz resonator on XTALS
//#define BOARD_XOSC_TYPE        XOSC_TYPE_XTAL

// External oscillator startup time
#define BOARD_XOSC_STARTUP_US  100000

/**
 * @}
 */


/**
 * \defgroup findmesat_config_group Configuration options
 * @{
 */

#if defined(__DOXYGEN__)

/**
 * \name Initialization
 * \note Define these symbols in \ref conf_board.h to enable the corresponding
 * features.
 */
//@{

/**
 * \def CONF_BOARD_TODO
 * \brief Initialize (whatever)
 */
#  if !defined(CONF_BOARD_TODO)
#    define CONF_BOARD_TODO
#  endif

//@}

#endif // __DOXYGEN__

/**
 * @}
 */

/**
 * @}
 */

#endif /* USER_BOARD_H_ */
