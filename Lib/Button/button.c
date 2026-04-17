#include "button.h"

void Button_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpio;
    
    // PA0 (Prev), PA1 (Play / Pause), PA2 (Next), PA3 (Vol-), PA4 (Vol+)
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_IPU; 
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio);
}

uint8_t Button_Scan(void) {

		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) return CMD_PREV;
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0) return	CMD_PP;
	  if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0) return CMD_NEXT;
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0) return	CMD_VOL_DOWN;
	  if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0) return CMD_VOL_UP;	
    
    return CMD_NONE;
}