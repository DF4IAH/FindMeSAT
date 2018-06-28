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
#include "stm32l4xx_nucleo_144.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"

#include "usb.h"
#include "exti.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern EventGroupHandle_t spiEventGroupHandle;
extern EventGroupHandle_t extiEventGroupHandle;
extern osSemaphoreId      usbToHostBinarySemHandle;

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


const uint16_t spiWait_EGW_MaxWaitTicks = 500;
uint8_t spiProcessSpiReturnWait(void)
{
  EventBits_t eb = xEventGroupWaitBits(spiEventGroupHandle, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, SPI_SPI1_EG__RDY | SPI_SPI1_EG__ERROR, 0, spiWait_EGW_MaxWaitTicks);
  if (eb & SPI_SPI1_EG__RDY) {
    return HAL_OK;
  }

  if (eb & SPI_SPI1_EG__ERROR) {
    Error_Handler();
  }
  return HAL_ERROR;
}

uint8_t spiProcessSpiMsg(uint8_t msgLen)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t           errCnt = 0;

  /* Activate low active NSS/SEL transaction */
  HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_RESET);

  do {
    status = HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t*) spi1TxBuffer, (uint8_t *) spi1RxBuffer, msgLen);
    if (status == HAL_BUSY)
    {
      osDelay(1);

      if (++errCnt >= 100) {
        /* Transfer error in transmission process */
        HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);
        Error_Handler();
      }
    }
  } while (status == HAL_BUSY);

  if (status == HAL_OK) {
    /* Wait until the data is transfered */
    status = spiProcessSpiReturnWait();
  }

  /* Release low active NSS/SEL transaction */
  HAL_GPIO_WritePin(SPI_A_SEL_GPIO_Port, SPI_A_SEL_Pin, GPIO_PIN_SET);

  return status;
}


void spiSX127xReset(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* nRESET: Pull pin 6 down of the SX127x <--> CN9 pin 1 (PA3) of the mbed board */
  HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin   = SX_RESET_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);

  /* At least for 100 µs */
  osDelay(1);

  /* Release the nRESET line */
  HAL_GPIO_WritePin(SX_RESET_GPIO_Port, SX_RESET_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  HAL_GPIO_Init(SX_RESET_GPIO_Port, &GPIO_InitStruct);

  /* Delay for 5 ms before accessing the chip */
  osDelay(5);
}

void spiSX127xFrequency_MHz(float mhz)
{
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

uint8_t spiSX127xPower_GetSetting(LoRaWANctx_t* ctx)
{
  int16_t pow_dBm = 14L - ctx->LinkADR_TxPowerReduction_dB;
  uint8_t reg = 0;

  if (pow_dBm >= +14) {
    reg = (0x4 << 4) | (0xf << 0);                                                              // MaxPower +14dBm, TXpwr @ RFO pin

  } else if (pow_dBm >= +12) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= +10) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  +8) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  +6) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  +4) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  +2) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=   0) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  -2) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  -4) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  -6) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >=  -8) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -10) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -12) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -14) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -16) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -18) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else if (pow_dBm >= -20) {
    reg = (0x0 << 4) | (0x0 << 0);

  } else {
    reg = (0x0 << 4) | (0x0 << 0);                                                              // Minimal power @ RFO pin
  }

  /* No PA used */
  return (0x0 << 7) || reg;
}

void spiSX127xDio_Mapping(TxRx_Mode_t mode)
{
  /* DIO0..DIO5 settings */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x40;

  switch (mode) {
  case TxRx_Mode_TX:
    {
      spi1TxBuffer[1] = (0b01 << 6) | (0b10 << 4) | (0b00 << 2) | (0b01 << 0);  // DIO0: TX done, DIO1: CadDetected, DIO2: FhssChangeChannel, DIO3: ValidHeader
      spi1TxBuffer[2] = (0b01 << 6) | (0b00 << 4);                              // DIO4: PllLock, DIO5: ModeReady
    }
    break;

  case TxRx_Mode_RX:
  case TxRx_Mode_RX_Randomizer:
    {
      spi1TxBuffer[1] = (0b00 << 6) | (0b10 << 4) | (0b00 << 2) | (0b01 << 0);  // DIO0: RX done, DIO1: CadDetected, DIO2: FhssChangeChannel, DIO3: ValidHeader
      spi1TxBuffer[2] = (0b01 << 6) | (0b00 << 4);                              // DIO4: PllLock, DIO5: ModeReady
    }
    break;

  default:
    return;
  }

  /* Push settings */
  spiProcessSpiMsg(3);
}

