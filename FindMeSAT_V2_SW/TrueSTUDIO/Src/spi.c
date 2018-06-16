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
#include <string.h>

#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"

#include "usb.h"
#include "exti.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern EventGroupHandle_t spiEventGroupHandle;
extern EventGroupHandle_t extiEventGroupHandle;
extern osSemaphoreId      usbToHostBinarySemHandle;

/* Holds RTOS timing info */
uint32_t spiPreviousWakeTime = 0UL;

/* Buffer used for transmission */
volatile uint8_t spi1TxBuffer[SPI1_BUFFERSIZE] = { 0 };

/* Buffer used for reception */
volatile uint8_t spi1RxBuffer[SPI1_BUFFERSIZE] = { 0 };

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
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
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
    /* Deactivate the NSS/SEL pin */
    HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);

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
    /* Deactivate the NSS/SEL pin */
    HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);

    xEventGroupSetBitsFromISR(spiEventGroupHandle, SPI_SPI1_EG__ERROR, &taskWoken);
  }
}


const uint16_t spiWait_MaxWaitEGMs = 500;
uint8_t spiProcessSpiReturnWait(void)
{
  EventBits_t eb = xEventGroupWaitBits(spiEventGroupHandle, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, 0, spiWait_MaxWaitEGMs);
  if (eb & SPI_SPI1_EG__RDY) {
    return 1;
  }

  if (eb & SPI_SPI1_EG__ERROR) {
    Error_Handler();
  }
  return 0;
}

uint8_t spiProcessSpiMsg(uint8_t msgLen)
{
  HAL_StatusTypeDef status = 0;
  uint8_t errCnt = 0;

  /* Activate low active NSS/SEL */
  HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_RESET);

  spi1TxBuffer[msgLen] = 0;
  do {
    status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t*) spi1TxBuffer, (uint8_t *) spi1RxBuffer, msgLen);
    if (status != HAL_OK)
    {
      osDelay(1);

      if (++errCnt >= 100) {
        /* Transfer error in transmission process */
        HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);
        Error_Handler();
      }
    }
  } while (status != HAL_OK);
  HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);

  return spiProcessSpiReturnWait();
}


void spiSX127xReset(void)
{
  // Activate nRESET
  {
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin   = SX_RESET_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);

    /* Pull pin 6 down of the SX127x <--> CN9 pin 1 (PA3) of the mbed board */
    HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_RESET);
  }

  /* At least for 100 µs */
  HAL_Delay(1);

  /* Release the nRESET line */
  {
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin   = SX_RESET_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);
  }

  /* Delay for 5 ms before accessing the chip */
  HAL_Delay(5);
}

void spiSX127xFrequency_MHz(float mhz)
{
  const uint32_t    fmt_mhz     = (uint32_t) mhz;
  const uint8_t     fmt_mhz_f1  = (uint8_t) (((uint32_t) (mhz * 10.f)) % 10);
  volatile uint8_t  buf[256]    = { 0 };
  volatile uint32_t bufLen      = 0UL;

  /* Debugging information */
  bufLen = sprintf((char*) buf, "f = %03ld.%01d MHz\r\n", fmt_mhz, fmt_mhz_f1);
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) buf, bufLen);
  osSemaphoreRelease(usbToHostBinarySemHandle);

  /* Register value calc */
  float     fVal    = (mhz * 1e6 * (1UL << 19)) / 32e6;
  uint32_t  regVal  = (uint32_t) (fVal + 0.5f);

  /* Set frequency register */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x06;
  spi1TxBuffer[1] = (uint8_t) ((regVal >> 16) & 0xffUL);
  spi1TxBuffer[2] = (uint8_t) ((regVal >>  8) & 0xffUL);
  spi1TxBuffer[3] = (uint8_t) ((regVal >>  0) & 0xffUL);
  spiProcessSpiMsg(4);
}

