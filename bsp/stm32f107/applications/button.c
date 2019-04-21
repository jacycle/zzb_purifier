#include <rtthread.h>
#include <rthw.h>
/* Includes ------------------------------------------------------------------*/
#include "integer.h"
#include "queue.h"
#include "key.h"
#include "button.h"
#include "wifi_port.h"

/* Private define ------------------------------------------------------------*/
#define BUTTON_QUEUE_SIZE (sizeof(BUTTON_PID_STATE) * 3)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const ButtonState_t BUTTON_STATE[BUTTONn] = {
  {BUTTON_KEY1, 0},
};

unsigned char button_time[BUTTONn];
QUEUE button_queue;

static uint8_t button_queue_buffer[BUTTON_QUEUE_SIZE];
static uint8_t press[BUTTONn];
//static uint8_t presscnt;
//static uint8_t presscnt[KEY_STAT_NUMBER];

//*********************************************************************
//
// function prototypes 
//
//*********************************************************************
void BUTTON_QueueInit(void)
{
    queue_create(&button_queue, sizeof(BUTTON_PID_STATE), 
                 (void *)button_queue_buffer, BUTTON_QUEUE_SIZE);
}

void BUTTON_TimerHandle(void)
{
    uint8_t i;
    uint32_t states;
    
    states = KEY_GetStates();
    for (i=0; i<BUTTONn; i++)
    {
        if (button_time[i])
        {
            if (button_time[i] == 1)
            {
                if (BUTTON_StoreState(i, states) != 0)
                {
                    button_time[i] = BUTTON_DELAY_100MS;
                }
                else
                {
                    button_time[i] = 0;
                }
            }
            else
            {
                button_time[i]--;
            }
        }
        else
        {
            if (BUTTON_StoreState(i, states) != 0)
            {
                button_time[i] = BUTTON_DELAY_100MS;
            }
        }
    }
    
}

uint8_t BUTTON_StoreState(uint8_t index, uint32_t states)
{
    BUTTON_PID_STATE Button;
    rt_base_t level;
    
    //if (Button_GetState(BUTTON_STATE[index].Button) == BUTTON_STATE[index].State)
    if ((states & (1 << index)) == BUTTON_STATE[index].State)
    {
        if (press[index] == 0)
        {
            Button.State = BUTTON_STATE[index].Button;
            Button.Notify = BUTTON_NOTIFICATION_CLICKED;
            Button.BtnStates = states;
            press[index] = 1;
            
//            if (BUTTON_STATE[index].Button == BUTTON_MATCHCODE)
//            {
//                presscnt = 0;
//            }
            level = rt_hw_interrupt_disable();
            queue_send(&button_queue, (void *)&Button);
            rt_hw_interrupt_enable(level);
        }
        else
        {
//            if (BUTTON_STATE[index].Button == BUTTON_MATCHCODE)
//            {
//                if (presscnt <= BUTTON_MATCH_CODE_TOTAL_DELAY)
//                {
//                    if ( (presscnt >= BUTTON_MATCH_CODE_CLICKED_DELAY) &&
//                         ((presscnt % BUTTON_MATCH_CODE_PRESSED_DELAY) == 0) )
//                    {
//                        Button.State = BUTTON_STATE[index].Button;
//                        Button.Notify = BUTTON_NOTIFICATION_PRESSED;
//                        Button.PressedCnt = presscnt;
//    
//                        queue_send(&button_queue, (void *)&Button);
//                    }
//                    presscnt++;
//                }
//            }
//            else
            {
                Button.State = BUTTON_STATE[index].Button;
                Button.Notify = BUTTON_NOTIFICATION_PRESSED;
                Button.BtnStates = states;
                level = rt_hw_interrupt_disable();
                queue_send(&button_queue, (void *)&Button);
                rt_hw_interrupt_enable(level);
            }
        }
    }
    else
    {
        if (press[index])
        {
            Button.State = BUTTON_STATE[index].Button;
            Button.Notify = BUTTON_NOTIFICATION_RELEASED;
            Button.BtnStates = states;
            press[index] = 0;
            
//            if (BUTTON_STATE[index].Button == BUTTON_MATCHCODE)
//            {
//                presscnt = 0;
//            }
            level = rt_hw_interrupt_disable();
            queue_send(&button_queue, (void *)&Button);
            rt_hw_interrupt_enable(level);
        }
    }
    
    return press[index];
}

/*********************************************************************************************************
** function    : BUTTON_Task
** description : 
** input       : 
** output      : 
** author      : pengjq
** datga       : 2015-07-28
**-------------------------------------------------------------------------------------------------------
** Modified :
** data     :
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/

void BUTTON_Task(void)
{
    UINT ret;
    BUTTON_PID_STATE Button;
    rt_base_t level;
    
    level = rt_hw_interrupt_disable();
    ret = queue_receive(&button_queue, (void *)&Button);
    rt_hw_interrupt_enable(level);
    if (QUEUE_SUCCESS == ret)
    {
        if( Button.State == BUTTON_KEY1)
        {
            if (Button.Notify == BUTTON_NOTIFICATION_CLICKED)
            {
                rt_kprintf("KEY1 clicked\r\n");
            }
            else if (Button.Notify == BUTTON_NOTIFICATION_PRESSED)
            {
                {
                    //printf("KEY1&KEY3 pressed\r\n");
                    if (wifi_link_state() != WIFI_CFGMODE)
                    {
                        wifi_module_reload(0);
                        rt_thread_delay(RT_TICK_PER_SECOND / 5);
                        wifi_module_reload(1);
                        wifi_link_setstate(WIFI_CFGMODE);
                        rt_kprintf("wifi enter config mode\r\n");
                    }
                }
            }
        }
    }
}
