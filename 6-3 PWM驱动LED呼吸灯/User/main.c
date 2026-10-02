#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "PWM.h"

uint8_t i;

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	PWM_Init();
	
	PWM_SetCompare2(2500);
	
	while (1)
	{
//		PWM_SetCompare2(500);
//		Delay_ms(10);
		
//		Delay_ms(10);
		
	}
}