uint8_t spiSX127xDR_to_SF(DataRates_t dr)
{
  if (DR0_SF12_125kHz_LoRa <= dr && dr <= DR5_SF7_125kHz_LoRa) {
    /* 125 kHz */
    return (12 - (uint8_t)dr);

  } else if (DR6_SF7_250kHz_LoRa) {
    /* 250 kHz */
    return 7;
  }

  /* No LoRa mode */
  return 0;
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
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x22;       // RegPayloadLength
  spi1TxBuffer[1] = payloadLen;
  spiProcessSpiMsg(2);
}

void spiSX127xLoRa_Fifo_Init(void)
{
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = 0x00;                     // 0x0D RegFifoAddrPtr
  spi1TxBuffer[2] = 0xC0;                     // 0x0E RegFifoTxBaseAddr
  spi1TxBuffer[3] = 0x00;                     // 0x0F RegFifoRxBaseAddr
  spiProcessSpiMsg(4);
}

void spiSX127xLoRa_Fifo_SetFifoPtrFromTxBase(void)
{
  spi1TxBuffer[0] = SPI_RD_FLAG | 0x0e;
  spiProcessSpiMsg(2);
  uint8_t fifoTxBaseAddr = spi1RxBuffer[1];   // RegFifoTxBaseAddr

  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = fifoTxBaseAddr;
  spiProcessSpiMsg(2);
}

void spiSX127xLoRa_Fifo_SetFifoPtrFromRxBase(void)
{
  spi1TxBuffer[0] = SPI_RD_FLAG | 0x0f;
  spiProcessSpiMsg(2);
  uint8_t fifoRxBaseAddr = spi1RxBuffer[1];   // RegFifoRxBaseAddr

  spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;
  spi1TxBuffer[1] = fifoRxBaseAddr;
  spiProcessSpiMsg(2);
}


void spiSX127xMode(spiSX127x_Mode_t mode)
{
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x01;
  spi1TxBuffer[1] = mode;

  /* Switch RX/TX at PE4259 */
  switch (mode & TXRX_MODE_MASK) {
  case FSTX:
  case TX:
    /* Switch to TX path */
    HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_SET);

    /* Write TX mode */
    spiProcessSpiMsg(2);
    break;

  default:
    /* Write any other RX mode */
    spiProcessSpiMsg(2);

    /* Switch to RX path */
    HAL_GPIO_WritePin(SX_RXTX_EXT_GPIO_Port, SX_RXTX_EXT_Pin, GPIO_PIN_RESET);
  }

  /* Delay after mode-change */
  osDelay(25);
}

void spiSX127xRegister_IRQ_clearAll(void)
{
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x12;     // LoRa: RegIrqFlags
  spi1TxBuffer[1] = 0xff;
  spiProcessSpiMsg(2);
}

