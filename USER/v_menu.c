/******************************************************************************/
//  GeekWorld
//  Author: fys3@163.com 
//  Version: 0.0
//  Date: 2013/05/11 22:17
//*****************************************************************************/
#include "v_menu.h"
#include "text.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"

#define V_MENU_LANGUAGE_EN		0x00
#define V_MENU_LANGUAGE_CN		0x01

#define V_MENU_DISPLAY_FORMAT_FLOW_L      	0x00
#define V_MENU_DISPLAY_FORMAT_PULSE		0x01

//char v_menu_show_str_en[]
v_menu_struct_t *v_cur_menu;

uint32_t v_menu_language;
uint32_t v_menu_display_format;
uint32_t v_menu_show_all_time;
uint32_t v_sensor_fuel_consum_sample_time;
uint32_t v_sensor_flow_smaple_time;
uint32_t v_sensor_A_B_correct;
uint32_t v_sensor_C_D_correct;
uint32_t v_sensor_A_Trip;
uint32_t v_sensor_B_Trip;
uint32_t v_sensor_C_Trip;
uint32_t v_sensor_D_Trip;
uint32_t v_sensor_A_Tot;
uint32_t v_sensor_B_Tot;
uint32_t v_sensor_C_Tot;
uint32_t v_sensor_D_Tot;
uint32_t v_sensor_A_B_travel_fuel_consum_time;
uint32_t v_sensor_A_B_travel_fuel_consum;
uint32_t v_sensor_C_D_travel_fuel_consum_time;
uint32_t v_sensor_C_D_travel_fuel_consum;

/*needed string*/
static char *v_menu_lang_fuel;
static char *v_menu_lang_trip;
static char *v_menu_lang_instant_fuel_consump;
static char *v_menu_lang_tot;
static char *v_menu_lang_flow;

static void v_init_language_hint_string(void){
	if(v_menu_language == V_MENU_LANGUAGE_EN){
		v_menu_lang_fuel = "Fuel";
		v_menu_lang_trip = "Trip";
		v_menu_lang_instant_fuel_consump = "Fuel(L/H)";
		v_menu_lang_tot = "Tot";
		v_menu_lang_flow = "Flow";
	}else if(v_menu_language == V_MENU_LANGUAGE_CN){
		v_menu_lang_fuel = "油耗";
		v_menu_lang_trip = "小计";
		v_menu_lang_instant_fuel_consump = "瞬时油耗(L/H)";
		v_menu_lang_tot = "总计";
		v_menu_lang_flow = "流速";
	}
}

static char v_sprintf_buff[64];
static void v_menu_show_str(uint8_t line, char *str){
	Show_Str(60,50+20*line,str,strlen(str),0);	
}
static void v_menu_show_time(uint8_t line, uint32_t time){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
//		sprintf(v_sprintf_buff,"%2dHH%2dMM%2dSS",(time/3600)%24,(time/60)%60,time%60);
		sprintf(v_sprintf_buff,"00HH00MM00SS");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		sprintf(v_sprintf_buff,"%2d时%2d分%2d秒",(time/3600)%24,(time/60)%60,time%60);
	}
	v_menu_show_str(line,v_sprintf_buff);
}

static void v_menu_show_value(uint8_t line, char *hint_pre, char *hint, uint32_t value){
	if(v_menu_display_format ==  V_MENU_DISPLAY_FORMAT_FLOW_L){
		double d_value = value/1000.0;
		sprintf(v_sprintf_buff,"%s%s:%5.3f",hint_pre,hint,d_value);
		
	}else if(v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){
		sprintf(v_sprintf_buff,"%s%s:%8d",hint_pre,hint,value);
	}
	v_menu_show_str(line,v_sprintf_buff);
}
static void v_menu_show_instant_fuel_consum(uint32_t value){
	v_menu_show_str(0,v_menu_lang_instant_fuel_consump);
	if(v_menu_display_format ==  V_MENU_DISPLAY_FORMAT_FLOW_L){
		double d_value = value/1000.0;
		sprintf(v_sprintf_buff,"   %5.3f",d_value);
		
	}else if(v_menu_display_format == V_MENU_DISPLAY_FORMAT_PULSE){
		sprintf(v_sprintf_buff,"   %8d",value);
	}
	v_menu_show_str(1,v_sprintf_buff);	
}

