#include <rtthread.h>
#include <stdint.h>
#include "eth_client.h"
#include "integer.h"
#include "queue.h"

#define ETH_QUEUE_SIZE (sizeof(EthMsg_t) * 12)
    
static uint8_t eth_queue_buffer[ETH_QUEUE_SIZE];
static QUEUE eth_queue;

/**
 * This function will int wifi message queue
 */
void eth_queue_init(void)
{
    queue_create(&eth_queue, sizeof(EthMsg_t), 
        (void *)eth_queue_buffer, ETH_QUEUE_SIZE);
}

/**
 * wifi message queue send msg
 */
void eth_queue_send(EthMsg_t *msg)
{
    queue_send(&eth_queue, (void *)msg);
}

/**
 * wifi message queue receive msg
 */
uint32_t eth_queue_receive(EthMsg_t *msg)
{
    return queue_receive(&eth_queue, (void *)msg);
}
