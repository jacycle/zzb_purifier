#ifndef __MB_APP_H__
#define __MB_APP_H__

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
#include <stdint.h>
//#include "queue.h"

#define MODBUS_BUFFER_SIZE      64
#define MODBUS_QUEUE_SIZE      (sizeof(ModbusMsg) * 2)

typedef struct {
  unsigned char len;
  unsigned char buffer[MODBUS_BUFFER_SIZE];
} ModbusMsg;

/*! \ingroup modbus
 * \brief Errorcodes used by all function in the protocol stack.
 */
typedef enum
{
    MB_ENOERR,                  /*!< no error. */
    MB_ENOREG,                  /*!< illegal register address. */
    MB_EINVAL,                  /*!< illegal argument. */
    MB_EPORTERR,                /*!< porting layer error. */
    MB_ENORES,                  /*!< insufficient resources. */
    MB_EIO,                     /*!< I/O error. */
    MB_EILLSTATE,               /*!< protocol stack in illegal state. */
    MB_ETIMEDOUT,               /*!< timeout error occurred. */
    MB_EBUFOVER,                /* Buffer overflow */
    MB_ECHK,                    /* Check Sum error */
    MB_ELEN,                    /* data length error */
    MB_EHDR,                    /* Header flag error */
} eMBErrorCode;

/*! \ingroup modbus
 * \brief Modbus serial transmission modes (RTU/ASCII).
 *
 * Modbus serial supports two transmission modes. Either ASCII or RTU. RTU
 * is faster but has more hardware requirements and requires a network with
 * a low jitter. ASCII is slower and more reliable on slower links (E.g. modems)
 */
typedef enum
{
    MB_RTU,                     /*!< RTU transmission mode. */
    MB_ASCII,                   /*!< ASCII transmission mode. */
    MB_TCP                      /*!< TCP mode. */
} eMBMode;

//extern QUEUE modbus_queue;
extern uint8_t modubs_uart_recvtout;
extern ModbusMsg mb_msg;

void Modbus_Init( void );
int xMBPortEventFree(void);
void eMBWrite(uint8_t *buffer, uint8_t size);
eMBErrorCode
eMBChkRecv(uint16_t usFnCode, uint8_t **pucFrame, uint16_t *pusLength, uint16_t usTimeout);
eMBErrorCode
eMBReadACSensorState(uint16_t usRegAddr, uint16_t usReadLen, uint8_t *pucFrame, uint16_t ucFrameLen, uint8_t *ucReadLen);

eMBErrorCode
eMBReadSensorAddr(uint8_t *sensor_addr);
eMBErrorCode
eMBReadData(uint8_t addr, uint8_t *pucBuf, uint8_t ucBufLen, uint8_t *ucReadLen);
extern void modubs_queue_send(ModbusMsg *msg);

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
