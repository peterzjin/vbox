/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 22:17
//*****************************************************************************/
#include "v_menu.h"
#include "text.h"
#include "sys.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#include "v_lcd.h"
#include "v_buzz.h"
#include "uart_timer.h"
#include "v_menu_timer.h"

//#define DEBUG

#define V_MENU_LANGUAGE_EN		0x01
#define V_MENU_LANGUAGE_CN		0x00

#ifdef DEBUG
#define V_MENU_SETTING_LOCAL_SHOW_ALL_TIME 1
#else
#define V_MENU_SETTING_LOCAL_SHOW_ALL_TIME	60	//60m
#endif

#define V_MENU_MAX_REAL_TIME_S   86400
#define V_MENU_SENSOR_STATUS_NOT_IN_USE		0x00
#define V_MENU_SENSOR_STATUS_NORMAL			0x01
#define V_MENU_SENSOR_STATUS_ERROR			0x02
#define V_MENU_SHOW_STATUS_NORMAL			0x00
#define V_MENU_SHOW_STATUS_ERROR			0xFFFFFFFF
#define V_MENU_SHOW_STATUS_ABNORMAL			0xFFFFFFFE
#define V_MENU_SHOW_STATUS_INVALID			0xFFFFFFFD

#define V_MENU_SHOW_TYPE_TRIP       0
#define V_MENU_SHOW_TYPE_TOT        1
#define V_MENU_SHOW_TYPE_TRAVEL     2
#define V_MENU_SHOW_TYPE_INSTANT    3
#define V_MENU_SHOW_TYPE_FLOW    4

#define V_MENU_SHOW_DATA_MASK 	((1<<SENSOR_DATA_PULSE)|(1<<SENSOR_DATA_TOT_TRIP)|(1<<SENSOR_DATA_LITER))
#define V_MENU_CMD_DISPLAY_MODE_MASK    ((1<<DISPLAY_FORMAT)|(1<<DISPLAY_DELAY_TIME))

typedef struct v_menu_struct
{ 
	struct v_menu_struct *enter_menu;
	struct v_menu_struct *esc_menu;
	struct v_menu_struct *up_menu;
	struct v_menu_struct *down_menu;
	void (*do_function) (void);
	void (*menu_show)(void);
}v_menu_struct_t;

//local variables
static v_menu_struct_t *v_cur_menu;

uint8_t v_menu_show_all_flag = 0;
uint8_t v_menu_sleep_wakeup_flag;
uint8_t v_menu_deep_sleep_flag;
//static uint32_t v_menu_show_all_time_origin;
static uint8_t v_menu_in_display = 1;		//in diplay flag
static uint32_t v_menu_show_last_value_0;
static uint32_t v_menu_show_last_value_1;
static uint32_t v_menu_show_error_count_0;
static uint32_t v_menu_show_error_count_1;

#if 0
static uint32_t v_menu_latest_sleep;
static uint32_t v_menu_latest_language;
static uint32_t v_menu_latest_show_all_time;
static uint32_t v_menu_latest_display_format;
static uint32_t v_sensor_latest_work_mode;
static uint32_t v_sensor_latest_error_delay_time;
static uint32_t v_sensor_latest_instant_fuel_consum_1_alarm;
static uint32_t v_sensor_latest_instant_fuel_consum_2_alarm;
static uint32_t v_sensor_latest_fuel_consum_sample_time;
static uint32_t v_sensor_latest_flow_smaple_time;
#endif

uint32_t v_menu_display_format_setting;
uint32_t v_menu_show_all_time_setting;

v_setting_struct_t v_setting;
v_setting_struct_t v_menu_setting;

#if 0
//0: disable 1: enable
uint32_t v_menu_sleep;                                     //0x0f
uint32_t v_menu_deep_sleep;                          //?
uint32_t v_menu_reset_tot_enable;			//0x01

uint32_t v_menu_language;					//0x0E
uint32_t v_menu_display_format;				//0x11
uint32_t v_menu_show_all_time;				//0x20
//like datasheet
uint32_t v_sensor_work_mode;
uint32_t v_sensor_error_delay_time;			//0x18
//0: no alarm 1:alarm
uint32_t v_sensor_instant_fuel_consum_1_alarm; //0x1A
uint32_t v_sensor_instant_fuel_consum_2_alarm; //0x1B

uint32_t v_sensor_fuel_consum_sample_time;	//0x19
uint32_t v_sensor_flow_smaple_time;			//0x09
#endif
uint32_t v_fuel_consum_1_correct;			 	//0x07|0x08	 noneed
uint32_t v_fuel_consum_2_correct;
//0:need correct  1: have corrected 
uint32_t v_fuel_consum_1_corrected;
uint32_t v_fuel_consum_2_corrected;


//0x3C|0x3F|0x3D
uint32_t v_data_real_time;
//0:not in use 1:normal 2:error
uint32_t v_sensor_A_status;
uint32_t v_sensor_B_status;
uint32_t v_sensor_C_status;
uint32_t v_sensor_D_status;

//0x3C|0x3F
uint32_t v_sensor_A_Trip;
uint32_t v_sensor_B_Trip;
uint32_t v_sensor_C_Trip;
uint32_t v_sensor_D_Trip;
uint32_t v_sensor_A_Tot;
uint32_t v_sensor_B_Tot;
uint32_t v_sensor_C_Tot;
uint32_t v_sensor_D_Tot;

//0x3D
uint32_t v_fuel_consum_1_travel_time;
uint32_t v_fuel_consum_1_travel_consum;
uint32_t v_fuel_consum_1_trip;
uint32_t v_fuel_consum_1_tot;
uint32_t v_fuel_consum_2_travel_time;
uint32_t v_fuel_consum_2_travel_consum;
uint32_t v_fuel_consum_2_trip;
uint32_t v_fuel_consum_2_tot;

static uint32_t v_data_latest_real_time;
//static uint32_t v_sensor_A_latest_Trip;
static uint32_t v_sensor_A_latest_Tot;
//static uint32_t v_sensor_B_latest_Trip;
static uint32_t v_sensor_B_latest_Tot;
//static uint32_t v_sensor_C_latest_Trip;
static uint32_t v_sensor_C_latest_Tot;
//static uint32_t v_sensor_D_latest_Trip;
static uint32_t v_sensor_D_latest_Tot;

//static uint32_t v_fuel_consum_1_latest_travel_time;
//static uint32_t v_fuel_consum_2_latest_travel_time;
static uint32_t v_fuel_consum_1_latest_travel_consum ;
static uint32_t v_fuel_consum_2_latest_travel_consum ;

static void reset_all_data(void){
//0:not in use 1:normal 2:abnormal
#ifdef DEBUG
cmd_data_available = 1;
v_data_real_time = 56789;
v_data_latest_real_time  = 56788;
//v_menu_sleep=1;
//v_menu_sleep_wakeup = 0;
v_fuel_consum_1_correct = 20000;			 	//0x07|0x08	 noneed
v_fuel_consum_2_correct = 20000;
//0:need correct  1: have corrected 
v_fuel_consum_1_corrected = 0;
v_fuel_consum_2_corrected = 0;

v_sensor_A_status = V_MENU_SENSOR_STATUS_NORMAL;
v_sensor_B_status = V_MENU_SENSOR_STATUS_NORMAL;
v_sensor_C_status = V_MENU_SENSOR_STATUS_NORMAL;
v_sensor_D_status = V_MENU_SENSOR_STATUS_NORMAL;
v_sensor_A_Trip = 12345;					 
v_sensor_B_Trip = 12345;
v_sensor_C_Trip = 12345;
v_sensor_D_Trip = 12345;
v_sensor_A_Tot = 23456;
v_sensor_B_Tot = 12345;
v_sensor_C_Tot = 23456;
v_sensor_D_Tot = 12345;
//v_sensor_A_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_A_latest_Tot = 2345;
//v_sensor_B_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_B_latest_Tot = 2345;
//v_sensor_C_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_C_latest_Tot = 2345;
//v_sensor_D_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_D_latest_Tot = 2345;
//0x3D
v_fuel_consum_1_travel_time = 0x443322;	
v_fuel_consum_1_travel_consum = 11111;
v_fuel_consum_1_trip = 23456;
v_fuel_consum_1_tot = 23456;
v_fuel_consum_2_travel_time = 0x443322;
v_fuel_consum_2_travel_consum = 11111;
v_fuel_consum_2_trip = 23456;
v_fuel_consum_2_tot = 23456;
//v_fuel_consum_1_latest_travel_time = 23456;
//v_fuel_consum_2_latest_travel_time = 23456;
v_fuel_consum_1_latest_travel_consum = 2345;
v_fuel_consum_2_latest_travel_consum = 2345;

v_menu_show_last_value_0 = V_MENU_SHOW_STATUS_INVALID;
v_menu_show_last_value_1 = V_MENU_SHOW_STATUS_INVALID;
v_menu_show_error_count_0 = V_MENU_SHOW_STATUS_INVALID;
v_menu_show_error_count_1 = V_MENU_SHOW_STATUS_INVALID;
#else
v_data_real_time = 0;
v_data_latest_real_time  = 0;
v_sensor_A_status = V_MENU_SENSOR_STATUS_NOT_IN_USE;
v_sensor_B_status = V_MENU_SENSOR_STATUS_NOT_IN_USE;
v_sensor_C_status = V_MENU_SENSOR_STATUS_NOT_IN_USE;
v_sensor_D_status = V_MENU_SENSOR_STATUS_NOT_IN_USE;
v_sensor_A_Trip = V_MENU_SHOW_STATUS_INVALID;					 
v_sensor_B_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_C_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_D_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_A_Tot = V_MENU_SHOW_STATUS_INVALID;
v_sensor_B_Tot = V_MENU_SHOW_STATUS_INVALID;
v_sensor_C_Tot = V_MENU_SHOW_STATUS_INVALID;
v_sensor_D_Tot = V_MENU_SHOW_STATUS_INVALID;
//v_sensor_A_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_A_latest_Tot = V_MENU_SHOW_STATUS_INVALID;
//v_sensor_B_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_B_latest_Tot = V_MENU_SHOW_STATUS_INVALID;
//v_sensor_C_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_C_latest_Tot = V_MENU_SHOW_STATUS_INVALID;
//v_sensor_D_latest_Trip = V_MENU_SHOW_STATUS_INVALID;
v_sensor_D_latest_Tot = V_MENU_SHOW_STATUS_INVALID;

//0x3D
v_fuel_consum_1_travel_time = 0;	
v_fuel_consum_1_travel_consum = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_1_trip = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_1_tot = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_2_travel_time = 0;
v_fuel_consum_2_travel_consum = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_2_trip = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_2_tot = V_MENU_SHOW_STATUS_INVALID;
//v_fuel_consum_1_latest_travel_time = 0;
//v_fuel_consum_2_latest_travel_time = 9;
v_fuel_consum_1_latest_travel_consum = V_MENU_SHOW_STATUS_INVALID;
v_fuel_consum_2_latest_travel_consum= V_MENU_SHOW_STATUS_INVALID;

v_menu_show_last_value_0 = V_MENU_SHOW_STATUS_INVALID;
v_menu_show_last_value_1 = V_MENU_SHOW_STATUS_INVALID;
v_menu_show_error_count_0 = 0;
v_menu_show_error_count_1 = 0;

#endif
}

