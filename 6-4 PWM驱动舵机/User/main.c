#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Servo.h"
#include "Key.h"

uint8_t KeyNum;			//定义用于接收键码的变量
float Angle;			//定义角度变量

int main(void)
{
	/*模块初始化*/
	OLED_Init();		
	Servo_Init();		
	Key_Init();			
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Angle:");	
	OLED_ShowString(2, 1, "KeyNum:");
	
	while (1)
	{
		KeyNum = Key_GetNum();			
		if (KeyNum == 2)				
		{
			Angle += 30;				
			if (Angle > 180)			
			{
				Angle = 0;				
			}
		}
		Servo_SetAngle(Angle);			
		OLED_ShowNum(1, 7, Angle, 3);	
		OLED_ShowNum(2, 7, KeyNum, 3);
	}
}
