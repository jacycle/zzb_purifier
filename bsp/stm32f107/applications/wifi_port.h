#ifndef __WIFI_PORT_H__
#define __WIFI_PORT_H__

#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
#define WIFI_RECV_BUFF_SIZE       800  // 380

/* Exported types ------------------------------------------------------------*/
//typedef enum
//{
//    WIFI_INIT = 0,
//    WIFI_UNLINK = 1,
//    WIFI_CFGMODE = 2,
//    WIFI_LINK,
//}WifiLinkState_e;

typedef struct 
{
    unsigned short len;
    unsigned char event;
    unsigned char buffer[WIFI_RECV_BUFF_SIZE];
}WifiMsg_t;

typedef enum
{
    WIFI_INIT = 0,
    WIFI_UNLINK = 1,
    WIFI_CFGMODE = 2,
    WIFI_LINK,
}WifiLinkState_e;

/* Exported function ---------------------------------------------------------*/
extern uint8_t wifi_uart_time;
extern uint8_t wifi_uart_recvtout;
extern WifiMsg_t wifi_uart_msg;

/* Exported function ---------------------------------------------------------*/
void wifi_gpio_init(void);
void wifi_config_init(void);
void wifi_module_reset(void);
void wifi_module_reload(uint8_t level);
int wifi_module_link(void);
void wifi_soft_reset(void);
void wifi_link_change(void);
WifiLinkState_e wifi_link_state(void);
void wifi_link_setstate(WifiLinkState_e mode);

int wifi_send_msg(uint8_t* msg, int msgLen);
void wifi_usart_txdata(uint8_t *buf, uint32_t buflen);
int wifi_usart_read(uint8_t *buf, uint16_t buflen, uint16_t *rlen);

void wifi_queue_init(void);
void wifi_queue_send(WifiMsg_t *msg);
uint32_t wifi_queue_receive(WifiMsg_t *msg);

void wifi_config_mode0(void);
void wifi_config_udpserver(void);
void wifi_config_mode1(uint32_t ipaddr, uint16_t port);

#endif