static void v_menu_function(void){
	if(v_cur_menu){
		if(v_menu_in_display){
			reset_all_data();
		}
		if(v_cur_menu->do_function) v_cur_menu->do_function();
	}
}

uint8_t v_menu_changed;
uint8_t v_setting_changed;

void v_menu_show(void){
	if(v_cur_menu){
		if(v_menu_setting.v_menu_sleep && !v_menu_sleep_wakeup_flag){
			return;
		}
		if(v_menu_in_display){
			if(!v_menu_changed){
		      	if( !(cmd_data_available&V_MENU_SHOW_DATA_MASK) ) return;
			}else{
				v_menu_changed = 0;
			}
		    if(v_cur_menu->menu_show)	v_cur_menu->menu_show();
		    cmd_data_available &= ~V_MENU_SHOW_DATA_MASK;
		}else{
		    if(!v_setting_changed) return;
		    if(v_cur_menu->menu_show) v_cur_menu->menu_show();
		    v_setting_changed = 0;
		}	
	}
}
/*needed string*/
//static char *v_menu_lang_init;
static char *v_menu_lang_fuel;
static char *v_menu_lang_fuel_trip;
static char *v_menu_lang_instant_fuel_consum;
static char *v_menu_lang_trip;
static char *v_menu_lang_tot;
static char *v_menu_lang_flow;
static char *v_menu_lang_error;
static char *v_menu_lang_abnormal;
static char *v_menu_lang_invalid_flow_L = "-----.---";
static char *v_menu_lang_invalid_pulse = "--------";

static void v_init_language_hint_string(void){
	if(v_menu_setting.v_menu_language == V_MENU_LANGUAGE_EN){
		v_menu_lang_fuel = " Fuel";
		v_menu_lang_fuel_trip = " Trip";
		v_menu_lang_instant_fuel_consum = "   Fuel(L/H)";
		v_menu_lang_trip = "_Trip";
		v_menu_lang_tot = "_Tot ";
		v_menu_lang_flow = "_Flow";
		v_menu_lang_error = "_Error_…";
		v_menu_lang_abnormal = "Abnormal";
	}else if(v_menu_setting.v_menu_language == V_MENU_LANGUAGE_CN){
		v_menu_lang_fuel = " 油耗";
		v_menu_lang_fuel_trip = " 小计";
		v_menu_lang_instant_fuel_consum = "  瞬时油耗(L/H)";
		v_menu_lang_trip = " 小计";
		v_menu_lang_tot = " 总计";
		v_menu_lang_flow = " 流速";
		v_menu_lang_error = "_故障__…";
		v_menu_lang_abnormal = " 回油异常";
	}
}
static char* get_invalid_str(){
	if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){
		return v_menu_lang_invalid_pulse;
	}else{
		return v_menu_lang_invalid_flow_L;
	}
}
static char v_sprintf_buff[64];
/*static void v_menu_show_str(uint8_t line, char *str){
	Show_Str(60,150+20*line,str,16,0);	
}
*/
//not exceed show error time ,just return last value
static uint32_t v_menu_show_check_value_error(uint8_t line,uint32_t value){
	if(line == 0){
		if(value >= V_MENU_SHOW_STATUS_INVALID){//error
			if(v_menu_show_error_count_0 < v_menu_setting.v_sensor_error_delay_time){ 
				value = v_menu_show_last_value_0;
				if(v_menu_setting.v_sensor_error_delay_time != 0xFF){
				v_menu_show_error_count_0++;
				}  
			}
			
		}else{
			v_menu_show_last_value_0 = value;
			v_menu_show_error_count_0 = 0;
		}
	}else{
		if(value >= V_MENU_SHOW_STATUS_INVALID){//error
			if(v_menu_show_error_count_1 < v_menu_setting.v_sensor_error_delay_time){ 
				value = v_menu_show_last_value_1;
				if(v_menu_setting.v_sensor_error_delay_time != 0xFF){
				v_menu_show_error_count_1++;
				}  
			}		 
		}else{
			v_menu_show_last_value_1 = value;
			v_menu_show_error_count_1= 0;
		}
	}
	return value;
}
static void v_menu_show_wating(){
//	v_menu_show_str(0,"   waiting...");
//	v_menu_show_str(1,"");
       v_menu_changed = 1;
}
static void v_menu_show_time(uint8_t line, uint32_t time){
       v_menu_clear_inverse();
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
//		sprintf(v_sprintf_buff,"  %02dHH%02dMM%02dSS",
//		time>>16&0xFF,time>>8&0xFF,time&0xFF);
		sprintf(v_sprintf_buff,"  %1d%1dHH%1d%1dMM%1d%1dSS",
		(time>>(16+4))&0x0F, (time>>(16))&0x0F, 
		(time>>(8+4))&0x0F, (time>>8)&0x0F,
		(time>>(4))&0x0F, (time)&0x0F);		

	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		sprintf(v_sprintf_buff,"  %1d%1d时%1d%1d分%1d%1d秒",
		//time>>16&0xFF,time>>8&0xFF,time&0xFF);
		(time>>(16+4))&0x0F, (time>>(16))&0x0F, 
		(time>>(8+4))&0x0F, (time>>8)&0x0F,
		(time>>(4))&0x0F, (time)&0x0F);		
	}
	v_menu_show_str(line,v_sprintf_buff);
}

static void v_menu_show_value(uint8_t line, char *hint_pre, char *hint, uint32_t value){
       v_menu_clear_inverse();
	value = v_menu_show_check_value_error(line,value);
	if (value == V_MENU_SHOW_STATUS_ABNORMAL){
		sprintf(v_sprintf_buff,"%s%s:%s",hint_pre,hint,v_menu_lang_abnormal);
	}else if (value == V_MENU_SHOW_STATUS_ERROR) {
		sprintf(v_sprintf_buff,"%s%s:%s",hint_pre,hint,v_menu_lang_error);
	}else if (value == V_MENU_SHOW_STATUS_INVALID){
		sprintf(v_sprintf_buff,"%s%s:%s",hint_pre,hint,get_invalid_str());
	}else{
		if(v_menu_setting.v_menu_display_format ==  V_MENU_DISPLAY_FORMAT_FLOW_L){
			double d_value = value/1000.0;
			sprintf(v_sprintf_buff,"%s%s:%9.3f",hint_pre,hint,d_value);
			
		}else if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){
			sprintf(v_sprintf_buff,"%s%s:%8d",hint_pre,hint,value);
		}
	}
	v_menu_show_str(line,v_sprintf_buff);
}
static void v_menu_show_instant_fuel_consum(uint32_t value){
       v_menu_clear_inverse();
	value = v_menu_show_check_value_error(1,value);
	v_menu_show_str(0,v_menu_lang_instant_fuel_consum);
	if (value == V_MENU_SHOW_STATUS_ABNORMAL){
		sprintf(v_sprintf_buff,"   %s",v_menu_lang_abnormal);
	}else if (value == V_MENU_SHOW_STATUS_ERROR) {
		sprintf(v_sprintf_buff,"   %s",v_menu_lang_error);
	}else if (value == V_MENU_SHOW_STATUS_INVALID){
		sprintf(v_sprintf_buff,"   %s",get_invalid_str());
	}else{//will not show in pulse uint
		double d_value = value/1000.0;
		sprintf(v_sprintf_buff,"   %9.3f",d_value);
	}
	v_menu_show_str(1,v_sprintf_buff);	
}

