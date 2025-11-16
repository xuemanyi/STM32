#include "stm32f10x.h"

void My_SI2C_Init(void);
void scl_write(uint8_t level);
void sda_write(uint8_t level);
uint8_t sda_read(void);
void delay_us(uint32_t us);
void SendStart(void);
void SendStop(void);
uint8_t send_byte(uint8_t byte);
uint8_t receive_byte(uint8_t ACK);
int My_SI2C_SendBytes(uint8_t addr, uint8_t *pData, uint16_t size);
int My_SI2C_ReceiveBytes(uint8_t addr, uint8_t *pData, uint16_t size);

int main(void)
{
	My_SI2C_Init();
	
	uint8_t command[] = {0x0, 0x8d, 0x14, 0xaf, 0xa5};
	
	My_SI2C_SendBytes(0x78, command, 5);

	while (1) {
	}
	
}

void My_SI2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//I2C初始化高电压
	GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
	GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
}

void scl_write(uint8_t level)
{
	if (level == 0) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
	}
	
}
void sda_write(uint8_t level)
{
	if (level == 0) {
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
	} else {
		GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
	}
}
uint8_t sda_read(void)
{
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_SET) {
		return 1;
	} else {
		return 0;
	}
}

void delay_us(uint32_t us)
{
	uint32_t n = us * 8;
	uint32_t i = 0;
	
	for (i = 0; i < n; i++) {
	}
}

void SendStart(void)
{
	sda_write(0);
	delay_us(1);
}

void SendStop(void)
{
	scl_write(0);
	sda_write(0);
	delay_us(1);
	
	scl_write(1);
	delay_us(1);
	
	sda_write(1);
	delay_us(1);
}

uint8_t send_byte(uint8_t byte)
{
	int8_t i = 0;
	
	scl_write(0);	
	for (i = 7; i >= 0; i--) {
		if ((byte & (0x01) << i) != 0) {
			sda_write(1);
		} else {
			sda_write(0);
		}
		
		delay_us(1);
		scl_write(1);
		delay_us(1);		
	}
	
	//读取ACK或NAK
	scl_write(0);
	sda_write(1);
	delay_us(1);
	
	scl_write(1);
	delay_us(1);
	
	return sda_read();
}

uint8_t receive_byte(uint8_t ACK)
{
	uint8_t byte = 0;
	int8_t i = 0;
	
	for (i = 7; i >= 0; i--) {
		scl_write(0);
		sda_write(1);
		delay_us(1);
	
		scl_write(1);
		delay_us(1);
	
		if (sda_read() != 0) {
			byte |= 0x01;
		}
	}
	
	//回复ACK或者NAK
	scl_write(0);
	sda_write(!ACK);
	delay_us(1);
	
	scl_write(1);
	delay_us(1);
	
	return byte;
}

int My_SI2C_SendBytes(uint8_t addr, uint8_t *pData, uint16_t size)
{
	SendStart();
	
	if (send_byte(addr & 0xfe) != 0) {
		SendStop();
		return -1;
	}
	
	for (uint32_t i = 0; i < size; i++) {
		if (send_byte(pData[i]) != 0) {
			SendStop();
			return -2;
		}
	}
	
	SendStop();
	
	return 0;
}
int My_SI2C_ReceiveBytes(uint8_t addr, uint8_t *pData, uint16_t size)
{
	SendStart();
	
	if (send_byte(addr | 0x01) != 0) {
		SendStop();
		return -1;
	}
	
	for (uint32_t i = 0; i < size -1; i++) {
		if (receive_byte(pData[i] != 0)) {
			SendStop();
			return -2;
		}
	}
	
	SendStop();
	return 0;
}

