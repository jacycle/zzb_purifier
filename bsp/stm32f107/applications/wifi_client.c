#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <board.h>
#include <rtthread.h>
#include <rthw.h>
#include "wifi_port.h"
#include "wifi_client.h"
#include "configcheck.h"
#include "devset.h"
#include "crc32.h"
#include "mbapp.h"
#include "flash.h"
#include "config.h"
//#include "crc.h"
//#include "mb.h"
//#include "mb_m.h"
//#include "mbrtu.h"
//#include "mbapp.h"

#ifdef ET_TEST_MODE
#define MODE_TEST  1
#endif

WifiRxMsg_t wifi_rxmsg;
uint8_t wifi_rxd_time;
uint8_t wifi_rxd_no;
uint16_t wifi_txd_time;
uint8_t mb_rxd_time;
uint8_t reset_flag = 0;
uint8_t d_sys_reset;
uint16_t check_push_delay;
uint16_t check_group_online;

#ifdef MODE_TEST
uint32_t group_online[2] = {0x0f, 0x1100};
#else
uint32_t group_online[2] = {0, 0};
#endif

#define UPDATE_FLAG_ADDRESS   0x0000f400

const char *typeId_str="01f2";

void et_package_stalist(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth);

#define LSD4WF_PIN_RELOAD           GPIO_Pin_6
#define LSD4WF_PORT_RELOAD          GPIOB
#define LSD4WF_PIN_RESET            GPIO_Pin_5
#define LSD4WF_PORT_RESET           GPIOB
#define LSD4WF_PIN_READY            GPIO_Pin_4
#define LSD4WF_PORT_READY           GPIOB
#define LSD4WF_PIN_LINK             GPIO_Pin_3
#define LSD4WF_PORT_LINK            GPIOB

#define AIR_MOD_WIND                    0
#define AIR_MOD_HOT                     1
#define AIR_MOD_COLD                    2
#define AIR_MOD_AUTO                    3
#define AIR_MOD_AER                     4 // take a breath
#define AIR_MOD_WET                     7

#define MAX_RX_BUFFER_LENGTH        UART2_RX_BUFFER_SIZE

//*****************************************************************************
//
// boot params address
//
//*****************************************************************************
#define BOOT_PARAMETER_ADDRESS       0x0803E000
#define STM32_FLASH_BASE_UPFLAG      0x0803F000
#define STM32_FLASH_BASE_PROID       0x0803F800      //STM32 FLASH的起始地址
#define APP_START_ADDRESS            0x08004000

//*****************************************************************************
//
// comm
//
//*****************************************************************************
DaKin_t g_DaKin[64];
uint32_t udpreq_counter;

static unsigned char GroupNumConvert(unsigned short num);

/* function
 * send the msg to server
 * 0: success, nonzero: fail
 */
static int sendMsg2Server(uint8_t* msg, int len, int eth)
{
    if (eth)
    {
        if (devset.fd >= 0)
            sendto(devset.fd, msg, len, devset.flags, devset.to, devset.tolen);
    }
    else
    {
        wifi_send_msg(msg, len);
    }
    return 0;
}

uint32_t htole32(uint32_t host_32bits)
{
    return host_32bits;
}

uint64_t htole64(uint64_t host_64bits)
{
    return host_64bits;
}

int EtSendHbtData(int fd, int flags, const struct sockaddr *to, socklen_t tolen,
                  uint64_t seq, uint64_t deviceId, int eth)
{
	int sendSuccess = -1;
    unsigned int crc_len = sizeof(pack_header_t);
    unsigned int total_len = crc_len + ET_CRC_LEN;
    unsigned char send_data[256];
    pack_header_t *phdr = NULL;
    unsigned char *pdata = NULL;
    unsigned int crc = 0xffffffff;

	phdr = (pack_header_t *)send_data;
	pdata = (unsigned char *)(send_data + sizeof(pack_header_t));
    /* should  header len + data len + crc len - store len, crc len == store len  */
    phdr->len = htole32(total_len);
    phdr->version = 0;
    phdr->seq = htole64(seq);
    phdr->devId = htole64(deviceId);
    phdr->command = ET_CMD_PING;

	crc = et_crc32(crc, send_data, crc_len);
	crc = htole32(crc);
	memcpy(pdata, (unsigned char *)&crc, ET_CRC_LEN);
    if (eth)
        sendto(fd, send_data, total_len, flags, to, tolen);
    else
        sendMsg2Server(send_data, total_len, eth);
    
	return sendSuccess;
}

void et_configinit(void)
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
}

