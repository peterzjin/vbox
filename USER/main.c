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
//Mini STM32开发板范例代码25
//图片显示 实验
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
void viewPictures(const char *fileDir){
    DIR dir;
	BOOL result;
	FRESULT res;
	FILINFO fileInfo;
	char *filename;
	char  temp[256];
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];
    fileInfo.lfname = lfn;
    fileInfo.lfsize = sizeof(lfn);
#endif
	delay_ms(300);
	LCD_Clear(BRRED);
 	while(1)
	{
  	   res= f_opendir(&dir,  fileDir);
	   if(res==FR_OK)
       {  	
	     for(;;)
		 {
            res =f_readdir(&dir,&fileInfo);
	        if(res!=0||fileInfo.fname[0]==0)break;
	        if(fileInfo.fname[0]=='.')continue  ;
#if _USE_LFN
            filename = *fileInfo.lfname ? fileInfo.lfname : fileInfo.fname;
#else
            filename = fileInfo.fname;
#endif
            if ( !(fileInfo.fattrib & AM_DIR) ) 
		    {
 		       strcpy(temp,fileDir);
			   strcat(temp,"/");
			   LCD_Clear(WHITE);//清屏,加载下一幅图片的时候,一定清屏
	           result=AI_LoadPicFile(((u8*)strcat(temp,filename)),0,0,240,320);
	           if(result==FALSE) continue;
		       POINT_COLOR=RED;
		       Show_Str(0,0,(u8 *)filename,16,1);//显示图片名字
	           delay_ms(1000); 
	        }
	      } 
	   }	 
	 }
  }
 int main(void)
 {
	const char filedir[]="0:/PICTURE";
	FATFS fs;
 	SystemInit();
	delay_init(72);	     //延时初始化
	NVIC_Configuration();
	uart_init(9600);
 	LED_Init();
  	v_key_init();	
    LCD_Init();
	SPI_Flash_Init();	//SPI FLASH初始化

	v_menu_init();
	f_mount(0, &fs);

	while(font_init())
	{
    	POINT_COLOR=RED;      
		LCD_Clear(WHITE);
		LCD_ShowString(60,50,"Mini STM32");	
		LCD_ShowString(60,70,"Font Updating..."); 	 
 		while(update_font())//从SD卡更新
	    {   
	       	LCD_ShowString(60,90,"FAT SYS ERROR.      ");	 
			delay_ms(200);     
			LED0=!LED0;	
		}
	}
	POINT_COLOR=RED;      
	Show_Str(60,50,"Mini STM32开发板",16,0);				    	 
	Show_Str(60,70,"图片显示 程序",16,0);				    	 
	Show_Str(60,90,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(60,110,"2011年1月2日",16,0);  
	Show_Str(60,130,"开始显示...",16,0); 
	delay_ms(1000);

	
    while(1)
	{		
//        v_key = v_key_scan();
		if(v_cur_menu->menu_show){
			v_cur_menu->menu_show();
		}
		v_cur_menu = v_cur_menu->down_menu;
		delay_ms(500);
/*		if(v_key != V_KEY_NONE){
			switch(v_key)
			{				 
				case V_KEY_ENTER_SHORT:
					LED0=!LED0;
					break;
				case V_KEY_UP_SHORT:
					LED0=!LED0;
					break;
				case V_KEY_UP_SHORT:				
					LED0=!LED0;
					break;
			}
		}else{
			delay_ms(10);
		}
*/	
//        v_key_judge();					/* 按键逻辑程序 */	
//        v_input();						/* 外部输入检测程序 */	
//        v_menu_display();				/* LCD 菜单显示程序 */			
//        logic();						/* 洗衣机主逻辑处理程序 */
//        output(); 					/* 洗衣机输出处理程序 */
//        alarm();						/* 洗衣机蜂鸣报警处理程序 */
	}
//	viewPictures(filedir);	//一级目录浏览

 }

int main1(void)
{
	u8 t;
	u8 len;	
	u16 times=0;  	
	SystemInit();//系统时钟等初始化
	delay_init(72);	     //延时初始化
	NVIC_Configuration();//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);//串口初始化为9600
	LED_Init();	 //LED端口初始化
	al_timer_init();
	al_timer_start(10000);
	while(1)
	{
		if(cmd_available)
		{					   
#if 0
			len=USART_RX_STA&0x3f;//得到此次接收到的数据长度
			//printf("\n您发送的消息为:\n");
			printf("\nmsg:\n");
			for(t=0;t<len;t++)
			{
				USART1->DR=USART_RX_BUF[t];
				while((USART1->SR&0X40)==0);//等待发送结束
			}
			printf("\n\n");//插入换行
			USART_RX_STA=0;
#endif
			if (cmd_data == 1)
				al_timer_stop();
			if (cmd_data == 2)
				al_timer_start(5000);
			if (cmd_data == 3)
				al_timer_start(10000);
			if (cmd_data == 4)
				al_timer_start(20000);
			cmd_available = 0;
		}else
		{
			times++;
			if(times%5000==0)
			{
				//printf("\nMiniSTM32开发板 串口实验\n");
				//printf("正点原子@ALIENTEK\n\n\n");
				printf("\nuart:\n");
			}
			if(times%200==0)printf("input\n");//printf("请输入数据,以回车键结束\n");  
			if(times%30==0)LED0=!LED0;//闪烁LED,提示系统正在运行.
			delay_ms(10);   
		}
	}

}

