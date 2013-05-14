/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 15:37
//*****************************************************************************/

#include "v_key.h"
#include "delay.h"

uint8_t v_key  = V_KEY_NONE;

void v_key_init(void) 
{
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin  = V_KEY_ENTER_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_ENTER_GPIO_MODE;
 	GPIO_Init(V_KEY_ETNER_GPIO, &GPIO_InitStructure);

// 	GPIO_InitStructure.GPIO_Pin  = V_KEY_ESC_GPIO_PIN;
//	GPIO_InitStructure.GPIO_Mode = V_KEY_ESC_GPIO_MODE;
// 	GPIO_Init(V_KEY_ESC_GPIO, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin  = V_KEY_UP_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_UP_GPIO_MODE;
 	GPIO_Init(V_KEY_UP_GPIO, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin  = V_KEY_DOWN_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_DOWN_GPIO_MODE;
 	GPIO_Init(V_KEY_DOWN_GPIO, &GPIO_InitStructure);		 

}
u8 v_key_scan(void)
{	 
	static u8 key_up=1;//按键按松开标志	
 
 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	if(key_up&&(V_KEY_ENTER==V_KEY_ENTER_DOWN
	                    ||V_KEY_UP==V_KEY_UP_DOWN
	                    ||V_KEY_DOWN==V_KEY_DOWN_DOWN))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(V_KEY_ENTER==V_KEY_ENTER_DOWN)
		{
	 
	    	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			return V_KEY_ENTER_SHORT;
		}
		else if(V_KEY_UP==V_KEY_UP_DOWN)
		{
	 
			 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			return V_KEY_UP_SHORT;
		}
		else if(V_KEY_DOWN==V_KEY_DOWN_DOWN)
		{
	 	 
	     	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			return V_KEY_DOWN_SHORT;
		}
		
	}else if(V_KEY_ENTER==V_KEY_ENTER_UP
	            &&V_KEY_UP==V_KEY_UP_UP
	            &&V_KEY_DOWN==V_KEY_DOWN_UP)
	            key_up=1; 	    
 
 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	return V_KEY_NONE;// 无按键按下
}

