#include "stm32f10x.h"                  // Device header

#include "pam8403.h"

#define PAM_PORT  GPIOB
#define PAM_PIN		GPIO_Pin_0

void PAM8403_Config(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = PAM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(PAM_PORT, &GPIO_InitStructure);
	
	PAM8403_Mute();
}

void PAM8403_Mute(void){
	GPIO_ResetBits(PAM_PORT, PAM_PIN);
}

void PAM8403_Unmute(void){
	GPIO_SetBits(PAM_PORT, PAM_PIN);
}