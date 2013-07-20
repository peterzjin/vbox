/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 22:17
//*****************************************************************************/
#ifndef __V_MENU_H__
#define __V_MENU_H__
#include "stm32f10x.h"

#define V_MENU_DATA_LOOP_TIME	20000          //1s
#define V_MENU_DISPLAY_FORMAT_FLOW_L      	0x00
#define V_MENU_DISPLAY_FORMAT_PULSE		0x01
typedef struct v_setting_struct
{ 
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
}v_setting_struct_t;
extern v_setting_struct_t v_setting;
#if 0
//0: disable 1: enable
extern uint32_t v_menu_sleep;                                     //0x0f
//extern uint32_t v_menu_deep_sleep;                          //?
extern uint32_t v_menu_reset_tot_enable;			//0x01

extern uint32_t v_menu_language;					//0x0E
extern uint32_t v_menu_display_format;				//0x11
extern uint32_t v_menu_show_all_time;				//0x20
//like datasheet
extern uint32_t v_sensor_work_mode;
extern uint32_t v_sensor_error_delay_time;			//0x18
//0: no alarm 1:alarm
extern uint32_t v_sensor_instant_fuel_consum_1_alarm; //0x1A
extern uint32_t v_sensor_instant_fuel_consum_2_alarm; //0x1B

extern uint32_t v_sensor_fuel_consum_sample_time;	//0x19
extern uint32_t v_sensor_flow_smaple_time;			//0x09
#endif

extern uint32_t v_fuel_consum_1_correct;			 	//0x07|0x08	 noneed
extern uint32_t v_fuel_consum_2_correct;
//0:need correct  1: have corrected 
extern uint32_t v_fuel_consum_1_corrected;
extern uint32_t v_fuel_consum_2_corrected;


//0x3C|0x3F|0x3D
extern uint32_t v_data_real_time;
//0:not in use 1:normal 2:error
extern uint32_t v_sensor_A_status;
extern uint32_t v_sensor_B_status;
extern uint32_t v_sensor_C_status;
extern uint32_t v_sensor_D_status;

//0x3C|0x3F
extern uint32_t v_sensor_A_Trip;
extern uint32_t v_sensor_B_Trip;
extern uint32_t v_sensor_C_Trip;
extern uint32_t v_sensor_D_Trip;
extern uint32_t v_sensor_A_Tot;
extern uint32_t v_sensor_B_Tot;
extern uint32_t v_sensor_C_Tot;
extern uint32_t v_sensor_D_Tot;

//0x3D
extern uint32_t v_fuel_consum_1_travel_time;
extern uint32_t v_fuel_consum_1_travel_consum;
extern uint32_t v_fuel_consum_1_trip;
extern uint32_t v_fuel_consum_1_tot;
extern uint32_t v_fuel_consum_2_travel_time;
extern uint32_t v_fuel_consum_2_travel_consum;
extern uint32_t v_fuel_consum_2_trip;
extern uint32_t v_fuel_consum_2_tot;

void v_menu_init(void);
void v_menu_enter_short(void);
void v_menu_enter_3s(void);
void v_menu_enter_6s(void);
void v_menu_enter_15s(void);
void v_menu_esc_short(void);
void v_menu_esc_3s(void);
void v_menu_esc_9s(void);
void v_menu_up_short(void);
void v_menu_down_short(void);

void v_menu_show(void);

void v_menu_notify_display_format_changed(uint32_t display_format);
void v_menu_notify_show_all_time_changed(uint32_t show_all_time);
void v_menu_show_all_time_action(void);
void v_menu_sleep_wakeup_action(void);

void v_check_setting(void);

extern uint32_t v_menu_display_format_setting;
extern uint32_t v_menu_show_all_time_setting;
extern uint8_t v_setting_changed;
#endif

