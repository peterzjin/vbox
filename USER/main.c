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
//Mini STM32�����巶������25
//ͼƬ��ʾ ʵ��
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
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
			   LCD_Clear(WHITE);//����,������һ��ͼƬ��ʱ��,һ������
	           result=AI_LoadPicFile(((u8*)strcat(temp,filename)),0,0,240,320);
	           if(result==FALSE) continue;
		       POINT_COLOR=RED;
		       Show_Str(0,0,(u8 *)filename,16,1);//��ʾͼƬ����
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
	delay_init(72);	     //��ʱ��ʼ��
	NVIC_Configuration();
	uart_init(9600);
 	LED_Init();
  	KEY_Init();	
    LCD_Init();
	SPI_Flash_Init();	//SPI FLASH��ʼ��

	f_mount(0, &fs);

	while(font_init())
	{
    	POINT_COLOR=RED;      
		LCD_Clear(WHITE);
		LCD_ShowString(60,50,"Mini STM32");	
		LCD_ShowString(60,70,"Font Updating..."); 	 
 		while(update_font())//��SD������
	    {   
	       	LCD_ShowString(60,90,"FAT SYS ERROR.      ");	 
			delay_ms(200);     
			LED0=!LED0;	
		}
	}
	POINT_COLOR=RED;      
	Show_Str(60,50,"Mini STM32������",16,0);				    	 
	Show_Str(60,70,"ͼƬ��ʾ ����",16,0);				    	 
	Show_Str(60,90,"����ԭ��@ALIENTEK",16,0);				    	 
	Show_Str(60,110,"2011��1��2��",16,0);  
	Show_Str(60,130,"��ʼ��ʾ...",16,0); 
	delay_ms(1000);

	viewPictures(filedir);	//һ��Ŀ¼���

 }