void spiSX127xDio_Mapping(TxRx_Mode_t mode)
{
  /* DIO0..DIO5 settings */

  switch (mode) {
  case TxRx_Mode_TX:
    {
      spi1TxBuffer[1] = (0b01 << 6) | (0b00 << 4) | (0b00 << 2) | (0b01 << 0);  // DIO0: TX done, DIO1: RxTimeout, DIO2: FhssChangeChannel, DIO3: ValidHeader
      spi1TxBuffer[2] = (0b00 << 6) | (0b00 << 4) | (0b1 << 0);                 // DIO4: CadDetected, DIO5: ModeReady, selection: Preamble detection
    }
    break;

  case TxRx_Mode_RX:
  case TxRx_Mode_RX_Randomizer:
    {
      spi1TxBuffer[1] = (0b00 << 6) | (0b00 << 4) | (0b00 << 2) | (0b01 << 0);  // DIO0: RX done, DIO1: RxTimeout, DIO2: FhssChangeChannel, DIO3: ValidHeader
      spi1TxBuffer[2] = (0b00 << 6) | (0b00 << 4) | (0b1 << 0);                 // DIO4: CadDetected, DIO5: ModeReady, selection: Preamble detection
    }
    break;

  default:
    return;
  }

  /* Push settings */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x40;
  spiProcessSpiMsg(3);
}


uint8_t spiSX127xMode_LoRa_GetBroadbandRSSI(void)
{
  /* Broadband RSSI */
  spi1TxBuffer[0] = SPI_RD_FLAG | 0x2c;
  spi1TxBuffer[1] = 0;
  spiProcessSpiMsg(2);

  return spi1RxBuffer[1];
}

void spiSX127xLoRa_setTxMsgLen(uint8_t payloadLen)
{
  /* Sanity correction */
  if (!payloadLen) {
    payloadLen = 1;
  }

  /* Message length to transmit */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x22;    // RegPayloadLength
  spi1TxBuffer[1] = payloadLen;
  spiProcessSpiMsg(2);
}

void spiSX127xLoRa_Fifo_Init(void)
{
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = 0x00;    // 0x0D RegFifoAddrPtr
  spi1TxBuffer[2] = 0x80;    // 0x0E RegFifoTxBaseAddr
  spi1TxBuffer[3] = 0x00;    // 0x0F RegFifoRxBaseAddr
  spiProcessSpiMsg(4);
}

void spiSX127xLoRa_Fifo_SetFifoPtrFromRxBase(void)
{
  spi1TxBuffer[0] = SPI_RD_FLAG | 0x0f;
  spiProcessSpiMsg(2);
  uint8_t fifoRxBaseAddr = spi1RxBuffer[1];    // RegFifoRxBaseAddr

  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = fifoRxBaseAddr;
  spiProcessSpiMsg(2);
}

void spiSX127xLoRa_Fifo_SetFifoPtrFromTxBase(void)
{
  spi1TxBuffer[0] = SPI_RD_FLAG | 0x0e;
  spiProcessSpiMsg(2);
  uint8_t fifoTxBaseAddr = spi1RxBuffer[1];    // RegFifoTxBaseAddr

  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = fifoTxBaseAddr;
  spiProcessSpiMsg(2);
}


void spiSX127xMode(spiSX127x_Mode_t mode)
{
  /* Switch RX/TX at PE4259 */
  switch (mode & TXRX_MODE_MASK) {
  case FSTX:
  case TX:
    HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_SET);
    //HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_RESET);

    /* Write TX mode */
    spi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
    spi1TxBuffer[1] = mode;
    spiProcessSpiMsg(2);
    break;

  default:
    /* Write any other RX mode */
    spi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
    spi1TxBuffer[1] = mode;
    spiProcessSpiMsg(2);

    HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_SET);
  }
}

void spiSX127xRegister_IRQ_clearAll(void)
{
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x12;     // LoRa: RegIrqFlags
  spi1TxBuffer[1] = 0xff;
  spiProcessSpiMsg(2);
}

