#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MyDMA.h"

//uint8_t aa = 0x66;  //存储在运行内存SRAM  0x2000 0000
//const uint8_t aa = 66;  //存储在程序存储器Flash 0x0800 0E00
uint8_t DataA[] = {0x01, 0x02, 0x03, 0x04};
uint8_t DataB[] = {0, 0, 0, 0};
	
int main(void)
{
	
	OLED_Init();	
	MyDMA_Init((uint32_t)DataA, (uint32_t)DataB, 4);
	
	OLED_ShowString(1, 1, "DataA");
	OLED_ShowString(3, 1, "DataB");
	OLED_ShowHexNum(1, 8, (uint32_t)DataA, 8);
	OLED_ShowHexNum(3, 8, (uint32_t)DataB, 8);
	
//	OLED_ShowHexNum(2, 1, DataA[0], 2);	
//	OLED_ShowHexNum(2, 4, DataA[1], 2);	
//	OLED_ShowHexNum(2, 7, DataA[2], 2);	
//	OLED_ShowHexNum(2, 10, DataA[3], 2);	
//	OLED_ShowHexNum(4, 1, DataB[0], 2);	
//	OLED_ShowHexNum(4, 4, DataB[1], 2);	
//	OLED_ShowHexNum(4, 7, DataB[2], 2);	
//	OLED_ShowHexNum(4, 10, DataB[3], 2);
//	

	
//	OLED_ShowHexNum(3, 1, DataA[0], 2);	
//	OLED_ShowHexNum(3, 4, DataA[1], 2);	
//	OLED_ShowHexNum(3, 7, DataA[2], 2);	
//	OLED_ShowHexNum(3, 10, DataA[3], 2);	
//	OLED_ShowHexNum(4, 1, DataB[0], 2);	
//	OLED_ShowHexNum(4, 4, DataB[1], 2);	
//	OLED_ShowHexNum(4, 7, DataB[2], 2);	
//	OLED_ShowHexNum(4, 10, DataB[3], 2);
	
//	OLED_ShowHexNum(1, 1, aa, 2);
//	OLED_ShowHexNum(2, 1, (uint32_t)&aa, 8);
//	OLED_ShowHexNum(3, 1, (uint32_t)&ADC1->DR, 8);  //外设寄存器地址 4001 244C

	while (1)
	{
		
		OLED_ShowHexNum(2, 1, DataA[0], 2);	
		OLED_ShowHexNum(2, 4, DataA[1], 2);	
		OLED_ShowHexNum(2, 7, DataA[2], 2);	
		OLED_ShowHexNum(2, 10, DataA[3], 2);	
		OLED_ShowHexNum(4, 1, DataB[0], 2);	
		OLED_ShowHexNum(4, 4, DataB[1], 2);	
		OLED_ShowHexNum(4, 7, DataB[2], 2);	
		OLED_ShowHexNum(4, 10, DataB[3], 2);
		
		Delay_ms(1000);
		
		MyDMA_Transfer();
		
		OLED_ShowHexNum(2, 1, DataA[0], 2);	
		OLED_ShowHexNum(2, 4, DataA[1], 2);	
		OLED_ShowHexNum(2, 7, DataA[2], 2);	
		OLED_ShowHexNum(2, 10, DataA[3], 2);	
		OLED_ShowHexNum(4, 1, DataB[0], 2);	
		OLED_ShowHexNum(4, 4, DataB[1], 2);	
		OLED_ShowHexNum(4, 7, DataB[2], 2);	
		OLED_ShowHexNum(4, 10, DataB[3], 2);
		
		Delay_ms(1000);
		
		DataA[0] ++;
		DataA[1] ++;
		DataA[2] ++;
		DataA[3] ++;
		
	}
}
