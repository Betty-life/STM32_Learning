#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"

uint16_t Timer_num;

int main(void)
{
	/*模块初始化*/
	OLED_Init();	
	Timer_Init();
	
	/*OLED显示*/
	OLED_ShowString(1, 1, "HelloWorld!");	//1行3列显示字符串HelloWorld!
	OLED_ShowString(2, 1, "Num:");
	

	while (1)
	{
		OLED_ShowNum(2, 5, Timer_num,5);
	}
}