//return status	 for individual sensor
//sensor_index  0:A 1:B 2:C 3:D
//is_trip            0:trip 1:tot
static float v_get_senosr_correct(uint8_t sensor_index){
       float correct_factor = 1.0;
       if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE) return correct_factor;

        switch(v_menu_setting.v_sensor_work_mode){
            case 0x00:	//A-B
            case 0x01:	//A
            case 0x02:	//A+B-C-D
            case 0x03:	//A+B-C
            case 0x04:	//A-B-C
            case 0x08:	//A+B+C-D
            case 0x09:	//A+B
            case 0x0A:	//A+B+C
            case 0x0B:	//A+B+C+D
                correct_factor = v_fuel_consum_1_corrected?1.0:(v_fuel_consum_1_correct/10000.0);
            break;
            case 0x05:      //A-B and C-D
            case 0x06:	//A-B and C
            case 0x07:	//A and C
            {
            	  switch(sensor_index){
            	        case 0://A
            	        case 1://B
            	            correct_factor = v_fuel_consum_1_corrected?1.0:(v_fuel_consum_1_correct/10000.0);
            	        break;
            	        case 2://C
            	        case 3://D
            	            correct_factor = v_fuel_consum_2_corrected?1.0:(v_fuel_consum_2_correct/10000.0);
            	        break;
            	  }
            }
            break;
        }
       return correct_factor;
}
static uint32_t v_get_sensor_data(uint8_t sensor_index,uint8_t type){
       int32_t time_interval;
       int32_t flow;
       float correct_factor = v_get_senosr_correct(sensor_index);
       switch(sensor_index){
            case 0://A
               if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL){
                     if(type == V_MENU_SHOW_TYPE_TOT){
		            return v_sensor_A_Tot*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_TRIP){
		            return v_sensor_A_Trip*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_FLOW){
		            if(v_sensor_A_latest_Tot== V_MENU_SHOW_STATUS_INVALID){
                		v_sensor_A_latest_Tot = v_sensor_A_Tot;
                		v_data_latest_real_time = v_data_real_time;
                		return 	V_MENU_SHOW_STATUS_INVALID;
                	}else{
                	       time_interval = (v_data_real_time-v_data_latest_real_time);
                	       time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
                		flow = (v_sensor_A_Tot - v_sensor_A_latest_Tot)*correct_factor/time_interval*3600;       		
                		v_sensor_A_latest_Tot = v_sensor_A_Tot;
                		v_data_latest_real_time = v_data_real_time;
                		return flow<0?0 : flow;	
                	}
		       }
               }
				if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
				    return V_MENU_SHOW_STATUS_INVALID;
				}
            break;
            case 1://B
                if(v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL){
                    if(type == V_MENU_SHOW_TYPE_TOT){
		            return v_sensor_B_Tot*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_TRIP){
		            return v_sensor_B_Trip*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_FLOW){
		            if(v_sensor_B_latest_Tot== V_MENU_SHOW_STATUS_INVALID){
                        		v_sensor_B_latest_Tot = v_sensor_B_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return 	V_MENU_SHOW_STATUS_INVALID;
                        	}else{
                        	       time_interval = (v_data_real_time-v_data_latest_real_time);
                                   time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
                        		flow = (v_sensor_B_Tot - v_sensor_B_latest_Tot)*correct_factor/time_interval*3600;
                        		v_sensor_B_latest_Tot = v_sensor_B_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return flow<0?0 : flow;	
                        	}
		       }
                }
                if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
                    return V_MENU_SHOW_STATUS_INVALID;
                }
            break;
            case 2://C
                if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
                    if(type == V_MENU_SHOW_TYPE_TOT){
		            return v_sensor_C_Tot*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_TRIP){
		            return v_sensor_C_Trip*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_FLOW){
		            if(v_sensor_C_latest_Tot== V_MENU_SHOW_STATUS_INVALID){
                        		v_sensor_C_latest_Tot = v_sensor_C_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return 	V_MENU_SHOW_STATUS_INVALID;
                        	}else{
                        	       time_interval = (v_data_real_time-v_data_latest_real_time);
                        	       time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
                        		flow = (v_sensor_C_Tot - v_sensor_C_latest_Tot)*correct_factor/time_interval*3600;
                        		v_sensor_C_latest_Tot = v_sensor_C_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return flow<0?0 : flow;	
                        	}
		       }
                }
                if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
                    return V_MENU_SHOW_STATUS_INVALID;
                }
            break;
            case 3://D
                if(v_sensor_D_status == V_MENU_SENSOR_STATUS_NORMAL){
                    if(type == V_MENU_SHOW_TYPE_TOT){
		            return v_sensor_D_Tot*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_TRIP){
		            return v_sensor_D_Trip*correct_factor;
		       }
		       if(type == V_MENU_SHOW_TYPE_FLOW){
		            if(v_sensor_D_latest_Tot== V_MENU_SHOW_STATUS_INVALID){
                        		v_sensor_D_latest_Tot = v_sensor_B_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return 	V_MENU_SHOW_STATUS_INVALID;
                        	}else{
                        	       time_interval = (v_data_real_time-v_data_latest_real_time);
                        	       time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
                        		flow = (v_sensor_D_Tot - v_sensor_D_latest_Tot)*correct_factor/time_interval*3600;
                        		v_sensor_D_latest_Tot = v_sensor_D_Tot;
                        		v_data_latest_real_time = v_data_real_time;
                        		return flow<0?0 : flow;	
                        	}
		       }
                }
                if(v_sensor_D_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
                    return V_MENU_SHOW_STATUS_INVALID;
                }
            break;
            default:
			return V_MENU_SHOW_STATUS_ERROR;
       }
       return V_MENU_SHOW_STATUS_ERROR;
}
//return status	 for fuel consum
//equip_index  1:equip 1 2:equip 2
//is_trip              0:trip      1:tot   2:travel
static float v_get_fuel_consum_correct(uint8_t equip_index){
       float correct_factor = 1.0;
       if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE) return correct_factor;

        if(equip_index == 1){
            correct_factor = v_fuel_consum_1_corrected?1.0:(v_fuel_consum_1_correct/10000.0);
        }else{
            correct_factor = v_fuel_consum_2_corrected?1.0:(v_fuel_consum_2_correct/10000.0);
        }
       return correct_factor;
}
static uint32_t v_get_fuel_consum_data(uint8_t equip_index,uint8_t type){
       int32_t fuel_consum;
       float correct_factor = v_get_fuel_consum_correct(equip_index);
       if(equip_index == 1){
        	switch(v_menu_setting.v_sensor_work_mode){
        		case 0x00:	//A-B
        		case 0x05:  //A-B and C-D
        		case 0x06:	//A-B and C
        		{ 
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL){
        			       if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot-v_sensor_B_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip-v_sensor_B_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if (v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        				return V_MENU_SHOW_STATUS_INVALID;
        			}
        		}
        		break;
        		case 0x01:	//A
        		case 0x07:	//A and C
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x02:	//A+B-C-D
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_D_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT)
        		 		{
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot
        			                        -v_sensor_C_Tot-v_sensor_D_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip
        			                        -v_sensor_C_Trip-v_sensor_D_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_D_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}
        		}
        		break;
        		case 0x03:	//A+B-C
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot
        			                        -v_sensor_C_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip
        			                        -v_sensor_C_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}
        		}
        		break;
        		case 0x04:	//A-B-C
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot-v_sensor_B_Tot
        			                        -v_sensor_C_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip-v_sensor_B_Trip
        			                        -v_sensor_C_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x08:	//A+B+C-D
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_D_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot
        			                        +v_sensor_C_Tot-v_sensor_D_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip
        			                        +v_sensor_C_Trip-v_sensor_D_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_D_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x09:	//A+B
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x0A:	//A+B+C
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot
        			                        +v_sensor_C_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip
        			                        +v_sensor_C_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x0B:	//A+B+C+D
        		{
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_B_status == V_MENU_SENSOR_STATUS_NORMAL
        			&& v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_D_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_A_Tot+v_sensor_B_Tot
        			                        +v_sensor_C_Tot+v_sensor_D_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_A_Trip+v_sensor_B_Trip
        			                        +v_sensor_C_Trip+v_sensor_D_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_A_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_B_status == V_MENU_SENSOR_STATUS_NOT_IN_USE
        			|| v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_D_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		default:
        			return V_MENU_SHOW_STATUS_INVALID;
        	}
	}else{
		switch(v_menu_setting.v_sensor_work_mode){
        		case 0x05:	//A-B and C-D
        		{
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL 
        			&& v_sensor_D_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum = v_sensor_C_Tot-v_sensor_D_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_C_Trip-v_sensor_D_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE 
        			|| v_sensor_D_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x06:	//A-B and C
        		{
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum =  v_sensor_C_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_C_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		case 0x07:	//A and C
        		{
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NORMAL){
        		 		if(type == V_MENU_SHOW_TYPE_TOT){
        			            fuel_consum =  v_sensor_C_Tot;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRIP){
        			            fuel_consum = v_sensor_C_Trip;
        			            return fuel_consum>=0 ? (fuel_consum*correct_factor) : V_MENU_SHOW_STATUS_ABNORMAL;
        			       }
        			       if(type == V_MENU_SHOW_TYPE_TRAVEL){
        			            return v_fuel_consum_1_travel_consum;
        			       }
        			}
        			if(v_sensor_C_status == V_MENU_SENSOR_STATUS_NOT_IN_USE){
        		 		return V_MENU_SHOW_STATUS_INVALID;
        			}	
        		}
        		break;
        		default:
        			return V_MENU_SHOW_STATUS_INVALID;
	        }
	}
	return 	V_MENU_SHOW_STATUS_ERROR;
}

