#include <board.h>
#include <rtthread.h>
#include <rthw.h>
#include "config.h"
#include "wifi_port.h"
#include "integer.h"
#include "queue.h"
#include "flash.h"
#include "devset.h"
#include "led.h"

/* Private define ------------------------------------------------------------*/
#define WIFI_RESETN_PORT    GPIOD
#define WIFI_RELOAD_PORT    GPIOD
#define WIFI_LINK_PORT      GPIOD
#define WIFI_READY_PORT     GPIOD

#define WIFI_RESETN_PIN     GPIO_Pin_7
#define WIFI_RELOAD_PIN     GPIO_Pin_2
#define WIFI_LINK_PIN       GPIO_Pin_0
#define WIFI_READY_PIN      GPIO_Pin_1

#define WIFI_UART_TX_TIME     22

/* Private variables ---------------------------------------------------------*/
uint8_t wifi_uart_time;
uint8_t wifi_uart_recvtout;
WifiMsg_t wifi_uart_msg;
//uint64_t self_dev_id = 0x020260fff801;

// rt-thread only
static rt_device_t device, write_device;

#define WIFI_UART_QUEUE_SIZE (sizeof(WifiMsg_t) * 8)
    
static uint8_t uart_queue_buffer[WIFI_UART_QUEUE_SIZE];
static QUEUE uart_queue;

#define WIFI_CFG_LED1  0
//#define WIFI_CFG_LED2

/**
 * This function will int wifi message queue
 */
void wifi_queue_init(void)
{
    queue_create(&uart_queue, sizeof(WifiMsg_t), 
        (void *)uart_queue_buffer, WIFI_UART_QUEUE_SIZE);
}

/**
 * wifi message queue send msg
 */
void wifi_queue_send(WifiMsg_t *msg)
{
    queue_send(&uart_queue, (void *)msg);
}

/**
 * wifi message queue receive msg
 */
uint32_t wifi_queue_receive(WifiMsg_t *msg)
{
    return queue_receive(&uart_queue, (void *)msg);
}

/**
 * byte order convert 
 */
//#define htonl(x)   ((((uint32_t)(x) & 0xff000000) >> 24) |  \
//                    (((uint32_t)(x) & 0x00ff0000) >> 8) |   \
//                    (((uint32_t)(x) & 0x0000ff00) << 8) |   \
//                    (((uint32_t)(x) & 0x000000ff) << 24) )

/**
 * uart input data indicate 
 */
rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
//    struct rx_msg msg;
//    msg.dev = dev;
//    msg.size = size;
//        
//    rt_mq_send(rx_mq, &msg, sizeof(struct rx_msg));
//    return RT_EOK;
    
    rt_uint32_t rx_length;
    
    
    if (wifi_uart_msg.len < WIFI_RECV_BUFF_SIZE)
    {
        rx_length = rt_device_read(dev, 0, &wifi_uart_msg.buffer[wifi_uart_msg.len],
                    WIFI_RECV_BUFF_SIZE - wifi_uart_msg.len);
        wifi_uart_msg.len += rx_length;
    }
    wifi_uart_recvtout = 10;
    
    return RT_EOK;
}

/**
 * wifi variables init 
 */
void wifi_config_init(void)
{
    uint8_t buffer[8];
    uint32_t temp[2];
    uint8_t i;

    //self_dev_id = 0x0201f0fff800;
    for (i=0; i<6; i++)
    {
        buffer[i] = FLASH_ReadByte(FLASH_ADDR_SRC_ADDR + i);
    }
    temp[1] = 0;
    temp[0] = 0;
    temp[1] |= buffer[0] << 8;
    temp[1] |= buffer[1] << 0;
    temp[0] |= buffer[2] << 24;
    temp[0] |= buffer[3] << 16;
    temp[0] |= buffer[4] << 8;
    temp[0] |= buffer[5] << 0;
    self_dev_id = (uint64_t)((uint64_t)temp[1] << 32);
    self_dev_id += temp[0];
    rt_kprintf("deviceid=%04x%08x\r\n", temp[1], temp[0]);
    
    device = rt_device_find("uart4");
    if (device!= RT_NULL)
    {
        rt_device_set_rx_indicate(device, uart_input);
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
    }
    write_device = device;
}

