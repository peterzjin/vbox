/*ST7920 SPI DEMO PROGRAME*/ 
/* JBG12832 122*32 */
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
//#define bit uint
uchar bit(uchar input){
 return (input ? 1:0);
}
//static uint overflow_count=0;

uchar mh,mt,st; 

//sbit CS=P3^0; 
//sbit SID=P3^1; 
//sbit SCLK=P3^2;
//sbit  RST=P3^3;
//sbit SET_TIME=P1^0;
//sbit UP_KEY=P1^1;
void display_cpubbs(void) ;
void display_bjtime(void) ;
void send_command(uchar command_data) ;
void send_data(uchar command_data) ;
void time_display(void)	;
//void gbchar1(uchar gotoy,uchar gotox);
/*
uchar code gbchar1[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x1E,0x22,0x42,0x42,0x3F,0x00,0x00};
uchar code gbchar2[]={0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x64,0x58,0x00,0x00};

uchar code dispone[]={"串口程序深圳市兴宇合电子有限公司"};
uchar code dispone2[]={"简体字库繁体字库半角全角英文字库"};
uchar code disptwo1[]={"**ST7920+ST7921**XING YU HE T&CH"};		
uchar code disptwo2[]={"JBG12864CHUANKOULCM-mode08/12/25"};
unsigned char code bmp1[]={
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x80,0x00,0x0F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x3F,0xC0,0x00,0x07,0x07,0x00,0x04,0x01,0x00,0x00,0x38,0x60,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x78,0xE0,0x00,0x3F,0x83,0x80,0x06,0x03,0x80,0x00,0x1C,0x70,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x02,0x38,0x70,0x00,0xFF,0x07,0x80,0x06,0x39,0x80,0x00,0x1F,0x70,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x02,0x1C,0x30,0x00,0x7C,0x0E,0x00,0x06,0x19,0xC0,0x00,0x1F,0x80,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x02,0x1C,0x30,0x00,0x3E,0x1C,0x00,0x06,0x1F,0xC0,0x00,0x7F,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x03,0x3E,0x30,0x00,0x62,0x1A,0x00,0x0F,0x79,0x00,0x03,0xFE,0x00,0x00,0x01,
0x80,0x00,0x00,0x60,0x0F,0x00,0x3F,0x00,0x00,0x00,0x3C,0x00,0xC0,0x00,0x00,0x01,
0x80,0x07,0x76,0x18,0x00,0xC3,0xC1,0x00,0x1F,0xFB,0x00,0x03,0xFE,0x00,0x00,0x01,
0x80,0x00,0x01,0xE0,0x3F,0xC0,0xF1,0xC0,0x00,0x00,0xF0,0x01,0xC0,0x00,0x00,0x01,
0x80,0x07,0x6E,0x18,0x00,0xCF,0xC1,0x80,0x7E,0x3F,0x80,0x01,0xE2,0x00,0x00,0x01,
0x80,0x00,0x07,0xE0,0x7F,0xE0,0xE0,0xE0,0x00,0x03,0xC0,0x03,0xC0,0x00,0x00,0x01,
0x80,0x07,0x7F,0x98,0x00,0xFE,0x83,0x81,0xFC,0x1E,0xC0,0x00,0x83,0x00,0x00,0x01,
0x80,0x00,0x0F,0xE0,0x63,0xE1,0xE0,0xE0,0x00,0x07,0x00,0x03,0xC0,0x00,0x00,0x01,
0x80,0x06,0xFF,0xD8,0x01,0xFA,0x87,0x00,0xFD,0xF8,0xC0,0x01,0x73,0x00,0x00,0x01,
0x80,0x00,0x01,0xE0,0xC1,0xE1,0xE0,0xE0,0x00,0x0F,0x00,0x07,0xC0,0x00,0x00,0x01,
0x80,0x0F,0xD9,0xD8,0x03,0xE3,0x0E,0x80,0xFE,0x7E,0xC0,0x03,0xF9,0x80,0x00,0x01,
0x80,0x00,0x01,0xE0,0x01,0xE1,0xF0,0xE0,0x00,0x1E,0x00,0x0B,0xC0,0x00,0x00,0x01,
0x80,0x0D,0xB8,0x18,0x1F,0xC3,0x08,0xC0,0x0E,0x6D,0x80,0x03,0xF1,0x80,0x00,0x01,
0x80,0x00,0x01,0xE0,0x01,0xE0,0xFD,0xC7,0xEF,0x9E,0x00,0x13,0xC0,0x00,0x00,0x01,
0x80,0x0D,0xFE,0x18,0x0F,0xC2,0x10,0xE0,0x1C,0x3F,0x00,0x03,0xE0,0xC0,0x00,0x01,
0x80,0x00,0x01,0xE0,0x01,0xC0,0xFF,0x83,0xC6,0x1F,0xE0,0x13,0xC0,0x00,0x00,0x01,
0x80,0x0D,0xCE,0x18,0x06,0xC2,0x01,0xC0,0x38,0x3E,0x00,0x00,0xE8,0xC0,0x00,0x01,
0x80,0x00,0x01,0xE0,0x01,0x80,0x3F,0x81,0xE4,0x3C,0x78,0x23,0xC0,0x00,0x00,0x01,
0x80,0x0C,0x4C,0x18,0x00,0xC2,0x03,0x80,0x78,0x1F,0xF0,0x00,0xFC,0x60,0x00,0x01,
0x80,0x00,0x01,0xE0,0x03,0x80,0x3F,0xC1,0xE8,0x3C,0x78,0x43,0xC0,0x00,0x00,0x01,
0x80,0x0C,0x5E,0x18,0x00,0xC2,0x07,0x00,0x68,0x3F,0xF8,0x03,0xFC,0x22,0x00,0x01,
0x80,0x00,0x01,0xE0,0x03,0x00,0x6F,0xC0,0xF0,0x3C,0x3C,0x83,0xC0,0x00,0x00,0x01,
0x80,0x06,0x3F,0x38,0x01,0x82,0x0E,0x00,0x09,0xF8,0x00,0x03,0xC0,0x32,0x00,0x01,
0x80,0x00,0x01,0xE0,0x06,0x00,0xC3,0xE0,0x78,0x3C,0x3C,0xFF,0xF0,0x00,0x00,0x01,
0x80,0x07,0x10,0x38,0x00,0x82,0x1C,0x00,0x0B,0x99,0xC0,0x03,0x00,0x1A,0x00,0x01,
0x80,0x00,0x01,0xE0,0x0C,0x11,0xC3,0xE0,0x78,0x3C,0x3C,0xFF,0xF0,0x00,0x00,0x01,
0x80,0x03,0xFF,0xF0,0x00,0x02,0x38,0x00,0x08,0x30,0xE0,0x02,0x00,0x1E,0x00,0x01,
0x80,0x00,0x01,0xE0,0x08,0x11,0xC1,0xE0,0x7C,0x3C,0x3C,0xFF,0xF0,0x00,0x00,0x01,
0x80,0x01,0xEF,0xF0,0x00,0x00,0xE0,0x00,0x08,0xE0,0x60,0x00,0x00,0x0E,0x00,0x01,
0x80,0x00,0x01,0xE0,0x1F,0xF1,0xC1,0xE0,0xBC,0x1C,0x3C,0x03,0xC0,0x00,0x00,0x01,
0x80,0x00,0x03,0xE0,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x07,0x00,0x01,
0x80,0x00,0x01,0xE0,0x3F,0xE1,0xC1,0xC0,0x9E,0x1E,0x38,0x03,0xC0,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,
0x80,0x00,0x01,0xF0,0x7F,0xE0,0xE3,0x83,0x1F,0x0E,0x70,0x03,0xC0,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x0F,0xFC,0xFF,0xE0,0x3F,0x07,0xDF,0x83,0xE0,0x03,0xC0,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x01,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

*/
/*
void delay_1ms(uint x) 
{  
	uchar i; 
	while(x--) 
{ 
for(i=0;i<125;i++); 
} 
}
*/
void delay_1T(uchar x) 
{
delay_us(100);
/*  
	uchar i; 
	while(x--) 
{ 
for(i=0;i<1;i++); 
} 
*/
}
/*

void timer0_ISR(void)interrupt 1
{
	TH0=(65536-46080)/256;TL0=(65536-46080)%256;

	overflow_count++ ;
}
*/
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
/*
void display_text() 
{ 
  uchar i;
  //send_command(0x01);  
  send_command(0x80); 		   
  for(i=0;i<=32;i++ )
	{   			
	send_data(disptwo1[i]);
	}  
  send_command(0x90); 
  for(i=0;i<=32;i++ )
	{ 			
	send_data(disptwo2[i]);
	} 
  delay_1ms(2000);

 //send_command(0x01); ; 
  send_command(0x80); 
  for(i=0;i<=32;i++ )
	{  				
	send_data(dispone[i]);
	} 
  send_command(0x90); 
  for(i=0;i<=32;i++ )
	{ 			
	send_data(dispone2[i]);
	} 	 
  delay_1ms(3000);
} 
*/



