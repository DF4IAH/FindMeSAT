/* USER CODE BEGIN Header */
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
  * Copyright (c) 2019 STMicroelectronics International N.V. 
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stddef.h>
#include <sys/_stdint.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
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
#define SX_RESET_Pin GPIO_PIN_3
#define SX_RESET_GPIO_Port GPIOA
#define SPI_A_SCK_Pin GPIO_PIN_5
#define SPI_A_SCK_GPIO_Port GPIOA
#define SPI_A_MISO_Pin GPIO_PIN_6
#define SPI_A_MISO_GPIO_Port GPIOA
#define SPI_A_MOSI_Pin GPIO_PIN_7
#define SPI_A_MOSI_GPIO_Port GPIOA
#define SX_RXTX_EXT_Pin GPIO_PIN_4
#define SX_RXTX_EXT_GPIO_Port GPIOC
#define GPS_TX_Pin GPIO_PIN_5
#define GPS_TX_GPIO_Port GPIOC
#define SX_DIO4_Pin GPIO_PIN_12
#define SX_DIO4_GPIO_Port GPIOF
#define SX_DIO4_EXTI_IRQn EXTI15_10_IRQn
#define SX_DIO2_Pin GPIO_PIN_14
#define SX_DIO2_GPIO_Port GPIOF
#define SX_DIO2_EXTI_IRQn EXTI15_10_IRQn
#define SX_DIO0_Pin GPIO_PIN_15
#define SX_DIO0_GPIO_Port GPIOF
#define SX_DIO0_EXTI_IRQn EXTI15_10_IRQn
#define SX_DIO3_Pin GPIO_PIN_11
#define SX_DIO3_GPIO_Port GPIOE
#define SX_DIO3_EXTI_IRQn EXTI15_10_IRQn
#define SX_DIO1_Pin GPIO_PIN_13
#define SX_DIO1_GPIO_Port GPIOE
#define SX_DIO1_EXTI_IRQn EXTI15_10_IRQn
#define GPS_RX_Pin GPIO_PIN_10
#define GPS_RX_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define SPI_A_SEL_Pin GPIO_PIN_14
#define SPI_A_SEL_GPIO_Port GPIOD
#define SX_DIO5_Pin GPIO_PIN_15
#define SX_DIO5_GPIO_Port GPIOD
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
#define PA_USB_N_Pin GPIO_PIN_11
#define PA_USB_N_GPIO_Port GPIOA
#define PA_USB_P_Pin GPIO_PIN_12
#define PA_USB_P_GPIO_Port GPIOA
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
/* USER CODE BEGIN Private defines */
#define USE_FULL_ASSERT     1U

#ifndef PI
# define PI                 3.14159265358979f
#endif

#ifndef min
# define min(a,b)   (a) < (b) ?  (a) : (b)
#endif

#ifndef max
# define max(a,b)   (a) > (b) ?  (a) : (b)
#endif


#define FINDMESAT_VERSION   20190223U


typedef enum ENABLE_MASK {

//ENABLE_MASK__AFSK_APRS1200                                  = 0x0001UL,                       // AFSK APRS 1200 baud - supported by AX5243, only
  ENABLE_MASK__FSK_APRS1200_AFSK_EMU                          = 0x0002UL,                       // AFSK APRS 1200 baud - audio Packet-Radio emulation
  ENABLE_MASK__FSK_APRS9600                                   = 0x0004UL,                       // FSK APRS 9600 baud
  ENABLE_MASK__LORA_BARE                                      = 0x0010UL,                       // LoRa bare Transceiver
  ENABLE_MASK__LORAWAN_DEVICE                                 = 0x0020UL,                       // LoRaWAN as Device role
  ENABLE_MASK__GPS_DATA                                       = 0x0100UL,                       // GPS data interpreter
  ENABLE_MASK__GPS_1PPS                                       = 0x0200UL,                       // GPS 1PPS controlled MCU clock

} ENABLE_MASK_t;


typedef enum MON_MASK {

  MON_MASK__LORA                                              = 0x01UL,
  MON_MASK__GPS_RX                                            = 0x04UL,
  MON_MASK__GPS_TIMESYNC                                      = 0x08UL,

} MON_MASK_t;


typedef void* TaskHandle_t;

void mainDefaultTaskInit(void);
void mainDefaultTaskLoop(void);

void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

int32_t setRealTime(uint64_t unixTime_us);
uint64_t getRealTime(void);

void SystemResetbyARMcore(void);
void vAssertCalled( const char *pcFile, uint32_t ulLine);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

void mainCalc_Float2Int(float in, uint32_t* out_i, uint16_t* out_p1000);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
