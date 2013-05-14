/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 22:17
//*****************************************************************************/
#ifndef __V_MENU_H__
#define __V_MENU_H__
#include "stm32f10x.h"

typedef struct v_menu_struct
{ 
	struct v_menu_struct *enter_menu;
	struct v_menu_struct *esc_menu;
	struct v_menu_struct *up_menu;
	struct v_menu_struct *down_menu;
	void (*do_function) (void);
	void (*menu_show)(void);
}v_menu_struct_t;

extern v_menu_struct_t *v_cur_menu;

extern uint32_t v_menu_language;					//0x0E
extern uint32_t v_menu_display_format;				//0x11
extern uint32_t v_menu_show_all_time;				//0x20
extern uint32_t v_sensor_fuel_consum_sample_time;	//0x19
extern uint32_t v_sensor_flow_smaple_time;			//0x09
extern uint32_t v_sensor_A_B_correct;			 	//0x07
extern uint32_t v_sensor_C_D_correct;			 	//0x08
extern uint32_t v_sensor_A_Trip;					//0x3B | 0x3c 
extern uint32_t v_sensor_B_Trip;
extern uint32_t v_sensor_C_Trip;
extern uint32_t v_sensor_D_Trip;
extern uint32_t v_sensor_A_Tot;
extern uint32_t v_sensor_B_Tot;
extern uint32_t v_sensor_C_Tot;
extern uint32_t v_sensor_D_Tot;
extern uint32_t v_sensor_A_B_travel_fuel_consum_time;//0x3D	
extern uint32_t v_sensor_A_B_travel_fuel_consum;
extern uint32_t v_sensor_C_D_travel_fuel_consum_time;
extern uint32_t v_sensor_C_D_travel_fuel_consum;

void v_menu_init(void);
#endif