#define calc_sensor_A_B_data(org_data) ((org_data)*v_sensor_A_B_correct)
#define calc_sensor_C_D_data(org_data) ((org_data)*v_sensor_C_D_correct)
/*display function for menu show*/
static void v_menu_show_sensor_A_B_Trip_Tot(void){
	uint32_t v_sensor_A_B_Trip = calc_sensor_A_B_data(v_sensor_A_Trip-v_sensor_B_Trip);
	uint32_t v_sensor_A_B_Tot = calc_sensor_A_B_data(v_sensor_A_Tot-v_sensor_B_Tot);	
	v_menu_show_value(0,"1 ",v_menu_lang_trip,v_sensor_A_B_Trip);
	v_menu_show_value(1,"1 ",v_menu_lang_fuel,v_sensor_A_B_Tot);
}
static void v_menu_show_sensor_A_B_travel_fuel_consum(void){
	v_menu_show_time(0,v_sensor_A_B_travel_fuel_consum_time);
    v_menu_show_value(1,"1 ",v_menu_lang_fuel,v_sensor_A_B_travel_fuel_consum);
}
static uint32_t v_sensor_latest_A_Tot = 0;
static uint32_t v_sensor_latest_B_Tot = 0;
static void v_menu_show_sensor_A_B_instant_fuel_consum(void){
	uint32_t v_sensor_cur_A_B_Tot = 
		calc_sensor_A_B_data(v_sensor_A_Tot - v_sensor_B_Tot);
	uint32_t v_sensor_latest_A_B_Tot = 
		calc_sensor_A_B_data(v_sensor_latest_A_Tot - v_sensor_latest_B_Tot);
	uint32_t v_sensor_A_B_instant_fuel_consum =
		(v_sensor_cur_A_B_Tot - v_sensor_latest_A_B_Tot)/v_sensor_fuel_consum_sample_time*0.36;
	v_sensor_latest_A_Tot = v_sensor_A_Tot;
	v_sensor_latest_B_Tot = v_sensor_B_Tot;
	v_menu_show_instant_fuel_consum(v_sensor_A_B_instant_fuel_consum);
}
static void v_menu_show_sensor_C_D_Trip_Tot(void){
	uint32_t v_sensor_C_D_Trip = calc_sensor_C_D_data(v_sensor_C_Trip-v_sensor_D_Trip);
	uint32_t v_sensor_C_D_Tot = calc_sensor_C_D_data(v_sensor_C_Tot-v_sensor_D_Tot); 
	v_menu_show_value(0,"2 ",v_menu_lang_trip,v_sensor_C_D_Trip);
	v_menu_show_value(1,"2 ",v_menu_lang_fuel,v_sensor_C_D_Tot);
}
static void v_menu_show_sensor_C_D_travel_fuel_consum(void){
	v_menu_show_time(0,v_sensor_C_D_travel_fuel_consum_time);
    v_menu_show_value(1,"2 ",v_menu_lang_fuel,v_sensor_C_D_travel_fuel_consum);
}
static uint32_t v_sensor_latest_C_Tot = 0;
static uint32_t v_sensor_latest_D_Tot = 0;
static void v_menu_show_sensor_C_D_instant_fuel_consum(void){
	uint32_t v_sensor_cur_C_D_Tot = 
		calc_sensor_C_D_data(v_sensor_C_Tot - v_sensor_D_Tot);
	uint32_t v_sensor_latest_C_D_Tot = 
		calc_sensor_C_D_data(v_sensor_latest_C_Tot - v_sensor_latest_D_Tot);
	uint32_t v_sensor_C_D_instant_fuel_consum =
		(v_sensor_cur_C_D_Tot - v_sensor_latest_C_D_Tot)/v_sensor_fuel_consum_sample_time*0.36;
	v_sensor_latest_C_Tot = v_sensor_C_Tot;
	v_sensor_latest_D_Tot = v_sensor_D_Tot;
	v_menu_show_instant_fuel_consum(v_sensor_C_D_instant_fuel_consum);
}
static void v_menu_show_sensor_A_Trip_Tot(void){
	v_menu_show_value(0,"A_",v_menu_lang_trip,v_sensor_A_Trip);
	v_menu_show_value(1,"A_",v_menu_lang_tot,v_sensor_A_Tot);
}
static void v_menu_show_sensor_B_Trip_Tot(void){
	v_menu_show_value(0,"B_",v_menu_lang_trip,v_sensor_B_Trip);
	v_menu_show_value(1,"B_",v_menu_lang_tot,v_sensor_B_Tot);
}
static void v_menu_show_sensor_C_Trip_Tot(void){
	v_menu_show_value(0,"C_",v_menu_lang_trip,v_sensor_C_Trip);
	v_menu_show_value(1,"C_",v_menu_lang_tot,v_sensor_C_Tot);
}
static void v_menu_show_sensor_D_Trip_Tot(void){
	v_menu_show_value(0,"D_",v_menu_lang_trip,v_sensor_D_Trip);
	v_menu_show_value(1,"D_",v_menu_lang_tot,v_sensor_D_Tot);
}
static void v_menu_show_sensor_A_B_Trip(void){
	v_menu_show_value(0,"A_",v_menu_lang_trip,v_sensor_A_Trip);
	v_menu_show_value(1,"B_",v_menu_lang_trip,v_sensor_B_Trip);
}
static void v_menu_show_sensor_A_B_Tot(void){
	v_menu_show_value(0,"A_",v_menu_lang_tot,v_sensor_A_Tot);
	v_menu_show_value(1,"B_",v_menu_lang_tot,v_sensor_B_Tot);
}
static void v_menu_show_sensor_C_D_Trip(void){
	v_menu_show_value(0,"C_",v_menu_lang_trip,v_sensor_C_Trip);
	v_menu_show_value(1,"D_",v_menu_lang_trip,v_sensor_D_Trip);
}
static void v_menu_show_sensor_C_D_Tot(void){
	v_menu_show_value(0,"C_",v_menu_lang_tot,v_sensor_C_Tot);
	v_menu_show_value(1,"D_",v_menu_lang_tot,v_sensor_D_Tot);
}
static void v_menu_show_sensor_A_B_flow(void){
	uint32_t v_sensor_A_flow =
		(v_sensor_A_Tot - v_sensor_latest_A_Tot)/v_sensor_flow_smaple_time*0.36;
	uint32_t v_sensor_B_flow = 
		(v_sensor_B_Tot - v_sensor_latest_B_Tot)/v_sensor_flow_smaple_time*0.36;
	v_sensor_latest_A_Tot = v_sensor_A_Tot;
	v_sensor_latest_B_Tot = v_sensor_B_Tot;
	v_menu_show_value(0,"A_",v_menu_lang_flow,v_sensor_A_flow);
	v_menu_show_value(1,"B_",v_menu_lang_flow,v_sensor_B_flow);
}
static void v_menu_show_sensor_C_D_flow(void){
	uint32_t v_sensor_C_flow =
		(v_sensor_C_Tot - v_sensor_latest_C_Tot)/v_sensor_flow_smaple_time*0.36;
	uint32_t v_sensor_D_flow = 
		(v_sensor_D_Tot - v_sensor_latest_D_Tot)/v_sensor_flow_smaple_time*0.36;
	v_sensor_latest_C_Tot = v_sensor_C_Tot;
	v_sensor_latest_D_Tot = v_sensor_D_Tot;
	v_menu_show_value(0,"C_",v_menu_lang_flow,v_sensor_C_flow);
	v_menu_show_value(1,"D_",v_menu_lang_flow,v_sensor_D_flow);
}