void spiSX127x_TxRx_Preps(LoRaWANctx_t* ctx, TxRx_Mode_t mode, LoRaWAN_Message_t* msg)
{
  float   l_f   = ctx->Frequency;
  uint8_t l_SF  = ctx->SpreadingFactor << SFx_SHIFT;

  /* Switching to LoRa via Reset and SLEEP mode */
  spiSX127xReset();
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | STANDBY);

  /* Set the frequency */
  spiSX127xFrequency_MHz(l_f);

  /* Sync word */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x39;
  spi1TxBuffer[1] = 0x34;
  spiProcessSpiMsg(2);

  /* ModemConfig1, ModemConfig2 */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x1d;
  spi1TxBuffer[1] = BW_125kHz | CR_4_5 | IHM_OFF;
  spi1TxBuffer[2] = l_SF | TXCONT_OFF | RX_PAYLOAD_CRC_ON | (0x0 << 0);  // SymbTmeoutMsb = 0
  spiProcessSpiMsg(3);

  /* ModemConfig3 */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x26;
  spi1TxBuffer[1] = (l_SF >= SF11_DR1 ?  LOW_DR_OPTI_ON : LOW_DR_OPTI_OFF) | AGC_AUTO_ON;
  spiProcessSpiMsg(2);

  /* Frequency hopping disabled */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x24;
  spi1TxBuffer[1] = 0x00;
  spiProcessSpiMsg(2);

  /* Reset the IRQ register */
  spiSX127xRegister_IRQ_clearAll();

  /* Interrupt DIO lines activation */
  spiSX127xDio_Mapping(mode);

  switch (mode) {
  case TxRx_Mode_TX:
    {
      /* TX @ RFO */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x09;
      spi1TxBuffer[1] = (0x0 << 7) | (0x0 << 4) | (0x3 << 0);     // PA off, MaxPower, TXpwr @ RFO pin
      spi1TxBuffer[2] = PA_RAMP_50us;                             // PA ramp time 50us
      spi1TxBuffer[3] = (0x1 << 5) | (0xb << 0);                  // OverCurrentProtection ON, normal: 100mA
      spiProcessSpiMsg(4);

      /* Preamble length */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x20;
      spi1TxBuffer[1] = 0x00;    // PreambleMsb, PreambleLsb
      spi1TxBuffer[2] = 0x0a;    // +4 = 14 symbols
      spiProcessSpiMsg(3);

      /* I/Q inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
      spi1TxBuffer[1] = 0x27;   // This is the default value, no inversion
      spiProcessSpiMsg(2);

      /* I/Q2 inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
      spi1TxBuffer[1] = 0x1d;   // This is the default value, no inversion
      spiProcessSpiMsg(2);

      /* Set transmit message length */
      spiSX127xLoRa_setTxMsgLen(msg->msg_Len);

      /* Prepare the transmitter circuits */
      spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | FSTX);
    }
    break;

  case TxRx_Mode_RX:
    {
      /* LNA to maximum */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x0c;
      spi1TxBuffer[1] = LnaGain_G1 | LnaBoost_Lf_XXX | LnaBoost_Hf_ON;
      spiProcessSpiMsg(2);

      /* Modem Config */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x1f;
      spi1TxBuffer[1] = (l_SF >= SF10_DR2 ?  0x05 : 0x08);        // SymbtimeoutLsb
      spiProcessSpiMsg(2);

      /* Max. payload length */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x23;
      spi1TxBuffer[1] = 0x40;
      spiProcessSpiMsg(2);

      /* DetectionOptimize */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x31;
      spi1TxBuffer[1] = 0xc0 | (l_SF >= SF7_DR5 ?  0x03 : 0x05);
      spiProcessSpiMsg(2);

      /* DetectionThreshold */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x37;
      spi1TxBuffer[1] = (l_SF >= SF7_DR5 ?  0x0a : 0x0c);
      spiProcessSpiMsg(2);

      /* I/Q inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
      spi1TxBuffer[1] = (0x1 << 6) | 0x27;   // Optimized for inverted IQ
      spiProcessSpiMsg(2);

#if 0  // No more used
      /* I/Q2 inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
      spi1TxBuffer[1] = 0x19;   // Optimized for inverted IQ
      spiProcessSpiMsg(2);
#endif

      /* Prepare the receiver circuits */
      spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | FSRX);
    }
    break;

  case TxRx_Mode_RX_Randomizer:
    {
      /* LNA to maximum */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x0c;
      spi1TxBuffer[1] = LnaGain_G1 | LnaBoost_Lf_XXX | LnaBoost_Hf_ON;
      spiProcessSpiMsg(2);

      /* Turn on receiver */
      spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | RXCONTINUOUS);
    }
    break;

  default:
    return;
  }
}