static int et_package_data(DevSet_t *devset, uint64_t seq, uint64_t deviceId, uint8_t cmd,
               char *data, uint32_t msg_len, int eth)
{
    unsigned int crc_len = msg_len;
    unsigned int total_len = crc_len + ET_CRC_LEN;
    pack_header_t *phdr = NULL;
    char *pdata = NULL;
    unsigned int crc = 0xffffffff;
	int rc = -1;
    
    if (data == NULL)
    {
      return rc;
    }
    if ((deviceId == 0) || (deviceId == (~0x00)))
    {
      return rc;
    }
	phdr = (pack_header_t *)data;
	/* should  header len + data len + crc len - store len, crc len == store len  */
    phdr->len = htole32(total_len);
    phdr->version = 0;
    phdr->seq = htole64(seq);
    phdr->devId = htole64(deviceId);
    phdr->command = cmd;
    
	crc = et_crc32(crc, (uint8_t *)data, crc_len);
	crc = htole32(crc);
    pdata = data + msg_len;
	memcpy(pdata, (unsigned char *)&crc, ET_CRC_LEN);

	/* send the message to server */
	rc = sendMsg2Server((uint8_t *)data, total_len, eth);
    //rc = sendto(devset->fd, data, total_len, devset->flags, devset->to, devset->tolen);
	
	return rc;
}

/*
uint64_t deviceIdStrToUint64(char* deviceId)
{
    uint64_t result = 0;
    for (int i = 0; i <= 5; i++)
    {
        char a[3];
        a[0] = deviceId[i*2];
        a[1] = deviceId[i*2 + 1];
        a[2] = '\0';
        result = result | (strtoll(a, NULL, 16) << (8*(5-i)));
    }

    return result;
}
*/
static uint8_t str2hex(uint8_t str)
{
    uint8_t ret;
    
    if (str >='0' && str <= '9')
    {
        ret = str - '0';
    }
    else if (str >='a' && str <= 'f')
    {
        ret = str - 'a' + 10;
    }
    else if (str >='A' && str <= 'F')
    {
        ret = str - 'A' + 10;
    }
    else
    {
        ret = 0;
    }
    
    return ret;
}

uint64_t deviceIdStrToUint64(char* deviceId)
{
    uint64_t result = 0;
    uint64_t temp;
    int i;

    for (i=0; i<12; i++)
    {
        temp = str2hex(deviceId[11-i]);
        result |= temp << (i * 4);
    }

    return result;
}

static void et_device_para(char *pbuf, int buflen, int *sendlen, DaKin_t dakin, int pushflag)
{
    int len;
    int send_len;
    char status_str[16];
    char *ptmp;
    uint8_t groupid;
    
    ptmp = pbuf;
    send_len = 0;
    if ((buflen-send_len) > 0)
    {
        if (dakin.pwr_enable) {
            strcpy(status_str, "on");
        } else {
            strcpy(status_str, "off");
        }
        if (pushflag)
        {
            //len = snprintf(ptmp, buflen-send_len, "\"switch1Status\":\"%s\", ",  status_str);
            len = snprintf(ptmp, buflen-send_len, "\"status\":\"%s\", ",  status_str);
        }
        else
        {
            len = snprintf(ptmp, buflen-send_len, "\"status\":\"%s\", ",  status_str);
        }
        ptmp += len;
        send_len += len;
    }
    if ((buflen-send_len) > 0)
    {
        uint16_t fanGroupId;
        
        groupid = dakin.group_num;
        fanGroupId = (groupid / 16) * 100 + 100 + (groupid % 16);
        len = snprintf(ptmp, buflen-send_len, "\"fanGroupId\":\"%d\", ",  fanGroupId);
        ptmp += len;
        send_len += len;
    }
    if ((buflen-send_len) > 0)
    {
        switch(dakin.mode_type)
        {
            case 1://hot
                strcpy(status_str, "heat");
                break;
            case 0://cold
                strcpy(status_str, "cool");
                break;
            case 2:  // send wind
                strcpy(status_str, "fanOnly");
                break;
            case 3:  // auto
                strcpy(status_str, "auto");
                break;
            case 4:  // wet
                strcpy(status_str, "dry");
                break;
            default:
                strcpy(status_str, "cool");
                break;
        }
        len = snprintf(ptmp, buflen-send_len, "\"targetMode\":\"%s\", ",  status_str);
        ptmp += len;
        send_len += len;
    }
    if ((buflen-send_len) > 0)
    {
        switch(dakin.wind_speed)
        {
            case 3:
                strcpy(status_str, "high");
                break;
            case 2:
                strcpy(status_str, "mid");
                break;
            case 1:
                strcpy(status_str, "low");
                break;
            default:
                strcpy(status_str, "unknow");
                break;
        }
        len = snprintf(ptmp, buflen-send_len, "\"fanSpeed\":\"%s\", ",  status_str);
        ptmp += len;
        send_len += len;
    }
    if ((buflen-send_len) > 0)
    {
        len = snprintf(ptmp, buflen-send_len, "\"targetTemp\":\"%d\", ",  dakin.set_temp / 2);
        ptmp += len;
        send_len += len;
    }
    if ((buflen-send_len) > 0)
    {
        len = snprintf(ptmp, buflen-send_len, "\"currentTemp\":\"%d.%d\", ",  dakin.temp/10, dakin.temp%10);
        ptmp += len;
        send_len += len;
    }
   

    *sendlen = send_len;
}