/**
 * wifi gpio init 
 */
void wifi_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = WIFI_RESETN_PIN;
    GPIO_Init(WIFI_RESETN_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = WIFI_RELOAD_PIN;
    GPIO_Init(WIFI_RELOAD_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin   = WIFI_LINK_PIN;
    GPIO_Init(WIFI_LINK_PORT, &GPIO_InitStructure);
    
    GPIO_SetBits(WIFI_RELOAD_PORT, WIFI_RELOAD_PIN);;
    GPIO_SetBits(WIFI_RESETN_PORT, WIFI_RESETN_PIN);
}

/**
 * port for delay function 
 *
 * @param ms delay time
 */
static void wifi_delay(uint32_t ms)
{
    rt_thread_delay(ms/(1000/RT_TICK_PER_SECOND));
}


/**
 * reset the wifi module 
 */
void wifi_module_reset(void)
{
    GPIO_SetBits(WIFI_RELOAD_PORT, WIFI_RELOAD_PIN);
    GPIO_ResetBits(WIFI_RESETN_PORT, WIFI_RESETN_PIN);
    wifi_delay(150);
    GPIO_SetBits(WIFI_RESETN_PORT, WIFI_RESETN_PIN);
    wifi_delay(150);
}

/**
 * enable smart-config mode or factory reset 
 */
void wifi_module_reload(uint8_t level)
{
    if (level)
    {
        GPIO_SetBits(WIFI_RELOAD_PORT, WIFI_RELOAD_PIN);
    }
    else
    {
        GPIO_ResetBits(WIFI_RELOAD_PORT, WIFI_RELOAD_PIN);
    }
}

/**
 * thie function will get wifi module link status 
 * 
 * return 0/link  1/unlink
 */
int wifi_module_link(void)
{
    if (GPIO_ReadInputDataBit(WIFI_LINK_PORT, WIFI_LINK_PIN))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
  * @brief  Reset the system
  * @param  None
  * @retval None
  */

void wifi_soft_reset(void)
{
    wifi_delay(500);
    NVIC_SystemReset();
}

void wifi_usart_txdata(uint8_t *buf, uint32_t buflen)
{
    if (write_device != RT_NULL)
        rt_device_write(write_device, 0, &buf[0], buflen);
    wifi_uart_time = 12;
}

/* function
 * send the msg to server
 * 0: success, nonzero: fail
 */
int wifi_send_msg(uint8_t* msg, int msgLen)
{
	wifi_usart_txdata(msg, msgLen);
    wifi_delay(WIFI_UART_TX_TIME);
	return 0;
}

int wifi_usart_read(uint8_t *buf, uint16_t buflen, uint16_t *rlen)
{
    WifiMsg_t msg;
    int ret;
    rt_base_t level;
    
    level = rt_hw_interrupt_disable();
    ret = wifi_queue_receive(&msg);
    rt_hw_interrupt_enable(level);
    
    if (ret == 0)
    {
        if (buflen > WIFI_RECV_BUFF_SIZE)
        {
            buflen = WIFI_RECV_BUFF_SIZE;
        }
        if (msg.len > buflen)
        {
            msg.len = buflen;
        }
        memcpy(buf, msg.buffer, (unsigned int)msg.len);
        *rlen = msg.len;
        
        return 0;
    }
    else
    {
        return -1;
    }
}

void wifi_config_mode0(void)
{
    uint8_t step1_counter;
    uint8_t i;
    uint8_t retry;
    uint8_t sockb_retry;
    uint8_t buflen = 128;
    uint8_t buf[128];
    uint16_t readlen;

    step1_counter = 0;
step1:
    if (step1_counter > 10)
        goto reset;
    step1_counter++;
#ifdef WIFI_CFG_LED1
    LEDToggle(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDToggle(WIFI_CFG_LED2);
#endif
    retry = 0;
    sockb_retry = 0;
    wifi_usart_txdata("+++", 3);
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "a", 1) == 0 && readlen == 1)
            {
                goto step2;
            }
            else if (memcmp(buf, "+++", 3) == 0 && readlen == 3)
            {
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    goto step1;
    
step2:
    wifi_usart_txdata("a", 1);
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "+ok", 3) == 0 && readlen == 3)
            {
                retry = 0;
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step2;
    
step3:
    wifi_usart_txdata("AT+NETP\r\n", strlen("AT+NETP\r\n"));
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            char *socka = "+ok=UDP,CLIENT,12999," SERVER_ADDR_URL;
            char *pair = strstr((const char *)buf, socka);
            if (pair)
            {
                retry = 0;
                goto reset;
            }
            else
            {
                sockb_retry++;
                break;
            }
        }
    }
    if (sockb_retry > 3)
    {
        retry = 0;
        goto step4;
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step3;
    
step4:
   {
        char *socka = "AT+NETP=UDP,CLIENT,12999," SERVER_ADDR_URL "\r\n";
        int len = strlen(socka);
        wifi_usart_txdata((uint8_t *)socka, strlen(socka));
        socka = "AT+NETP=UDP,CLIENT,12999," SERVER_ADDR_URL;
        for (i=0; i<10; i++)
        {
            wifi_delay(50);
            if (wifi_usart_read(buf, buflen, &readlen) == 0)
            {
                char *pair = strstr((char *)buf, socka);
                if (pair)
                {
                     pair = strstr((char *)buf, "+ok");
                    if (pair)
                    {
                        retry = 0;
                        goto reset;
                    }
                }
                else
                {
                    sockb_retry++;
                    break;
                }
            }
        }
        if (retry > 5)
        {
            goto step1;
        }
        retry++;
        goto step4;
   }
   
reset:
#ifdef WIFI_CFG_LED1
    LEDOff(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDOff(WIFI_CFG_LED2);
#endif
    wifi_module_reset();
    return;
}

void wifi_config_udpserver(void)
{
    uint8_t step1_counter;
    uint8_t i;
    uint8_t retry;
    uint8_t sockb_retry;
    uint8_t buflen = 128;
    uint8_t buf[128];
    uint16_t readlen;

    step1_counter = 0;
step1:
//    if (step1_counter > 10)
//        goto reset;
    step1_counter++;
#ifdef WIFI_CFG_LED1
    LEDToggle(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDToggle(WIFI_CFG_LED2);
#endif
    retry = 0;
    sockb_retry = 0;
    wifi_usart_txdata("+++", 3);
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "a", 1) == 0 && readlen == 1)
            {
                goto step2;
            }
            else if (memcmp(buf, "+++", 3) == 0 && readlen == 3)
            {
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    goto step1;
    
step2:
    wifi_usart_txdata("a", 1);
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "+ok", 3) == 0 && readlen >= 3)
            {
                retry = 0;
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step2;
    
step3:
    wifi_usart_txdata("AT+NETP\r\n", strlen("AT+NETP\r\n"));
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            char *socka = "+ok=UDP,CLIENT,"UDP_SERVER_PORT"," UDP_SERVER_ADDR_URL;
            char *pair = strstr((const char *)buf, socka);
            if (pair)
            {
                retry = 0;
                goto reset;
            }
            else
            {
                sockb_retry++;
                break;
            }
        }
    }
    if (sockb_retry > 3)
    {
        retry = 0;
        goto step4;
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step3;
    
step4:
   {
        char *socka = "AT+NETP=UDP,CLIENT,"UDP_SERVER_PORT"," UDP_SERVER_ADDR_URL "\r\n";
        int len = strlen(socka);
        wifi_usart_txdata((uint8_t *)socka, strlen(socka));
        socka = "AT+NETP=UDP,CLIENT,"UDP_SERVER_PORT"," UDP_SERVER_ADDR_URL;
        for (i=0; i<10; i++)
        {
            wifi_delay(50);
            if (wifi_usart_read(buf, buflen, &readlen) == 0)
            {
                char *pair = strstr((char *)buf, socka);
                if (pair)
                {
                     pair = strstr((char *)buf, "+ok");
                    if (pair)
                    {
                        retry = 0;
                        goto reset;
                    }
                }
                else
                {
                    sockb_retry++;
                    break;
                }
            }
        }
        if (retry > 5)
        {
            goto step1;
        }
        retry++;
        goto step4;
   }
    
reset:
#ifdef WIFI_CFG_LED1
    LEDOff(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDOff(WIFI_CFG_LED2);
#endif
    wifi_module_reset();
    return;
}

void wifi_config_mode1(uint32_t ipaddr, uint16_t port)
{
    uint8_t i;
    uint8_t retry;
    uint8_t sockb_retry;
    uint8_t buflen = 128;
    uint8_t buf[128];
    uint16_t readlen;

step1:
    retry = 0;
    sockb_retry = 0;
    wifi_usart_txdata("+++", 3);
#ifdef WIFI_CFG_LED1
    LEDToggle(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDToggle(WIFI_CFG_LED2);
#endif
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "a", 1) == 0 && readlen == 1)
            {
                goto step2;
            }
            else if (memcmp(buf, "+++", 3) == 0 && readlen == 3)
            {
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    goto step1;
    
step2:
    wifi_usart_txdata("a", 1);
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            if (memcmp(buf, "+ok", 3) == 0 && readlen == 3)
            {
                retry = 0;
                goto step3;
            }
            else
            {
                break;
            }
        }
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step2;
    
step3:
    wifi_usart_txdata("AT+NETP\r\n", strlen("AT+NETP\r\n"));
    for (i=0; i<10; i++)
    {
        wifi_delay(50);
        if (wifi_usart_read(buf, buflen, &readlen) == 0)
        {
            char sockb[64];
            char *pair;
            uint8_t sockb_ipaddr[4];
            
            sockb_ipaddr[0] = ipaddr >> 24;
            sockb_ipaddr[1] = ipaddr >> 16;
            sockb_ipaddr[2] = ipaddr >>  8;
            sockb_ipaddr[3] = ipaddr >>  0;
            snprintf(sockb, 64, "+ok=UDP,CLIENT,%d,%d.%d.%d.%d",
                     port, 
                     sockb_ipaddr[0], 
                     sockb_ipaddr[1], 
                     sockb_ipaddr[2],
                     sockb_ipaddr[3]);
            pair = strstr((const char *)buf, sockb);
            if (pair)
            {
                retry = 0;
                goto reset;
            }
            else
            {
                sockb_retry++;
                break;
            }
        }
    }
    if (sockb_retry > 3)
    {
        retry = 0;
        goto step4;
    }
    if (retry > 5)
    {
        goto step1;
    }
    retry++;
    goto step3;
    
step4:
   {
        char sockb[64];
        int len;
        uint8_t sockb_ipaddr[4];
        
        sockb_ipaddr[0] = ipaddr >> 24;
        sockb_ipaddr[1] = ipaddr >> 16;
        sockb_ipaddr[2] = ipaddr >>  8;
        sockb_ipaddr[3] = ipaddr >>  0;
        snprintf(sockb, 64, "AT+NETP=UDP,CLIENT,%d,%d.%d.%d.%d\r\n", 
                 port, 
                 sockb_ipaddr[0], 
                 sockb_ipaddr[1], 
                 sockb_ipaddr[2],
                 sockb_ipaddr[3]);
        len = strlen(sockb);
        wifi_usart_txdata((uint8_t *)sockb, len);
        snprintf(sockb, 64, "AT+NETP=UDP,CLIENT,%d,%d.%d.%d.%d", 
                 port, 
                 sockb_ipaddr[0], 
                 sockb_ipaddr[1], 
                 sockb_ipaddr[2],
                 sockb_ipaddr[3]);
        for (i=0; i<10; i++)
        {
            wifi_delay(50);
            if (wifi_usart_read(buf, buflen, &readlen) == 0)
            {
                char *pair = strstr((char *)buf, sockb);
                if (pair)
                {
                    pair = strstr((char *)buf, "+ok");
                    if (pair)
                    {
                        retry = 0;
                        goto reset;
                    }
                }
                else
                {
                    sockb_retry++;
                    break;
                }
            }
        }
        if (retry > 5)
        {
            goto step1;
        }
        retry++;
        goto step4;
   }

reset:
   wifi_module_reset(); 
#ifdef WIFI_CFG_LED1
    LEDOff(WIFI_CFG_LED1);
#endif
#ifdef WIFI_CFG_LED2
    LEDOff(WIFI_CFG_LED2);
#endif
    return;
}

