/**
  ******************************************************************************
  * File Name          : SPI.c
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

/* Includes ------------------------------------------------------------------*/
#include "spi.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern EventGroupHandle_t spiEventGroupHandle;

/* Buffer used for transmission */
uint8_t aSpi1TxBuffer[SPI1_BUFFERSIZE] = { 0 };

/* Buffer used for reception */
uint8_t aSpi1RxBuffer[SPI1_BUFFERSIZE] = { 0 };

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    GPIO_InitStruct.Pin = SPI_A_SCK_Pin|SPI_A_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPI_A_MISO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(SPI_A_MISO_GPIO_Port, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_1;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(spiHandle,hdmatx,hdma_spi1_tx);

    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_1;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(spiHandle,hdmarx,hdma_spi1_rx);

    /* SPI1 interrupt Init */
    HAL_NVIC_SetPriority(SPI1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    HAL_GPIO_DeInit(GPIOA, SPI_A_SCK_Pin|SPI_A_MISO_Pin|SPI_A_MOSI_Pin);

    /* SPI1 DMA DeInit */
    HAL_DMA_DeInit(spiHandle->hdmatx);
    HAL_DMA_DeInit(spiHandle->hdmarx);

    /* SPI1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI1_IRQn);
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of DMA TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  BaseType_t taskWoken = 0;

  if (&hspi1 == hspi) {
    xEventGroupSetBitsFromISR(spiEventGroupHandle, SPI_SPI1_EG__RDY, &taskWoken);
  }
}

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  BaseType_t taskWoken = 0;

  if (&hspi1 == hspi) {
    xEventGroupSetBitsFromISR(spiEventGroupHandle, SPI_SPI1_EG__ERROR, &taskWoken);
  }
}


void spiProcessSpiMsg(uint8_t msgLen)
{
  HAL_StatusTypeDef status = 0;
  uint8_t errCnt = 0;

  aSpi1TxBuffer[msgLen] = 0;
  do {
    HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_RESET);

    status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t*) aSpi1TxBuffer, (uint8_t *) aSpi1RxBuffer, msgLen);
    if (status != HAL_OK)
    {
      HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);

      osDelay(1);

      if (++errCnt >= 100) {
        /* Transfer error in transmission process */
        Error_Handler();
      }
    }
  } while (status != HAL_OK);
  HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);
}

const uint16_t spiWait_MaxWaitEGMs = 500;
uint8_t spiProcessSpiReturnWait(void)
{
  uint8_t ret = 0;

  EventBits_t eb = xEventGroupWaitBits(spiEventGroupHandle, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, 0, spiWait_MaxWaitEGMs);
  if (eb & SPI_SPI1_EG__ERROR) {
    Error_Handler();
  } else if (eb & SPI_SPI1_EG__RDY) {
    ret = 1;
  }
  return ret;
}


float spiSX1272Calc_Channel_to_MHz(uint8_t channel)
{
  /* EU863-870*/
  float mhz = 0.f;

  switch (channel) {
  case 1:
    mhz = 868.1f;   // SF7BW125 to SF12BW125
    break;

  case 2:
    mhz = 868.3f;   // SF7BW125 to SF12BW125  and  SF7BW250
    break;

  case 3:
    mhz = 868.5f;   // SF7BW125 to SF12BW125
    break;

  case 4:
    mhz = 867.1f;   // SF7BW125 to SF12BW125
    break;

  case 5:
    mhz = 867.3f;   // SF7BW125 to SF12BW125
    break;

  case 6:
    mhz = 867.5f;   // SF7BW125 to SF12BW125
    break;

  case 7:
    mhz = 867.7f;   // SF7BW125 to SF12BW125
    break;

  case 8:
    mhz = 867.9f;   // SF7BW125 to SF12BW125
    break;

  case 9:
    mhz = 868.8f;   // FSK
    break;

  case  0:
  case 16:
    mhz = 869.525f; // RX2 channel
    break;

  default:
    Error_Handler();
  }
  return mhz;
}