/*********************************************************************************************************
** function    : Ethome_QueryDev
** description : 
** input       : pBuf -- buffer point
                 wLen -- buffer length
                 rLen -- read length
** output      :
** return      : 0  -- success
                 -1 -- failed
** author      : pengjq
** datga       : 2015-06-26
**-------------------------------------------------------------------------------------------------------
** Modified :
** data     :
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static int Ethome_QueryDev(uint8_t group, uint8_t devaddr, uint8_t *pBuf, uint8_t wLen, uint8_t *rLen, uint8_t *err)
{
    uint8_t ucReadLen;
    uint8_t query_ok = 1;
    uint8_t *ptr;
    eMBErrorCode eStatus = MB_ENOERR;
    uint8_t buffer[32];
    uint8_t buflen = 32;
    
    ptr = pBuf;
    *rLen = 0;
   
    if (query_ok)
    {
#ifdef MODE_TEST
        memset(buffer, 0, 12);  ucReadLen = 12;
#else

#endif
    }
        return -1; 
}

static char *et_device_parget(char *pbuf, int buflen, int *sendlen, DevSet_t *set, DevState_t state)
{
    char *ptmp;
    uint8_t buffer[16];
    uint8_t rlen;
    uint8_t err;
    uint8_t groupid;
    DaKin_t dakin;
   
    devset = *set;
    ptmp = pbuf;
    if (groupid >= 64)
    {
        return NULL;
    }
    dakin.group_num = groupid;
#ifdef MODE_TEST
    buffer[0] = 1;
    buffer[1] = 16;
    buffer[2] = 1;
    buffer[3] = 1;
    buffer[4] = 0;
    buffer[5] = 160;
    buffer[6] = 0;
    if (1)
#else
    if (0 == Ethome_QueryDev(groupid/16, groupid%16, buffer, 16, &rlen, &err))
#endif
    
    {
        rt_kprintf("device query ok\n");
        dakin.pwr_enable = buffer[0];
        dakin.set_temp = buffer[1];
        dakin.wind_speed = buffer[2];
        dakin.mode_type = buffer[3];
        dakin.temp  = buffer[4] << 8;
        dakin.temp |= buffer[5];
        dakin.err_code = buffer[6];
//        if (state.setflag & (1 << 0))
//        {
//            dakin.pwr_enable = state.pwr_enable;
//        }
//        if (state.setflag & (1 << 1))
//        {
//            dakin.set_temp = state.set_temp;
//        }
//        if (state.setflag & (1 << 2))
//        {
//            dakin.wind_speed = state.wind_speed;
//        }
//        if (state.setflag & (1 << 3))
//        {
//            dakin.mode_type = state.mode_type;
//        }
        g_DaKin[groupid] = dakin;
    }
    else
    {
        rt_kprintf("device query failed\n");
        return NULL;
    }
    *sendlen = 0;
    et_device_para(pbuf, buflen, sendlen, dakin, 0);
   ptmp = pbuf + (*sendlen);
   return ptmp;
}

/* function 
 * station list api  /sta/list
 * exameple for circle 3 way switchs  
 */ 
void et_package_stalist(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
 
}

