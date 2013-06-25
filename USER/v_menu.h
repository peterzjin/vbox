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

extern uint32_t v_fuel_consum_1_correct;			 	//0x07|0x08	 noneed
extern uint32_t v_fuel_consum_2_correct;

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
void v_menu_show_all_timer_stop(void);
extern uint8_t v_menu_show_all_flag; //0:donothing, 1:changed by timer 
#endif