void spiSX1272Frequency_MHz(float mhz)
{
  float fVal = (mhz * 1e6 * (1UL << 19)) / 32e6;
  uint32_t regVal = (uint32_t) (fVal + 0.5f);

  /* Set frequency register */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x06;
  aSpi1TxBuffer[1] = (uint8_t) ((regVal >> 16) & 0xffUL);
  aSpi1TxBuffer[2] = (uint8_t) ((regVal >>  8) & 0xffUL);
  aSpi1TxBuffer[3] = (uint8_t) ((regVal >>  0) & 0xffUL);
  spiProcessSpiMsg(4);
}

void spiSX1272Dio_Mapping(void)
{
  /* DIO0..DIO5 settings */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x40;
  aSpi1TxBuffer[1] = (0b00 << 6) | (0b00 << 4) | (0b00 << 2) | (0b00 << 0);  // DIO0: SYNC Addr, DIO1: DCLK, DIO2: DATA, DIO3: Timeout
  aSpi1TxBuffer[2] = (0b11 << 6) | (0b11 << 4) | (0b1 << 0);  // DIO4: Mode ready, DIO5: RSSI / Preamble detected, selection: Preamble detection
  spiProcessSpiMsg(3);
}


void spiSX1272LoRa_setTxMsgLen(uint8_t payloadLen)
{
  /* Message length to transmit */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x22;    // RegPayloadLength
  aSpi1TxBuffer[1] = payloadLen;
  spiProcessSpiMsg(2);
}

void spiSX1272LoRa_Fifo_Init(void)
{
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  aSpi1TxBuffer[1] = 0x00;    // RegFifoAddrPtr
  aSpi1TxBuffer[2] = 0x80;    // RegFifoTxBaseAddr
  aSpi1TxBuffer[3] = 0x00;    // RegFifoRxBaseAddr
  aSpi1TxBuffer[4] = 0x00;    // FifoRxCurrentAddr
  spiProcessSpiMsg(5);
}

void spiSX1272LoRa_Fifo_RxSetToBasePtr(void)
{
  aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x0f;
  spiProcessSpiMsg(2);
  uint8_t fifoRxBaseAddr = aSpi1RxBuffer[1];    // RegFifoRxBaseAddr

  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  aSpi1TxBuffer[1] = fifoRxBaseAddr;
  spiProcessSpiMsg(2);
}

void spiSX1272LoRa_Fifo_TxSetToBasePtr(void)
{
  aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x0e;
  spiProcessSpiMsg(2);
  uint8_t fifoTxBaseAddr = aSpi1RxBuffer[1];    // RegFifoTxBaseAddr

  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  aSpi1TxBuffer[1] = fifoTxBaseAddr;
  spiProcessSpiMsg(2);
}


void spiSX1272Mode(spiSX1272_Mode_t mode)
{
  /* Read current register */
  aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x01;
  spiProcessSpiMsg(2);
  uint8_t curMode = aSpi1RxBuffer[1];

  /* Modify */
  curMode &= ~(0b111 << 0);   // Mask out Mode
  curMode |=   0b111 & ((uint8_t) mode);

  /* Write back current mode */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  aSpi1TxBuffer[1] = curMode;
  spiProcessSpiMsg(2);
}