void gbchar(uchar data1,uchar data2,uchar gotoy,uchar gotox)
 { 
  uchar i,j;
/*  for(j=0;j<16;j++)
  { 
	  for(i=0;i<32;i++)
	  {     
	   send_command(0x36); //扩充指令开 绘图显示开
	   send_command(gotoy+i);
	   send_command(gotox+j);
	   send_command(0x30);        ///以绘图的方式来写
	   send_data(data1);  //gbchar1[i];
	   send_data(data2);  //gbchar2[i];
	  }
  }
  send_command(0x36);
 // delay_1ms(0x10);
 }	 
*/
//	uint ver_cnt,hor_cnt;

	send_command(0x36);

	for (i=0;i<32;i++)
	{
		send_command(gotox + i);
		send_command(gotoy);	
		for (j=0;j<16;j++)
		{
			if ((i%2) == 0)
			{
				send_data(data1);
				send_data(data1);
			}
			else
			{
				send_data(data2);
				send_data(data2);
			}
		}
  
	}

//  delay_1ms(1000);
 
  }



  void LcmClearBMP( void )
{
         unsigned char i,j;
         send_command(0x34);
         send_command(0x36);
         for(i=0;i<32;i++)
         {
                 send_command(0x80|i);
                 send_command(0x80);
                 for(j=0;j<32;j++)
                    send_data(0);
					//send_data(0);
         }
}
/*
void PutBMP(unsigned char *puts)
{
         unsigned int x=0;
         unsigned char i,j;
         send_command(0x34);
         send_command(0x36);
         for(i=0;i<32;i++) 
         {
                 send_command(0x80|i);
                 send_command(0x80); 
                 for(j=0;j<32;j++)    
                 {                     
                         send_data(puts[x]);
                         x++;
                 }
         }
}


 void gbchar_ab(uchar gotoy,uchar gotox)
 {
  uchar command_data; 
  uchar i,j; 
  for(j=0;j<1;j++)
  { 
  for(i=0;i<16;i++)
  {
   command_data=0x34;         //扩充指令开 绘图显示开
   send_command(command_data);
   command_data=(gotoy+i) ;          // gotoy =0-63
   send_command(command_data);
   command_data=(gotox+j) ;         //gotox = 0-15
   send_command(command_data);
   command_data=0x30; 
   send_command(command_data);
   command_data=gbchar1[i];
   send_data(command_data);
   command_data=gbchar2[i];
   send_data(command_data);
   }
  }
  //delay_1ms(0x10);
  command_data=0x36;         //扩充指令开 绘图显示开
  send_command(command_data);
 // delay_1ms(0x10);
 }	
*/ 

 void wr_bord(void)
{
	uint hor_address,ver_address;
	uint ver_cnt,hor_cnt;

	ver_address=0x80;	
	hor_address=0x80;
	send_command(0x36);
	send_command(0x36);

	send_command(ver_address);
	send_command(hor_address);

	for (hor_cnt=0;hor_cnt<16;hor_cnt++)
	{
		send_data(0xFF);
		send_data(0xFF);
	}

	for (ver_cnt=1;ver_cnt<31;ver_cnt++)
	{
		send_command(ver_address + ver_cnt);
		send_command(hor_address);

		send_data(0x80);
		
		for (hor_cnt=0;hor_cnt<14;hor_cnt++)
		send_data(0x00);

		send_data(0x01);

		/*send_data(0x80);
		
		for (hor_cnt=0;hor_cnt<14;hor_cnt++)
		send_data(0x00);

		send_data(0x01);*/
	};

	send_command(ver_address + 31);
	send_command(hor_address);
	for (hor_cnt=0;hor_cnt<16;hor_cnt++)
	{
		send_data(0xFF);
		send_data(0xFF);
	}

	delay_1ms(500);

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

 lcd_init();
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
void v_lcd_init() 
{
// 	RST=0;
//	delay_1ms(50);	
//	RST=1;
//	delay_1ms(50);
//	uchar command_data;
	v_lcd_gpio_init();
	LEDA = (bit)(0);
	lcd_init();
	delay_1ms(2);
/*    while(1)
	{
	lcd_init();
	delay_1ms(500); 
	gbchar(0xff,0xff,0x80,0x80);
	delay_1ms(500);
	gbchar(0x00,0x00,0x80,0x80);
	delay_1ms(500);
	gbchar(0x55,0x55,0x80,0x80);
	delay_1ms(500);
	gbchar(0xaa,0xaa,0x80,0x80);
	delay_1ms(500);
    gbchar(0xff,0x00,0x80,0x80);
    delay_1ms(500);
    gbchar(0x00,0xff,0x80,0x80);
    delay_1ms(500);
    gbchar(0x55,0xaa,0x80,0x80);
    delay_1ms(500);
	gbchar(0xaa,0x55,0x80,0x80);
    delay_1ms(500);
 //gbchar_ab(0x90,0x87);
    wr_bord();
	delay_1ms(2000);
	LcmClearBMP();
	PutBMP(bmp1);
    delay_1ms(2000);      
    LcmClearBMP();
	delay_1ms(10);
	send_command(0x30);
    delay_1ms(10);
	display_text();
	delay_1ms(2000);

	send_command(0x06);
	delay_1ms(10);
	display_text();
	delay_1ms(4000);
	LcmClearBMP();
	send_command(0x0c);
	delay_1ms(10);
*/
//	  }

}
