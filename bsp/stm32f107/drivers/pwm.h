#ifndef __PWM_H__
#define __PWM_H__

#include <stdint.h>

void pwm_init(void);
void pwm_set_duty(uint8_t ch, uint16_t duty);

#endif
