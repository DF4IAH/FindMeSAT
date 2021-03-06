/**
  ******************************************************************************
  * File Name          : SPI.h
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __spi_H
#define __spi_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "LoRaWAN.h"

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN Private defines */
#define SPI1_BUFFERSIZE 64

#define SPI_WR_FLAG   (1 << 7)
#define SPI_RD_FLAG   (0 << 7)

typedef enum spiSX1272_Mode {
  TXRX_MODE_MASK          = 0x0f,
  SLEEP                   = (0b000 << 0),
  STANDBY                 = (0b001 << 0),
  FSTX                    = (0b010 << 0),
  TX                      = (0b011 << 0),
  FSRX                    = (0b100 << 0),
  RXCONTINUOUS            = (0b101 << 0),
  RXSINGLE                = (0b110 << 0),
  CAD                     = (0b111 << 0),

  /* Non-LoRa Mode */
  MOD_TYPE_FSK            = (0b00 << 5),
  MOD_TYPE_OOK            = (0b01 << 5),

  /* LoRa Mode */
  ACCESS_SHARE_OFF        = (0b0 << 6),
  ACCESS_SHARE_ON         = (0b1 << 6),

  MODE_FSK_OOK            = (0b0 << 7),
  MODE_LoRa               = (0b1 << 7)
} spiSX1272_Mode_t;

typedef enum spiSX1272_ModemConfig1 {
  LOW_DR_OPTI_OFF         = (0b0 << 0),
  LOW_DR_OPTI_ON          = (0b1 << 0),

  RX_PAYLOAD_CRC_OFF      = (0b0 << 1),
  RX_PAYLOAD_CRC_ON       = (0b1 << 1),

  IHM_OFF                 = (0b0 << 2),
  IHM_ON                  = (0b1 << 2),

  CR_4_5                  = (0b001 << 3),
  CR_4_6                  = (0b010 << 3),
  CR_4_7                  = (0b011 << 3),
  CR_4_8                  = (0b100 << 3),

  BW_125kHz               = (0b00 << 6),
  BW_250kHz               = (0b01 << 6),
  BW_500kHz               = (0b10 << 6)
} spiSX1272_ModemConfig1_t;

typedef enum spiSX1272_ModemConfig2 {
  AGC_AUTO_OFF            = (0b0 << 2),
  AGC_AUTO_ON             = (0b1 << 2),

  TXCONT_OFF              = (0b0 << 3),
  TXCONT_ON               = (0b1 << 3),

  SFx_SHIFT               =         4 ,
  SF6_DR6_VAL             =    6      ,
  SF6_DR6                 = (  6 << 4),
  SF7_DR5_VAL             =    7      ,
  SF7_DR5                 = (  7 << 4),
  SF8_DR4_VAL             =    8      ,
  SF8_DR4                 = (  8 << 4),
  SF9_DR3_VAL             =    9      ,
  SF9_DR3                 = (  9 << 4),
  SF10_DR2_VAL            =   10      ,
  SF10_DR2                = ( 10 << 4),
  SF11_DR1_VAL            =   11      ,
  SF11_DR1                = ( 11 << 4),
  SF12_DR0_VAL            =   12      ,
  SF12_DR0                = ( 12 << 4)
} spiSX1272_ModemConfig2_t;

typedef enum spiSX127x_DetectOptimize {
  OPTI_SF7_to_SF12        = (0b011 << 0),
  OPTI_SF6                = (0b101 << 0)
} spiSX127x_DetectOptimize_t;

typedef enum spiSX127x_DetectThreshold {
  THRESH_SF7_to_SF12      = 0x0A,
  THRESH_SF6              = 0x0C
} spiSX127x_DetectThreshold_t;