void spiSX1272Mode_LoRa_TX(uint8_t channel)
{
  /* Switch to LoRa mode (standby) */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  aSpi1TxBuffer[1] = (1 << 7) | (STANDBY << 0);   // bit7:LoRangeMode
  spiProcessSpiMsg(2);

  /* Set channel */
  spiSX1272Frequency_MHz(spiSX1272Calc_Channel_to_MHz(channel));

  /* Interrupt DIO lines activation */
  spiSX1272Dio_Mapping();

  /* TX @ RFO with +14dBm */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x09;
  aSpi1TxBuffer[1] = (0 << 7) | (0x0f << 0);    // PA off, TX @ RFO pin
  spiProcessSpiMsg(2);

  /* PA ramp time 50us */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0a;
  aSpi1TxBuffer[1] = 0b1000;
  spiProcessSpiMsg(2);

  /* Modem Config */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x1d;
  aSpi1TxBuffer[1] = (  0b00 << 6) | (0b001 << 3) | (0b0 << 2) | ( 0b1 << 1) | (0b1 << 0);
  aSpi1TxBuffer[2] = (0b1100 << 4) | (  0b0 << 3) | (0x1 << 2) | (0b00 << 0);
  spiProcessSpiMsg(3);

  /* Preamble length */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x20;    // RegPreambleMSB, LSB
  aSpi1TxBuffer[1] = 0x00;
  aSpi1TxBuffer[2] = 0x08;  // +4 = 12 symbols
  spiProcessSpiMsg(3);

  /* Sync word */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x39;
  aSpi1TxBuffer[1] = 0x34;
  spiProcessSpiMsg(2);

  /* I/Q inversion bits */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
  aSpi1TxBuffer[1] = 0x27;
  spiProcessSpiMsg(2);

  /* I/Q2 inversion bits */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
  aSpi1TxBuffer[1] = 0x1d;
  spiProcessSpiMsg(2);

  /* Change to TX mode */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  aSpi1TxBuffer[1] = (1 << 7) | (0b011 << 0);
  spiProcessSpiMsg(2);
}

void spiSX1272Mode_LoRa_RX(uint8_t channel)
{
  /* Switch to LoRa mode (standby) */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  aSpi1TxBuffer[1] = (1 << 7) | (STANDBY << 0);   // bit7:LoRangeMode
  spiProcessSpiMsg(2);

  /* Set channel */
  spiSX1272Frequency_MHz(spiSX1272Calc_Channel_to_MHz(channel));

  /* Interrupt DIO lines activation */
  spiSX1272Dio_Mapping();

  /* LNA to maximum */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0c;
  aSpi1TxBuffer[1] = (0b001 << 5) | (0b11 << 0);
  spiProcessSpiMsg(2);

  /* RegRxConfig */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  aSpi1TxBuffer[1] = 0x8e;
  spiProcessSpiMsg(2);

  /* Modem Config */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x1d;
  aSpi1TxBuffer[1] = (  0b00 << 6) | (0b001 << 3) | ( 0b0 << 2) | (0b1 << 1) | (0b1 << 0);
  aSpi1TxBuffer[2] = (0b1100 << 4) | (  0x1 << 2) | (0b00 << 0);
  aSpi1TxBuffer[3] = 0x05;
  spiProcessSpiMsg(4);

  /* Max. payload length */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x23;
  aSpi1TxBuffer[1] = 0x40;
  spiProcessSpiMsg(2);

  /* Preamble length */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x20;    // RegPreambleMSB, LSB
  aSpi1TxBuffer[1] = 0x01;
  aSpi1TxBuffer[2] = 0xf8;    // +4 = symbols at max.
  spiProcessSpiMsg(3);

  /* Sync word */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x39;
  aSpi1TxBuffer[1] = 0x34;    // LoRaWAN SYNC (instead of 0x12 of other networks)
  spiProcessSpiMsg(2);

  /* I/Q inversion bits */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
  aSpi1TxBuffer[1] = 0x67;
  spiProcessSpiMsg(2);

  /* I/Q2 inversion bits */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
  aSpi1TxBuffer[1] = 0x19;
  spiProcessSpiMsg(2);

  /* Reset RX FIFO */
  spiSX1272LoRa_Fifo_RxSetToBasePtr();

  /* Change to RX mode */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  //aSpi1TxBuffer[1]  = (1 << 7) | (RXSINGLE      << 0);   // LongRangeMode, RX_SINGLE
  aSpi1TxBuffer[1]    = (1 << 7) | (RXCONTINUOUS  << 0);   // LongRangeMode, RX_CONTINUOUS
  spiProcessSpiMsg(2);
}

