#include "stm32f10x.h"                  // Device header
#include "Encoder.h"
#include "OLED.h"

int16_t Encoder_num;

int main(void)
{
	/*模块初始化*/
	OLED_Init();	
	Encoder_Init();	
	
	OLED_ShowString(1, 1, "HelloWorld!");	
	OLED_ShowString(2, 1, "Count:");
	
	while(1)
	{
		Encoder_num += EncoderCount_Get();
		OLED_ShowSignedNum(2,8,Encoder_num,5);
	}
	
}