/*********************************************************************************************************
** function    : GroupNumConvert
** input       : num  -- message length
** output      :
** return      :
** author      : pengjq
** datga       : 2017-06-22
**-------------------------------------------------------------------------------------------------------
** Modified :
** data     :
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
static unsigned char GroupNumConvert(unsigned short num)
{
    unsigned char ret;
    
    if (num >= 100 && num < 200)
    {
        ret = (num - 100) & 0x0f;
    }
    else if (num >= 200 && num < 300)
    {
        ret = ((num - 200) & 0x0f) + 0x10;
    }
    else if (num >= 300 && num < 400)
    {
        ret = ((num - 300) & 0x0f) + 0x20;
    }
    else if (num >= 400 && num < 500)
    {
        ret = ((num - 400) & 0x0f) + 0x30;
    }
    else
    {
        ret = 0;
    }
    
    return ret;
}

/*********************************************************************************************************
** function    : et_package_staset
                 num  -- message length
** output      :
** return      :
** author      : pengjq
** datga       : 2017-06-22
**-------------------------------------------------------------------------------------------------------
** Modified :
** data     :
**-------------------------------------------------------------------------------------------------------
********************************************************************************************************/
void et_package_staset(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
    char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
    int count;
    int iParam;
    uint64_t deviceid;
    int send_len;
    int len;
    char *strDeviceId;
    long fanGroupId;
    int errcode;
    int group_num;
    uint8_t buffer[64];
    uint8_t buflen = 64;
    uint16_t usLen;
    uint8_t *pRecvBuf;
    int isetCounter;
    rt_base_t level;
            
    //eMBErrorCode eStatus = MB_ENOERR;
    isetCounter = 0;
    count = extract_uri_parameters(params, param_vals, uri);
    iParam = ConfigFindCGIParameter("deviceId", params, count);
    if(iParam != -1)
    {
        strDeviceId = param_vals[iParam];
        deviceid = deviceIdStrToUint64(strDeviceId);
    }
    
    /* checking device id */
    if (deviceid != self_dev_id) 
    {
        errcode = 2204;
        goto errp;
    }
    else
    {
        g_list_seq = phdr->seq;
        
        fanGroupId = -1;
        iParam = ConfigFindCGIParameter("fanGroupId", params, count);
        if(iParam != -1)
        {
            char *strTemp;
            
            strTemp = param_vals[iParam];
            fanGroupId = strtol(strTemp, NULL, 0);
        }
        if (fanGroupId < 0 && fanGroupId > 999)
        {
            errcode = 2205;
            goto errp;
        }
        
        group_num = GroupNumConvert(fanGroupId);
        if (group_num < 32)
        {
            if (group_online[0] & (1 << group_num))
            {
                
            }
            else
            {
                errcode = 2206;
                goto errp;
            }
        }
        else if (group_num < 64)
        {
            if (group_online[1] & (1 << (group_num -32)))
            {
                
            }
            else
            {
                errcode = 2206;
                goto errp;
            }
        }
 
    }
    
    return ;
errp:
    {
        char pbuf[128];
        int buflen = 128;
        pack_header_t *pSndHdr;
        char * ptmp;
        uint32_t crc = 0xffffffff;
        
        pSndHdr = (pack_header_t *)pbuf;
        ptmp = (char *)pbuf + sizeof(pack_header_t);
        send_len = sizeof(pack_header_t);
        if (errcode == 2204)
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2204," \
                " \"errorMsg\":\"Device Id is mismatch!\"}");
        }
        else if (errcode == 2205)
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2205," \
                " \"errorMsg\":\"fanGroupId is mismatch!\"}");
        }
        else if (errcode == 2206)
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2206," \
                " \"errorMsg\":\"device timeout!\"}");
        }
        else if (errcode == 2207)
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2207," \
                " \"errorMsg\":\"set timeout!\"}");
        }
        ptmp += len;
        send_len += len;
            
        pSndHdr->seq = phdr->seq;
        pSndHdr->devId = deviceid;
        pSndHdr->command = ET_CMD_HOST_RESP;
        pSndHdr->len = send_len + ET_CRC_LEN;
        pSndHdr->version = 0;
        crc = et_crc32(crc, (uint8_t *)pbuf, send_len);
        crc = htole32(crc);
        memcpy(ptmp, (unsigned char *)&crc, ET_CRC_LEN);
        
        sendMsg2Server((uint8_t *)pbuf, pSndHdr->len, eth);
    }
}

/* function 
 * station sta/version
 * example for device
 */
void et_package_staversion(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
    char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
    int count;
    int iParam;
    uint64_t deviceid;
    int send_len;
    int len;
    char *strDeviceId;
//    tMusicMsg MusicMsg;
    char *ptmp;
    char pbuf[128];
    int buflen = 128;
    
//    MusicMsg.Event = MSC_EVENT_DING;
    count = extract_uri_parameters(params, param_vals, uri);
    iParam = ConfigFindCGIParameter("deviceId", params, count);
    if(iParam != -1)
    {
        strDeviceId = param_vals[iParam];
        deviceid = deviceIdStrToUint64(strDeviceId);
    }
    
    ptmp = (char *)pbuf + sizeof(pack_header_t);
    send_len = sizeof(pack_header_t);
	/* checking device id */
	if (deviceid != self_dev_id) 
    {
		len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2204," \
            " \"errorMsg\":\"Device Id is mismatch!\"}");
        ptmp += len;
        send_len += len;
	}
    else
    {
        len = snprintf(ptmp, buflen-send_len, "{\"result\":true, \"version\":\"%s\"}", ET_SOFTWARE_VERSION);
        rt_kprintf("et software version:%s\n", ET_SOFTWARE_VERSION);
        ptmp += len;
        send_len += len;
    }
    if (buflen >= (send_len +ET_CRC_LEN ))
        et_package_data(devset, phdr->seq, deviceid, ET_CMD_HOST_RESP, pbuf, send_len, eth);
}

void et_reset(void)
{
    rt_base_t level;
    
    rt_thread_delay(RT_TICK_PER_SECOND / 2);
    level = rt_hw_interrupt_disable();
    NVIC_SystemReset();
    rt_hw_interrupt_enable(level);
}

void et_package_stareset(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
    char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
    int count;
    int iParam;
    uint64_t deviceid;
    int send_len;
    int len;
    char *strDeviceId;
//    tMusicMsg MusicMsg;
    char *ptmp;
    char pbuf[128];
    int buflen = 128;
    
//    MusicMsg.Event = MSC_EVENT_DING;
    count = extract_uri_parameters(params, param_vals, uri);
    iParam = ConfigFindCGIParameter("deviceId", params, count);
    if(iParam != -1)
    {
        strDeviceId = param_vals[iParam];
        deviceid = deviceIdStrToUint64(strDeviceId);
    }
    
    ptmp = (char *)pbuf + sizeof(pack_header_t);
    send_len = sizeof(pack_header_t);
	/* checking device id */
	if (deviceid != self_dev_id) 
    {
		len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2204," \
            " \"errorMsg\":\"Device Id is mismatch!\"}");
        ptmp += len;
        send_len += len;
	}
    else
    {
        len = snprintf(ptmp, buflen-send_len, "{\"result\":true}");
        ptmp += len;
        send_len += len;
    }
    if (buflen >= (send_len +ET_CRC_LEN ))
        et_package_data(devset, phdr->seq, deviceid, ET_CMD_HOST_RESP, pbuf, send_len, eth);
    
    et_reset();
}