/*display function for menu setting*/
static void v_menu_setting_select_display_format(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Pick L/Pulse");
		v_menu_show_str(1,"     Open All");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"选择 公升/脉冲");
		v_menu_show_str(1,"     开启全显");
	}
}
static void v_menu_setting_select_show_all(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"      L/Pulse");
		v_menu_show_str(1,"Pick Open All");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"     公升/脉冲");
		v_menu_show_str(1,"选择 开启全显");
	}
}
static void v_menu_setting_display_format_flow_L(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"   DisplayPulse");
		v_menu_show_str(1,"Set Display L");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"     显示脉冲");
		v_menu_show_str(1,"设定 显示公升");
	}
}
static void v_menu_setting_display_format_pulse(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"SetDisplayPulse");
		v_menu_show_str(1,"    Display L");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"设定 显示脉冲");
		v_menu_show_str(1,"     显示公升");
	}
}
static void v_menu_setting_show_all_set(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"Set Open ALL");
		v_menu_show_str(1,"    Close All");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"设定 开启全显");
		v_menu_show_str(1,"     关闭全显");
	}
}
static void v_menu_setting_show_all_close(void){
	if(v_menu_language ==  V_MENU_LANGUAGE_EN){
		v_menu_show_str(0,"    Open ALL");
		v_menu_show_str(1,"Set Close All");
	}else if(v_menu_language ==  V_MENU_LANGUAGE_CN){
		v_menu_show_str(0,"     开启全显");
		v_menu_show_str(1,"设定 关闭全显");
	}
}

