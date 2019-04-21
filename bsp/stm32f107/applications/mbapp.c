/*
 * FreeModbus Libary: LPC214X Port
 * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: demo.c,v 1.1 2007/04/24 23:15:18 wolti Exp $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include <stdint.h>
#include <string.h>
#include <board.h>
#include <rtthread.h>
#include <rthw.h>
#include "integer.h"
#include "mbapp.h"
#include "queue.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4
/* ----------------------- Defines ------------------------------------------*/
#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     MODBUS_BUFFER_SIZE /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */

#define SENSOR_RCC                    RCC_APB2Periph_GPIOD
#define SENSOR_GPIO                   GPIOD
#define SENSOR_PIN                    (GPIO_Pin_10)

#define RS485_TXEN_RCC                RCC_APB2Periph_GPIOA
#define RS485_TXEN_GPIO               GPIOA
#define RS485_TXEN_PIN                (GPIO_Pin_8)

/* ----------------------- Static variables ---------------------------------*/

QUEUE modbus_queue;
UCHAR modbus_buffer[MODBUS_QUEUE_SIZE];
rt_device_t uart_mobus;
uint8_t modubs_uart_recvtout;
ModbusMsg mb_msg;

unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
};

unsigned char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};

uint16_t CRC_Compute(uint8_t * pushMsg, uint8_t usDataLen)
{
  uint8_t uchCRCHi = 0xff;
  uint8_t uchCRCLo = 0xff;
  uint8_t uIndex;
  
  while(usDataLen--)
  {
    uIndex = uchCRCLo^ *pushMsg++;
    uchCRCLo = uchCRCHi^ auchCRCHi[uIndex];
    uchCRCHi = auchCRCLo[uIndex];
  }
  
  return (((uint16_t)uchCRCHi) << 8 | uchCRCLo);
}

/* ----------------------- Start implementation -----------------------------*/
int xMBPortEventFree(void)
{
    rt_base_t level;
    
    level = rt_hw_interrupt_disable();
    queue_flush(&modbus_queue);
    rt_hw_interrupt_enable(level);
//    uint8_t buffer[64];
//    uint8_t buflen = 64;
//    uint8_t readlen;
//    
//    do
//    {
//        readlen = rt_device_read(uart_mobus, 0, buffer, buflen);
//    }while(readlen == buflen);
    
    return 0;
}

/**
 * wifi message queue send msg
 */
void modubs_queue_send(ModbusMsg *msg)
{
    queue_send(&modbus_queue, (void *)msg);
}

UINT modubs_queue_empty(void)
{
    return queue_empty(&modbus_queue);
}

UINT modubs_queue_receive(ModbusMsg *msg)
{
    return queue_receive(&modbus_queue, (void *)msg);
}

void eMBWrite(uint8_t *buffer, uint8_t size)
{
    GPIO_SetBits(RS485_TXEN_GPIO, RS485_TXEN_PIN);
    rt_device_write(uart_mobus, 0, buffer, size);
    GPIO_ResetBits(RS485_TXEN_GPIO, RS485_TXEN_PIN);
}
    
uint8_t checksum(uint8_t *pbuf, uint8_t len)
{
    uint8_t sum;
    uint8_t i;
    
    sum = 0;
    for (i=0; i<len ;i++)
    {
        sum += pbuf[i];
    }
    
    return sum;
}

eMBErrorCode
eMBChkRecv(uint16_t usFnCode, uint8_t **pucFrame, uint16_t *pusLength, uint16_t usTimeout)
{
    //static uint8_t buffer[MODBUS_BUFFER_SIZE];
    uint8_t *buffer;
    uint8_t buflen = MODBUS_BUFFER_SIZE;
    static ModbusMsg msg;
    uint16_t usRcvBufferPos;
    uint16_t crc;
    eMBErrorCode    eStatus = MB_ENOERR;
    UINT ret;
    uint16_t t;
    rt_base_t level;
    
    buffer = msg.buffer;
    usRcvBufferPos = 0;
    while(usTimeout && (usRcvBufferPos == 0))
    {
        //DelayMs(1);
        t = (1000 / RT_TICK_PER_SECOND);
        if (usTimeout >= t)  usTimeout -= t;
        else    usTimeout = 0;
        rt_thread_delay(t);
        {
            level = rt_hw_interrupt_disable();
            ret = modubs_queue_empty();
            if( ret != QUEUE_EMPTY )
            {
                ret = modubs_queue_receive(&msg);
                if (ret == QUEUE_SUCCESS)
                {
                    usRcvBufferPos = msg.len;
                }
            }
            rt_hw_interrupt_enable(level);
        }
    }
    /* Length and CRC check */
    if(usRcvBufferPos >= MB_SER_PDU_SIZE_MIN && usRcvBufferPos <= buflen)
    {
//        if (buffer[0] == 0xfe)
        {
            uint16_t temp;
          
            crc = CRC_Compute((UCHAR * )buffer, usRcvBufferPos - 2);
            temp = buffer[usRcvBufferPos - 2] + (buffer[usRcvBufferPos - 1] * 8);
            if (crc == temp)
            {
                /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
                 * size of address field and CRC checksum.
                 */
                *pusLength = ( USHORT )usRcvBufferPos;

                /* Return the start of the Modbus PDU to the caller. */
                *pucFrame = ( UCHAR * ) &buffer[0];
                eStatus = MB_ENOERR;
            }
            else
            {
                eStatus = MB_ECHK;
            }
        }
//        else
//        {
//            eStatus = MB_EHDR;
//        }
    }
    else
    {
        eStatus = MB_ELEN;
    }
    return eStatus;
}