void et_package_staupdate(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
    int IAP_Get(void);
    char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
    int count;
    int iParam;
    uint64_t deviceid;
    int send_len;
    int len;
    char *strDeviceId;
    char *ptmp;
    char pbuf[128];
    int buflen = 128;
    rt_base_t level;
    
//    tMusicMsg MusicMsg;
    
//    MusicMsg.Event = MSC_EVENT_DING;
    count = extract_uri_parameters(params, param_vals, uri);
    iParam = ConfigFindCGIParameter("deviceId", params, count);
    if(iParam != -1)
    {
        strDeviceId = param_vals[iParam];
        deviceid = deviceIdStrToUint64(strDeviceId);
    }
    
    ptmp = (char *)pbuf + sizeof(pack_header_t);
    send_len = sizeof(pack_header_t);
	/* checking device id */
    if (deviceid != self_dev_id) 
    {
		len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2204," \
            " \"errorMsg\":\"Device Id is mismatch!\"}");
        ptmp += len;
        send_len += len;

        if (buflen >= (send_len +ET_CRC_LEN ))
            et_package_data(devset, phdr->seq, deviceid, ET_CMD_HOST_RESP, pbuf, send_len, eth);
	}
    else
    {
        if (IAP_Get())
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":true}");
        }
        else
        {
            len = snprintf(ptmp, buflen-send_len, "{\"result\":false}");
        }
        ptmp += len;
        send_len += len;
    }
    if (buflen >= (send_len +ET_CRC_LEN ))
        et_package_data(devset, phdr->seq, deviceid, ET_CMD_HOST_RESP, pbuf, send_len, eth);
    
    {
        uint32_t flag;
        
        if (IAP_Get())
        {
            flag = 0x11223344;
            level = rt_hw_interrupt_disable();
            FLASH_ProgramWords(STM32_FLASH_BASE_UPFLAG, (uint32_t *)&flag, 1);
            rt_hw_interrupt_enable(level);
            
            /* Reset module and boot into application */
            et_reset();
        }
    }
}

/* function 
 * station sta/onLine
 * example for device
 */
void et_package_staonline(pack_header_t *phdr, char *uri, 
        DevSet_t *devset, DevState_t devstate, int eth)
{
    char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
    char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
    int count;
    int iParam;
    uint64_t deviceid;
    int send_len;
    int len;
    char *strDeviceId;
    char *ptmp;
    char pbuf[128];
    int buflen = 128;
    
    count = extract_uri_parameters(params, param_vals, uri);
    iParam = ConfigFindCGIParameter("deviceId", params, count);
    if(iParam != -1)
    {
        strDeviceId = param_vals[iParam];
        deviceid = deviceIdStrToUint64(strDeviceId);
    }
    
    ptmp = (char *)pbuf + sizeof(pack_header_t);
    send_len = sizeof(pack_header_t);
	/* checking device id */
	if (deviceid != self_dev_id) 
    {
		len = snprintf(ptmp, buflen-send_len, "{\"result\":false, \"errorCode\":2204," \
            " \"errorMsg\":\"Device Id is mismatch!\"}");
        ptmp += len;
        send_len += len;
	}
    else
    {
        len = snprintf(ptmp, buflen-send_len, "{\"result\":true}");
        ptmp += len;
        send_len += len;
    }
    if (buflen >= (send_len +ET_CRC_LEN ))
        et_package_data(devset, phdr->seq, self_dev_id, ET_CMD_HOST_RESP, pbuf, send_len, eth);
}

typedef void (*tHttpHandler)(pack_header_t *phdr, char *uri, DevSet_t *devset, DevState_t devstate, int eth);
typedef struct __HttpRequestHdlMap
{
    char *cmdname;
    tHttpHandler pHttpHandler; 
}HttpRequestHdlMap;

HttpRequestHdlMap hdlMapTable[] = 
{
    {"/sta/list", et_package_stalist  },
    {"/sta/set", et_package_staset    },
    {"/sta/update", et_package_staupdate },
    {"/sta/version", et_package_staversion},
    {"/sta/restart", et_package_stareset},
    {"/sta/onLine", et_package_staonline},
};

int commandIndex(const char *command)
{
    int i;
    int size = sizeof(hdlMapTable) / sizeof(HttpRequestHdlMap);
    for (i = 0; i < size; i++)
    {
        if (strcmp(hdlMapTable[i].cmdname, command) == 0)
            return i;    
    }

    return -1;
}

