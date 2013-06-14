#ifndef __UART_TIMER_H
#define __UART_TIMER_H

extern uint32_t cmd_data_available;
extern uint32_t recv_cmd;

void timo_timer_init(void);
void al_timer_init(void);
void start_cmd_loop(uint16_t arr, uint8_t cmd_index, uint8_t* param, uint8_t param_len);
void stop_cmd_loop(void);

void send_cmd(uint8_t cmd_index, uint8_t* param, uint8_t param_len);
int wait_for_cmd(uint8_t cmd_index);

enum e_uart_cmds {
	ENABLE_DISABLE_RESET,		//0x01
	CAR_PLATE,			//0x06
	FC1_CORRECTION,			//0x07
	FC2_CORRECTION,			//0x08
	FLOW_SAMPLE_TIME,		//0x09
	TRIP_RESET,			//0x0C
	TOTAL_RESET,			//0x0D
	MENU_LANGUAGE,			//0x0E
	DISPLAY_FORMAT,			//0x11
	ERR_DELAY_TIME,			//0x18
	FC_SAMPLE_TIME,			//0x19
	FC1_ALARM,			//0x1A
	FC2_ALARM,			//0x1B
	DISPLAY_DELAY_TIME,		//0x20
	QUERY_DATA_TOT_CUR_ID,		//0x31
	QUERY_DATA_BY_ID,		//0x3A
	//SENSOR_DATA_TOT,		//0x3B
	SENSOR_DATA_PULSE,		//0x3C
	SENSOR_DATA_TOT_TRIP,		//0x3D
	SENSOR_DATA_LITER,		//0x3F
	SENSOR_WOKR_MODE,		//0x40
	LAST_UART_CMD
};

#define MAX_CMD_DATA_LEN 8
typedef struct uart_cmd_struct
{
	uint8_t cmd;
	uint8_t length;
	uint8_t cmd_data[MAX_CMD_DATA_LEN];
}uart_cmd_t;

#endif
