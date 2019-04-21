#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>

/* Private define ------------------------------------------------------------*/
/**
  * @}
  */ 

/** @addtogroup STM320518_EVAL_LOW_LEVEL_BUTTON
  * @{
  */  
#define BUTTONn                            4

/**
 * @brief Tamper push-button
 */
#define MATCHCODE_BUTTON_PIN                BIT0
#define MATCHCODE_BUTTON_GPIO_PORT          P3
#define MATCHCODE_BUTTON_GPIO_CLK           RCC_AHBPeriph_GPIOB
#define MATCHCODE_BUTTON_EXTI_LINE          EXTI_Line15
#define MATCHCODE_BUTTON_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOB
#define MATCHCODE_BUTTON_EXTI_PIN_SOURCE    EXTI_PinSource15
#define MATCHCODE_BUTTON_EXTI_IRQn          EXTI4_15_IRQn 

/**
 * @brief Key push-button
 */
#define KEY1_GPIO_PIN              GPIO_Pin_12
#define KEY1_GPIO_PORT             GPIOE
#define KEY1_GPIO_RCC              RCC_APB2Periph_GPIOE

#define KEY2_GPIO_PIN              GPIO_Pin_13
#define KEY2_GPIO_PORT             GPIOE
#define KEY2_GPIO_RCC              RCC_APB2Periph_GPIOE

#define KEY3_GPIO_PIN              GPIO_Pin_14
#define KEY3_GPIO_PORT             GPIOE
#define KEY3_GPIO_RCC              RCC_APB2Periph_GPIOE

#define KEY4_GPIO_PIN              GPIO_Pin_15
#define KEY4_GPIO_PORT             GPIOE
#define KEY4_GPIO_RCC              RCC_APB2Periph_GPIOE


/* Exported types ------------------------------------------------------------*/
typedef enum 
{
  BUTTON_KEY1 = 0,
  BUTTON_KEY2,
  BUTTON_KEY3,
  BUTTON_KEY4,
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;

typedef enum 
{ 
  JOY_NONE = 0,
  JOY_SEL = 1,
  JOY_DOWN = 2,
  JOY_LEFT = 3,
  JOY_RIGHT = 4,
  JOY_UP = 5
} JOYState_TypeDef;

/* Exported functions --------------------------------------------------------*/
//void Button_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
void KEY_Init(void);
uint32_t KEY_GetStates(void);
uint8_t KEY_GetState(Button_TypeDef Button);

#endif
