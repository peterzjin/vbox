/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/06/15 10:14
//*****************************************************************************/
#ifndef __V_BUZZ_H__
#define __V_BUZZ_H__
#include "sys.h"
#include "stm32f10x.h"

#define V_BUZZ_GPIO                GPIOA
#define V_BUZZ_GPIO_PIN            GPIO_Pin_5
#define V_BUZZ_GPIO_MODE         	GPIO_Mode_Out_PP
#define V_BUZZ                     PAout(5)
#define V_BUZZ_ON                  0
#define V_BUZZ_OFF				   1

void v_buzz_init(void);
void v_buzz_alarm_2s(void);
void v_buzz_key(void); //200ms
#endif
