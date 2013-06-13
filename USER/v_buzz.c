/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/06/12 23:19
//*****************************************************************************/

#include "v_buzz.h"
#include "delay.h"
#include "sys.h"

#define V_BUZZ_GPIO                GPIOA
#define V_BUZZ_GPIO_PIN            GPIO_Pin_5
#define V_BUZZ_GPIO_MODE         	GPIO_Mode_Out_PP
#define V_BUZZ                     PAout(5)
#define V_BUZZ_ON                  0
#define V_BUZZ_OFF				   1

void v_buzz_timer_start(u16 arr)
{
	TIM_SetCounter(TIM4, 0);
	TIM_SetAutoreload(TIM4, arr);
	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

void v_buzz_timer_stop(void)
{
	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
}
// use TIM4 as buzz timer, 2s
static void v_buzz_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	/* Enable the TIM4 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	v_buzz_timer_start(0);
	v_buzz_timer_stop();
}

void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		// reset the uart state to UART_IDLE
		V_BUZZ = V_BUZZ_OFF;
		v_buzz_timer_stop();
	}
	
}

void v_buzz_init(void)
{ 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
 GPIO_InitStructure.GPIO_Pin = V_BUZZ_GPIO_PIN;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 GPIO_Init(V_BUZZ_GPIO, &GPIO_InitStructure);
 V_BUZZ = V_BUZZ_OFF;
 v_buzz_timer_init();
}

void v_buzz_2s(void)
{
	V_BUZZ = V_BUZZ_ON;
	v_buzz_timer_start(20000);//2s
}