int et_data_process(int fd, int flags, struct sockaddr *to, socklen_t tolen,
        uint8_t *pbuf, uint16_t len, int eth)
{
    pack_header_t *phdr;
    char *pdata;
    uint32_t crc;
    uint32_t hostcrc;
    rt_base_t level;

etstart:
    crc = 0xffffffff;
    hostcrc = 0;
    if (pbuf == NULL)  return -1;
    if (len > MAX_RX_BUFFER_LENGTH ||
        len < (4 + sizeof(pack_header_t))) 
    {
       return -2;
    }
    
    phdr = (pack_header_t *)pbuf;
    if (phdr->len > len)
    {
        return -3;
    }
    if (phdr->len > MAX_RX_BUFFER_LENGTH ||
        phdr->len < (4 + sizeof(pack_header_t))) 
    {
       return -2;
    }

    crc = et_crc32(crc, pbuf, phdr->len-4);
    crc = htole32(crc);
    hostcrc  = pbuf[phdr->len - 4] << 0;
    hostcrc |= pbuf[phdr->len - 3] << 8;
    hostcrc |= pbuf[phdr->len - 2] << 16;
    hostcrc |= pbuf[phdr->len - 1] << 24;   
    if (crc != hostcrc)
    {
        return -4;
    }
    pbuf[phdr->len-4] = 0;
    pdata = (char *)(pbuf + sizeof(pack_header_t));
    if (phdr->command == ET_CMD_HOST)
    {
        //et_package_stalist(phdr->seq, phdr->devId, devset, devstate);
        char *params;
        char *command;
        char* subcommand;
        
        params = pdata;
        command = params;
        params = strchr(params, '?');
        if(params) {
            *params = '\0';
            params++;
        }
        subcommand = params;
        if (command)
        {
            int cmdIdx;
            
            devset.fd = fd;
            devset.flags = flags;
            devset.to = to;
            devset.tolen = tolen;
            cmdIdx = commandIndex(command);
            if (cmdIdx >= 0)
            {
                tHttpHandler f = hdlMapTable[cmdIdx].pHttpHandler;
                if (subcommand == NULL) {
                    subcommand = "";
                }
                if (f)
                {
                  (f)(phdr, subcommand, &devset, devstate, eth);
                }
            }
            
        }
    }
    if (len > phdr->len)
    {
        len -= phdr->len;
        pbuf += phdr->len;
        goto etstart;
    }
    
    return 0;
}

void et_pushOfflineMessage(uint8_t group_num, int eth)
{
    char pbuf[300];
    int buflen = 300;
    char id_str[16];
    uint32_t devid[2];
    int len;
    int sendlen;
    
    {
        char *ptmp = (char *)pbuf + sizeof(pack_header_t);

        devid[0] = (self_dev_id >> 32);
        devid[1] = self_dev_id;
        snprintf(id_str, 32, "%.4x%.8x", devid[0], devid[1]);
        id_str[13] = '\0';
        
        sendlen = sizeof(pack_header_t);
        len = snprintf(ptmp, buflen-sendlen, "/sta/pushmessage{\"deviceId\":\"%s\", ", id_str);
        ptmp += len;
        sendlen += len;
        
        if ((buflen-sendlen) > 0)
        {
            uint16_t fanGroupId;
            uint8_t groupid;
            
            groupid = group_num;
            fanGroupId = (groupid / 16) * 100 + 100 + (groupid % 16);
            len = snprintf(ptmp, buflen-sendlen, "\"fanGroupId\":\"%d\", ",  fanGroupId);
            ptmp += len;
            sendlen += len;
        }
        if ((buflen - sendlen) > 0)
        {
            len = snprintf(ptmp, buflen-sendlen, "\"status\":\"%s\", ",  "offline");
            ptmp = ptmp + len;
            sendlen += len;
        }
        if ((buflen - sendlen) > 0)
        {
            len = snprintf(ptmp, buflen-sendlen, "}");
            ptmp += len;
            sendlen += len;
        }
        if ((buflen-sendlen) >= ET_CRC_LEN)
        {
            uint32_t crc = 0xffffffff;
            pack_header_t *pSndHdr;
            
            pSndHdr = (pack_header_t *)pbuf;
            pSndHdr->seq = htole64(g_seq);
            pSndHdr->devId = htole64(self_dev_id);
            pSndHdr->command = ET_CMD_PUSH;
            pSndHdr->len = htole32(sendlen + ET_CRC_LEN);
            pSndHdr->version = 0;
            crc = et_crc32(crc, (uint8_t *)pbuf, sendlen);
            crc = htole32(crc);
            memcpy(ptmp, (unsigned char *)&crc, ET_CRC_LEN);
            
            sendMsg2Server((uint8_t *)pbuf, pSndHdr->len, eth);
        }
    }
}

