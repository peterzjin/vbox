								/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/06/15 10:06
//*****************************************************************************/

#include "v_buzz.h"

void v_buzz_init(void)
{ 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
 GPIO_InitStructure.GPIO_Pin = V_BUZZ_GPIO_PIN;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 GPIO_Init(V_BUZZ_GPIO, &GPIO_InitStructure);
 V_BUZZ = V_BUZZ_OFF;
}

