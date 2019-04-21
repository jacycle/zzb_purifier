#include <stdio.h>
#include "stm32f10x_flash.h"
#include "flash.h"

void FLASH_ProgramWords(uint32_t Address,  uint32_t *pData, uint16_t Length)
{
    uint16_t i;
    
    if (pData == NULL)
    {
        return;
    }

    /* Unlock the Flash to enable the flash control register access *************/ 
    FLASH_Unlock();

    /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Clear pending flags (if any) */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 

    if (FLASH_ErasePage(Address) != FLASH_COMPLETE)
    {
     /* Error occurred while sector erase. 
         User can add here some code to deal with this error  */
    }
    
    for (i=0; i<Length; i++)
    {
        uint32_t temp;
        
        temp = pData[i];
        FLASH_ProgramWord(Address & (~0x07), temp);
        Address += 4;
    } 
    
    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock(); 
}

void FLASH_ReadWords(uint32_t Address, uint32_t *pData, uint16_t Length)
{
    uint16_t i;
    
    for (i=0; i<Length; i++)
    {
        pData[i] = *(__IO uint32_t *)Address;
        Address += 4;
    }
}

uint8_t FLASH_ReadByte(uint32_t Address)
{
    uint32_t temp;
    uint32_t addr;
    uint8_t offset;
    uint8_t ret;
    
    addr = Address & (~0x03);
    offset = Address % 4;
    offset = offset * 8;
    FLASH_ReadWords(addr, &temp, 1);
    
    ret = temp >> offset;
    return ret;
}

void FLASH_ReadBytes(uint32_t Address, uint8_t *pData, uint16_t Length)
{
    uint16_t i;
    
    if (pData == NULL)  return;
    
    for (i=0; i<Length; i++)
    {
        pData[i] = FLASH_ReadByte(Address + i);
    }
}