/*display function for menu show*/
static uint32_t get_fuel_instant_consum_data(uint8_t equip_index){
       int32_t time_interval ;
       int32_t instant_consum;
       if(equip_index == 1){
        	if(v_fuel_consum_1_latest_travel_consum== V_MENU_SHOW_STATUS_INVALID){
        		v_fuel_consum_1_latest_travel_consum = v_fuel_consum_1_travel_consum;
        		v_data_latest_real_time= v_data_real_time;
        		return 	V_MENU_SHOW_STATUS_INVALID;
        	}else{
        	       time_interval = (v_data_real_time-v_data_latest_real_time);
        	       time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
        	       instant_consum = (v_fuel_consum_1_travel_consum - v_fuel_consum_1_latest_travel_consum)/time_interval*3600;
        		v_fuel_consum_1_latest_travel_consum = v_fuel_consum_1_travel_consum;
        		v_data_latest_real_time= v_data_real_time;
        		if(v_menu_setting.v_sensor_instant_fuel_consum_1_alarm 
        		    && instant_consum>v_menu_setting.v_sensor_instant_fuel_consum_1_alarm){
                            v_buzz_alarm_2s();
                      }
        		return instant_consum<0?0 : instant_consum;	
        	}
        }else{
              if(v_fuel_consum_2_latest_travel_consum== V_MENU_SHOW_STATUS_INVALID){
        		v_fuel_consum_2_latest_travel_consum = v_fuel_consum_2_travel_consum;
        		v_data_latest_real_time= v_data_real_time;
        		return 	V_MENU_SHOW_STATUS_INVALID;
        	}else{
        	       time_interval = (v_data_real_time-v_data_latest_real_time);
        	       time_interval += time_interval<0?V_MENU_MAX_REAL_TIME_S:0;
        		instant_consum = (v_fuel_consum_2_travel_consum - v_fuel_consum_2_latest_travel_consum)/time_interval*3600;
        		v_fuel_consum_2_latest_travel_consum = v_fuel_consum_2_travel_consum;
        		v_data_latest_real_time= v_data_real_time;
        		if(v_menu_setting.v_sensor_instant_fuel_consum_1_alarm 
        		    && instant_consum>v_menu_setting.v_sensor_instant_fuel_consum_2_alarm){
                            v_buzz_alarm_2s();
                      }
        		return instant_consum<0?0 : instant_consum;	
        	}
        }
}

static void v_menu_show_sensor_fuel_consum_1_Trip_Tot(void){	
	v_menu_show_value(0,"1",v_menu_lang_fuel_trip, v_get_fuel_consum_data(1,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"1",v_menu_lang_fuel,v_get_fuel_consum_data(1,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_fuel_consum_1_travel_consum(void){
	v_menu_show_time(0,v_fuel_consum_1_travel_time);
       v_menu_show_value(1,"1",v_menu_lang_fuel,v_get_fuel_consum_data(1,V_MENU_SHOW_TYPE_TRAVEL));
}
static void v_menu_show_sensor_fuel_consum_1_instant_consum(void){     
	v_menu_show_instant_fuel_consum(get_fuel_instant_consum_data(1));
}
static void v_menu_show_sensor_fuel_consum_2_Trip_Tot(void){
	v_menu_show_value(0,"2",v_menu_lang_fuel_trip,v_get_fuel_consum_data(2,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"2",v_menu_lang_fuel,v_get_fuel_consum_data(2,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_fuel_consum_2_travel_consum(void){
	v_menu_show_time(0,v_fuel_consum_2_travel_time);
    v_menu_show_value(1,"2",v_menu_lang_fuel,v_get_fuel_consum_data(2,V_MENU_SHOW_TYPE_TRAVEL));
}
static void v_menu_show_sensor_fuel_consum_2_instant_consum(void){
	v_menu_show_instant_fuel_consum(get_fuel_instant_consum_data(2));
}
static void v_menu_show_sensor_A_Trip_Tot(void){
	v_menu_show_value(0,"A",v_menu_lang_trip,v_get_sensor_data(0,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"A",v_menu_lang_tot,v_get_sensor_data(0,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_B_Trip_Tot(void){
	v_menu_show_value(0,"B",v_menu_lang_trip,v_get_sensor_data(1,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"B",v_menu_lang_tot,v_get_sensor_data(1,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_C_Trip_Tot(void){
	v_menu_show_value(0,"C",v_menu_lang_trip,v_get_sensor_data(2,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"C",v_menu_lang_tot,v_get_sensor_data(2,V_MENU_SHOW_TYPE_TOT));

}
static void v_menu_show_sensor_D_Trip_Tot(void){
	v_menu_show_value(0,"D",v_menu_lang_trip,v_get_sensor_data(3,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"D",v_menu_lang_tot,v_get_sensor_data(3,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_A_B_Trip(void){
	v_menu_show_value(0,"A",v_menu_lang_trip,v_get_sensor_data(0,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"B",v_menu_lang_trip,v_get_sensor_data(1,V_MENU_SHOW_TYPE_TRIP));
}
static void v_menu_show_sensor_A_B_Tot(void){
	v_menu_show_value(0,"A",v_menu_lang_tot,v_get_sensor_data(0,V_MENU_SHOW_TYPE_TOT));
	v_menu_show_value(1,"B",v_menu_lang_tot,v_get_sensor_data(1,V_MENU_SHOW_TYPE_TOT));
}
static void v_menu_show_sensor_C_D_Trip(void){
	v_menu_show_value(0,"C",v_menu_lang_trip,v_get_sensor_data(2,V_MENU_SHOW_TYPE_TRIP));
	v_menu_show_value(1,"D",v_menu_lang_trip,v_get_sensor_data(3,V_MENU_SHOW_TYPE_TRIP));
}
static void v_menu_show_sensor_C_D_Tot(void){
	v_menu_show_value(0,"C",v_menu_lang_tot,v_get_sensor_data(2,V_MENU_SHOW_TYPE_TOT));
	v_menu_show_value(1,"D",v_menu_lang_tot,v_get_sensor_data(3,V_MENU_SHOW_TYPE_TOT));
}

static void v_menu_show_sensor_A_B_flow(void){
	v_menu_show_value(0,"A",v_menu_lang_flow,v_get_sensor_data(0,V_MENU_SHOW_TYPE_FLOW));	
	v_menu_show_value(1,"B",v_menu_lang_flow,v_get_sensor_data(1,V_MENU_SHOW_TYPE_FLOW));
}
static void v_menu_show_sensor_C_D_flow(void){
	v_menu_show_value(0,"C",v_menu_lang_flow,v_get_sensor_data(2,V_MENU_SHOW_TYPE_FLOW));	
	v_menu_show_value(1,"D",v_menu_lang_flow,v_get_sensor_data(3,V_MENU_SHOW_TYPE_FLOW));
}

/*display function for menu setting*/
static void v_menu_setting_select_display_format(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Pick L/Pulse");
		v_menu_show_str(1,"     Open All");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"选择  公升/ 脉冲");
		v_menu_show_str(1,"      开启全显");
	}
	v_menu_clear_inverse();
	v_menu_show_inverse(0);
}
static void v_menu_setting_select_show_all(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"     L/Pulse");
		v_menu_show_str(1,"Pick Open All");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"      公升/ 脉冲");
		v_menu_show_str(1,"选择  开启全显");
	}
	 v_menu_clear_inverse();
	v_menu_show_inverse(1);
}
static void v_menu_setting_select_reboot(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"     Open All");
		v_menu_show_str(1,"Pick Reboot");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"      开启全显");
		v_menu_show_str(1,"选择  重新启动");
	}
	v_menu_clear_inverse();
	v_menu_show_inverse(1);
}
static void v_menu_setting_select_reboot_hint(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Sure to reboot?");
		v_menu_show_str(1,"                   yes");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"确定重启设备?");
		v_menu_show_str(1,"              是");
	}
	v_menu_clear_inverse();
	v_menu_show_inverse(1);
}
static void v_menu_setting_display_format_flow_L(void){
       if(v_menu_display_format_setting == V_MENU_DISPLAY_FORMAT_FLOW_L){
        	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
        		v_menu_show_str(0,"    DisplayPulse");
        		v_menu_show_str(1,"Set Display L");
        	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
        		v_menu_show_str(0,"      显示脉冲");
        		v_menu_show_str(1,"设定  显示公升");
        	}
        	v_menu_clear_inverse();
        	v_menu_show_inverse(1);
	}else{
	        if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
        		v_menu_show_str(0,"Set DisplayPulse");
        		v_menu_show_str(1,"    Display L");
        	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
        		v_menu_show_str(0,"设定  显示脉冲");
        		v_menu_show_str(1,"      显示公升");
        	}
        	v_menu_clear_inverse();
        	v_menu_show_inverse(0);
	}
}
/*
static void v_menu_setting_display_format_pulse(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Set DisplayPulse");
		v_menu_show_str(1,"    Display L");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"设定  显示脉冲");
		v_menu_show_str(1,"      显示公升");
	}
}
*/
static void v_menu_setting_show_all_set(void){
       if(v_menu_show_all_time_setting){
        	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
        		v_menu_show_str(0,"Set Open ALL");
        		v_menu_show_str(1,"    Close All");
        	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
        		v_menu_show_str(0,"设定  开启全显");
        		v_menu_show_str(1,"      关闭全显");
        	}
        	 v_menu_clear_inverse();
        	v_menu_show_inverse(0);
        }else{
              if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
        		v_menu_show_str(0,"    Open ALL");
        		v_menu_show_str(1,"Set Close All");
        	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
        		v_menu_show_str(0,"      开启全显");
        		v_menu_show_str(1,"设定  关闭全显");
        	}
        	v_menu_clear_inverse();
        	v_menu_show_inverse(1);
        }
}
/*
static void v_menu_setting_show_all_close(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"    Open ALL");
		v_menu_show_str(1,"Set Close All");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"      开启全显");
		v_menu_show_str(1,"设定  关闭全显");
	}
}
*/
static void v_menu_show_save_history_data(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Start Data copy?");
		v_menu_show_str(1,"                   yes");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"开始历史数据复制?");
		v_menu_show_str(1,"              是");
	}
	v_menu_clear_inverse();
	v_menu_show_inverse(1);
}
static void v_menu_show_saving_history_data(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0," Data copying...");
		v_menu_show_str(1,"");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"历史数据  复制中...");
		v_menu_show_str(1,"");
	}
	v_menu_clear_inverse();
}

static void v_menu_show_save_history_data_succeed(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0," Save succeed!");
		v_menu_show_str(1,"");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"历史数据复制成功!");
		v_menu_show_str(1,"");
	}
	v_menu_clear_inverse();
}

