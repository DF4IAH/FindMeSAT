/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stddef.h>
#include <sys/_stdint.h>

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define B1_UserButton_Pin GPIO_PIN_13
#define B1_UserButton_GPIO_Port GPIOC
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define GPS_1PPS_Pin GPIO_PIN_7
#define GPS_1PPS_GPIO_Port GPIOF
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define SX_NRESET_Pin GPIO_PIN_3
#define SX_NRESET_GPIO_Port GPIOA
#define SPI_A_SCK_Pin GPIO_PIN_5
#define SPI_A_SCK_GPIO_Port GPIOA
#define SPI_A_MISO_Pin GPIO_PIN_6
#define SPI_A_MISO_GPIO_Port GPIOA
#define SPI_A_MOSI_Pin GPIO_PIN_7
#define SPI_A_MOSI_GPIO_Port GPIOA
#define SX_DIO2_Pin GPIO_PIN_14
#define SX_DIO2_GPIO_Port GPIOF
#define SX_DIO0_Pin GPIO_PIN_15
#define SX_DIO0_GPIO_Port GPIOF
#define SX_DIO3_Pin GPIO_PIN_11
#define SX_DIO3_GPIO_Port GPIOE
#define SX_DIO1_Pin GPIO_PIN_13
#define SX_DIO1_GPIO_Port GPIOE
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define SPI_A_SEL_Pin GPIO_PIN_14
#define SPI_A_SEL_GPIO_Port GPIOD
#define USB_OverCurrent_Pin GPIO_PIN_5
#define USB_OverCurrent_GPIO_Port GPIOG
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define STLK_RX_Pin GPIO_PIN_7
#define STLK_RX_GPIO_Port GPIOG
#define STLK_TX_Pin GPIO_PIN_8
#define STLK_TX_GPIO_Port GPIOG
#define LD1_Pin GPIO_PIN_7
#define LD1_GPIO_Port GPIOC
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SIM7000_TX_Pin GPIO_PIN_12
#define SIM7000_TX_GPIO_Port GPIOC
#define SIM7000_RX_Pin GPIO_PIN_2
#define SIM7000_RX_GPIO_Port GPIOD
#define SMPS_V1_Pin GPIO_PIN_10
#define SMPS_V1_GPIO_Port GPIOG
#define SMPS_EN_Pin GPIO_PIN_11
#define SMPS_EN_GPIO_Port GPIOG
#define SMPS_PG_Pin GPIO_PIN_12
#define SMPS_PG_GPIO_Port GPIOG
#define SMPS_SW_Pin GPIO_PIN_13
#define SMPS_SW_GPIO_Port GPIOG
#define PG14__Pin GPIO_PIN_14
#define PG14__GPIO_Port GPIOG
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define SIM7000_RTS_Pin GPIO_PIN_4
#define SIM7000_RTS_GPIO_Port GPIOB
#define SIM7000_CTS_Pin GPIO_PIN_5
#define SIM7000_CTS_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_9
#define I2C1_SDA_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */
#define USE_FULL_ASSERT     1U

#define FINDMESAT_VERSION   20180510U

typedef void * TaskHandle_t;

void mainDefaultTaskInit(void);
void mainDefaultTaskLoop(void);

void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void SystemResetbyARMcore(void);
void vAssertCalled( const char *pcFile, uint32_t ulLine);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