void spiSX127x_TxRx_Preps(LoRaWANctx_t* ctx, TxRx_Mode_t mode, LoRaWAN_TX_Message_t* msg)
{
  const float       l_f               = ctx->FrequencyMHz;
  const uint8_t     l_SF              = (ctx->SpreadingFactor & 0x0f) << SFx_SHIFT;
  const uint32_t    fmt_mhz           = (uint32_t) l_f;
  const uint16_t    fmt_mhz_f1        = (uint16_t) (((uint32_t) (l_f * 1000.f)) % 1000);
  uint32_t          PreviousWakeTime  = 0UL;
  volatile uint8_t  buf[256]          = { 0 };
  volatile uint32_t bufLen            = 0UL;

  if (TxRx_Mode_IQ_Balancing == mode) {
    /* Switching to FSK/OOK via SLEEP mode */
    spiSX127xMode(MODE_FSK_OOK | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);

    /* Set the frequency */
    spiSX127xFrequency_MHz(l_f * (1 + 1e-6 * ctx->CrystalPpm));

    /* Warm up receiver before balancing - wait until USB DCD is ready */
    spiSX127xMode(MODE_FSK_OOK | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | RXCONTINUOUS);
    osDelayUntil(&PreviousWakeTime, 3600);
    spiSX127xMode(MODE_FSK_OOK | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | STANDBY);

    /* Start I/Q balancing */
    {
      uint8_t balState;
      uint8_t temp;

      /* Blue LED on */
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);

      /* Set bit to start */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x3b;
      spi1TxBuffer[1] = 0x42;                                                                 // RegImageCal
      spiProcessSpiMsg(2);

      /* Wait until the balancing process has finished */
      uint32_t t0 = getRunTimeCounterValue();
      uint32_t t1;
      do {
        osDelay(1);

        /* Request balancing state */
        spi1TxBuffer[0] = SPI_RD_FLAG | 0x3b;
        spiProcessSpiMsg(3);
        balState  = spi1RxBuffer[1];
        temp      = spi1RxBuffer[2];

        /* Test for completion */
        if (!(balState & 0x20)) {
          t1 = getRunTimeCounterValue();
          ctx->LastIqBalTemp        = temp - 212;                                             // Temperature device compensated
          ctx->LastIqBalTimeUs      = t0;
          ctx->LastIqBalDurationUs  = t1 - t0;
          break;
        }
      } while (1);
    }

    /* Blue LED off */
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);

    /* Return to LoRa mode */
    spiSX127xMode(MODE_FSK_OOK | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);
    spiSX127xMode(MODE_LoRa    | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);

    return;
  }

  /* Skip for RX2 where only frequency and SpreadingFactor is changed */
  if (TxRx_Mode_RX2 != mode) {
    /* Switching to LoRa via SLEEP mode */
    spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);
    spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | STANDBY);
  }

  /* Debugging information */
  bufLen = sprintf((char*) buf, "LoRaWAN: f = %03lu.%03u MHz,\t SF = %02u\r\n", fmt_mhz, fmt_mhz_f1, ctx->SpreadingFactor);
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) buf, bufLen);
  osSemaphoreRelease(usbToHostBinarySemHandle);

  /* Common presets for TX / RX */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x1d;
#ifdef PPM_CALIBRATION
  spi1TxBuffer[1] = BW_7kHz8  | CR_4_5 | IHM_OFF;                                             // ModemConfig1
#else
  spi1TxBuffer[1] = BW_125kHz | CR_4_5 | IHM_OFF;                                             // ModemConfig1