rt_err_t modus_uart_input(rt_device_t dev, rt_size_t size)
{    
    rt_uint32_t rx_length;
    
    
    if (mb_msg.len < MODBUS_BUFFER_SIZE)
    {
        rx_length = rt_device_read(dev, 0, &mb_msg.buffer[mb_msg.len],
                    MODBUS_BUFFER_SIZE - mb_msg.len);
        mb_msg.len += rx_length;
    }
    modubs_uart_recvtout = 10;
    
    return RT_EOK;
}

/* ----------------------- Start implementation -----------------------------*/
void Modbus_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* init sensor */
    RCC_APB2PeriphClockCmd(SENSOR_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = SENSOR_PIN;
    GPIO_Init(SENSOR_GPIO, &GPIO_InitStructure);
    /* enable sensor */
    GPIO_SetBits(SENSOR_GPIO, SENSOR_PIN);
  
    RCC_APB2PeriphClockCmd(RS485_TXEN_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = RS485_TXEN_PIN;
    GPIO_Init(RS485_TXEN_GPIO, &GPIO_InitStructure);
  
    uart_mobus = rt_device_find("uart1");  
  
    if(uart_mobus!=RT_NULL)  
    {
        rt_device_set_rx_indicate(uart_mobus, modus_uart_input);
        rt_device_open(uart_mobus, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);
    }
    queue_create(&modbus_queue, sizeof(ModbusMsg), (void *)modbus_buffer, MODBUS_QUEUE_SIZE);
}

eMBErrorCode 
eMBReadSensorAddr(uint8_t *sensor_addr)
{
    uint8_t index;
    uint8_t buffer[32];
    uint8_t *pRecvBuf;
    uint16_t usLen;
    uint16_t usTimeOut;
    eMBErrorCode eStatus;
    uint16_t crc;
    
    usTimeOut = 300;
    index = 0;
    buffer[index++] = 0xfe;
    buffer[index++] = 0x17;
    buffer[index++] = 0x00;
    buffer[index++] = 0x00;
    buffer[index++] = 0x00;
    buffer[index++] = 0x01;
    crc = CRC_Compute(buffer, 6);
    buffer[index++] = crc >> 0;
    buffer[index++] = crc >> 8;
    
    xMBPortEventFree();
    eMBWrite(buffer, 8);  
    eStatus = eMBChkRecv(0, &pRecvBuf,  &usLen, usTimeOut);
    if (eStatus == MB_ENOERR)
    {
        uint8_t version;
        uint8_t addr;
        
        if (pRecvBuf[0] == 0xfe && 
            pRecvBuf[1] == 0x17 && 
            pRecvBuf[2] == 0x02)
        {
            version = pRecvBuf[3];
            addr = pRecvBuf[4];
            *sensor_addr = addr;
        }
        rt_kprintf("sensor version:%d addr=%d\r\n", version, addr);
    }
    
    return eStatus;
}

eMBErrorCode
eMBReadData(uint8_t addr, uint8_t *pucBuf, uint8_t ucBufLen, uint8_t *ucReadLen)
{
    uint8_t index;
    uint8_t buffer[32];
    uint8_t *pRecvBuf;
    uint16_t usLen;
    uint16_t usTimeOut;
    eMBErrorCode eStatus;
    uint16_t crc;
    
    usTimeOut = 300;
    index = 0;
    buffer[index++] = addr;
    buffer[index++] = 0x03;
    buffer[index++] = 0x00;
    buffer[index++] = 0x00;  // register start addr 
    buffer[index++] = 0x00;
    buffer[index++] = 0x07;  // register nubmer
    crc = CRC_Compute(buffer, 6);
    buffer[index++] = crc >> 0;
    buffer[index++] = crc >> 8;
    
    xMBPortEventFree();
    eMBWrite(buffer, 8);  
    eStatus = eMBChkRecv(0, &pRecvBuf,  &usLen, usTimeOut);
    if (eStatus == MB_ENOERR)
    {       
        if (pRecvBuf[0] == addr && 
            pRecvBuf[1] == 0x03 && 
            pRecvBuf[2] == 14)
        {
            if (ucBufLen >= 14)
            {
                ucBufLen = 14;
            }
            *ucReadLen = ucBufLen;
            memcpy(pucBuf, &pRecvBuf[3], ucBufLen);
        }
        rt_kprintf("sensor datalen=%d\r\n", ucBufLen);
    }
    
    return eStatus;
}
