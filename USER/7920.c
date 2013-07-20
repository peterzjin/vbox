#include  <stdio.h>
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"
#include "v_lcd.h"

#define code  
#define delay_1ms delay_ms

#define V_SCREEN_RCC_APB2Periph_GPIO  RCC_APB2Periph_GPIOB
#define V_SCREEN_RCC_APB2Periph_BL_GPIO  RCC_APB2Periph_GPIOA
#define V_SCREEN_GPIO                  GPIOB
#define V_SCREEN_BL_GPIO                  GPIOA
#define V_SCREEN_GPIO_CS_PIN              GPIO_Pin_10
#define V_SCREEN_GPIO_SID_PIN              GPIO_Pin_11
#define V_SCREEN_GPIO_SCLK_PIN              GPIO_Pin_12
#define V_SCREEN_GPIO_LEDA_PIN              GPIO_Pin_4

#define V_SCREEN_GPIO_GPIO_MODE     		GPIO_Mode_Out_PP
#define CS PBout(10)
#define SID PBout(11)
#define SCLK PBout(12)
#define LEDA PAout(4) 

#define uint unsigned int 
#define uchar unsigned char

uchar bit(uchar input){
 return (input ? 1:0);
}

uchar mh,mt,st; 

void display_cpubbs(void) ;
void display_bjtime(void) ;
void send_command(uchar command_data) ;
void send_data(uchar command_data) ;
void time_display(void)	;

void delay_1T(uchar x) 
{
delay_us(100);
}

void send_command(uchar command_data) 
{ 
	uchar i; 
	uchar i_data;//,temp_data1,temp_data2; 
	i_data=0xf8; 
	delay_1T(1); 
	CS=1; 
	SCLK=0; 
	for(i=0;i<8;i++) 
		{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
		} 
	i_data=command_data; 
	i_data&=0xf0; 
	for(i=0;i<8;i++) 
		{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
     	}  
	i_data=command_data&0x0f;  
	i_data<<=4; 
	for(i=0;i<8;i++) 
    	{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
		} 
	CS=0; 
} 

void send_data(uchar command_data) 
{ 
	uchar i; 
	uchar i_data;//,temp_data1,temp_data2; 
	i_data=0xfa; 
	delay_1T(1) ;
	CS=1; 
	for(i=0;i<8;i++) 
		{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
		} 
	i_data=command_data; 
	i_data&=0xf0; 
	for(i=0;i<8;i++) 
		{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
		} 
	i_data=command_data&0x0f; 
	i_data<<=4; 
	for(i=0;i<8;i++) 
		{ 
		SID=(bit)(i_data&0x80); 
		SCLK=0; 
		SCLK=1; 
		i_data=i_data<<1; 
		} 
	CS=0; 
}
void lcd_init() 
{ 
	delay_1ms(100);  
	send_command(0x30);  
	send_command(0x04);
	send_command(0x0c);  
	send_command(0x0); 
	send_command(0x02); 
	send_command(0x80); 
}

void v_lcd_gpio_init(void)
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(V_SCREEN_RCC_APB2Periph_GPIO, ENABLE);
	
 GPIO_InitStructure.GPIO_Pin = V_SCREEN_GPIO_CS_PIN | V_SCREEN_GPIO_SID_PIN 
 								|V_SCREEN_GPIO_SCLK_PIN;
 GPIO_InitStructure.GPIO_Mode = V_SCREEN_GPIO_GPIO_MODE; 
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 GPIO_Init(V_SCREEN_GPIO, &GPIO_InitStructure);

 RCC_APB2PeriphClockCmd(V_SCREEN_RCC_APB2Periph_BL_GPIO, ENABLE);
	
 GPIO_InitStructure.GPIO_Pin = V_SCREEN_GPIO_LEDA_PIN;
 GPIO_InitStructure.GPIO_Mode = V_SCREEN_GPIO_GPIO_MODE; 
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 GPIO_Init(V_SCREEN_BL_GPIO, &GPIO_InitStructure);
 LEDA = (bit)(1);
}
//v_location 0:line1    1:line2    0xFF:center
void v_menu_show_str(uint8_t v_location, char *str){
	uint8_t i;
	if(v_location!=0xFF){
		send_command(v_location?0x90:0x80);
	}else{
		send_command(0x88);
	} 		   
	for(i=0;i<32&&str[i];i++ )
	{   			
		send_data(str[i]);
	}
		 
	for(;i<32;i++){
		send_data(' ');
	} 	
} 
static uint8_t v_menu_show_inverse_0 = 0;
static uint8_t v_menu_show_inverse_1 = 0;
void v_menu_show_inverse(uint8_t v_location){
       if(!v_location){
           if(1 == v_menu_show_inverse_0) return;
           send_command(0x34);
	    send_command(0x04);
	    v_menu_show_inverse_0 = 1;
	}else{
	     if(1 == v_menu_show_inverse_1) return;
	    send_command(0x34);
	    send_command(0x05);
	    v_menu_show_inverse_1 = 1;
	}
	send_command(0x30);
}
void v_menu_clear_inverse(){
         if(v_menu_show_inverse_0){
            send_command(0x34);
	     send_command(0x04);
	     v_menu_show_inverse_0 = 0;
         }
         if(v_menu_show_inverse_1){
            send_command(0x34);
	     send_command(0x05);
	     v_menu_show_inverse_1 = 0;
         }
         send_command(0x30);
}

void v_lcd_backlight(uint8_t open)
{
    if(open){
        LEDA = (bit)(0);
    }else{
        send_command(0x01);
        LEDA = (bit)(1);
    }
}
void v_lcd_init() 
{
	v_lcd_gpio_init();
	lcd_init();
	delay_1ms(2);
}