static void v_menu_show_save_history_data_failed(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"  Save failed!");
		v_menu_show_str(1,"");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"历史数据复制失败!");
		v_menu_show_str(1,"");
	}
	v_menu_clear_inverse();
}

static void v_menu_start_sensor_data_loop_flow_l(uint16_t arr){
#ifndef DEBUG
	stop_cmd_loop();
	start_cmd_loop(arr,SENSOR_DATA_LITER,NULL,NULL);
#endif
}
static void v_menu_start_sensor_data_loop_pulse(uint16_t arr){
#ifndef DEBUG
	stop_cmd_loop();
	start_cmd_loop(arr,SENSOR_DATA_PULSE,NULL,NULL);
#endif
}
static void v_menu_start_sensor_data_loop(uint16_t arr){
#ifndef DEBUG
	if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_FLOW_L){
		v_menu_start_sensor_data_loop_flow_l(arr);
	}else{
		v_menu_start_sensor_data_loop_pulse(arr);
	}
#endif
}

//will not show in pulse unit
static void v_menu_sart_equip_data_loop(uint16_t arr){
#ifndef DEBUG
	stop_cmd_loop();
	start_cmd_loop(arr,SENSOR_DATA_TOT_TRIP,NULL,NULL);
#endif
}
/*do function for menu show*/
static void v_do_function_sensor_fuel_consum_1_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_in_display = 1;
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_fuel_consum_1_travel_consum(void){
	v_menu_show_wating();
//	v_menu_sart_equip_data_loop(V_MENU_DATA_LOOP_TIME);
       v_menu_sart_equip_data_loop(10000);//1s
}
static void v_do_function_sensor_fuel_consum_1_instant_consum(void){
	v_menu_show_wating();
	v_menu_sart_equip_data_loop(v_menu_setting.v_sensor_fuel_consum_sample_time*10000);
}
static void v_do_function_sensor_fuel_consum_2_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_fuel_consum_2_travel_consum(void){
	v_menu_show_wating();
//	v_menu_sart_equip_data_loop(V_MENU_DATA_LOOP_TIME);
       v_menu_sart_equip_data_loop(10000);//1s
}
static void v_do_function_sensor_fuel_consum_2_instant_consum(void){
	v_menu_show_wating();
	v_menu_sart_equip_data_loop(v_menu_setting.v_sensor_fuel_consum_sample_time*10000);
}
static void v_do_function_sensor_A_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_B_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_C_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_D_Trip_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_A_B_Trip(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_A_B_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_C_D_Trip(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_C_D_Tot(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop(V_MENU_DATA_LOOP_TIME);
}
static void v_do_function_sensor_A_B_flow(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop_flow_l(v_menu_setting.v_sensor_flow_smaple_time*10000);
}
static void v_do_function_sensor_C_D_flow(void){
	v_menu_show_wating();
	v_menu_start_sensor_data_loop_flow_l(v_menu_setting.v_sensor_flow_smaple_time*10000);
}

/*do function for menu setting*/
static void v_do_function_setting_select_display_format(void){
	//v_menu_show_wating();
	v_setting_changed = 1;
	v_menu_in_display = 0;
}
static void v_do_function_setting_select_show_all(void){
	//v_menu_show_wating();
	v_setting_changed = 1;
}
static void v_do_function_setting_select_reboot(void){
	//v_menu_show_wating();
	v_setting_changed = 1;
}
static void v_do_function_setting_select_reboot_hint(void){
	//v_menu_show_wating();
	v_setting_changed = 1;
}
static void v_do_function_setting_select_reboot_enter(void){
	//v_menu_show_wating();
	v_setting_changed = 1;
	v_buzz_alarm_2s();
	delay_ms(500);
	NVIC_SystemReset();
}
static void v_do_function_setting_display_format_flow_L(void){
       v_setting_changed = 1;
       v_menu_display_format_setting= v_menu_setting.v_menu_display_format;
	//v_menu_show_wating();
}
/*
static void v_do_function_setting_display_format_pulse(void){
	//v_menu_show_wating();
}
*/
static void v_do_function_setting_show_all_set(void){
       v_setting_changed = 1;
       v_menu_show_all_time_setting = v_menu_setting.v_menu_show_all_time;
	//v_menu_show_wating();
}
/*
static void v_do_function_setting_show_all_close(void){
	//v_menu_show_wating();
}
*/
static void v_init_menu_display_struct_table(void);
#if 0
static void v_do_function_setting_display_format_flow_L_enter(void){
	//v_menu_show_wating();
	v_menu_display_format = V_MENU_DISPLAY_FORMAT_FLOW_L;
	v_init_menu_display_struct_table();
}
static void v_do_function_setting_display_format_pulse_enter(void){
	//v_menu_show_wating();
	v_menu_display_format = V_MENU_DISPLAY_FORMAT_PULSE;
	v_init_menu_display_struct_table();
}
static void v_do_function_setting_show_all_set_enter(void){
	//v_menu_show_wating();
	v_menu_show_all_time = V_MENU_SETTING_LOCAL_SHOW_ALL_TIME;	
	v_init_menu_display_struct_table();
	v_menu_show_all_start();
}
void v_do_function_setting_show_all_close_enter(void){
	//v_menu_show_wating();
	v_menu_show_all_time = 0;
	v_menu_timer_stop();
	v_init_menu_display_struct_table();
}
#endif
/*struct for menu display*/
static v_menu_struct_t v_struct_show_sensor_fuel_consum_1_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_fuel_consum_1_travel_consum;
static v_menu_struct_t v_struct_show_sensor_fuel_consum_1_instant_consum;
static v_menu_struct_t v_struct_show_sensor_fuel_consum_2_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_fuel_consum_2_travel_consum;
static v_menu_struct_t v_struct_show_sensor_fuel_consum_2_instant_consum;
static v_menu_struct_t v_struct_show_sensor_A_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_B_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_C_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_D_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_A_B_Trip;
static v_menu_struct_t v_struct_show_sensor_A_B_Tot;
static v_menu_struct_t v_struct_show_sensor_C_D_Trip;
static v_menu_struct_t v_struct_show_sensor_C_D_Tot;
static v_menu_struct_t v_struct_show_sensor_A_B_flow;
static v_menu_struct_t v_struct_show_sensor_C_D_flow;

/*struct for menu setting*/
static v_menu_struct_t v_struct_setting_select_display_format;
static v_menu_struct_t v_struct_setting_select_show_all;
static v_menu_struct_t v_struct_setting_select_reboot;
static v_menu_struct_t v_struct_setting_select_reboot_hint;
static v_menu_struct_t v_struct_setting_select_reboot_enter;
static v_menu_struct_t v_struct_setting_display_format_flow_L;
//static v_menu_struct_t v_struct_setting_display_format_flow_L_enter;
//static v_menu_struct_t v_struct_setting_display_format_pulse;
//static v_menu_struct_t v_struct_setting_display_format_pulse_enter;
static v_menu_struct_t v_struct_setting_show_all_set;
//static v_menu_struct_t v_struct_setting_show_all_set_enter;
//static v_menu_struct_t v_struct_setting_show_all_close;
//static v_menu_struct_t v_struct_setting_show_all_close_enter;

/*struct for menu copy to sdcard*/
static v_menu_struct_t v_struct_save_history_data;
static v_menu_struct_t v_struct_save_history_data_enter;
static v_menu_struct_t v_struct_save_history_data_succeed;
static v_menu_struct_t v_struct_save_history_data_failed;

static v_init_menu_struct(v_menu_struct_t *source_struct,
	v_menu_struct_t *enter_menu, v_menu_struct_t *esc_menu,v_menu_struct_t *up_menu,
	v_menu_struct_t *down_menu, void (*do_function) (void), void (*menu_show)(void))
{
	source_struct->enter_menu = enter_menu;
	source_struct->esc_menu = esc_menu;
	source_struct->up_menu = up_menu;
	source_struct->down_menu = down_menu;
	source_struct->do_function = do_function;
	source_struct->menu_show = menu_show;
}

static void v_init_menu_show_struct_3_flow_L_table(void){
    if(v_menu_setting.v_sensor_work_mode>0x04 && v_menu_setting.v_sensor_work_mode<0x08){
        v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_instant_consum,
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		v_do_function_sensor_fuel_consum_1_Trip_Tot,
		v_menu_show_sensor_fuel_consum_1_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_instant_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		v_do_function_sensor_fuel_consum_2_instant_consum,
		v_menu_show_sensor_fuel_consum_2_instant_consum
	);
    }else{
        v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,0,
		&v_struct_show_sensor_fuel_consum_1_instant_consum,
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		v_do_function_sensor_fuel_consum_1_Trip_Tot,
		v_menu_show_sensor_fuel_consum_1_Trip_Tot
	);
	v_init_menu_struct(
    		&v_struct_show_sensor_fuel_consum_1_instant_consum,
    		0,0,
    		&v_struct_show_sensor_fuel_consum_1_travel_consum,
    		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
    		v_do_function_sensor_fuel_consum_1_instant_consum,
    		v_menu_show_sensor_fuel_consum_1_instant_consum
    	);
    }
	
}

