/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/06/15 10:06
//*****************************************************************************/
#include "stm32f10x.h"
#include "v_menu_timer.h"
#include "v_menu.h"
#include "v_buzz.h"

// use TIM4 as timeout timer, 100us
void v_menu_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM4, 0xFF);

	/* Enable the TIM4 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void v_menu_timer_start(uint16_t arr)
{
	TIM_SetCounter(TIM4, 0);
	TIM_SetAutoreload(TIM4, arr);
	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

void v_menu_timer_stop(void)
{
	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
}
static uint32_t v_menu_timer_tmp;
extern uint32_t v_menu_show_all_time;
void v_menu_show_all_start()
{
	v_menu_timer_tmp = 0;
//	v_buzz_key(); 
	v_menu_timer_start(10000);//1s cycle
}
void v_menu_show_all_stop()
{
	v_menu_show_all_time = 0;
	v_buzz_key(); 
	v_menu_timer_stop();
	v_menu_show_all_time_setting = 0;
	v_setting_changed = 1;
}
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		if(v_menu_timer_tmp > v_menu_show_all_time*60){
			v_menu_show_all_stop();
		}else{
			v_menu_timer_tmp++;
		}
	}
}

// use TIM5 as timeout timer, 100us
void v_buzz_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM5, 0xFF);

	/* Enable the TIM5 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void v_buzz_timer_start(uint16_t arr)
{
	TIM_SetCounter(TIM5, 0);
	TIM_SetAutoreload(TIM5, arr);
	TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
}

void v_buzz_timer_stop(void)
{
	TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM5, DISABLE);
}

void TIM5_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		V_BUZZ =  V_BUZZ_OFF;
		v_buzz_timer_stop();
	}
}
void v_buzz_alarm_2s(void)
{
	v_buzz_timer_stop();
	V_BUZZ = V_BUZZ_ON;
	v_buzz_timer_start(20000);//2s
}

void v_buzz_key(void)
{
	v_buzz_timer_stop();
	V_BUZZ = V_BUZZ_ON;
	v_buzz_timer_start(1000); //200ms
}

