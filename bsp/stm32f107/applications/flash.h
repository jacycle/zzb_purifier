#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
#define FLASH_USER_START_ADDR   ((uint32_t)0x0803ff00)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t)0x08040000)   /* End @ of user Flash area */

#define FLASH_ADDR_LEN           6
#define FLASH_ADDR_SRC_OFFSET    0x00
#define FLASH_ADDR_SRC_ADDR      (FLASH_USER_START_ADDR + FLASH_ADDR_SRC_OFFSET)

#define FLASH_ADDR_DST_OFFSET    0x10
#define FLASH_ADDR_DST_ADDR      (FLASH_USER_START_ADDR + FLASH_ADDR_DST_OFFSET)

#define FLASH_PRODUCT_ID_LEN     3
#define FLASH_PRODUCT_ID_OFFSET  0x20
#define FLASH_PRODUCT_ID_ADDR    (FLASH_USER_START_ADDR + FLASH_PRODUCT_ID_OFFSET)

#define FLASH_MEX_LEN            40

void FLASH_Init(void);
void FLASH_ProgramWords(uint32_t Address,  uint32_t *pData, uint16_t Length);
void FLASH_ReadWords(uint32_t Address, uint32_t *pData, uint16_t Length);
uint8_t FLASH_ReadByte(uint32_t Address);
void FLASH_ReadBytes(uint32_t Address, uint8_t *pData, uint16_t Length);
void FLASH_SaveDeviceID(uint8_t *pBuf, uint16_t Length);

#endif