uint32_t spiSX127x_WaitUntil_TxDone(uint8_t doPreviousWakeTime, uint32_t stopTime)
{
  uint32_t              ts              = 0UL;
  volatile EventBits_t  eb              = { 0 };
  volatile uint8_t      irq             = 0U;
  char                  debugBuf[1024]  = { 0 };
  int                   debugLen        = 0;

  /* Use TxDone and RxDone - mask out all other IRQs */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x11;       // RegIrqFlagsMask
  spi1TxBuffer[1] = (0x1 << RxTimeoutMask);   // Mask out: bit7 RxTimeout
  spiProcessSpiMsg(2);

  {
    /* Wait for EXTI / IRQ line(s) */
    TickType_t ticks = 1;
    uint32_t now = osKernelSysTick();
    if (stopTime > now) {
      ticks = (stopTime - now) / portTICK_PERIOD_MS;
    }
    eb = xEventGroupWaitBits(extiEventGroupHandle, EXTI_SX__DIO0, EXTI_SX__DIO0, 0, ticks);

    spi1TxBuffer[0] = SPI_RD_FLAG | 0x12;     // RegIrqFlags
    spiProcessSpiMsg(2);
    irq = spi1RxBuffer[1];

    if ((eb & EXTI_SX__DIO0) || (irq & (1U << TxDoneMask))) {
      /* Remember point of time when TxDone was set */
      ts = osKernelSysTick();
      if (doPreviousWakeTime) {
        spiPreviousWakeTime = ts;
      }

      /* Reset all IRQ flags */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x12;   // RegIrqFlags
      spi1TxBuffer[1] = 0xff;
      spiProcessSpiMsg(2);

#if 1
      debugLen = sprintf(debugBuf, "WaitUntil_TxDone: eb=0x%08lx | irq=0x%02x: ts=%09ld.\r\n", eb, irq, ts);

      osSemaphoreWait(usbToHostBinarySemHandle, 0);
      usbToHostWait((uint8_t*) debugBuf, debugLen);
      osSemaphoreRelease(usbToHostBinarySemHandle);
#endif
      return ts;
    }
  }

  return 0UL;
}

