/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 15:37
//*****************************************************************************/

#include "v_key.h"
#include "delay.h"
#include "v_menu.h"
#include "sys.h"
#include "led.h"
#include "v_buzz.h"
#include "v_menu_timer.h"

#define V_KEY_NONE		    0x00

#define V_KEY_ETNER_GPIO                GPIOA
#define V_KEY_ENTER_GPIO_PIN            GPIO_Pin_0
#define V_KEY_ENTER_GPIO_MODE         	GPIO_Mode_IPU
#define V_KEY_ENTER                     PAin(0)
#define V_KEY_ENTER_UP                  0x01
#define V_KEY_ENTER_DOWN                0x00
#define V_KEY_ENTER_SHORT               0x10
#define V_KEY_ENTER_3S                  0x11
#define V_KEY_ENTER_6S                  0x12
#define V_KEY_ENTER_15S                 0x13

#define V_KEY_ESC_GPIO                  GPIOA
#define V_KEY_ESC_GPIO_PIN           	GPIO_Pin_1
#define V_KEY_ESC_GPIO_MODE 			GPIO_Mode_IPU
#define V_KEY_ESC                       PAin(1)
#define V_KEY_ESC_UP                    0x01
#define V_KEY_ESC_DOWN                  0x00
#define V_KEY_ESC_SHORT        			0x20
#define V_KEY_ESC_3S               		0x21
#define V_KEY_ESC_9S                	0x22

#define V_KEY_UP_GPIO                  GPIOA
#define V_KEY_UP_GPIO_PIN              GPIO_Pin_3
#define V_KEY_UP_GPIO_MODE     		GPIO_Mode_IPU
#define V_KEY_UP                       PAin(3)
#define V_KEY_UP_UP                    0x01
#define V_KEY_UP_DOWN                  0x00
#define V_KEY_UP_SHORT		            0x40

#define V_KEY_DOWN_GPIO                 GPIOA
#define V_KEY_DOWN_GPIO_PIN             GPIO_Pin_2
#define V_KEY_DOWN_GPIO_MODE            GPIO_Mode_IPU
#define V_KEY_DOWN                      PAin(2)
#define V_KEY_DOWN_UP                   0x01
#define V_KEY_DOWN_DOWN                 0x00
#define V_KEY_DOWN_SHORT		        0x80

static uint8_t v_key  = V_KEY_NONE;
static u8 v_key_down=0;//按键按下的标志

void v_key_init(void) 
{
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin  = V_KEY_ENTER_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_ENTER_GPIO_MODE;
 	GPIO_Init(V_KEY_ETNER_GPIO, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin  = V_KEY_ESC_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_ESC_GPIO_MODE;
 	GPIO_Init(V_KEY_ESC_GPIO, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin  = V_KEY_UP_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_UP_GPIO_MODE;
 	GPIO_Init(V_KEY_UP_GPIO, &GPIO_InitStructure);

 	GPIO_InitStructure.GPIO_Pin  = V_KEY_DOWN_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = V_KEY_DOWN_GPIO_MODE;
 	GPIO_Init(V_KEY_DOWN_GPIO, &GPIO_InitStructure);
			 
	v_key_down = 0;
	v_key  = V_KEY_NONE;
}
#define V_KEY_CHECK_INTERVAL 	10 //10ms
#define V_KEY_CHECK_SHORT		1
#define V_KEY_CHECK_3S			(3000/V_KEY_CHECK_INTERVAL)		
#define V_KEY_CHECK_6S			(6000/V_KEY_CHECK_INTERVAL)
#define V_KEY_CHECK_9S			(9000/V_KEY_CHECK_INTERVAL)
#define V_KEY_CHECK_15S			(15000/V_KEY_CHECK_INTERVAL)	

extern uint8_t v_menu_sleep_wakeup_flag;
extern uint8_t v_menu_deep_sleep_flag;
extern v_setting_struct_t v_menu_setting;
void v_key_scan(void)
{	 
 	int i = 0;
 	if(v_menu_setting.v_menu_sleep && v_menu_deep_sleep_flag){
	             return;
	}
	v_key = V_KEY_NONE;

// 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	if(!v_key_down&&(V_KEY_ESC==V_KEY_ESC_DOWN || V_KEY_ENTER==V_KEY_ENTER_DOWN
	                 ||V_KEY_UP==V_KEY_UP_DOWN ||V_KEY_DOWN==V_KEY_DOWN_DOWN))
	{
	       uint8_t flag_over = 0;
		delay_ms(V_KEY_CHECK_INTERVAL);//去抖动
		if(V_KEY_ESC==V_KEY_ESC_DOWN)
		{     
		       uint8_t flag_3S_over = 0;
			uint8_t flag_9S_over = 0;
		       flag_over = 1;
		       v_buzz_key(); 
			v_key_down = 1;
			for(i=0;i<V_KEY_CHECK_15S+1;i++){
				if(V_KEY_ESC==V_KEY_ESC_UP)	
				{
					v_key_down = 0;
					break;
				}
				delay_ms(V_KEY_CHECK_INTERVAL);
				{
			    	       if(i>V_KEY_CHECK_9S){
						if(flag_9S_over == 0)
						{
							flag_9S_over = 1;						
							v_buzz_key();
						}
					}else if(i>V_KEY_CHECK_3S){
						if(flag_3S_over == 0)
						{
							flag_3S_over = 1;
							v_buzz_key();
						}
					}else{	    	
						//v_key = V_KEY_ENTER_SHORT;
					}
					v_menu_show();
				}
			}
//			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			if(i>V_KEY_CHECK_9S){
				v_key = V_KEY_ESC_9S;
			}else if(i>V_KEY_CHECK_3S){
				v_key = V_KEY_ESC_3S;
			}else{	    	
				v_key = V_KEY_ESC_SHORT;
			}
		}else if(V_KEY_ENTER==V_KEY_ENTER_DOWN)
		{
		       
			uint8_t flag_3S_over = 0;
			uint8_t flag_6S_over = 0;
			uint8_t flag_15S_over = 0;
                     flag_over = 1;
		       v_buzz_key();
			v_key_down = 1;
			for(i=0;i<V_KEY_CHECK_15S+1;i++){
				if(V_KEY_ENTER==V_KEY_ENTER_UP)	
				{
					v_key_down = 0;
					break;
				}
				delay_ms(V_KEY_CHECK_INTERVAL);
				//add 
				{
			    	if(i>V_KEY_CHECK_15S){
						if(flag_15S_over == 0)
						{
							flag_15S_over = 1;						
							v_menu_enter_15s();
						}
					}else if(i>V_KEY_CHECK_6S){
						if(flag_6S_over == 0)
						{
							flag_6S_over = 1;						
							v_menu_enter_6s();
						}
					}else if(i>V_KEY_CHECK_3S){
						if(flag_3S_over == 0)
						{
							flag_3S_over = 1;
							v_menu_enter_3s();
						}
					}else{	    	
						//v_key = V_KEY_ENTER_SHORT;
					}
					v_menu_show();
				}
			}
//			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			if(i>V_KEY_CHECK_15S){
				v_key = V_KEY_ENTER_15S;
			}else if(i>V_KEY_CHECK_6S){
				v_key = V_KEY_ENTER_6S;
			}else if(i>V_KEY_CHECK_3S){
				v_key = V_KEY_ENTER_3S;
			}else{	    	
				v_key = V_KEY_ENTER_SHORT;
			}
		}else if(V_KEY_UP==V_KEY_UP_DOWN)
		{
		       flag_over = 1;
		       v_buzz_key();
			v_key_down = 1;
			for(i=0;i<V_KEY_CHECK_15S+1;i++){
				if(V_KEY_UP==V_KEY_UP_UP)	
				{
					v_key_down = 0;
					break;
				}
				delay_ms(V_KEY_CHECK_INTERVAL);
			}
//			GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	    	
			v_key = V_KEY_UP_SHORT;
		}else if(V_KEY_DOWN==V_KEY_DOWN_DOWN)
		{
		       flag_over = 1;
		       v_buzz_key();
	 	 	v_key_down = 1;
			for(i=0;i<V_KEY_CHECK_15S+1;i++){
				if(V_KEY_DOWN==V_KEY_DOWN_UP)	
				{
					v_key_down = 0;
					break;
				}
				delay_ms(V_KEY_CHECK_INTERVAL);
			}
//	     	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
			v_key = V_KEY_DOWN_SHORT;
		}
		
	}else if(V_KEY_ENTER==V_KEY_ENTER_UP
        &&V_KEY_UP==V_KEY_UP_UP
        &&V_KEY_DOWN==V_KEY_DOWN_UP) {
        	v_key_down = 0; 	    
	}
 
