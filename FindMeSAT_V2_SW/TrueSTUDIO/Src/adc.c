/**
  ******************************************************************************
  * File Name          : ADC.c
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
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
#include "adc.h"

#include "dma.h"

/* USER CODE BEGIN 0 */
#include "FreeRTOS.h"
#include "cmsis_os.h"


extern EventGroupHandle_t adcEventGroupHandle;
uint32_t adcTempCal1 = 0;
uint32_t adcTempCal2 = 0;
uint32_t adcVrefintCal = 0;
float adcTempCorFact = 0.0f;
float adcTempCorOffs = 0.0f;

static uint16_t adc1_dma_buf[3];  // NbrOfConversion

void prvAdcDmaStart(void);
void prvAdcDmaStop(void);

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the ADC multi-mode 
    */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_VBAT;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

    /* Pre-calculate factors and offsets */
    {
      /* Temperature */
      adcTempCal1 = *TEMPSENSOR_CAL1_ADDR;
      adcTempCal2 = *TEMPSENSOR_CAL2_ADDR;
      adcTempCorFact = (110 - 30) / ((float) (adcTempCal2 - adcTempCal1));
      adcTempCorOffs = 36.14f + (30.f - adcTempCorFact * adcTempCal1);  // Local offset is first term

      /* Vrefint */
      adcVrefintCal = *VREFINT_CAL_ADDR;
    }

    /* ADC calibration */
    if (HAL_ADCEx_Calibration_Start(adcHandle, ADC_SINGLE_ENDED) !=  HAL_OK)
    {
      Error_Handler();
    }

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();
  
    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Request = DMA_REQUEST_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspInit 1 */

#if 0
    if (HAL_ADCEx_Calibration_Start(adcHandle, ADC_SINGLE_ENDED) !=  HAL_OK)
    {
      Error_Handler();
    }
#endif

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    /* ADC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  BaseType_t pxHigherPriorityTaskWoken = 0;
  xEventGroupSetBitsFromISR(adcEventGroupHandle, ADC__CONV_AVAIL, &pxHigherPriorityTaskWoken);
}

uint16_t adcGetVdda_mV(void)
{
  /* Activate ADC DMA */
  prvAdcDmaStart();

  EventBits_t eb = xEventGroupWaitBits(adcEventGroupHandle, ADC__CONV_AVAIL, ADC__CONV_AVAIL, 0, portMAX_DELAY);
  if (eb & ADC__CONV_AVAIL) {
    float valVdda = adc1_dma_buf[0];
    valVdda /= 16.f;
    valVdda  = 0.9640f * (3000.f * adcVrefintCal / valVdda);  // Local correction is first term
    return (uint16_t) valVdda;
  }
  return 0UL;
}

uint16_t adcGetVbat_mV(void)
{
  /* Activate ADC DMA */
  prvAdcDmaStart();

  EventBits_t eb = xEventGroupWaitBits(adcEventGroupHandle, ADC__CONV_AVAIL, ADC__CONV_AVAIL, 0, portMAX_DELAY);
  if (eb & ADC__CONV_AVAIL) {
    float valVdda = adc1_dma_buf[0];
    valVdda /= 16.f;
    valVdda  = 0.9640f * (3000.f * adcVrefintCal / valVdda);  // Local correction is first term

    float valVbat = adc1_dma_buf[1];
    valVbat /= 16.f;
    valVbat *= 1.5605f * valVdda / (1UL << 11);  // Local correction is first term
    return (uint16_t) valVbat;
  }
  return 0UL;
}

int16_t adcGetTemp_100(void)
{
  /* Activate ADC DMA */
  prvAdcDmaStart();

  EventBits_t eb = xEventGroupWaitBits(adcEventGroupHandle, ADC__CONV_AVAIL, ADC__CONV_AVAIL, 0, portMAX_DELAY);
  if (eb & ADC__CONV_AVAIL) {
    volatile float val = adc1_dma_buf[2];
    val /= 16.f;
    val *= adcTempCorFact;
    val += adcTempCorOffs;
    val *= 100.f;
    return (int16_t) (val >= 0 ?  (0.5f + val) : (-0.5f + val));
  }
  return 0L;
}

void prvAdcDmaStart(void)
{
  /* Check if ADC DMA is already active */
  EventBits_t eb = xEventGroupGetBits(adcEventGroupHandle);
  if (!(eb & ADC__CONV_RUN)) {
    /* Start ADC DMA */
    HAL_StatusTypeDef adc1Std = HAL_ADC_Start_DMA(&hadc1, (uint32_t *) adc1_dma_buf, sizeof(adc1_dma_buf) / sizeof(uint16_t));
    if (adc1Std != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

void prvAdcDmaStop(void)
{
  /* Check if ADC DMA is already active */
  EventBits_t eb = xEventGroupGetBits(adcEventGroupHandle);
  if (eb & ADC__CONV_RUN) {
    /* Stop ADC DMA */
    HAL_StatusTypeDef adc1Std = HAL_ADC_Stop_DMA(&hadc1);
    if (adc1Std != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