uint8_t EtSendUdqReq(uint64_t seq, uint64_t deviceId, char *data, uint32_t msg_len, int eth)
{
	uint8_t sendSuccess = 1;
    unsigned int crc_len = sizeof(pack_header_t) + msg_len;
    unsigned int total_len = crc_len + ET_CRC_LEN;
    unsigned char send_data[256];
    pack_header_t *phdr = NULL;
    unsigned char *pdata = NULL;
    unsigned int crc = 0xffffffff;

	phdr = (pack_header_t *)send_data;
	pdata = (unsigned char *)(send_data + sizeof(pack_header_t));
    /* should  header len + data len + crc len - store len, crc len == store len  */
    phdr->len = htole32(total_len);
    phdr->version = 0;
    phdr->seq = htole64(seq);
    phdr->devId = htole64(deviceId);
    phdr->command = ET_CMD_UDP_REQ;

	if (data != NULL) {
		memcpy(pdata, data, msg_len);
		pdata = pdata + msg_len;
	}
	crc = et_crc32(crc, send_data, crc_len);
	crc = htole32(crc);
	memcpy(pdata, (unsigned char *)&crc, ET_CRC_LEN);
    sendMsg2Server(send_data, total_len, eth);
    
	return sendSuccess;
}

#define BigLittleSwap32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                             (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                             (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                             (((uint32_t)(A) & 0x000000ff) << 24))

int et_udp_data_process(uint32_t *addr, uint16_t *port,
        uint8_t *pbuf, uint16_t len)
{
    pack_header_t *phdr;
    char *pdata;
    uint32_t crc = 0xffffffff;
    uint32_t hostcrc = 0;
    
    if (pbuf == NULL)  return -1;
    if ( (len < (4 + sizeof(pack_header_t))) ) 
    {
       return -2;
    }
    if (len > MAX_RX_BUFFER_LENGTH) 
    {
       return -2;
    }
    
    phdr = (pack_header_t *)pbuf;
    if (phdr->len > len)
    {
        return -3;
    }
    if (phdr->len > MAX_RX_BUFFER_LENGTH ||
        phdr->len < (4 + sizeof(pack_header_t))) 
    {
       return -2;
    }
    
    crc = et_crc32(crc, pbuf, len-4);
	crc = htole32(crc);
    hostcrc  = pbuf[len - 4] << 0;
    hostcrc |= pbuf[len - 3] << 8;
    hostcrc |= pbuf[len - 2] << 16;
    hostcrc |= pbuf[len - 1] << 24;
    //data_dump(pbuf, len);
    if (crc != hostcrc)
    {
      //printf("crc=%x\n, hostcrc=%x", crc, hostcrc);
      return -1;
    }
    
    pbuf[len-4] = 0;
    phdr = (pack_header_t *)pbuf;
    pdata = (char *)(pbuf + sizeof(pack_header_t));
    if (phdr->command == 0xef)
    {
        char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
        char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
        int count;
        int iParam;
        char *strIp;
        char *strPort;
        
        strIp = NULL;
        strPort = NULL;
        printf("pdata=%s\n", pdata);
        count = extract_json_parameters(params, param_vals, pdata);
        iParam = ConfigFindCGIParameter("ip", params, count);
        if(iParam != -1)
        {
            strIp = param_vals[iParam];
        }
        iParam = ConfigFindCGIParameter("port", params, count);
        if(iParam != -1)
        {
            strPort = param_vals[iParam];
        }
        if (strIp && strPort)
        {
            uint32_t ipaddr;
            //int ret;

            ipaddr = inet_addr(strIp);
            //if (ret == 0)
            {
                *addr = BigLittleSwap32(ipaddr);
                *port = (uint16_t)atoi(strPort);
            }
            return 0;
        }
    }
    else
    {

    }
    return -1;
}

int et_get_server_addr(int fd, int flags, struct in_addr *sin_addr, uint16_t *port,
        uint8_t *pbuf, uint16_t len)
{
    pack_header_t *phdr;
    char *pdata;
    uint32_t crc = 0xffffffff;
    uint32_t hostcrc = 0;
    
    if (pbuf == NULL)  return -1;
    if ( (len < (4 + sizeof(pack_header_t))) ) 
    {
       return -1;
    }
    
    crc = et_crc32(crc, pbuf, len-4);
	crc = htole32(crc);
    hostcrc  = pbuf[len - 4] << 0;
    hostcrc |= pbuf[len - 3] << 8;
    hostcrc |= pbuf[len - 2] << 16;
    hostcrc |= pbuf[len - 1] << 24;
    //data_dump(pbuf, len);
    if (crc != hostcrc)
    {
      rt_kprintf("crc=%x\n, hostcrc=%x", crc, hostcrc);
      return -1;
    }
    
    pbuf[len-4] = 0;
    phdr = (pack_header_t *)pbuf;
    pdata = (char *)(pbuf + sizeof(pack_header_t));
    if (phdr->command == 0xef)
    {
        char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
        char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
        int count;
        int iParam;
        char *strIp;
        char *strPort;
        
        strIp = NULL;
        strPort = NULL;
        rt_kprintf("pdata=%s\n", pdata);
        count = extract_json_parameters(params, param_vals, pdata);
        iParam = ConfigFindCGIParameter("ip", params, count);
        if(iParam != -1)
        {
            strIp = param_vals[iParam];
        }
        iParam = ConfigFindCGIParameter("port", params, count);
        if(iParam != -1)
        {
            strPort = param_vals[iParam];
        }
        if (strIp && strPort)
        {
            sin_addr->s_addr = inet_addr(strIp);
            //sin_addr->s_addr = htonl(sin_addr->s_addr);
            *port = (uint16_t)atoi(strPort);
            *port = htons(*port);
            rt_kprintf("strIp=%s, strPort=%s\n", strIp, strPort);
            return 0;
        }
    }
    else
    {
        rt_kprintf("unknow command=%x\n", phdr->command);
    }
    return -1;
}

