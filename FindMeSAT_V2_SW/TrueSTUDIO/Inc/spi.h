/**
  ******************************************************************************
  * File Name          : SPI.h
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "stm32l4xx_hal.h"
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN Private defines */
#define SPI1_BUFFERSIZE 64

#define SPI_WR_FLAG   (1 << 7)
#define SPI_RD_FLAG   (0 << 7)

typedef enum spiSX1272_Mode {
  SLEEP                   = (0b000 << 0),
  STANDBY                 = (0b001 << 0),
  FSTX                    = (0b010 << 0),
  TX                      = (0b011 << 0),
  FSRX                    = (0b100 << 0),
  RXCONTINUOUS            = (0b101 << 0),
  RXSINGLE                = (0b110 << 0),
  CAD                     = (0b111 << 0),

  ACCES_SHARE_OFF         = (0b0 << 6),
  ACCES_SHARE_ON          = (0b1 << 6),

  MODE_FSK_OOK            = (0b0 << 7),
  MODE_LoRa               = (0b1 << 7)
} spiSX1272_Mode_t;

typedef enum spiSX1272_ModemConfig1 {
  LDRO_OFF                = (0b0 << 0),
  LDRO_ON                 = (0b1 << 0),

  PAYLOAD_CRC_OFF         = (0b0 << 1),
  PAYLOAD_CRC_ON          = (0b1 << 1),

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

  SF6_DR6                 = ( 6 << 4),
  SF7_DR5                 = ( 7 << 4),
  SF8_DR4                 = ( 8 << 4),
  SF9_DR3                 = ( 9 << 4),
  SF10_DR2                = (10 << 4),
  SF11_DR1                = (11 << 4),
  SF12_DR0                = (12 << 4)
} spiSX1272_ModemConfig2_t;

typedef enum spiSX1272_PaRamp {
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
} spiSX1272_PaRamp_t;

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

/* USER CODE END Private defines */

extern void _Error_Handler(char *, int);

void MX_SPI1_Init(void);

/* USER CODE BEGIN Prototypes */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

uint8_t  spiProcessSpiMsg(uint8_t msgLen);

float spiSX1272Calc_Channel_to_MHz(uint8_t channel);

void spiSX1272Frequency_MHz(float mhz);
void spiSX1272Dio_Mapping(void);

void spiSX1272LoRa_setTxMsgLen(uint8_t payloadLen);
void spiSX1272LoRa_Fifo_Init(void);
void spiSX1272LoRa_Fifo_SetRxBaseToFifoPtr(void);
void spiSX1272LoRa_Fifo_SetTxBaseToFifoPtr(void);

void spiSX1272Mode(spiSX1272_Mode_t mode);
void spiSX1272Mode_LoRa_TX_Preps(uint8_t channel, uint8_t msgLen);
void spiSX1272Mode_LoRa_TX_Run(void);
void spiSX1272Mode_LoRa_RX(uint8_t channel);

void spiSX1272_WaitUntil_TxDone(uint8_t doPreviousWakeTime);
void spiSX1272_WaitUntil_RxDone(uint32_t processUntil);

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
