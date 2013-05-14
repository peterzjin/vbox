/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 15:37
//*****************************************************************************/
#ifndef __V_KEY_H__
#define __V_KEY_H__
#include "stm32f10x.h"
#include "sys.h"

#define V_KEY_NONE		    0x00

#define V_KEY_ETNER_GPIO                GPIOA
#define V_KEY_ENTER_GPIO_PIN            GPIO_Pin_0
#define V_KEY_ENTER_GPIO_MODE         	GPIO_Mode_IPD
#define V_KEY_ENTER                     PAin(0)
#define V_KEY_ENTER_UP                  0x00
#define V_KEY_ENTER_DOWN                0x01
#define V_KEY_ENTER_SHORT               0x10
#define V_KEY_ENTER_3S                  0x11
#define V_KEY_ENTER_6S                  0x12
#define V_KEY_ENTER_15S                 0x13

#define V_KEY_ESC_GPIO                  0
#define V_KEY_ESC_GPIO_PIN           	0
#define V_KEY_ESC_GPIO_MODE 			0
#define V_KEY_ESC_UP                    0x00
#define V_KEY_ESC_DOWN                  0x01
#define V_KEY_ESC                       0
#define V_KEY_ESC_SHORT        			0x20
#define V_KEY_ESC_3S               		0x21
#define V_KEY_ESC_9S                	0x22

#define V_KEY_UP_GPIO                  GPIOA
#define V_KEY_UP_GPIO_PIN              GPIO_Pin_13
#define V_KEY_UP_GPIO_MODE     		GPIO_Mode_IPU
#define V_KEY_UP                       PAin(13)
#define V_KEY_UP_UP                    0x01
#define V_KEY_UP_DOWN                  0x00
#define V_KEY_UP_SHORT		            0x40

#define V_KEY_DOWN_GPIO                 GPIOA
#define V_KEY_DOWN_GPIO_PIN             GPIO_Pin_15
#define V_KEY_DOWN_GPIO_MODE            GPIO_Mode_IPU
#define V_KEY_DOWN                      PAin(15)
#define V_KEY_DOWN_UP                   0x01
#define V_KEY_DOWN_DOWN                 0x00
#define V_KEY_DOWN_SHORT		        0x80

extern uint8_t v_key;

void v_key_Init(void);
uint8_t v_key_scan(void);

#endif