/*do function for menu show*/
static void v_do_function_sensor_A_B_Trip_Tot(void){
}
static void v_do_function_sensor_A_B_travel_fuel_consum(void){
}
static void v_do_function_sensor_A_B_instant_fuel_consum(void){
}
static void v_do_function_sensor_C_D_Trip_Tot(void){
}
static void v_do_function_sensor_C_D_travel_fuel_consum(void){
}
static void v_do_function_sensor_C_D_instant_fuel_consum(void){
}
static void v_do_function_sensor_A_Trip_Tot(void){
}
static void v_do_function_sensor_B_Trip_Tot(void){
}
static void v_do_function_sensor_C_Trip_Tot(void){
}
static void v_do_function_sensor_D_Trip_Tot(void){
}
static void v_do_function_sensor_A_B_Trip(void){
}
static void v_do_function_sensor_A_B_Tot(void){
}
static void v_do_function_sensor_C_D_Trip(void){
}
static void v_do_function_sensor_C_D_Tot(void){
}
static void v_do_function_sensor_A_B_flow(void){
}
static void v_do_function_sensor_C_D_flow(void){
}

/*do function for menu setting*/
static void v_do_function_setting_select_display_format(void){
}
static void v_do_function_setting_select_show_all(void){
}
static void v_do_function_setting_display_format_flow_L(void){
}
static void v_do_function_setting_display_format_pulse(void){
}
static void v_do_function_setting_show_all_set(void){
}
static void v_do_function_setting_show_all_close(void){
}

/*struct for menu display*/
static v_menu_struct_t v_struct_show_sensor_A_B_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_A_B_travel_fuel_consum;
static v_menu_struct_t v_struct_show_sensor_A_B_instant_fuel_consum;
static v_menu_struct_t v_struct_show_sensor_C_D_Trip_Tot;
static v_menu_struct_t v_struct_show_sensor_C_D_travel_fuel_consum;
static v_menu_struct_t v_struct_show_sensor_C_D_instant_fuel_consum;
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
static v_menu_struct_t v_struct_setting_display_format_flow_L;
static v_menu_struct_t v_struct_setting_display_format_pulse;
static v_menu_struct_t v_struct_setting_show_all_set;
static v_menu_struct_t v_struct_setting_show_all_close;

static v_init_menu_struct(v_menu_struct_t *source_struct, v_menu_struct_t *up_menu,
	v_menu_struct_t *enter_menu, v_menu_struct_t *esc_menu,
	v_menu_struct_t *down_menu, void (*do_function) (void), void (*menu_show)(void)){
	source_struct->enter_menu = enter_menu;
	source_struct->esc_menu = esc_menu;
	source_struct->up_menu = up_menu;
	source_struct->down_menu = down_menu;
	source_struct->do_function = do_function;
	source_struct->menu_show = menu_show;
}