void spiSX127x_WaitUntil_RxDone(LoRaWAN_Message_t* msg, uint32_t stopTime)
{
  volatile EventBits_t  eb;
  volatile uint8_t      irq;
  volatile uint32_t     now             = osKernelSysTick();
#if 1
  volatile char         debugBuf[512]   = { 0 };
  volatile int          debugLen        =   0;
#endif

   /* Use TxDone and RxDone - mask out all other IRQs */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x11;       // RegIrqFlagsMask
  spi1TxBuffer[1] = (0x1 << RxTimeoutMask);   // Mask out: bit7 RxTimeout
  spiProcessSpiMsg(2);

  do {
    /* Wait for EXTI / IRQ line(s) */
    volatile TickType_t ticks = 1;

    now = osKernelSysTick();
    if (stopTime > now) {
      //ticks = (stopTime - now) / portTICK_PERIOD_MS;
    } else {
      break;
    }

    eb = xEventGroupWaitBits(extiEventGroupHandle, EXTI_SX__DIO0, EXTI_SX__DIO0, 0, ticks);

    /* Get the current IRQ flags */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x12;     // LoRa: RegIrqFlags
    spiProcessSpiMsg(2);
    irq = spi1RxBuffer[1];

#if 1
    if (irq) {
      debugLen += sprintf((char*) debugBuf, "irq=0x%02X\r\n", irq);
    }
#endif

    if ((eb & EXTI_SX__DIO0) || (irq & (1U << RxDoneMask))) {
      /* Reset all IRQ flags */
      spiSX127xRegister_IRQ_clearAll();

      /* Check for CRC */
      if (irq & (1U << PayloadCrcErrorMask)) {
        __asm volatile ( "" );
        continue;
      }

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x18;     // LoRa: RegModemStat
      spiProcessSpiMsg(5);
      uint8_t modemStat   = spi1RxBuffer[1];
      uint8_t packetSnr   = spi1RxBuffer[2];
      uint8_t packetRssi  = spi1RxBuffer[3];
      uint8_t rssi        = spi1RxBuffer[4];

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x2c;     // LoRa: RegRssiWideband
      spiProcessSpiMsg(2);
      uint8_t rssiWideband = spi1RxBuffer[1];

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x14;    // ValidHeaderCnt, ValidPacketCnt
      spiProcessSpiMsg(5);
      uint16_t rxHeaderCnt   = ((uint16_t)spi1RxBuffer[1] << 8) | spi1RxBuffer[2];
      uint16_t rxValidPktCnt = ((uint16_t)spi1RxBuffer[3] << 8) | spi1RxBuffer[4];

#if 1
      debugLen = sprintf((char*) debugBuf, "eb=0x%08lx | irq=%02x: modem=%02x rssiWB=%03u rssi=%03u packetRssi=%03u packetSnr=%03u rxHdr=%05u rxPkt=%05u",
          (uint32_t)eb, irq,
          modemStat,
          rssiWideband, rssi, packetRssi, packetSnr,
          rxHeaderCnt, rxValidPktCnt);
#endif

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x13;    // RegRxNbBytes
      spiProcessSpiMsg(2);
      volatile uint8_t rxNbBytes = spi1RxBuffer[1];

      /* FIFO readout */
      if (rxNbBytes) {
        /* Positioning of the FIFO addr ptr */
        {
          spi1TxBuffer[0] = SPI_RD_FLAG | 0x10;    // RegFifoRxCurrentAddr
          spiProcessSpiMsg(2);
          uint8_t fifoRxCurrentAddr = spi1RxBuffer[1];

          spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;    // RegFifoAddrPtr
          spi1TxBuffer[1] = fifoRxCurrentAddr;
          spiProcessSpiMsg(2);

#if 1
          debugLen += sprintf((char*)debugBuf + debugLen, " rxNbBytes=%03u fifoRxCurrentAddr=%02x:", rxNbBytes, fifoRxCurrentAddr);
#endif

          /* FIFO read out */
          if (rxNbBytes < sizeof(spi1RxBuffer)) {
            spi1TxBuffer[0] = SPI_RD_FLAG | 0x00;    // RegFifo
            spiProcessSpiMsg(1 + rxNbBytes);

            /* Copy SPI receive buffer content to msg object */
            memcpy((void*)msg->msg_Buf, (const void*)spi1RxBuffer + 1, rxNbBytes);
            msg->msg_Len = rxNbBytes;

#if 1
            for (uint8_t idx = 0; idx < rxNbBytes; ++idx) {
              debugLen += sprintf((char*)debugBuf + debugLen, " %02x", spi1RxBuffer[1 + idx]);
            }
#endif
          } else {
            /* Buffer to small */
            Error_Handler();
          }
        }
      }

#if 1
      /* Debugging */
      debugLen += sprintf((char*)debugBuf + debugLen, "\r\n");
      osSemaphoreWait(usbToHostBinarySemHandle, 0);
      usbToHostWait((uint8_t*) debugBuf, debugLen);
      osSemaphoreRelease(usbToHostBinarySemHandle);
#endif
      return;
    }
  } while (1);
}


uint8_t spiDetectShieldSX127x(void)
{
  /* Reset pulse for SX127x */
  spiSX127xReset();

  /* Change to sleep mode for modulation change */
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);

  /* Request RD-address 0x42 RegVersion */
  {
    uint8_t sxVersion = 0;

    spi1TxBuffer[0] = SPI_RD_FLAG | 0x42;
    if (spiProcessSpiMsg(2)) {
      sxVersion = spi1RxBuffer[1];
    }

    if ((sxVersion != 0x22)  // SX1272
        &&
        (sxVersion != 0x12)) {  // SX1276
      /* We can handle Version  0x22 (SX1272)  and  0x12 (SX1276) only */
      return 0;
    }
  }

  /* SX127x mbed shield found and ready for transmissions */
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
