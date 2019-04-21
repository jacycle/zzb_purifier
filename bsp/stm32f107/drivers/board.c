/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>
#include <rthw.h>
#include "board.h"
#include "usart.h"
#include "config.h"

/**
 * @addtogroup STM32
 */

/*@{*/
/*******************************************************************************
* Function Name  : IAP_Set
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static int iap_enable;
void IAP_Set(void)
{
    iap_enable = 1;
    
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x10000);
}

int IAP_Get(void)
{
    return iap_enable;
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern int Image$$ER_IROM1$$RO$$Base;
void NVIC_Configuration(void)
{
    static int rom_base;
    #define STM32_ROM_BEGIN (&Image$$ER_IROM1$$RO$$Base)
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */
    rom_base = (int)STM32_ROM_BEGIN;
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, rom_base - NVIC_VectTab_FLASH);
#endif
    
    IAP_Set();
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}

/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init(void)
{
    /* NVIC Configuration */
    NVIC_Configuration();

    /* Configure the SysTick */
    SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

    rt_hw_usart_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
}

/*@}*/
