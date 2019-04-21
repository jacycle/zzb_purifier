#ifndef __WIFI_CLIENT_H__
#define __WIFI_CLIENT_H__

#include <stdint.h>
#include "lwip/inet.h"
#include "lwip/sockets.h"
/* Exported types ------------------------------------------------------------*/
#define WIFI_LINK_FAIL      0
#define WIFI_LINK_UNLINK    1
#define WIFI_LINK_CONFIG    2
#define WIFI_LINK_LINK      3

#define WIFI_CMD_CONFIG     0
#define WIFI_CMD_STATE      1
#define WIFI_CMD_RESET      2

#define ET_CRC_LEN            (4)
#define ET_CMD_PING           0x01
#define ET_CMD_PONG           0x02
#define ET_CMD_HOST           0x03
#define ET_CMD_HOST_RESP      0x04
#define ET_CMD_PUSH           0x05
#define ET_CMD_PUSH_RESP      0x06
#define ET_CMD_UDP_REQ        0xee
#define ET_CMD_UDP_REQ_RESP   0xef

#define ET_HBT_TIMEOUT_2S     (2 * 1000)
#define ET_HBT_TIMEOUT_15S    (15 * 1000)

#define WIFI_LINK_FAIL      0
#define WIFI_LINK_UNLINK    1
#define WIFI_LINK_CONFIG    2
#define WIFI_LINK_LINK      3

/* Exported types ------------------------------------------------------------*/
/* define type */
#pragma pack(1)
typedef struct __pack_header_s {
        uint32_t len;            /* packet len */
        uint8_t version;         /* protocol */
        uint64_t seq;            /* sequence */
        uint64_t devId;          /* device id */
        uint8_t command;         /* command */
//        uint8_t reserved[0];     /* data + crc */
} pack_header_t;
#pragma pack()

/* Private typedef------------------------------------------------------------*/
#define UART_MAX_BUFFER_SIZE             255
#define UART_TX_BUFFER_SIZE              200
#define UART_RX_BUFFER_SIZE              128

#define UART2_TX_BUFFER_SIZE              380
#define UART2_RX_BUFFER_SIZE              380

//
// ethome protocol
//
#define ETHOME_SUCCESS                  0x00
#define ETHOME_ERR_ADDR                 0x01
#define ETHOME_ERR_PTR                  0x02
#define ETHOME_ERR_PKT_HEAD             0x03
#define ETHOME_ERR_PKG_LEN              0x04
#define ETHOME_ERR_PKG_CMD              0x05
#define ETHOME_ERR_PKG_DATA             0x06
#define ETHOME_ERR_BTN_PRESENT          0x07
#define ETHOME_ERR_CRC                  0x08

/* Private typedef------------------------------------------------------------*/
typedef struct 
{
    unsigned short len;
    unsigned char buffer[UART2_TX_BUFFER_SIZE];
}WifiTxMsg_t;

typedef struct 
{
    unsigned short len;
    unsigned char buffer[UART2_RX_BUFFER_SIZE];
}WifiRxMsg_t;

/* Exported types ------------------------------------------------------------*/
extern const char *typeId_str;
extern WifiRxMsg_t wifi_rxmsg;
extern uint32_t group_online[];
extern uint8_t reset_flag;
extern uint8_t d_sys_reset;
extern uint32_t udpreq_counter;
extern rt_mutex_t timer_mutex;
extern uint16_t check_push_delay;
extern uint16_t check_group_online;
extern uint16_t g_check_delay[];

/* Exported types ------------------------------------------------------------*/
void et_configinit(void);
int et_get_server_addr(int fd, int flags, struct in_addr *sin_addr, uint16_t *port,
        uint8_t *pbuf, uint16_t len);
int wifi_get_server_info(struct in_addr *sin_addr, uint16_t *port,
        uint8_t *pbuf, uint16_t len);
int EtSendUdpReq(int fd, int flags, const struct sockaddr *to, socklen_t tolen,
                  uint64_t seq, uint64_t deviceId, int eth);
int et_data_process(int fd, int flags, struct sockaddr *to, socklen_t tolen,
        uint8_t *pbuf, uint16_t len, int eth);
int EtSendHbtData(int fd, int flags, const struct sockaddr *to, socklen_t tolen,
                  uint64_t seq, uint64_t deviceId, int eth);
int et_group_online(void);
uint8_t et_checkpush(int eth);
void wifi_snd_udpreq(int eth);
void wifi_hbt_process(int eth);
uint8_t wifi_checkpush(int eth);

#endif
