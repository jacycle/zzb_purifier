#ifndef __BUTTION_H__
#define __BUTTION_H__

#include <stdint.h>
#include "integer.h"
#include "queue.h"
#include "key.h"

//*********************************************************************
//
// private define
//
//*********************************************************************
#define BUTTON_NOTIFICATION_CLICKED             1   // message key clicked
#define BUTTON_NOTIFICATION_RELEASED            2   // message key released
#define BUTTON_NOTIFICATION_PRESSED             3   // message Continuous pressed

#define BUTTON_DELAY_100MS                      (100)
#define BUTTON_MATCH_CODE_CLICKED_DELAY          10
#define BUTTON_MATCH_CODE_PRESSED_DELAY          10     
#define BUTTON_MATCH_CODE_TOTAL_DELAY           (BUTTON_MATCH_CODE_CLICKED_DELAY + \
                                                (BUTTON_MATCH_CODE_PRESSED_DELAY * 20)) 

//*********************************************************************
//
// Common types
//
//*********************************************************************
typedef uint8_t BTN_STATE;

typedef struct {
  BTN_STATE State;
  uint8_t Notify;
  uint8_t PressedCnt;
  uint32_t BtnStates;
//  uint8_t *PressedCnt;
} BUTTON_PID_STATE;

typedef struct 
{
  Button_TypeDef Button;
  uint8_t State;
}ButtonState_t;

//*********************************************************************
//
// Private variables
//
//*********************************************************************
extern unsigned char button_time[];
extern QUEUE btn_queue;
extern const ButtonState_t BUTTON_STATE[];

//*********************************************************************
//
// Private function prototypes 
//
//*********************************************************************
extern void BUTTON_QueueInit(void);
extern void BUTTON_TimerHandle(void);
extern void BUTTON_Task(void);
uint8_t BUTTON_StoreState(uint8_t index, uint32_t states);

#endif
