#include "stm32f10x.h"
#define _WRITE_DATA_

void My_I2C_Init(void);
int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, 
	uint8_t *pData, uint16_t Size);
int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr,
	uint8_t *pBuffer, uint16_t Size);
void My_Led_Init(void);

int main(void)
{
	My_I2C_Init();

#ifdef _WRITE_DATA_
	uint8_t commands[] = {0x00, 0x8d, 0x14, 0xaf, 0xa5};

	//send datas
	My_I2C_SendBytes(I2C1, 0x78, commands, 5);
#endif
	
	My_Led_Init();
	
	//receive datas
	uint8_t rcvd;
	My_I2C_ReceiveBytes(I2C1, 0x78, &rcvd, 1);
	
	if ((rcvd & (0x01 << 6) == 0)) {
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
	} else {
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
	}
	
	while(1)
	{
	}
}

void My_Led_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void My_I2C_Init(void)
{
	//init io
	//ioremap
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
	
	//init PB8 && PB9
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//init I2C
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
	
	I2C_InitTypeDef I2C_InitStruct;
	
	I2C_InitStruct.I2C_ClockSpeed = 400000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
		
	I2C_Init(I2C1, &I2C_InitStruct);
	
	I2C_Cmd(I2C1, ENABLE);
}

int My_I2C_SendBytes(I2C_TypeDef *I2Cx, uint8_t Addr, 
	uint8_t *pData, uint16_t Size)
{
	//等待总线空闲
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) == SET);
	
	//发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	//判断起始位是否发送完成
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET);
	
	///清除AF
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	I2C_SendData(I2Cx, Addr & 0xfe);
	
	//寻址阶段
	while (1) {
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == SET) {
			break;
		}
		
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET) {
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1; //寻址失败
		}
	}

	//清除addr
	I2C_ReadRegister(I2Cx, I2C_Register_SR1);
	I2C_ReadRegister(I2Cx, I2C_Register_SR2);
	
	//发送数据
	for (uint16_t i = 0; i < Size; i++) {
		while (1) {
			if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET) {
				I2C_GenerateSTOP(I2Cx, ENABLE);
			}
			if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE)) {
				break;
			}
			
			I2C_SendData(I2Cx, pData[i]);
		}
	}
	
	while (1) {
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET) {
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -2;
		}
		
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) == SET) {
			break;
		}
	}
    
	//发送停止位
	I2C_GenerateSTOP(I2Cx, ENABLE);
	return 0;
}

int My_I2C_ReceiveBytes(I2C_TypeDef *I2Cx, uint8_t Addr,
	uint8_t *pBuffer, uint16_t Size)
{
	//发送起始位
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_SB) == RESET);
	
	//寻址阶段
	I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
	
	while (1) {
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF) == SET) {
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return -1;
		}
		if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_ADDR) == SET) {
			break;
		}
	}
	
	if (Size == 1) {
		//清除addr标志位
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		
		pBuffer[0] = I2C_ReceiveData(I2Cx);
	} else if (Size == 2) {
		//清除addr标志位
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		//ack = 1
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
		
		//等待接收完成
		while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		
		//读取第一个字节
		pBuffer[0] = I2C_ReceiveData(I2Cx);
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		//等待接收完成
		while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		pBuffer[1] = I2C_ReceiveData(I2Cx);
	} else {
		//清除addr标志位
		I2C_ReadRegister(I2Cx, I2C_Register_SR1);
		I2C_ReadRegister(I2Cx, I2C_Register_SR2);
		
		//ack = 1
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
		
		for (uint16_t i = 0; i < Size - 1; i++) {
			while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
			//读取第一个字节
			pBuffer[i] = I2C_ReceiveData(I2Cx);
		}
		
		//NACK && STOP
		I2C_AcknowledgeConfig(I2Cx, DISABLE);
		I2C_GenerateSTOP(I2Cx, ENABLE);
		
		//等待接收完成
		while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) == RESET);
		pBuffer[Size - 1] = I2C_ReceiveData(I2Cx);
	}
	
	return 0;
}
