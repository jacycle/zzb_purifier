#ifndef __ETH_CLIENT_H__
#define __ETH_CLIENT_H__

#include "integer.h"
#include "queue.h"

/* Exported types ------------------------------------------------------------*/
#define ETH_RECV_BUFF_SIZE       380

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
    unsigned short len;
    unsigned char event;
    unsigned char buffer[ETH_RECV_BUFF_SIZE];
}EthMsg_t;

/* Exported types ------------------------------------------------------------*/
void eth_queue_init(void);

/**
 * wifi message queue send msg
 */
void eth_queue_send(EthMsg_t *msg);

/**
 * wifi message queue receive msg
 */
uint32_t eth_queue_receive(EthMsg_t *msg);

#endif
