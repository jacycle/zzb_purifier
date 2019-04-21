#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void TIM3_Int_Init(uint16_t arr, uint16_t psc);
void TIM6_Int_Init(uint16_t arr, uint16_t psc);

#endif
