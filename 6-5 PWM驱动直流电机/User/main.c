#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MOTOR.h"
#include "KEY.h"


int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Motor_Init();
	Key_Init();
	
	OLED_ShowString(1,1,"Speed:");
	OLED_ShowString(3,1,"PA4:");
	OLED_ShowString(4,1,"PA5:");
	
	
	int8_t Speed = 0;
	
	while (1)
	{
		
		if(Key_GetNum() == 2)
		{
			Speed += 20;
			if(Speed > 100)
			{
				Speed = -100;
			}
		}
		Motor_SetSeed(Speed);
		OLED_ShowSignedNum(1,7,Speed,5);
		OLED_ShowNum(3, 7, GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_4), 1);
		OLED_ShowNum(4, 7, GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5), 1);
	}

}
