/*
 * exti.c
 *
 *  Created on: 21.05.2018
 *      Author: DF4IAH
 */

#include "exti.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"


extern EventGroupHandle_t extiEventGroupHandle;
extern EventGroupHandle_t loraEventGroupHandle;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  BaseType_t taskWoken = 0;

  switch (GPIO_Pin) {
  case GPIO_PIN_11:
    /* Check for PE11 pin */
    if (GPIOE->IDR & GPIO_IDR_ID11) {
      xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO3, &taskWoken);
    }
    break;

  case GPIO_PIN_13:
    /* Check for B1_UserButton PC13 */
    if (GPIOC->IDR & GPIO_IDR_ID13) {
      xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_B1, &taskWoken);
    }

    /* Check for PE13 pin */
    if (GPIOE->IDR & GPIO_IDR_ID13) {
      xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO1, &taskWoken);
    }
    break;

  case GPIO_PIN_14:
    /* Check for PF14 pin */
    if (GPIOF->IDR & GPIO_IDR_ID14) {
      xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO2, &taskWoken);
    }
    break;

  case GPIO_PIN_15:
    /* Check for PF15 pin */
    if (GPIOF->IDR & GPIO_IDR_ID15) {
      xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO0, &taskWoken);
      xEventGroupSetBitsFromISR(loraEventGroupHandle, /*Lora_EGW__EXTI_DIO0*/ 0x00001000UL, &taskWoken);
    }
    break;

  default:
    //xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO4, &taskWoken);
    //xEventGroupSetBitsFromISR(extiEventGroupHandle, EXTI_SX__DIO5, &taskWoken);
    break;
  }
}
