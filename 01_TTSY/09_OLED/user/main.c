#include "stm32f10x.h"
#include "si2c.h"
#include "oled.h"

SI2C_TypeDef si2c;
OLED_TypeDef oled;

void My_SoftwareI2C_Init(void);
void My_OLEDScreen_Init(void);
int i2c_write_bytes(uint8_t addr, const uint8_t *pData,
	uint16_t size); 

int main(void)
{
	My_SoftwareI2C_Init();
	My_OLEDScreen_Init();

	OLED_SetPen(&oled, PEN_COLOR_TRANSPARENT, 1);
	OLED_SetBrush(&oled, PEN_COLOR_TRANSPARENT);
	OLED_SetCursor(&oled, 24, 50);
	OLED_DrawString(&oled, "Hello wrold");
	//以上绘制的内容只是在单片机的内存中
	
	//将绘制的内容发送到屏幕上
	OLED_SendBuffer(&oled);
	
	while (1) {
	}
	
}

void My_SoftwareI2C_Init(void)
{
	si2c.SCL_GPIOx = GPIOB;
	si2c.SCL_GPIO_Pin = GPIO_Pin_6;
	si2c.SDA_GPIOx = GPIOB;
	si2c.SDA_GPIO_Pin = GPIO_Pin_7;
	
	My_SI2C_Init(&si2c);
}

void My_OLEDScreen_Init(void)
{
	OLED_InitTypeDef OLED_InitStruct;
	
	OLED_InitStruct.i2c_write_cb= i2c_write_bytes;
	OLED_Init(&oled, &OLED_InitStruct);
}

int i2c_write_bytes(uint8_t addr, const uint8_t *pData,
	uint16_t size)
{
}

