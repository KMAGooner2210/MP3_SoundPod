#ifndef __BUTTON_H
#define __BUTTON_H
#include "stm32f10x.h"

#define 	CMD_NONE 			0
#define 	CMD_PREV			1
#define 	CMD_PP				2
#define 	CMD_NEXT			3
#define 	CMD_VOL_DOWN	4
#define 	CMD_VOL_UP		5

void Button_Init(void);
uint8_t Button_Scan(void);
#endif