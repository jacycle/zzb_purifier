#include <rtthread.h>
#include <stm32f10x.h>
#include "key.h"

/* Private variables ---------------------------------------------------------*/

static GPIO_TypeDef* GPIO_PORT[BUTTONn] = 
{
    KEY1_GPIO_PORT,
    KEY2_GPIO_PORT,
    KEY3_GPIO_PORT,
    KEY4_GPIO_PORT,
};

static const uint32_t GPIO_PIN[BUTTONn] = 
{
    KEY1_GPIO_PIN,
    KEY2_GPIO_PIN,
    KEY3_GPIO_PIN,
    KEY4_GPIO_PIN,
};
                                       
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  Button: Specifies the Button to be configured.
  *          This parameter can be one of following parameters:
  *            @arg BUTTON_TAMPER: Tamper Push Button  
  *            @arg BUTTON_KEY: Key Push Button
  *            @arg BUTTON_RIGHT: Joystick Right Push Button 
  *            @arg BUTTON_LEFT: Joystick Left Push Button 
  *            @arg BUTTON_UP: Joystick Up Push Button 
  *            @arg BUTTON_DOWN: Joystick Down Push Button
  *            @arg BUTTON_SEL: Joystick Sel Push Button      
  * @param  Button_Mode: Specifies Button mode.
  *          This parameter can be one of following parameters:   
  *            @arg BUTTON_MODE_GPIO: Button will be used as simple IO 
  *            @arg BUTTON_MODE_EXTI: Button will be connected to EXTI line with interrupt
  *                     generation capability
  * @retval None
  */
//void Button_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode)
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(KEY1_GPIO_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY2_GPIO_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY3_GPIO_RCC, ENABLE);
    RCC_APB2PeriphClockCmd(KEY4_GPIO_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = KEY1_GPIO_PIN;
    GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = KEY2_GPIO_PIN;
    GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);
  
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = KEY3_GPIO_PIN;
    GPIO_Init(KEY3_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = KEY4_GPIO_PIN;
    GPIO_Init(KEY4_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief  Returns the selected Button state.
  * @param  Button: Specifies the Button to be checked.
  *          This parameter can be one of following parameters:
  *            @arg BUTTON_TAMPER: Tamper Push Button  
  *            @arg BUTTON_KEY: Key Push Button 
  *            @arg BUTTON_RIGHT: Joystick Right Push Button 
  *            @arg BUTTON_LEFT: Joystick Left Push Button 
  *            @arg BUTTON_UP: Joystick Up Push Button 
  *            @arg BUTTON_DOWN: Joystick Down Push Button
  *            @arg BUTTON_SEL: Joystick Sel Push Button     
  * @retval The Button GPIO pin value.
  */
uint8_t KEY_GetState(Button_TypeDef Button)
{
    uint8_t ret;
    switch(Button)
    {
        case BUTTON_KEY1:
            ret = GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN);
            break;
        case BUTTON_KEY2:
            ret = GPIO_ReadInputDataBit(KEY2_GPIO_PORT, KEY2_GPIO_PIN);
            break;
        case BUTTON_KEY3:
            ret = GPIO_ReadInputDataBit(KEY3_GPIO_PORT, KEY3_GPIO_PIN);
            break;
        case BUTTON_KEY4:
            ret = GPIO_ReadInputDataBit(KEY4_GPIO_PORT, KEY4_GPIO_PIN);
            break;
        default:
            ret = 0;
    }
    
    return ret;
}

uint32_t KEY_GetStates(void)
{
    uint8_t i;
    uint32_t state;
    uint32_t ret;

    ret = 0;
    for (i=0; i<BUTTONn; i++)
    {
        state = KEY_GetState((Button_TypeDef)i);
        if (state) ret |= 1 << i;
    }

    return ret;
}