#endif
  spi1TxBuffer[2] = l_SF | TXCONT_OFF | RX_PAYLOAD_CRC_ON | (0b00 << 0);                      // ModemConfig2 with SymbTmeoutMsb = 0b00
  spiProcessSpiMsg(3);

  spi1TxBuffer[0] = SPI_WR_FLAG | 0x26;
  spi1TxBuffer[1] = (l_SF >= SF11_DR1 ?  LOW_DR_OPTI_ON : LOW_DR_OPTI_OFF) | AGC_AUTO_ON;     // ModemConfig3
  spi1TxBuffer[2] = (uint8_t) (ctx->CrystalPpm *  0.95f);                                     // PPM Correction
  spiProcessSpiMsg(3);

  /* Reset the IRQ register */
  spiSX127xRegister_IRQ_clearAll();

  /* Skip for RX2 where only frequency and SpreadingFactor is changed */
  if (TxRx_Mode_RX2 != mode) {
    /* Frequency hopping disabled */
    spi1TxBuffer[0] = SPI_WR_FLAG | 0x24;
    spi1TxBuffer[1] = 0x00;
    spiProcessSpiMsg(2);

    /* Sync word */
    spi1TxBuffer[0] = SPI_WR_FLAG | 0x39;
    spi1TxBuffer[1] = 0x34;
    spiProcessSpiMsg(2);

    /* Interrupt DIO lines activation */
    spiSX127xDio_Mapping(mode);
  }

  switch (mode) {
  case TxRx_Mode_TX:
    {
      /* Set the frequency */
      spiSX127xFrequency_MHz(l_f * (1 + 1e-6 * ctx->CrystalPpm));

      spi1TxBuffer[0] = SPI_WR_FLAG | 0x09;
#ifdef PPM_CALIBRATION
      spi1TxBuffer[1] = (0x0 << 7) | (0x0 << 4) | (0x0 << 0);             // Minimal power @ RFO pin
#elif 1  // POWER_CALIBRATION
      ctx->LinkADR_TxPowerReduction_dB  = 0;
      ctx->LinkADR_DataRate_TX1         = DR5_SF7_125kHz_LoRa;
      ctx->LinkADR_DataRate_RXTX2       = DR5_SF7_125kHz_LoRa;
      spi1TxBuffer[1] = spiSX127xPower_GetSetting(ctx);
#else
      spi1TxBuffer[1] = spiSX127xPower_GetSetting(ctx);
#endif
      spi1TxBuffer[2] = PA_RAMP_50us;                                     // PA ramp time 50us
      spi1TxBuffer[3] = (0x1 << 5) | (0xb << 0);                          // OverCurrentProtection ON, normal: 100mA
      spiProcessSpiMsg(4);

      /* Preamble length */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x20;
      spi1TxBuffer[1] = 0x00;                                             // PreambleMsb, PreambleLsb
      spi1TxBuffer[2] = 0x0a;                                             // +4 = 14 symbols
      spiProcessSpiMsg(3);

      /* I/Q inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
      spi1TxBuffer[1] = 0x27;                                             // This is the default value, no inversion
      spiProcessSpiMsg(2);

      /* I/Q2 inversion bits */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
      spi1TxBuffer[1] = 0x1d;                                             // This is the default value, no inversion
      spiProcessSpiMsg(2);

      /* Set transmit message length */
      if (!msg) {
        Error_Handler();
      }
      spiSX127xLoRa_setTxMsgLen(msg->msg_encoded_Len);

      /* Prepare the transmitter circuits */
      spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | FSTX);
    }
    break;

  case TxRx_Mode_RX:
  case TxRx_Mode_RX2:
    {
      /* Set the frequency */
      spiSX127xFrequency_MHz(l_f * (1 + (1e-6 * ctx->CrystalPpm) - (1e-6 * ctx->GatewayPpm)));

      /* SymbtimeoutLsb */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x1f;
      spi1TxBuffer[1] = (l_SF >= SF10_DR2 ?  0x05 : 0x08);
      spiProcessSpiMsg(2);

      /* DetectionThreshold */
      spi1TxBuffer[0] = SPI_WR_FLAG | 0x37;
      spi1TxBuffer[1] = (l_SF >= SF7_DR5 ?  0x0a : 0x0c);
      spiProcessSpiMsg(2);

      /* Bugfix 2013-09 Rev.1 Section 2.3 - Receiver Spurious Reception of a LoRa Signal */
      {
        /* DetectionOptimize & BugFix (automatic disabled) */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x31;
        spi1TxBuffer[1] = 0x40 | (l_SF >= SF7_DR5 ?  0x03 : 0x05);        // Instead of 0xc0 --> 0x40
        spiProcessSpiMsg(2);

        spi1TxBuffer[0] = SPI_WR_FLAG | 0x2f;
        spi1TxBuffer[1] = 0x40;
        spi1TxBuffer[2] = 0x00;
        spiProcessSpiMsg(3);
      }

      /* Skip for RX2 where only frequency and SpreadingFactor is changed */
      if (TxRx_Mode_RX2 != mode) {
        /* LNA to maximum */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x0c;
        spi1TxBuffer[1] = LnaGain_G1 | LnaBoost_Lf_XXX | LnaBoost_Hf_ON;
        spiProcessSpiMsg(2);

        /* Max. payload length */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x23;
        spi1TxBuffer[1] = 0x40;
        spiProcessSpiMsg(2);

        /* I/Q inversion bits */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x33;
        spi1TxBuffer[1] = (0x1 << 6) | 0x27;                                // Optimized for inverted IQ
        spiProcessSpiMsg(2);

        /* I/Q2 inversion bits */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x38;
        spi1TxBuffer[1] = 0x19;                                             // Optimized for inverted IQ
        spiProcessSpiMsg(2);
      }

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

uint32_t spiSX127x_WaitUntil_TxDone(uint32_t stopTime)
{
  uint32_t              ts              = 0UL;
  volatile EventBits_t  eb              = { 0 };
  volatile uint8_t      irq             = 0U;

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
      eb = xEventGroupWaitBits(extiEventGroupHandle, EXTI_SX__DIO0, EXTI_SX__DIO0, 0, ticks);

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x12;     // RegIrqFlags
      spiProcessSpiMsg(2);
      irq = spi1RxBuffer[1];

      if ((eb & EXTI_SX__DIO0) || (irq & (1U << TxDoneMask))) {
        /* Remember point of time when TxDone was set */
        ts = xTaskGetTickCount();

        /* Reset all IRQ flags */
        spi1TxBuffer[0] = SPI_WR_FLAG | 0x12;   // RegIrqFlags
        spi1TxBuffer[1] = 0xff;
        spiProcessSpiMsg(2);
      }
    }
  }

  /* Time of TX completed */
  return ts;
}


