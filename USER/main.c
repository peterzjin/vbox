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
  	v_key_init();	
    LCD_Init();
	SPI_Flash_Init();	//SPI FLASH��ʼ��

	v_menu_init();
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
//        v_key_judge();					/* �����߼����� */	
//        v_input();						/* �ⲿ��������� */	
//        v_menu_display();				/* LCD �˵���ʾ���� */			
//        logic();						/* ϴ�»����߼�������� */
//        output(); 					/* ϴ�»����������� */
//        alarm();						/* ϴ�»���������������� */
	}
//	viewPictures(filedir);	//һ��Ŀ¼���

 }

int main1(void)
{
	u8 t;
	u8 len;	
	u16 times=0;  	
	SystemInit();//ϵͳʱ�ӵȳ�ʼ��
	delay_init(72);	     //��ʱ��ʼ��
	NVIC_Configuration();//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);//���ڳ�ʼ��Ϊ9600
	LED_Init();	 //LED�˿ڳ�ʼ��
	al_timer_init();
	al_timer_start(10000);
	while(1)
	{
		if(cmd_available)
		{					   
#if 0
			len=USART_RX_STA&0x3f;//�õ��˴ν��յ������ݳ���
			//printf("\n�����͵���ϢΪ:\n");
			printf("\nmsg:\n");
			for(t=0;t<len;t++)
			{
				USART1->DR=USART_RX_BUF[t];
				while((USART1->SR&0X40)==0);//�ȴ����ͽ���
			}
			printf("\n\n");//���뻻��
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
				//printf("\nMiniSTM32������ ����ʵ��\n");
				//printf("����ԭ��@ALIENTEK\n\n\n");
				printf("\nuart:\n");
			}
			if(times%200==0)printf("input\n");//printf("����������,�Իس�������\n");  
			if(times%30==0)LED0=!LED0;//��˸LED,��ʾϵͳ��������.
			delay_ms(10);   
		}
	}

}

