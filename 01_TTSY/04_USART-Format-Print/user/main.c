#include "stm32f10x.h"
#include <stdio.h>
#include "delay.h"

void My_USART_SendBytes(USART_TypeDef *UASRTx, uint8_t *pData, uint16_t size);
void My_USART_Init(void);
void My_USART_AFIO(void);
int fputc(int ch, FILE *f);

int main(void)
{
	uint32_t currentTick = 0;
	uint32_t milsec = 0;
	uint32_t sec = 0;
	uint32_t min = 0;
	uint32_t hour = 0;
	
	Delay_Init();
	My_USART_Init();
	
	printf("hello world\r\n");

	while(1)
	{
		currentTick = GetTick();
		milsec = currentTick % 1000;
		currentTick = currentTick / 1000;
		sec = currentTick % 60;
		currentTick = currentTick / 60;
		min = currentTick % 60;
		currentTick = currentTick / 60;
		hour = currentTick;
		
		printf("%02u:%02u:%02u:.%03u\r\n", hour, min, sec, milsec);
		
		Delay(1000); //delay 1000 ms
	}
} 

void My_USART_SendBytes(USART_TypeDef *USARTx, uint8_t *pData, uint16_t size)
{
	uint32_t i = 0;
	
	for (i = 0; i < size; i++) {
		//wait
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
		//send
		USART_SendData(USARTx, pData[i]);
	}
	
	while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
}

void My_USART_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitTypeDef USART_InitStruct;
	
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

int fputc(int ch, FILE *f)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		
	USART_SendData(USART1, (uint8_t)ch);
	
	return ch;
}