static void v_init_menu_show_struct_flow_L_table(){
	v_cur_menu = &v_struct_show_sensor_A_B_Trip_Tot;
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_D_flow,
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		v_do_function_sensor_A_B_Trip_Tot,
		v_menu_show_sensor_A_B_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		0,0,
		&v_struct_show_sensor_A_B_Trip_Tot,
		&v_struct_show_sensor_A_B_instant_fuel_consum,
		v_do_function_sensor_A_B_travel_fuel_consum,
		v_menu_show_sensor_A_B_travel_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_instant_fuel_consum,
		0,0,
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		&v_struct_show_sensor_C_D_Trip_Tot,
		v_do_function_sensor_A_B_instant_fuel_consum,
		v_menu_show_sensor_A_B_instant_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Trip_Tot,
		0,0,
		&v_struct_show_sensor_A_B_instant_fuel_consum,
		&v_struct_show_sensor_C_D_travel_fuel_consum,
		v_do_function_sensor_C_D_Trip_Tot,
		v_menu_show_sensor_C_D_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_travel_fuel_consum,
		0,0,
		&v_struct_show_sensor_C_D_Trip_Tot,
		&v_struct_show_sensor_C_D_instant_fuel_consum,
		v_do_function_sensor_C_D_travel_fuel_consum,
		v_menu_show_sensor_C_D_travel_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_instant_fuel_consum,
		0,0,
		&v_struct_show_sensor_C_D_travel_fuel_consum,
		&v_struct_show_sensor_A_Trip_Tot,
		v_do_function_sensor_C_D_instant_fuel_consum,
		v_menu_show_sensor_C_D_instant_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_D_instant_fuel_consum,
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
		&v_struct_show_sensor_A_B_Trip_Tot,
		v_do_function_sensor_C_D_flow,
		v_menu_show_sensor_C_D_flow
	);
}
static void v_init_menu_show_struct_pulse_table(){
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_Trip_Tot,
		&v_struct_setting_select_display_format,
		0,
		&v_struct_show_sensor_C_D_flow,
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		v_do_function_sensor_A_B_Trip_Tot,
		v_menu_show_sensor_A_B_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		0,0,
		&v_struct_show_sensor_A_B_Trip_Tot,
		&v_struct_show_sensor_C_D_Trip_Tot,
		v_do_function_sensor_A_B_travel_fuel_consum,
		v_menu_show_sensor_A_B_travel_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_Trip_Tot,
		0,0,
		&v_struct_show_sensor_A_B_travel_fuel_consum,
		&v_struct_show_sensor_C_D_travel_fuel_consum,
		v_do_function_sensor_C_D_Trip_Tot,
		v_menu_show_sensor_C_D_Trip_Tot
	);
	v_init_menu_struct(
		&v_struct_show_sensor_C_D_travel_fuel_consum,
		0,0,
		&v_struct_show_sensor_C_D_Trip_Tot,
		&v_struct_show_sensor_A_Trip_Tot,
		v_do_function_sensor_C_D_travel_fuel_consum,
		v_menu_show_sensor_C_D_travel_fuel_consum
	);
	v_init_menu_struct(
		&v_struct_show_sensor_A_Trip_Tot,
		0,0,
		&v_struct_show_sensor_C_D_travel_fuel_consum,
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
		&v_struct_show_sensor_A_B_Trip_Tot,
		v_do_function_sensor_C_D_flow,
		v_menu_show_sensor_C_D_flow
	);
}

static void init_menu_setting_struct_table(void){
	v_init_menu_struct(
		&v_struct_setting_select_display_format,
		&v_struct_setting_display_format_flow_L,
		&v_struct_show_sensor_A_B_Trip_Tot,
		0,
		&v_struct_setting_select_show_all,
		v_do_function_setting_select_display_format,
		v_menu_setting_select_display_format
	);
	v_init_menu_struct(
		&v_struct_setting_select_show_all,
		&v_struct_setting_show_all_set,
		0,
		&v_struct_setting_select_display_format,
		0,
		v_do_function_setting_select_show_all,
		v_menu_setting_select_show_all
	);
	v_init_menu_struct(
		&v_struct_setting_display_format_flow_L,
		&v_struct_setting_select_display_format,
		&v_struct_setting_select_display_format,
		0,
		&v_struct_setting_display_format_pulse,
		v_do_function_setting_display_format_flow_L,
		v_menu_setting_display_format_flow_L
	);
	v_init_menu_struct(
		&v_struct_setting_display_format_pulse,
		0,
		&v_struct_setting_select_display_format,
		&v_struct_setting_display_format_flow_L,
		0,
		v_do_function_setting_display_format_pulse,
		v_menu_setting_display_format_pulse
	);
	v_init_menu_struct(
		&v_struct_setting_show_all_set,
		&v_struct_setting_select_show_all,
		&v_struct_setting_select_show_all,
		0,
		&v_struct_setting_show_all_close,
		v_do_function_setting_show_all_set,
		v_menu_setting_show_all_set
	);
	v_init_menu_struct(
		&v_struct_setting_show_all_set,
		0,
		&v_struct_setting_select_show_all,
		&v_struct_setting_show_all_set,
		0,
		v_do_function_setting_show_all_close,
		v_menu_setting_show_all_close
	);
}

void v_menu_init(void){
	v_menu_language = V_MENU_LANGUAGE_EN;
	v_init_language_hint_string();

	v_menu_display_format = V_MENU_DISPLAY_FORMAT_FLOW_L;
	v_cur_menu = &v_struct_show_sensor_A_B_Trip_Tot;
	if(v_menu_display_format == V_MENU_DISPLAY_FORMAT_FLOW_L){
		v_init_menu_show_struct_flow_L_table();
	}else{
		v_init_menu_show_struct_pulse_table();
	}
	init_menu_setting_struct_table();
}