static void v_init_menu_show_struct_all_flow_L_table(void){
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_D_flow,
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		v_do_function_sensor_fuel_consum_1_Trip_Tot,
		v_menu_show_sensor_fuel_consum_1_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		&v_struct_show_sensor_fuel_consum_1_instant_consum,
		v_do_function_sensor_fuel_consum_1_travel_consum,
		v_menu_show_sensor_fuel_consum_1_travel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_instant_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		v_do_function_sensor_fuel_consum_1_instant_consum,
		v_menu_show_sensor_fuel_consum_1_instant_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		0,0,
		&v_struct_show_sensor_fuel_consum_1_instant_consum,
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
		v_do_function_sensor_fuel_consum_2_Trip_Tot,
		v_menu_show_sensor_fuel_consum_2_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		&v_struct_show_sensor_fuel_consum_2_instant_consum,
		v_do_function_sensor_fuel_consum_2_travel_consum,
		v_menu_show_sensor_fuel_consum_2_travel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_instant_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
		&v_struct_show_sensor_A_Trip_Tot,
		v_do_function_sensor_fuel_consum_2_instant_consum,
		v_menu_show_sensor_fuel_consum_2_instant_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_Trip_Tot,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_instant_consum,
		&v_struct_show_sensor_B_Trip_Tot,
		v_do_function_sensor_A_Trip_Tot,
		v_menu_show_sensor_A_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_B_Trip_Tot,
		0,0,
		&v_struct_show_sensor_A_Trip_Tot,
		&v_struct_show_sensor_C_Trip_Tot,
		v_do_function_sensor_B_Trip_Tot,
		v_menu_show_sensor_B_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_Trip_Tot,
		0,0,
		&v_struct_show_sensor_B_Trip_Tot,
		&v_struct_show_sensor_D_Trip_Tot,
		v_do_function_sensor_C_Trip_Tot,
		v_menu_show_sensor_C_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_D_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_Trip_Tot,
		&v_struct_show_sensor_A_B_Trip,
		v_do_function_sensor_D_Trip_Tot,
		v_menu_show_sensor_D_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Trip,
		0,0,
		&v_struct_show_sensor_D_Trip_Tot,
		&v_struct_show_sensor_A_B_Tot,
		v_do_function_sensor_A_B_Trip,
		v_menu_show_sensor_A_B_Trip
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Tot,
		0,0,
		&v_struct_show_sensor_A_B_Trip,
		&v_struct_show_sensor_C_D_Trip,
		v_do_function_sensor_A_B_Tot,
		v_menu_show_sensor_A_B_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Trip,
		0,0,
		&v_struct_show_sensor_A_B_Tot,
		&v_struct_show_sensor_C_D_Tot,
		v_do_function_sensor_C_D_Trip,
		v_menu_show_sensor_C_D_Trip
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Tot,
		0,0,
		&v_struct_show_sensor_C_D_Trip,
		&v_struct_show_sensor_A_B_flow,
		v_do_function_sensor_C_D_Tot,
		v_menu_show_sensor_C_D_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_flow,
		0,0,
		&v_struct_show_sensor_C_D_Tot,
		&v_struct_show_sensor_C_D_flow,
		v_do_function_sensor_A_B_flow,
		v_menu_show_sensor_A_B_flow
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_flow,
		0,0,
		&v_struct_show_sensor_A_B_flow,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		v_do_function_sensor_C_D_flow,
		v_menu_show_sensor_C_D_flow
	);
}
static void v_init_menu_show_struct_all_pulse_table(void){
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_D_flow,
#if 0
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
#else
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
#endif
		v_do_function_sensor_fuel_consum_1_Trip_Tot,
		v_menu_show_sensor_fuel_consum_1_Trip_Tot
	);
#if 0
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		v_do_function_sensor_fuel_consum_1_travel_consum,
		v_menu_show_sensor_fuel_consum_1_travel_consum
	);
#endif
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		0,0,
#if 0
		&v_struct_show_sensor_fuel_consum_1_travel_consum,
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
#else
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		&v_struct_show_sensor_A_Trip_Tot,	
#endif
		v_do_function_sensor_fuel_consum_2_Trip_Tot,
		v_menu_show_sensor_fuel_consum_2_Trip_Tot
	);
#if 0
	v_init_menu_struct(
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
		0,0,
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
		&v_struct_show_sensor_A_Trip_Tot,
		v_do_function_sensor_fuel_consum_2_travel_consum,
		v_menu_show_sensor_fuel_consum_2_travel_consum
	);
#endif
	v_init_menu_struct(
		&v_struct_show_sensor_A_Trip_Tot,
		0,0,
#if 0
		&v_struct_show_sensor_fuel_consum_2_travel_consum,
#else
		&v_struct_show_sensor_fuel_consum_2_Trip_Tot,
#endif
		&v_struct_show_sensor_B_Trip_Tot,
		v_do_function_sensor_A_Trip_Tot,
		v_menu_show_sensor_A_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_B_Trip_Tot,
		0,0,
		&v_struct_show_sensor_A_Trip_Tot,
		&v_struct_show_sensor_C_Trip_Tot,
		v_do_function_sensor_B_Trip_Tot,
		v_menu_show_sensor_B_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_Trip_Tot,
		0,0,
		&v_struct_show_sensor_B_Trip_Tot,
		&v_struct_show_sensor_D_Trip_Tot,
		v_do_function_sensor_C_Trip_Tot,
		v_menu_show_sensor_C_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_D_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_Trip_Tot,
		&v_struct_show_sensor_A_B_Trip,
		v_do_function_sensor_D_Trip_Tot,
		v_menu_show_sensor_D_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Trip,
		0,0,
		&v_struct_show_sensor_D_Trip_Tot,
		&v_struct_show_sensor_A_B_Tot,
		v_do_function_sensor_A_B_Trip,
		v_menu_show_sensor_A_B_Trip
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Tot,
		0,0,
		&v_struct_show_sensor_A_B_Trip,
		&v_struct_show_sensor_C_D_Trip,
		v_do_function_sensor_A_B_Tot,
		v_menu_show_sensor_A_B_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Trip,
		0,0,
		&v_struct_show_sensor_A_B_Tot,
		&v_struct_show_sensor_C_D_Tot,
		v_do_function_sensor_C_D_Trip,
		v_menu_show_sensor_C_D_Trip
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Tot,
		0,0,
		&v_struct_show_sensor_C_D_Trip,
		&v_struct_show_sensor_A_B_flow,
		v_do_function_sensor_C_D_Tot,
		v_menu_show_sensor_C_D_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_flow,
		0,0,
		&v_struct_show_sensor_C_D_Tot,
		&v_struct_show_sensor_C_D_flow,
		v_do_function_sensor_A_B_flow,
		v_menu_show_sensor_A_B_flow
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_flow,
		0,0,
		&v_struct_show_sensor_A_B_flow,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		v_do_function_sensor_C_D_flow,
		v_menu_show_sensor_C_D_flow
	);
}

