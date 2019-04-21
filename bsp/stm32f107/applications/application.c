/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <stm32_eth.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif
#include "wifi_client.h"
#include "key.h"
#include "led.h"
#include "devset.h"
#include "mbapp.h"
#include "config.h"
#include "wifi_port.h"
#include "wifi_client.h"
#include "eth_client.h"
#include "multi_button.h"
#include "timer.h"
#include "pwm.h"

//*****************************************************************************
//
//! \internal
//!
//*****************************************************************************
#define MAX_SOCKETS             2
#define ETH_MODE                1
#define WIFI_MODE               0

//*****************************************************************************
//
//! \rtthread variable
//!
//*****************************************************************************
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[1024];
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t btn_stack[1536];
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t wifiserv_stack[1536];
static struct rt_thread led_thread;
static struct rt_thread btn_thread;
static struct rt_thread wifi_thread;
static rt_mutex_t pkg_mutex = RT_NULL;

//*****************************************************************************
//
//! \private variable
//!
//*****************************************************************************
static struct Button btn1;
static struct Button btn2;
static struct Button btn3;
static struct Button btn4;

static rt_timer_t btn_tiemr;

//*****************************************************************************
//
//! \read button gpio
//!
//*****************************************************************************
static uint8_t read_button1_GPIO() 
{
	return KEY_GetState(BUTTON_KEY1);
}

static uint8_t read_button2_GPIO() 
{
	return KEY_GetState(BUTTON_KEY2);
}

static uint8_t read_button3_GPIO() 
{
	return KEY_GetState(BUTTON_KEY3);
}

static uint8_t read_button4_GPIO() 
{
	return KEY_GetState(BUTTON_KEY4);
}

void BTN1_PRESS_DOWN_Handler(void* btn)
{
	//do something...
}

void BTN1_PRESS_UP_Handler(void* btn)
{
	//do something...
}

void BTN2_PRESS_DOWN_Handler(void* btn)
{
	//do something...
}

void BTN2_PRESS_UP_Handler(void* btn)
{
	//do something...
}

void BTN3_PRESS_DOWN_Handler(void* btn)
{
	//do something...
}

void BTN3_PRESS_UP_Handler(void* btn)
{
	//do something...
}

void BTN4_PRESS_DOWN_Handler(void* btn)
{
	//do something...
}

void BTN4_PRESS_UP_Handler(void* btn)
{
	//do something...
}
//*****************************************************************************
//
//! \function: udpserv
//!
//*****************************************************************************

static void button_task(void* parameter)
{
    rt_base_t level;
    int sensor_addr = -1;
    uint8_t buffer[16];
    uint8_t readlen;

  	button_init(&btn1, read_button1_GPIO, 0);
	  button_init(&btn2, read_button2_GPIO, 0);
    button_init(&btn1, read_button3_GPIO, 0);
	  button_init(&btn2, read_button4_GPIO, 0);
  
  	button_attach(&btn1, PRESS_DOWN,       BTN1_PRESS_DOWN_Handler);
	  button_attach(&btn1, PRESS_UP,         BTN1_PRESS_UP_Handler);
  
    button_attach(&btn2, PRESS_DOWN,       BTN2_PRESS_DOWN_Handler);
	  button_attach(&btn2, PRESS_UP,         BTN2_PRESS_UP_Handler);
  
    button_attach(&btn3, PRESS_DOWN,       BTN3_PRESS_DOWN_Handler);
	  button_attach(&btn3, PRESS_UP,         BTN3_PRESS_UP_Handler);
  
    button_attach(&btn4, PRESS_DOWN,       BTN4_PRESS_DOWN_Handler);
	  button_attach(&btn4, PRESS_UP,         BTN4_PRESS_UP_Handler);
  
    button_start(&btn1);
    button_start(&btn2);
    button_start(&btn3);
    button_start(&btn4);
	
    //make the timer invoking the button_ticks() interval 5ms.
	  btn_tiemr = rt_timer_create("btn", button_ticks,
                             RT_NULL, RT_TICK_PER_SECOND / 100,
                             RT_TIMER_FLAG_PERIODIC);
    if (btn_tiemr != RT_NULL) rt_timer_start(btn_tiemr);
    
    while (1)
    {
        eMBErrorCode eStatus;
      
        if (sensor_addr == -1)
        {
            uint8_t addr;
          
            eStatus = eMBReadSensorAddr(&addr);
            if (eStatus == MB_ENOERR)
            {
                sensor_addr = addr;
            }
        }
        else
        {
            eStatus = eMBReadData(sensor_addr, buffer, 16, &readlen);
            if (eStatus == MB_ENOERR)
            {
                int index = 3;
                level = rt_hw_interrupt_disable();
                devstate.CO2  = buffer[index++] << 8;
                devstate.CO2 |= buffer[index++] << 0;
                devstate.TVOC  = buffer[index++] << 8;
                devstate.TVOC |= buffer[index++] << 0;
                devstate.CH2O  = buffer[index++] << 8;
                devstate.CH2O |= buffer[index++] << 0;
                devstate.PM2_5  = buffer[index++] << 8;
                devstate.PM2_5 |= buffer[index++] << 0;
                devstate.Humidity  = buffer[index++] << 8;
                devstate.Humidity |= buffer[index++] << 0;
                devstate.Temperature  = buffer[index++] << 8;
                devstate.Temperature |= buffer[index++] << 0;
                devstate.PM10  = buffer[index++] << 8;
                devstate.PM10 |= buffer[index++] << 0;
                rt_hw_interrupt_enable(level);              
            }
        }
        rt_thread_delay(RT_TICK_PER_SECOND / 2);

    }
}