// 	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

void v_key_judge(void)
{
	if(V_KEY_NONE == v_key){
		return;
	}
//	v_buzz_key(); 
	if(v_menu_setting.v_menu_sleep){
		if(!v_menu_sleep_wakeup_flag){
		       v_menu_sleep_wakeup_flag = 1;
		       v_menu_sleep_wakeup_action();
		       v_menu_sleep_wakeup_timer_start();
		       return;
		}
		v_menu_sleep_wakeup_timer_start();
	}
	switch(v_key){
		case V_KEY_ENTER_SHORT:
//			Show_Str(60,130,"V_KEY_ENTER_SHORT",16,0);
//			v_menu_show_str(0,"V_KEY_ENTER_SHORT");
			v_menu_enter_short();
		break;
		case V_KEY_ENTER_3S:
//			Show_Str(60,130,"V_KEY_ENTER_3S",16,0);
//			v_menu_show_str(0,"V_KEY_ENTER_3S");
//			LED0=!LED0	;
			v_menu_enter_3s();
		break;
		case V_KEY_ENTER_6S:
//			Show_Str(60,130,"V_KEY_ENTER_6S",16,0);
//			v_menu_show_str(0,"V_KEY_ENTER_6S");
			v_menu_enter_6s();
		break;
		case V_KEY_ENTER_15S:
//			Show_Str(60,130,"V_KEY_ENTER_15S",16,0);
//			v_menu_show_str(0,"V_KEY_ENTER_15S");
			v_menu_enter_15s();
		break;
		case V_KEY_ESC_SHORT:
//			Show_Str(60,130,"V_KEY_ESC_SHORT",16,0);
//			v_menu_show_str(0,"V_KEY_ESC_SHORT");
			v_menu_esc_short();
		break;
		case V_KEY_ESC_3S:
//			Show_Str(60,130,"V_KEY_ESC_3S",16,0);
//			v_menu_show_str(0,"V_KEY_ESC_3S");
			v_menu_esc_3s();
		break;
		case V_KEY_ESC_9S:
//			Show_Str(60,130,"V_KEY_ESC_9S",16,0);
//			v_menu_show_str(0,"V_KEY_ESC_9S");
			v_menu_esc_9s();
		break;
		case V_KEY_UP_SHORT:
//			Show_Str(60,130,"V_KEY_UP_SHORT",16,0);
//			v_menu_show_str(0,"V_KEY_UP_SHORT");
			v_menu_up_short();
		break;
		case V_KEY_DOWN_SHORT:
//			Show_Str(60,130,"V_KEY_DOWN_SHORT",16,0);
//			v_menu_show_str(0,"V_KEY_DOWN_SHORT");
			v_menu_down_short();
		break;
	}
}

