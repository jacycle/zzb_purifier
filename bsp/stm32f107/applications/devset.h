#ifndef __DEVSET_H__
#define __SEVSET_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include "lwip/sockets.h"

typedef struct
{
    int fd;
    int flags;
    struct sockaddr *to;
    socklen_t tolen;
    uint8_t status;
    uint8_t targetTemp;
    uint8_t setFlag;
} DevSet_t;

typedef struct
{
    unsigned short CO2;
    unsigned short TVOC;
    unsigned short CH2O;
    unsigned short PM2_5;
    unsigned short Humidity;
    unsigned short Temperature;
    unsigned short PM10;
} DevState_t;

typedef struct 
{
    unsigned short group_num;
    unsigned char pwr_enable;
    signed short set_temp;
    unsigned char wind_speed;
    unsigned char wind_dir;
    unsigned char mode_type;
    signed short temp;
    unsigned char err_code;
    unsigned char err_counter;
}DaKin_t;

typedef struct
{
    uint8_t link_state;
    uint16_t level;
}WifiStatus_t;

/* function ------------------------------------------------------------------*/
extern uint8_t link_state;
extern uint8_t wifi_link;
extern uint16_t et_hbt_time;
extern uint16_t et_udp_time;
extern DevSet_t devset;
extern DevState_t devstate;
extern uint16_t et_hbt_time;
extern uint64_t self_dev_id;
extern uint64_t g_seq;
extern uint64_t g_set_seq;
extern uint64_t g_list_seq;
extern uint8_t host_mac_addr[];
extern WifiStatus_t g_WifiStatus;

#endif /* __MAIN_H */