static void v_init_menu_setting_struct_table(void){
	v_init_menu_struct(
		&v_struct_setting_select_display_format,
		&v_struct_setting_display_format_flow_L,//enter
		0,										//esc
		0,										//up
		&v_struct_setting_select_show_all,		//down
		v_do_function_setting_select_display_format,
		v_menu_setting_select_display_format
	);
	v_init_menu_struct(
		&v_struct_setting_select_show_all,
		&v_struct_setting_show_all_set,
		0,
		&v_struct_setting_select_display_format,
		&v_struct_setting_select_reboot,
		v_do_function_setting_select_show_all,
		v_menu_setting_select_show_all
	);
	v_init_menu_struct(
		&v_struct_setting_select_reboot,
		&v_struct_setting_select_reboot_hint,
		0,
		&v_struct_setting_select_show_all,
		0,
		v_do_function_setting_select_reboot,
		v_menu_setting_select_reboot
	);
	v_init_menu_struct(
		&v_struct_setting_select_reboot_hint,
		&v_struct_setting_select_reboot_enter,
		&v_struct_setting_select_reboot,
		0,
		0,
		v_do_function_setting_select_reboot_hint,
		v_menu_setting_select_reboot_hint
	);
	v_init_menu_struct(
		&v_struct_setting_select_reboot_enter,
		0,
		0,
		0,
		0,
		v_do_function_setting_select_reboot_enter,
		v_menu_setting_select_reboot_hint
	);
	v_init_menu_struct(
		&v_struct_setting_display_format_flow_L,
		0,//&v_struct_setting_display_format_flow_L_enter,
		&v_struct_setting_select_display_format,
		0,//&v_struct_setting_display_format_pulse,
		0,
		v_do_function_setting_display_format_flow_L,
		v_menu_setting_display_format_flow_L
	);
/*
	v_init_menu_struct(
		&v_struct_setting_display_format_flow_L_enter,
		0,
		&v_struct_setting_select_display_format,
		&v_struct_setting_display_format_pulse,
		0,
		v_do_function_setting_display_format_flow_L_enter,
		v_menu_setting_display_format_flow_L
	);
	v_init_menu_struct(
		&v_struct_setting_display_format_pulse,
		&v_struct_setting_display_format_pulse_enter,
		&v_struct_setting_select_display_format,
		0,
		&v_struct_setting_display_format_flow_L,
		v_do_function_setting_display_format_pulse,
		v_menu_setting_display_format_pulse
	);
	v_init_menu_struct(
		&v_struct_setting_display_format_pulse_enter,
		0,
		&v_struct_setting_select_display_format,
		0,
		&v_struct_setting_display_format_flow_L,
		v_do_function_setting_display_format_pulse_enter,
		v_menu_setting_display_format_pulse
	);
*/	v_init_menu_struct(
		&v_struct_setting_show_all_set,
		0,//&v_struct_setting_show_all_set_enter,
		&v_struct_setting_select_show_all,
		0,
		0,//&v_struct_setting_show_all_close,
		v_do_function_setting_show_all_set,
		v_menu_setting_show_all_set
	);
/*	v_init_menu_struct(
		&v_struct_setting_show_all_set_enter,
		0,
		&v_struct_setting_select_show_all,
		0,
		&v_struct_setting_show_all_close,
		v_do_function_setting_show_all_set_enter,
		v_menu_setting_show_all_set
	);
	v_init_menu_struct(
		&v_struct_setting_show_all_close,
		&v_struct_setting_show_all_close_enter,
		&v_struct_setting_select_show_all,
		&v_struct_setting_show_all_set,
		0,
		v_do_function_setting_show_all_close,
		v_menu_setting_show_all_close
	);
	v_init_menu_struct(
		&v_struct_setting_show_all_close_enter,
		0,
		&v_struct_setting_select_show_all,
		&v_struct_setting_show_all_set,
		0,
		v_do_function_setting_show_all_close_enter,
		v_menu_setting_show_all_close
	);
*/
}

static void v_menu_enter_setting(void){
	v_menu_in_display = 0;
	v_cur_menu = &v_struct_setting_select_display_format;
	stop_cmd_loop();
}
static void v_menu_enter_display(void){
	v_menu_in_display = 1;
	v_init_menu_display_struct_table();
	v_cur_menu = &v_struct_show_sensor_fuel_consum_1_Trip_Tot;
}
static void v_menu_enter_save_history_data(void){
	    v_cur_menu = &v_struct_save_history_data;
}
static void v_menu_enter_save_history_data_succeed(void){
	v_cur_menu = &v_struct_save_history_data_succeed;
	v_menu_function();
}
static void v_menu_enter_save_history_data_failed(void){
	v_cur_menu = &v_struct_save_history_data_failed;
	v_menu_function();
}

static void v_menu_enter_reset_all_trip(void){
	send_cmd(TRIP_RESET,NULL,0);
       v_menu_function();
}
static uint8_t v_menu_send_and_wait_messge(uint8_t cmd_index){
	int i=0;
	for(i=0;i<3;i++){
		delay_ms(50);
		send_cmd(cmd_index, NULL, 0);
		if(!wait_for_cmd(cmd_index)) return 0;
	}
	return 1;
}
static void v_menu_enter_reset_all(void){
	//send_cmd(ENABLE_DISABLE_RESET,NULL,0);
	//delay_ms(50);
	v_menu_send_and_wait_messge(GET_RESET_ENABLE);
	delay_ms(50);
	if(v_setting.v_menu_reset_tot_enable){
	    send_cmd(TOTAL_RESET,NULL,0);
	}
	v_menu_function();
}
//do function for sd card
static void v_do_function_save_history_data(void){
        v_setting_changed = 1;
        v_menu_in_display = 0;
}

static void v_do_function_saving_history_data(void){
        v_setting_changed = 1;
        v_menu_in_display = 0;
        if(query_store_history()){
            v_menu_enter_save_history_data_failed();
        }else{
            v_menu_enter_save_history_data_succeed();
        }
	//v_copy_to_sdcard(v_menu_enter_save_data_succeed,v_menu_enter_save_data_failed);
}

static void v_do_function_save_history_data_succeed(void){
        v_setting_changed = 1;
}

static void v_do_function_save_history_data_failed(void){
        v_setting_changed = 1;
}

static void v_init_menu_display_struct_table(void){
	if(!v_menu_setting.v_menu_show_all_time){
		v_menu_setting.v_menu_display_format = V_MENU_DISPLAY_FORMAT_FLOW_L;
		v_init_menu_show_struct_all_flow_L_table();
		v_init_menu_show_struct_3_flow_L_table();
	}else{
		if(v_menu_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_FLOW_L){
			v_init_menu_show_struct_all_flow_L_table();
		}else{
			v_init_menu_show_struct_all_pulse_table();
		}
	}
}
static void v_init_menu_struct_save_history_data_table(void){
    v_init_menu_struct(
		&v_struct_save_history_data,
		&v_struct_save_history_data_enter,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,
		0,
		v_do_function_save_history_data,
		v_menu_show_save_history_data
	);
	v_init_menu_struct(
		&v_struct_save_history_data_enter,
		0,0,
		0,
		0,
		v_do_function_saving_history_data,
		v_menu_show_saving_history_data
	);
	v_init_menu_struct(
		&v_struct_save_history_data_succeed,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,
		0,
		v_do_function_save_history_data_succeed,
		v_menu_show_save_history_data_succeed
	);
	v_init_menu_struct(
		&v_struct_save_history_data_failed,
		0,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,
		0,
		v_do_function_save_history_data_failed,
		v_menu_show_save_history_data_failed
	);
}
//show communicate error!
static void v_menu_show_connection_error(void){
       v_menu_show_str(0,"    连接错误!");
	v_menu_show_str(1,"Connection error!");
	v_menu_clear_inverse();
}
//show parameter error
static v_menu_struct_t v_struct_param_error;
static void v_menu_show_param_error(void){
	if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Parameters error!");
		v_menu_show_str(1,"");
	}else if(v_menu_setting.v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"    参数错误!");
		v_menu_show_str(1,"");
	}
	v_menu_clear_inverse();
}
static void v_do_function_param_error(void){
        v_setting_changed = 1;
}
static void v_init_menu_struct_param_error_table(void){
    v_init_menu_struct(
		&v_struct_param_error,
		0,
		&v_struct_show_sensor_fuel_consum_1_Trip_Tot,
		0,
		0,
		v_do_function_param_error,
		v_menu_show_param_error
	);
}

void v_menu_notify_display_format_changed(uint32_t display_format){
       uint8_t tmp = display_format;
	send_cmd(SET_DISPLAY_FORMAT,&tmp,1);
}
void v_menu_notify_show_all_time_changed(uint32_t show_all_time){
       uint8_t tmp = show_all_time;
	send_cmd(SET_DISPLAY_DELAY_TIME,&tmp,1);
}
void v_menu_show_all_time_action(void){
	if(v_menu_setting.v_menu_show_all_time == 0){ 
              v_menu_timer_stop();    	        
	}else{
		v_menu_show_all_start();
	}
}
void v_menu_sleep_wakeup_action(void){
       if(v_menu_sleep_wakeup_flag){
            v_lcd_backlight(1);
            v_menu_enter_display();
	     v_menu_function();
       }else{
             v_lcd_backlight(0);
       }
}
void v_menu_sleep_action(void){
	if(v_menu_setting.v_menu_sleep){
		v_lcd_backlight(0);
              v_menu_sleep_timer_start();
	}else{
	       v_lcd_backlight(1);
	       v_menu_sleep_timer_stop();
	}
}

