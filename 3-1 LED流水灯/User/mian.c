#include "stm32f10x.h"                  // Device header
#include "Delay.h"

int main(void)
{
	
	//利用库函数文件
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOA, GPIO_Pin_All); //高电平 灭
//	GPIO_ResetBits(GPIOA, GPIO_Pin_All);//低电平 亮
	while(1)
	{
//		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
//		Delay_ms(500);
//		GPIO_SetBits(GPIOA, GPIO_Pin_0);
//		Delay_ms(500);
//		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
//		Delay_ms(500);
//		GPIO_SetBits(GPIOA, GPIO_Pin_1);
//		Delay_ms(500);
//		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
//		Delay_ms(500);
//		GPIO_SetBits(GPIOA, GPIO_Pin_2);
//		Delay_ms(500);
//		GPIO_ResetBits(GPIOA, GPIO_Pin_3);
//		Delay_ms(500);
//		GPIO_SetBits(GPIOA, GPIO_Pin_3);
//		Delay_ms(500);

		
		GPIO_Write(GPIOA, ~0x01);
		Delay_ms(500);
		GPIO_Write(GPIOA, ~0x02);
		Delay_ms(500);
		GPIO_Write(GPIOA, ~0x04);
		Delay_ms(500);
		GPIO_Write(GPIOA, ~0x08);
		Delay_ms(500);
	}

}