#ifdef DEBUG_RX
static void spiSX127x_WaitUntil__RX_analyzer(LoRaWANctx_t* ctx, uint32_t now, uint32_t stopTime)
{
  char  debugBuf[512]   = { 0 };
  int   debugLen        = 0;

  do {
    /* Get the current IRQ flags */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x12;                                           // LoRa: RegIrqFlags
    spiProcessSpiMsg(2);
    uint8_t irq = spi1RxBuffer[1];

    /* Get the RF states */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x18;
    spiProcessSpiMsg(5);
#if 1
    uint8_t   modemStat   = spi1RxBuffer[1];                                        // LoRa: RegModemStat
    int8_t    packetSnr   = spi1RxBuffer[2];                                        // LoRa: RegPktSnrValue
    uint8_t   packetRssi  = spi1RxBuffer[3];                                        // LoRa: RegPktRssiValue
    uint16_t  rssi        = spi1RxBuffer[4];                                        // LoRa: RegRssiValue
#else
    if (modemStat & 0x0b) {
      packetRssi  = spi1RxBuffer[3];
    }
    if (modemStat & 0x40) {
      packetSnr   = spi1RxBuffer[2];
      rssi        = spi1RxBuffer[4];
    }
#endif
    ctx->LastRSSIDbm            = (int16_t) (packetSnr >= 0 ?  (-157 + 16.0/15.0 * packetRssi)  : (-157 +                                 (int16_t)rssi));
    ctx->LastPacketStrengthDBm  = (int16_t) (packetSnr >= 0 ?  (-157 +             rssi)        : (-157 + packetRssi + 0.25 * packetSnr                ));

    /* RX offset and PPM calculation */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x28;
    spiProcessSpiMsg(5);
    int32_t fei         = ((uint32_t)spi1RxBuffer[1] << 16) | ((uint32_t)spi1RxBuffer[2] << 8) | (spi1RxBuffer[3]);   // LoRa: RegFeiMsb, RegFeiMid, RegFeiLsb
    if (fei >= (1UL << 19)) {
      fei -= (1UL << 20);
    }

    float feiHz         = ((fei / 32e6) * (1UL << 24) * 125.) / 500.;
    float feiPpm        = feiHz / ctx->FrequencyMHz;

    spi1TxBuffer[0] = SPI_RD_FLAG | 0x2c;
    spiProcessSpiMsg(2);
    uint8_t rssiWideband = spi1RxBuffer[1];                                         // LoRa: RegRssiWideband

    spi1TxBuffer[0] = SPI_RD_FLAG | 0x14;
    spiProcessSpiMsg(5);
    uint16_t rxHeaderCnt   = ((uint16_t)spi1RxBuffer[1] << 8) | spi1RxBuffer[2];    // LoRa: RxHeaderCnt
    uint16_t rxValidPktCnt = ((uint16_t)spi1RxBuffer[3] << 8) | spi1RxBuffer[4];    // LoRa: RxValidPacketCnt

    /* Check FIFO pointer */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x10;
    spiProcessSpiMsg(2);
    uint8_t fifoRxCurAddr = spi1RxBuffer[1];                                        // LoRa: FifoRxCurrentAddr

    /* Check FIFO RX byte address */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x25;
    spiProcessSpiMsg(2);
    uint8_t fifoRxByteAddr = spi1RxBuffer[1];                                       // LoRa: FifoRxByteAddr

    debugLen += sprintf((char*) debugBuf + debugLen,
        "timespan=%10lu -->\t" \
        "irq=0x%02x \t" \
        "modemStat=0x%02x \t" \
        "rssiWideband=%+-i dBm \tRSSI=%-4i dBm \tPacket Strength=%-4i dBm \tpacketSnr=%-4i dB \t\t" \
        "rxHdrCnt=%5u \trxValidPktCnt=%5u \t\t" \
        "fifoRxByteAddr=0x%02x \tfifoCurAddr=0x%02x \t\t" \
        "feiHz=%7ld \tfeiPpm=%+7ld\r\n",
        (now - ctx->TsEndOfTx),
        irq,
        modemStat,
        -157 + rssiWideband, ctx->LastRSSIDbm, ctx->LastPacketStrengthDBm, packetSnr,
        rxHeaderCnt, rxValidPktCnt,
        fifoRxByteAddr, fifoRxCurAddr,
        (int32_t)feiHz, (int32_t)feiPpm);

    /* Push debugging string to the USB DCD */
    if (debugLen) {
      osSemaphoreWait(usbToHostBinarySemHandle, 0);
      usbToHostWait((uint8_t*) debugBuf, debugLen);
      osSemaphoreRelease(usbToHostBinarySemHandle);

      debugLen = 0;
    }

    /* Next iteration */
    HAL_Delay(20);
    now = osKernelSysTick();
  } while (stopTime > now);
}
#endif