void spiSX1272_WaitUntil_TxRxDone(void)
{
  char  debugBuf[1024]  = { 0 };
  int   debugLen        =   0;

  /* Use TxDone and RxDone - mask out all other IRQs */
  aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x11;    // RegIrqFlagsMask
  aSpi1TxBuffer[1] = ~0b01001000;
  spiProcessSpiMsg(2);

  do {
    aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x12;    // RegIrqFlags
    spiProcessSpiMsg(2);
    uint8_t irq = aSpi1RxBuffer[1];

    if (irq) {
      aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x12;    // RegIrqFlags
      aSpi1TxBuffer[1] = 0xff;    // Reset all flags
      spiProcessSpiMsg(2);

      aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x18;    // RegModemStat
      spiProcessSpiMsg(2);
      uint8_t modemStat = aSpi1RxBuffer[1];

      aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x2c;    // RegRssiWideband
      spiProcessSpiMsg(2);
      uint8_t rssiWideband = aSpi1RxBuffer[1];

      aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x14;    // ValidHeaderCnt, ValidPacketCnt
      spiProcessSpiMsg(5);
      uint16_t rxHeaderCnt   = ((uint16_t)aSpi1RxBuffer[1] << 8) | aSpi1RxBuffer[2];
      uint16_t rxValidPktCnt = ((uint16_t)aSpi1RxBuffer[3] << 8) | aSpi1RxBuffer[4];

      debugLen = sprintf(debugBuf, "irq=%02x: modem=%02x, rssiWB=%03u rxHdr=%05u rxPkt=%05u", irq, modemStat, rssiWideband, rxHeaderCnt, rxValidPktCnt);

      aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x13;    // RegRxNbBytes
      spiProcessSpiMsg(2);
      uint8_t rxNbBytes = aSpi1RxBuffer[1];
      __asm volatile( "nop" );

      /* FIFO readout */
      if (rxNbBytes) {
        /* Positioning of the FIFO addr ptr */
        {
          aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x10;    // RegFifoRxCurrentAddr
          spiProcessSpiMsg(2);
          uint8_t fifoRxCurrentAddr = aSpi1RxBuffer[1];

          aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;    // RegFifoAddrPtr
          aSpi1TxBuffer[1] = fifoRxCurrentAddr - rxNbBytes;
          spiProcessSpiMsg(2);

          debugLen += sprintf(debugBuf + debugLen, " rxNbBytes=%03u fifoRxCurrentAddr=%02x:", rxNbBytes, fifoRxCurrentAddr);

          /* FIFO read out */
          if (rxNbBytes < sizeof(aSpi1RxBuffer)) {
            aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x00;    // RegFifo
            spiProcessSpiMsg(1 + rxNbBytes);
            for (uint8_t idx = 0; idx < rxNbBytes; ++idx) {
              debugLen += sprintf(debugBuf + debugLen, " %02x", aSpi1RxBuffer[1 + idx]);
            }
          } else {
            /* Buffer to small */
            Error_Handler();
          }
        }
        __asm volatile( "nop" );

        /* Prepare FIFO for next packet */
        spiSX1272LoRa_Fifo_RxSetToBasePtr();

        debugBuf[debugLen] = 0;
        __asm volatile( "nop" );
        (void) debugBuf;
      }
      break;
    }

    /* Avoid busy loop on IRQ state */
    osDelay(1);
  } while (1);
}


uint8_t spiDetectShieldSX1272(void)
{
  /* Prepare SX_RESET structure */
  {
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = SX_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Reset pulse for SX1272 */
    {
      HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_SET);
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);

      osDelay(1);

      HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_RESET);
      osDelay(5);
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);
    }

    /* Have the device in sleep mode */
    spiSX1272Mode(SLEEP);
  }

  /* Request RD-address 0x42 RegVersion */
  {
    uint8_t sxVersion = 0;

    aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x42;
    spiProcessSpiMsg(2);

    if (spiProcessSpiReturnWait()) {
      sxVersion = aSpi1RxBuffer[1];
    }

    if (sxVersion != 0x22) {
      /* We can handle Version 0x22 only */
      return 0;
    }
  }

  /* SX1272 mbed shield found and ready for transmissions */
  return 1;
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
