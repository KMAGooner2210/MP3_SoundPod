#include "stm32f10x.h"                  // Device header
#include "usart_dri.h"

void USART1_Config(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef gpio;
	
	gpio.GPIO_Pin = GPIO_Pin_9;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_10;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);
	
	USART_InitTypeDef uart;
	uart.USART_BaudRate = 9600;
	uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	uart.USART_Parity = USART_Parity_No;
	uart.USART_StopBits = USART_StopBits_1;
	uart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &uart);
	USART_Cmd(USART1, ENABLE);
}

void USART1_SendChar(uint8_t ch){
    // Ch? cho thanh ghi tr?ng (S?n sàng)
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    // Ð?y data vào thanh ghi
    USART_SendData(USART1, ch);
}