void spiSX127x_WaitUntil_RxDone(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg, uint32_t stopTime)
{
  volatile EventBits_t  eb;
  volatile uint8_t      irq;
  uint32_t              now             = xTaskGetTickCount();
//uint8_t               modemStat       = 0;
  uint8_t               packetSnr       = 0;
  uint8_t               packetRssi      = 0;
  uint8_t               rssi            = 0;

  /* Use TxDone and RxDone - mask out all other IRQs */
  spi1TxBuffer[0] = SPI_WR_FLAG | 0x11;                                                         // RegIrqFlagsMask
  spi1TxBuffer[1] = (0x1 << RxTimeoutMask);                                                     // Mask out: bit7 RxTimeout
  spiProcessSpiMsg(2);

#ifdef DEBUG_RX
  spiSX127x_WaitUntil__RX_analyzer(ctx, now, stopTime);

  while (1)  HAL_Delay(250);
#endif

  do {
    /* Wait for EXTI / IRQ line(s) */
    volatile TickType_t ticks = 1;

    if (stopTime > now) {
      ticks = (stopTime - now) / portTICK_PERIOD_MS;
    } else {
      break;
    }

    eb  = xEventGroupWaitBits(extiEventGroupHandle, EXTI_SX__DIO0, EXTI_SX__DIO0, 0, ticks);
    now = xTaskGetTickCount();

    /* Get the current IRQ flags */
    spi1TxBuffer[0] = SPI_RD_FLAG | 0x12;     // LoRa: RegIrqFlags
    spiProcessSpiMsg(2);
    irq = spi1RxBuffer[1];

    if ((eb & EXTI_SX__DIO0) || (irq & (1U << RxDoneMask))) {
      /* Reset all IRQ flags */
      spiSX127xRegister_IRQ_clearAll();

      /* Check for CRC */
      if (irq & (1U << PayloadCrcErrorMask)) {
        continue;
      }

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x18;                                                     // LoRa: RegModemStat
      spiProcessSpiMsg(5);
//    modemStat   = spi1RxBuffer[1];
      packetSnr   = spi1RxBuffer[2];
      packetRssi  = spi1RxBuffer[3];
      rssi        = spi1RxBuffer[4];
      ctx->LastRSSIDbm           = (int16_t) (packetSnr >= 0 ?  (-157 + 16.0/15.0 * packetRssi) : (-157 + (int16_t)rssi));
//    ctx->LastPacketStrengthDbm = (int16_t) (packetSnr >= 0 ?  (-157 +                   rssi) : (-157 +    packetRssi + 0.25 * packetSnr));
      ctx->LastPacketSnrDb       = packetSnr;

      /* RX offset and PPM calculation */
      spi1TxBuffer[0] = SPI_RD_FLAG | 0x28;
      spiProcessSpiMsg(5);
      int32_t fei         = ((uint32_t)spi1RxBuffer[1] << 16) | ((uint32_t)spi1RxBuffer[2] << 8) | (spi1RxBuffer[3]);   // LoRa: RegFeiMsb, RegFeiMid, RegFeiLsb
      if (fei >= (1UL << 19)) {
        fei   -= (1UL << 20);
      }
      ctx->LastFeiHz  = ((fei / 32e6) * (1UL << 24) * 125.) / 500.;
      ctx->LastFeiPpm = ctx->LastFeiHz / ctx->FrequencyMHz;

#ifdef DEBUG2_RX
      spi1TxBuffer[0] = SPI_RD_FLAG | 0x14;    // ValidHeaderCnt, ValidPacketCnt
      spiProcessSpiMsg(5);
      uint16_t rxHeaderCnt   = ((uint16_t)spi1RxBuffer[1] << 8) | spi1RxBuffer[2];
      uint16_t rxValidPktCnt = ((uint16_t)spi1RxBuffer[3] << 8) | spi1RxBuffer[4];

      /* Check FIFO pointer */
      spi1TxBuffer[0] = SPI_RD_FLAG | 0x10;
      spiProcessSpiMsg(2);
      uint8_t fifoRxCurAddr = spi1RxBuffer[1];                                                  // LoRa: FifoRxCurrentAddr

      /* Check FIFO RX byte address */
      spi1TxBuffer[0] = SPI_RD_FLAG | 0x25;
      spiProcessSpiMsg(2);
      uint8_t fifoRxByteAddr = spi1RxBuffer[1];                                                 // LoRa: FifoRxByteAddr

      debugLen += sprintf((char*) debugBuf + debugLen,
            "timespan=%5lu ms -->\t" \
            "eb=0x%08lx | irq=0x%02x \t" \
            "modemStat=0x%02x \t" \
/*          "rssi=%-4i dBm \t" \ */
            "RSSI=%-4i dBm \t" \
/*          "Packet Strength=%-4i dBm \t" \ */
            "packetSnr=%-4i dB \t\t" \
            "rxHdrCnt=%5u \trxValidPktCnt=%5u \t\t" \
            "fifoRxByteAddr=0x%02x \tfifoCurAddr=0x%02x \t\t" \
            "feiHz=%7ld \tfeiPpm=%+7ld\r\n",
            (now - spiPreviousWakeTime),
            (uint32_t)eb, irq,
            modemStat,
//          (-157 + rssi),
            ctx->LastRSSIDbm,
//          ctx->LastPacketStrength_dBm,
            packetSnr,
            rxHeaderCnt, rxValidPktCnt,
            fifoRxByteAddr, fifoRxCurAddr,
            (int32_t)feiHz, (int32_t)feiPpm);
#endif

      spi1TxBuffer[0] = SPI_RD_FLAG | 0x13;                                                     // RegRxNbBytes
      spiProcessSpiMsg(2);
      volatile uint8_t rxNbBytes = spi1RxBuffer[1];

      /* FIFO readout */
      if (rxNbBytes) {
        /* Positioning of the FIFO addr ptr */
        {
          spi1TxBuffer[0] = SPI_RD_FLAG | 0x10;                                                 // RegFifoRxCurrentAddr
          spiProcessSpiMsg(2);
          uint8_t fifoRxCurrentAddr = spi1RxBuffer[1];

          spi1TxBuffer[0] = SPI_WR_FLAG | 0x0d;                                                 // RegFifoAddrPtr
          spi1TxBuffer[1] = fifoRxCurrentAddr;
          spiProcessSpiMsg(2);

#ifdef DEBUG2_RX
        debugLen += sprintf((char*)debugBuf + debugLen, " rxNbBytes=%03u fifoRxCurrentAddr=%02x:", rxNbBytes, fifoRxCurrentAddr);
#endif
        }

        /* FIFO read out */
        if (rxNbBytes < sizeof(spi1RxBuffer)) {
          spi1TxBuffer[0] = SPI_RD_FLAG | 0x00;    // RegFifo
          spiProcessSpiMsg(1 + rxNbBytes);

          /* Copy SPI receive buffer content to msg object */
          memcpy((void*)msg->msg_encoded_Buf, (const void*)spi1RxBuffer + 1, rxNbBytes);
          msg->msg_encoded_Len = rxNbBytes;

#ifdef DEBUG2_RX
          for (uint8_t idx = 0; idx < rxNbBytes; ++idx) {
            debugLen += sprintf((char*)debugBuf + debugLen, " %02x", spi1RxBuffer[1 + idx]);
          }
#endif
        } else {
          /* Buffer to small */
          Error_Handler();
        }
      }

#ifdef DEBUG2_RX
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

  /* Turn to sleep mode if not already done */
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | SLEEP);

  /* Request RD-address 0x42 RegVersion */
  {
    uint8_t sxVersion = 0;

    spi1TxBuffer[0] = SPI_RD_FLAG | 0x42;
    if (HAL_OK == spiProcessSpiMsg(2)) {
      sxVersion = spi1RxBuffer[1];
    }

    if ((sxVersion != 0x22)  // SX1272
        &&
        (sxVersion != 0x12)) {  // SX1276
      /* We can handle Version  0x22 (SX1272)  and  0x12 (SX1276) only */
      return HAL_ERROR;
    }
  }

  /* SX127x mbed shield found and ready for transmissions */
  return HAL_OK;
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