//*****************************************************************************
//
//! \function: wifi server
//!
//*****************************************************************************
static void wifi_server(void* parameter)
{
    WifiMsg_t et_msg;
    rt_base_t level;
    int ret;

    pwm_init();
    pwm_set_duty(3, 500);
    pwm_set_duty(4, 500);
    while (1)
    {
        rt_thread_delay( RT_TICK_PER_SECOND / 100 );

        level = rt_hw_interrupt_disable();
        ret = wifi_queue_receive(&et_msg);
        rt_hw_interrupt_enable(level);
        if (ret == 0)
        {
            rt_kprintf("et_data_process\r\n");
        }
    }
}


//*****************************************************************************
//
//! \function: 1ms server
//!
//*****************************************************************************
void TIM6_IRQHandler(void)
{
  rt_base_t level;
    
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
	{
        if(wifi_uart_recvtout)
        {
            wifi_uart_recvtout--;
            if (wifi_uart_recvtout == 0)
            {
                level = rt_hw_interrupt_disable();
                wifi_queue_send(&wifi_uart_msg);
                wifi_uart_msg.len = 0;
                rt_hw_interrupt_enable(level);
            }
        }
        if (modubs_uart_recvtout)
        {
            modubs_uart_recvtout--;
            if (modubs_uart_recvtout == 0)
            {
                level = rt_hw_interrupt_disable();
                modubs_queue_send(&mb_msg);
                mb_msg.len = 0;
                rt_hw_interrupt_enable(level);
            }
        }
	}
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

/**
 * Tout = Prescaler * reloadcounter / 40K
 */
void IWDG_Configuration(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(1000);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

//*****************************************************************************
//
//! \function: 1s server
//!
//*****************************************************************************
static void led_thread_entry(void* parameter)
{
   
#ifdef WATCHDOG
    IWDG_Configuration();
#endif
    while (1)
    {
#ifdef WATCHDOG
        IWDG_ReloadCounter();
#endif
        rt_thread_delay( RT_TICK_PER_SECOND / 100);
#ifdef WATCHDOG
        IWDG_ReloadCounter();
#endif      
    }
}


void rt_init_thread_entry(void* parameter)
{
    {
        extern void rt_platform_init(void);
        rt_platform_init();
    }

    /* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* initialize the device file system */
	dfs_init();

	/* initialize the elm chan FatFS file system*/
	elm_init();
    
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
    {
        rt_kprintf("File System initialzation failed!\n");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

#ifdef RT_USING_LWIP
	/* initialize lwip stack */
	/* register ethernetif device */
	eth_system_device_init();

	/* initialize lwip system */
	lwip_system_init();
	rt_kprintf("TCP/IP initialized!\n");
#endif

#ifdef RT_USING_FINSH
	/* initialize finsh */
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}

int rt_application_init(void)
{
    rt_thread_t tid;
    rt_err_t result;

    //et_configinit();
    KEY_Init();
    Modbus_Init();
    rt_hw_led_init();
    wifi_gpio_init();
    wifi_queue_init();
    wifi_config_init();
//    TIM3_Int_Init(999, SystemCoreClock/1000000);
    TIM6_Int_Init(999, SystemCoreClock/1000000);
    
    pkg_mutex = rt_mutex_create("pkg_mutex", RT_IPC_FLAG_FIFO);
    if (pkg_mutex == RT_NULL)
    {
        rt_kprintf("rt_mutex_create pkg_mutex failed!\n");
    }
    
    /* init eth thread */
    result = rt_thread_init(&btn_thread,
                            "btn",
                            button_task,
                            RT_NULL,
                            (rt_uint8_t*)&btn_stack[0],
                            sizeof(btn_stack),
                            22,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&btn_thread);
    }
    
    /* init wifi thread */
    result = rt_thread_init(&wifi_thread,
                            "wifi",
                            wifi_server,
                            RT_NULL,
                            (rt_uint8_t*)&wifiserv_stack[0],
                            sizeof(wifiserv_stack),
                            21,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&wifi_thread);
    }
    
    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }
    
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    return 0;
}

/*@}*/