int wifi_get_server_info(struct in_addr *sin_addr, uint16_t *port,
        uint8_t *pbuf, uint16_t len)
{
    pack_header_t *phdr;
    char *pdata;
    uint32_t crc = 0xffffffff;
    uint32_t hostcrc = 0;
    
    if (pbuf == NULL)  return -1;
    if ( (len < (4 + sizeof(pack_header_t))) ) 
    {
       return -1;
    }
    
    crc = et_crc32(crc, pbuf, len-4);
	crc = htole32(crc);
    hostcrc  = pbuf[len - 4] << 0;
    hostcrc |= pbuf[len - 3] << 8;
    hostcrc |= pbuf[len - 2] << 16;
    hostcrc |= pbuf[len - 1] << 24;
    //data_dump(pbuf, len);
    if (crc != hostcrc)
    {
      rt_kprintf("crc=%x\n, hostcrc=%x", crc, hostcrc);
      return -1;
    }

    
    pbuf[len-4] = 0;
    phdr = (pack_header_t *)pbuf;
    pdata = (char *)(pbuf + sizeof(pack_header_t));
    if (phdr->command == 0xef)
    {
        char *params[MAX_CGI_PARAMETERS];     /* Params extracted from the request URI */
        char *param_vals[MAX_CGI_PARAMETERS]; /* Values for each extracted param */
        int count;
        int iParam;
        char *strIp;
        char *strPort;
        
        strIp = NULL;
        strPort = NULL;
        rt_kprintf("pdata=%s\n", pdata);
        count = extract_json_parameters(params, param_vals, pdata);
        iParam = ConfigFindCGIParameter("ip", params, count);
        if(iParam != -1)
        {
            strIp = param_vals[iParam];
        }
        iParam = ConfigFindCGIParameter("port", params, count);
        if(iParam != -1)
        {
            strPort = param_vals[iParam];
        }
        if (strIp && strPort)
        {
            sin_addr->s_addr = inet_addr(strIp);
            sin_addr->s_addr = htonl(sin_addr->s_addr);
            *port = (uint16_t)atoi(strPort);
//            *port = htons(*port);
            rt_kprintf("strIp=%s, strPort=%s\n", strIp, strPort);
            return 0;
        }
    }
    else
    {
        rt_kprintf("unknow command=%x\n", phdr->command);
    }
    return -1;
}

int EtSendUdpReq(int fd, int flags, const struct sockaddr *to, socklen_t tolen,
                  uint64_t seq, uint64_t deviceId, int eth)
{
	int sendSuccess = -1;
    unsigned int crc_len = sizeof(pack_header_t);
    unsigned int total_len = crc_len + ET_CRC_LEN;
    unsigned char send_data[256];
    pack_header_t *phdr = NULL;
    unsigned char *pdata = NULL;
    unsigned int crc = 0xffffffff;

	phdr = (pack_header_t *)send_data;
	pdata = (unsigned char *)(send_data + sizeof(pack_header_t));
    /* should  header len + data len + crc len - store len, crc len == store len  */
    phdr->len = htole32(total_len);
    phdr->version = 0;
    phdr->seq = htole64(seq);
    phdr->devId = htole64(deviceId);
    phdr->command = ET_CMD_UDP_REQ;

	crc = et_crc32(crc, send_data, crc_len);
	crc = htole32(crc);
	memcpy(pdata, (unsigned char *)&crc, ET_CRC_LEN);
    if (eth)
        sendto(fd, send_data, total_len, flags, to, tolen);
    else
        sendMsg2Server(send_data, total_len, eth);
    
	return sendSuccess;
}

void wifi_snd_udpreq(int eth)
{
    rt_base_t level;
    
    if (et_udp_time == 0)
    {
        EtSendUdqReq(g_seq, self_dev_id, NULL, 0, eth);
        rt_kprintf("wifi send udp request\r\n");
        g_seq++;
        level = rt_hw_interrupt_disable();
        et_udp_time = (uint16_t)ET_HBT_TIMEOUT_2S;
        rt_hw_interrupt_enable(level);
    }
}

void wifi_hbt_process(int eth)
{
    rt_base_t level;
    
    if (et_hbt_time == 0)
    {
        EtSendHbtData(0, 0, NULL, 0, g_seq, self_dev_id, eth);
        rt_kprintf("wifi send hbt\r\n");
        g_seq++;
        level = rt_hw_interrupt_disable();
        et_hbt_time = ET_HBT_TIMEOUT_2S;
        rt_hw_interrupt_enable(level);
    }
}