static int v_menu_init_settings(void){
#ifdef DEBUG
	v_setting.v_menu_language = V_MENU_LANGUAGE_EN;
	v_setting.v_menu_display_format = V_MENU_DISPLAY_FORMAT_PULSE;//V_MENU_DISPLAY_FORMAT_FLOW_L;
	v_setting.v_menu_show_all_time = 0;
	v_setting.v_menu_sleep = 0;

	v_setting.v_sensor_work_mode = 0x05;
	v_setting.v_sensor_error_delay_time = 15;
	
	v_setting.v_sensor_instant_fuel_consum_1_alarm = 5;
	v_setting.v_sensor_instant_fuel_consum_2_alarm = 5;
	
	v_setting.v_sensor_fuel_consum_sample_time = 1;
	v_setting.v_sensor_flow_smaple_time = 3;

#else
       if(v_menu_send_and_wait_messge(MENU_LANGUAGE))   return -1;
	if(v_menu_send_and_wait_messge(DISPLAY_FORMAT))  return -2;
	if(v_menu_send_and_wait_messge(DISPLAY_DELAY_TIME)) return -3;
    if(v_menu_send_and_wait_messge(SENSOR_WOKR_MODE)) return -4;
    if(v_menu_send_and_wait_messge(ERR_DELAY_TIME))  return -5;
    if(v_menu_send_and_wait_messge(FC1_ALARM)) return -6;
    if(v_menu_send_and_wait_messge(FC2_ALARM)) return -7;
    if(v_menu_send_and_wait_messge(FC_SAMPLE_TIME)) return -8;
    if(v_menu_send_and_wait_messge(FLOW_SAMPLE_TIME)) return -9;
	if(v_menu_send_and_wait_messge(CAR_PLATE)) return -10;
//	if(v_menu_send_and_wait_messge(REQUEST_SLEEP)) return -11;
//  if(v_menu_send_and_wait_messge(FC1_CORRECTION)) return -11;
//  if(v_menu_send_and_wait_messge(FC2_CORRECTION)) return -12;
     v_setting.v_menu_sleep = 0;
#endif
	return 0;	
}
void v_menu_init(void){
//	int ret = 0;
//	v_menu_show_str(0,"    Init...");
	if((v_menu_init_settings()) != 0){
//		sprintf(v_sprintf_buff,"Init failed %d!",ret);
//		v_menu_show_str(0,v_sprintf_buff);
//		v_menu_show_str(1,"");
        v_menu_show_connection_error();
		while(1);
	}
	
//	v_menu_show_str(0," Init succeed!");	
       v_menu_setting = v_setting;
	v_init_language_hint_string();
//	v_cur_menu = &v_struct_setting_select_display_format;
       if(v_setting.v_menu_show_all_time == 0 && v_setting.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){ 
	    v_menu_show_param_error();
	    delay_ms(1000);
	    delay_ms(1000);
    	}
	v_init_menu_display_struct_table();
	v_init_menu_setting_struct_table();
	v_init_menu_struct_save_history_data_table();
	v_init_menu_struct_param_error_table();
	v_menu_sleep_action();
	v_menu_enter_display();
	v_menu_function();
	v_menu_show_all_time_action();
}

void v_menu_enter_short(void){
       if(!v_menu_in_display){
             v_setting_changed = 1;
             if(&v_struct_setting_display_format_flow_L == v_cur_menu){
                v_setting.v_menu_display_format = v_menu_display_format_setting;
		  		v_menu_notify_display_format_changed(v_menu_display_format_setting);
                return;
             }else if (&v_struct_setting_show_all_set == v_cur_menu){
                v_setting.v_menu_show_all_time  = v_menu_show_all_time_setting;
		 		v_menu_notify_show_all_time_changed(v_menu_show_all_time_setting);
		 //		v_menu_show_all_time_action();
                return;
             }
       }
	if(v_cur_menu && v_cur_menu->enter_menu){
		v_cur_menu = v_cur_menu->enter_menu;
		v_menu_function();
	}
}
void v_menu_enter_3s(void){
        if(v_menu_in_display){
	    v_menu_enter_setting();
	    v_menu_function();
	    v_buzz_key();
	 }
}
void v_menu_enter_6s(void){
       if(detect_sdcard()){
    	    v_menu_enter_save_history_data();
    	    v_menu_function();
    	    v_buzz_key();
    	 }
}
void v_menu_enter_15s(void){
	if(!v_menu_in_display){
		v_menu_enter_display();
		v_menu_function();
		v_buzz_key();
	}
}
void v_menu_esc_short(void){
	if(!v_menu_in_display){
		if(v_cur_menu &&v_cur_menu->esc_menu) {
			v_cur_menu = v_cur_menu->esc_menu;
			v_menu_function();
		}else{
			v_menu_enter_display();
			v_menu_function();
		}
	}
}
void v_menu_esc_3s(void){
	v_menu_enter_reset_all_trip();
}
void v_menu_esc_9s(void){
	v_menu_enter_reset_all();
}
void v_menu_up_short(void){
#if 0
	if(!v_menu_in_display && v_menu_show_all_time == 0){ //in setting, not in show all, can't select pulse unit
		if(v_cur_menu ==  &v_struct_setting_display_format_flow_L ||
			v_cur_menu ==  &v_struct_setting_display_format_flow_L_enter){
			return;
		} 
	}
#else
       if(!v_menu_in_display){
             v_setting_changed = 1;
             if(&v_struct_setting_display_format_flow_L == v_cur_menu){
                if(v_menu_setting.v_menu_show_all_time ==0){
                    v_menu_display_format_setting = V_MENU_DISPLAY_FORMAT_FLOW_L;
                    return;
                }
                v_menu_display_format_setting = V_MENU_DISPLAY_FORMAT_PULSE;
                return;
             }else if (&v_struct_setting_show_all_set == v_cur_menu){
                v_menu_show_all_time_setting = V_MENU_SETTING_LOCAL_SHOW_ALL_TIME;
                return;
             }
       }
#endif
	if(v_cur_menu && v_cur_menu->up_menu) {
		v_cur_menu = v_cur_menu->up_menu;
		v_menu_function();
	}
}
void v_menu_down_short(void){
       if(!v_menu_in_display){
             v_setting_changed = 1;
             if(&v_struct_setting_display_format_flow_L == v_cur_menu){
                v_menu_display_format_setting = V_MENU_DISPLAY_FORMAT_FLOW_L;
                return;
             }else if (&v_struct_setting_show_all_set == v_cur_menu){
                v_menu_show_all_time_setting = 0;
                return;
             }
       }
	if(v_cur_menu && v_cur_menu->down_menu) {
		v_cur_menu = v_cur_menu->down_menu;
		v_menu_function();
	}
}

void v_check_setting(void){
	int need_restart = 0;
	v_setting_struct_t v_setting_tmp = v_setting;
	if(v_setting_tmp.v_menu_sleep != v_menu_setting.v_menu_sleep){
	       v_menu_setting.v_menu_sleep = v_setting_tmp.v_menu_sleep;
 		v_menu_sleep_action();
 		if(v_setting_tmp.v_menu_sleep == 0 && v_menu_setting.v_menu_show_all_time!=0){
 		    v_menu_show_all_resume();
 		}
 		
		need_restart = 1;
	}
	if(v_setting_tmp.v_menu_language!=v_menu_setting.v_menu_language){
	       v_menu_setting.v_menu_language = v_setting_tmp.v_menu_language;
	       v_init_language_hint_string();
		v_setting_changed = 1;
	}
	if(v_setting_tmp.v_menu_show_all_time != v_menu_setting.v_menu_show_all_time){
	       v_menu_setting.v_menu_show_all_time = v_setting_tmp.v_menu_show_all_time;
		v_menu_show_all_time_setting = v_setting_tmp.v_menu_show_all_time;
		v_menu_show_all_time_action();
		v_setting_changed = 1;
		need_restart = 1;
	}
	if(v_setting_tmp.v_menu_display_format != v_menu_setting.v_menu_display_format){
	       v_menu_setting.v_menu_display_format = v_setting_tmp.v_menu_display_format;
		v_menu_display_format_setting = v_setting_tmp.v_menu_display_format;
		v_setting_changed = 1;
		need_restart = 1;
	}
	if(v_setting_tmp.v_sensor_work_mode != v_menu_setting.v_sensor_work_mode){
	    v_menu_setting.v_sensor_work_mode = v_setting_tmp.v_sensor_work_mode;
	    need_restart = 1;
	}
	if(v_setting_tmp.v_sensor_error_delay_time != v_menu_setting.v_sensor_error_delay_time){
	    v_menu_setting.v_sensor_error_delay_time = v_setting_tmp.v_sensor_error_delay_time;
	    need_restart = 1;
	}
	if(v_setting_tmp.v_sensor_instant_fuel_consum_1_alarm !=v_menu_setting.v_sensor_instant_fuel_consum_1_alarm){
	    v_menu_setting.v_sensor_instant_fuel_consum_1_alarm = v_setting_tmp.v_sensor_instant_fuel_consum_1_alarm;
	}
	if(v_setting_tmp.v_sensor_instant_fuel_consum_2_alarm != v_menu_setting.v_sensor_instant_fuel_consum_2_alarm){
	    v_menu_setting.v_sensor_instant_fuel_consum_1_alarm = v_setting_tmp.v_sensor_instant_fuel_consum_1_alarm;
	}
	if(v_setting_tmp.v_sensor_fuel_consum_sample_time !=v_menu_setting.v_sensor_fuel_consum_sample_time){
	    v_menu_setting.v_sensor_fuel_consum_sample_time = v_setting_tmp.v_sensor_fuel_consum_sample_time;
	    need_restart = 1;
	}
	if(v_setting_tmp.v_sensor_flow_smaple_time != v_menu_setting.v_sensor_flow_smaple_time){
	    v_menu_setting.v_sensor_flow_smaple_time = v_setting_tmp.v_sensor_flow_smaple_time;
	    need_restart = 1;
	}
	if(v_setting_tmp.v_menu_show_all_time == 0 && v_setting_tmp.v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){ 
    	    v_menu_setting.v_menu_display_format = v_setting.v_menu_display_format = v_setting_tmp.v_menu_display_format 
    	    = v_menu_display_format_setting = V_MENU_DISPLAY_FORMAT_FLOW_L;
    	    //v_menu_notify_display_format_changed();  
//    	    v_cur_menu = &v_struct_param_error;
//	    v_menu_function();
	    v_setting_changed = 1;
//    	    return;
           need_restart = 1;
    	}
	if(v_menu_in_display){
	   if(need_restart){
		v_menu_enter_display();
	       v_menu_function();
	   }
	}
}

