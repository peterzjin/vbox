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
#include "v_buzz.h"
#include "v_lcd.h"
#include "v_menu_timer.h"


 int main(void)
 {
 	SystemInit();
	delay_init(72);	     //—” ±≥ı ºªØ
	NVIC_Configuration();
	uart_init(9600);
 	LED_Init();
  	v_key_init();	
	v_lcd_init();
	v_buzz_init();
	cd_key_init();
	al_timer_init();
	timo_timer_init();
	v_menu_timer_init();
	v_buzz_timer_init();
	v_menu_sleep_timer_init();
	v_menu_init();
    while(1)
	{		
      	        v_key_scan();
		v_key_judge();	
		v_menu_show();
	}
 }
