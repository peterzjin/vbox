#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "spi.h"
#include "flash.h"
#include "mmc_sd.h"
#include "ff.h"
#include "integer.h"
#include "diskio.h"
#include "text.h"
#include "24cxx.h"
#include "fontupd.h"
#include "stdio.h"
#include "string.h"
#include "picdecoder.h"

#include "v_key.h"
#include "v_menu.h"
#include "uart_timer.h"


 int main(void)
 {
//	FATFS fs;
 	SystemInit();
	delay_init(72);	     //延时初始化
	NVIC_Configuration();
	uart_init(9600);
 	LED_Init();
  	v_key_init();	
	v_lcd_init();
	al_timer_init();
	timo_timer_init();
	v_menu_show_str(0,"  Starting...");
//	SPI_Flash_Init();	//SPI FLASH初始化
//	f_mount(0, &fs);
	v_menu_init();
    while(1)
	{		
      	v_key_scan();
		v_key_judge();	
		v_menu_show();
	}
 }