typedef enum spiSX127x_PaRamp {
  PA_RAMP_3ms4            = (0b0000 << 0),
  PA_RAMP_2ms             = (0b0001 << 0),
  PA_RAMP_1ms             = (0b0010 << 0),
  PA_RAMP_500us           = (0b0011 << 0),
  PA_RAMP_250us           = (0b0100 << 0),
  PA_RAMP_125us           = (0b0101 << 0),
  PA_RAMP_100us           = (0b0110 << 0),
  PA_RAMP_62us            = (0b0111 << 0),
  PA_RAMP_50us            = (0b1000 << 0),
  PA_RAMP_40us            = (0b1001 << 0),
  PA_RAMP_31us            = (0b1010 << 0),
  PA_RAMP_25us            = (0b1011 << 0),
  PA_RAMP_20us            = (0b1100 << 0),
  PA_RAMP_15us            = (0b1101 << 0),
  PA_RAMP_12us            = (0b1110 << 0),
  PA_RAMP_10us            = (0b1111 << 0),

  LOW_PWR_PLL_OFF         = (0b0 << 4),
  LOW_PWR_PLL_ON          = (0b1 << 4)
} spiSX127x_PaRamp_t;

typedef enum spiSX1272_LNA {
  LnaBoost_OFF            = (0b00 << 0),
  LnaBoost_ON             = (0b11 << 0),

  LnaGain_G1              = (0b001 << 5),
  LnaGain_G2              = (0b010 << 5),
  LnaGain_G3              = (0b011 << 5),
  LnaGain_G4              = (0b100 << 5),
  LnaGain_G5              = (0b101 << 5),
  LnaGain_G6              = (0b110 << 5)
} spiSX1272_LNA_t;

typedef enum spiSX127x_IRQ_Mask {
  CadDetectedMask         = 0,
  FhssChangeChannelMask   = 1,
  CadDoneMask             = 2,
  TxDoneMask              = 3,
  ValidHeaderMask         = 4,
  PayloadCrcErrorMask     = 5,
  RxDoneMask              = 6,
  RxTimeoutMask           = 7
} spiSX127x_IRQ_Mask_t;


typedef enum DIO_TxRx_Mode {
  DIO_TxRx_Mode_TX            = 0x01,
  DIO_TxRx_Mode_RX            = 0x11,
  DIO_TxRx_Mode_RX2           = 0x12,
  DIO_TxRx_Mode_RX_Randomizer = 0x1f,
  DIO_TxRx_Mode_IQ_Balancing  = 0x9e,
} DIO_TxRx_Mode_t;

/* USER CODE END Private defines */

void MX_SPI1_Init(void);

/* USER CODE BEGIN Prototypes */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

uint8_t spiProcessSpiReturnWait(void);
uint8_t spiProcessSpiMsg(uint8_t msgLen);

void spiSX127xReset(void);
void spiSX127xFrequency_MHz(float mhz);
void spiSX127xDio_Mapping(DIO_TxRx_Mode_t mode);
uint8_t spiSX127xDR_to_SF(DataRates_t dr);

uint8_t spiSX127xMode_LoRa_GetBroadbandRSSI(void);
void spiSX127xLoRa_setTxMsgLen(uint8_t payloadLen);
void spiSX127xLoRa_Fifo_Init(void);
void spiSX127xLoRa_Fifo_SetFifoPtrFromTxBase(void);
void spiSX127xLoRa_Fifo_SetFifoPtrFromRxBase(void);

uint8_t spiSX127xGetMode(void);
void spiSX1272Mode(spiSX1272_Mode_t mode);
void spiSX127xRegister_IRQ_clearAll(void);
void spiSX127xRegister_IRQ_enableBits(uint8_t enaBits);
void spiSX1272_TxRx_Preps(LoRaWANctx_t* ctx, DIO_TxRx_Mode_t mode, LoRaWAN_TX_Message_t* msg);
uint32_t spiSX127x_WaitUntil_TxDone(uint32_t stopTime);
void spiSX127x_WaitUntil_RxDone(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg, uint32_t stopTime1, uint32_t stopTime2);
void spiSX127x_Process_RxDone(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg);

uint8_t spiDetectShieldSX1272(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ spi_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
