#include <string.h>
#include "sys.h"
#include "ff.h"
#include "led.h"
#include "uart_timer.h"
#include "v_menu.h"

#define UART_IDLE 0
#define UART_PREA 1
#define UART_RCV1 2
#define UART_RCV2 3
#define UART_PREAMBLE1 0x4F
#define UART_PREAMBLE2 0x6C
#define UART_RES_M 0x4D
#define UART_RES_R 0x52
#define CMD_DATA_END 0xFF
#define MAX_U_BUF_LEN 256
#define CAR_PLATE_LEN 8
#define HISTORY_DATA_MAX_LEN 44
#define FS_SECTOR_SIZE 512
#define DATE_BUF_LEN 7

#define MAX_CMD_DATA_LEN 8
typedef struct uart_cmd_struct
{
	uint8_t cmd;
	uint8_t length;
	uint8_t cmd_data[MAX_CMD_DATA_LEN];
	uint8_t cmd_len;
}uart_cmd_t;


static uint8_t u_state = UART_IDLE;
static uint8_t u_cmd;
static uint8_t u_cmd_index;
static uint8_t u_data_len;
static uint8_t *u_buf;
static uint8_t u_raw_data_len;
static uint8_t u_raw_buf[MAX_U_BUF_LEN];
static uint8_t u_buf_offset;
static uint8_t u_tx_buf[MAX_U_BUF_LEN];
static uint8_t recv_cmd_id[2];
static uint16_t send_cmd_id = 0;

// used for SD storage
static uint16_t data_tot_num;
static uint16_t cur_data_id;
static uint16_t cur_query_id;
static uint8_t history_data[HISTORY_DATA_MAX_LEN];
static uint8_t data_write_buf[FS_SECTOR_SIZE + HISTORY_DATA_MAX_LEN];
static uint8_t car_plate[CAR_PLATE_LEN];
static uint8_t cur_date[DATE_BUF_LEN];

// for start_cmd_loop
static uint8_t loop_cmd_index;
static uint8_t* loop_cmd_param;
static uint8_t loop_cmd_param_len;

uint32_t cmd_data_available;
uint32_t recv_cmd;

// support cmds
uart_cmd_t g_uart_cmds[] = 
{
	{0x00, 0x87, {0x00, 0x80, CMD_DATA_END},             2},
	{0x01, 0x81, {0x01, 0x80, CMD_DATA_END},             2},
	{0x06, 0xFF, {0x06, 0x80, CMD_DATA_END},             2},
	{0x07, 0x82, {0x07, 0x80, CMD_DATA_END},             2},
	{0x08, 0x82, {0x08, 0x80, CMD_DATA_END},             2},
	{0x09, 0x81, {0x09, 0x80, CMD_DATA_END},             2},
	{0x0C, 0x02, {0x0C, 0x00, CMD_DATA_END},             2},
	{0x0D, 0x02, {0x0D, 0x02, 0x47, 0x54, CMD_DATA_END}, 4},
	{0x0E, 0x81, {0x0E, 0x80, CMD_DATA_END},             2},
	{0x11, 0x81, {0x11, 0x80, CMD_DATA_END},             2},
	{0x18, 0x81, {0x18, 0x80, CMD_DATA_END},             2},
	{0x19, 0x81, {0x19, 0x80, CMD_DATA_END},             2},
	{0x1A, 0x81, {0x1A, 0x80, CMD_DATA_END},             2},
	{0x1B, 0x81, {0x1B, 0x80, CMD_DATA_END},             2},
	{0x20, 0x81, {0x20, 0x80, CMD_DATA_END},             2},
	{0x31, 0x84, {0x31, 0x80, CMD_DATA_END},             2},
	{0x3A, 0xAC, {0x3A, 0x82, CMD_DATA_END},             2},
//	{0x3B, 0xAC, {0x3B, 0x82, 0x00, 0x00, CMD_DATA_END}, 4},
	{0x3C, 0xAC, {0x3C, 0x82, 0x00, 0x00, CMD_DATA_END}, 4},
	{0x3D, 0xAC, {0x3D, 0x82, 0x00, 0x00, CMD_DATA_END}, 4},
	{0x3F, 0xAC, {0x3F, 0x82, 0x00, 0x00, CMD_DATA_END}, 4},
	{0x40, 0x81, {0x40, 0x80, CMD_DATA_END},             2},
	{0xF0, 0x01, {CMD_DATA_END},                         0},
// commands which needs another direction
	{0x01, 0x01, {0x01, 0x01, 0x01, CMD_DATA_END},       3},
	{0x11, 0x01, {0x11, 0x01, CMD_DATA_END},             2},
	{0x20, 0x01, {0x20, 0x01, CMD_DATA_END},             2},
};

typedef struct u_raw_data_struct
{
	uint8_t preamble1;
	uint8_t preamble2;
	uint8_t len;
	uint8_t response;
	uint8_t id_l;
	uint8_t id_h;
	uint8_t cmd;
	uint8_t cmd_len;
	uint8_t cmd_data;
}u_raw_data;

static uint8_t checksum(uint8_t len, uint8_t *strCmd)
{
	uint8_t crc = 0, i;
	for(i = 0; i < len; i++)
		crc ^= strCmd[i];
	return crc;
}

void send_cmd(uint8_t cmd_index, uint8_t* param, uint8_t param_len)
{
	int i;
	u_raw_data *tx_data;

	if (cmd_index >= LAST_UART_CMD || g_uart_cmds[cmd_index].cmd_len == 0)
		return;

	cmd_data_available &= ~(1 << cmd_index);

	tx_data = (u_raw_data *)u_tx_buf;
	tx_data->preamble1 = UART_PREAMBLE1;
	tx_data->preamble2 = UART_PREAMBLE2;
	tx_data->len = g_uart_cmds[cmd_index].cmd_len + 4;
	tx_data->response = UART_RES_M;
	tx_data->id_l = send_cmd_id & 0xFF;
	tx_data->id_h = ((send_cmd_id++) & 0xFF00) >> 8;
	memcpy(&(tx_data->cmd), g_uart_cmds[cmd_index].cmd_data, g_uart_cmds[cmd_index].cmd_len);
	if (param != NULL) {
		memcpy(u_tx_buf + tx_data->len + 2, param, param_len);
		tx_data->len += param_len;
	}
	u_tx_buf[tx_data->len + 2] = checksum(tx_data->len + 2, u_tx_buf);

	for (i = 0; i < tx_data->len + 3; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, u_tx_buf[i]);
	}
}

// 0: cmd sucessfully return
// 1: timeout
// 2: error
int wait_for_cmd(uint8_t cmd_index)
{
	int timeout = 500;

	if (cmd_index >= LAST_UART_CMD)
		return 2;

	for (; timeout > 0 && !(cmd_data_available & (1 << cmd_index)); timeout--)
		delay_ms(1);

	cmd_data_available &= ~(1 << cmd_index);

	return timeout ? 0 : 1;
}

// return -1 if not a cmd, else return the cmd index.
static uint8_t get_cmd_index(uint8_t value)
{
	uint8_t i;

	for (i = 0; i < LAST_UART_CMD; i++) {
		if (g_uart_cmds[i].cmd == value)
			break;
	}

	return i;
}

static uint32_t dec_to_hex(uint32_t dec)
{
	uint32_t hex = 0, mask = 0xF, p = 1, i;

	for (i = 0; i < 8; i ++) {
		hex += ((dec & (mask << (i * 4))) >> (i * 4)) * p;
		p *= 10;
	}

	return hex;
}

#define MAX_DAY_TO_SEC (24 * 3600)
static uint32_t date_to_sec(uint8_t hour, uint8_t minu, uint8_t sec)
{
	if (hour > 23 || minu > 59 || sec > 59)
		return MAX_DAY_TO_SEC;

	return (hour * 3600 + minu * 60 + sec);
}

static void send_OK(void)
{
	uint8_t ok[4], i;
	u_raw_data *tx_data;

	ok[0] = u_cmd;
	ok[1] = 0x02;
	ok[2] = 'O';
	ok[3] = 'K';

	tx_data = (u_raw_data *)u_tx_buf;
	tx_data->preamble1 = UART_PREAMBLE1;
	tx_data->preamble2 = UART_PREAMBLE2;
	tx_data->len = 8;
	tx_data->response = UART_RES_R;
	tx_data->id_l = recv_cmd_id[0];
	tx_data->id_h = recv_cmd_id[1];
	memcpy(&(tx_data->cmd), ok, 4);
	u_tx_buf[tx_data->len + 2] = checksum(tx_data->len + 2, u_tx_buf);

	for (i = 0; i < tx_data->len + 3; i++) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		USART_SendData(USART1, u_tx_buf[i]);
	}
}

static void parse_uart_data(void)
{
	uint32_t i, tmp, is_cmd = 0;

	if ((u_data_len == 0x02 || u_data_len == 0x82) &&
			(u_buf[0] == 'N' && u_buf[1] == 'O'))
		goto no_cmd;

	switch (u_cmd) {
		case 0x00:
			memcpy(cur_date, u_buf, DATE_BUF_LEN);
			break;
		case 0x01:
			if (u_data_len == 0x02 && u_buf[0] == 'O' && u_buf[1] == 'K')
				break;

			if (u_buf[0] > 1)
				goto no_cmd;

			v_menu_reset_tot_enable = u_buf[0];

			break;
		case 0x0C:
		case 0x0D:
			break;
		case 0x06:
			tmp = 0;
			for (i = 0; i < (u_data_len & 0x7F) && tmp < CAR_PLATE_LEN; i++) {
				if (u_buf[i] < 0x80)
					car_plate[tmp++] = u_buf[i];
			}

			if (tmp < CAR_PLATE_LEN)
				car_plate[tmp] = 0;
			else
				car_plate[tmp - 1] = 0;
			break;
		case 0x07:
			v_fuel_consum_1_correct =
				u_buf[0] * 100 + (u_buf[1] & 0x7F) ;
                    v_fuel_consum_1_corrected =
                            u_buf[1] & 0x80;
			if (u_data_len == 0x02)
				is_cmd = 1;
			break;
		case 0x08:
			v_fuel_consum_2_correct =
				u_buf[0] * 100 + (u_buf[1] & 0x7F) ;
                    v_fuel_consum_2_corrected =
                            u_buf[1] & 0x80;
			if (u_data_len == 0x02)
				is_cmd = 1;
			break;
		case 0x09:
			if (u_buf[0] > 60)
				goto no_cmd;
			v_sensor_flow_smaple_time = u_buf[0];

			if (u_data_len == 0x01)
				is_cmd = 1;
			break;
		case 0x0E:
			if (u_buf[0] > 1)
				goto no_cmd;
			v_menu_language = u_buf[0];

			if (u_data_len == 0x01)
				is_cmd = 1;
			break;
		case 0x11:
			if (u_data_len == 0x02 && u_buf[0] == 'O' && u_buf[1] == 'K')
				break;

			if (u_buf[0] > 1)
				goto no_cmd;

			v_menu_display_format = u_buf[0];

			if (u_data_len == 0x01)
				is_cmd = 1;
			break;
		case 0x18:
			v_sensor_error_delay_time = u_buf[0];
			break;
		case 0x19:
			if (u_buf[0] > 60)
				goto no_cmd;
			v_sensor_fuel_consum_sample_time = u_buf[0];
			break;
		case 0x1A:
			v_sensor_instant_fuel_consum_1_alarm = u_buf[0];
			break;
		case 0x1B:
			v_sensor_instant_fuel_consum_2_alarm = u_buf[0];
			break;
		case 0x20:
			if (u_data_len == 0x02 && u_buf[0] == 'O' && u_buf[1] == 'K')
				break;

			v_menu_show_all_time = u_buf[0];

			if (u_data_len == 0x01)
				is_cmd = 1;
			break;
		case 0x31:
			data_tot_num = (u_buf[1] << 8) | u_buf[0];
			cur_data_id = (u_buf[3] << 8) | u_buf[2];
			break;
		case 0x3A:
			if ((u_data_len & 0x7F) != HISTORY_DATA_MAX_LEN)
				goto no_cmd;
			memcpy(history_data, u_buf, HISTORY_DATA_MAX_LEN);
			break;
#if 0
		case 0x3B:
			if (u_buf[2] != 0xBB)
				goto no_cmd;

			tmp = (u_buf[9] << 24) | (u_buf[10] << 16) |
				(u_buf[11] << 8) | u_buf[12];
			v_sensor_A_Tot = dec_to_hex(tmp);
			tmp = (u_buf[13] << 24) | (u_buf[14] << 16) |
				(u_buf[15] << 8) | u_buf[16];
			v_sensor_B_Tot = dec_to_hex(tmp);
			tmp = (u_buf[17] << 24) | (u_buf[18] << 16) |
				(u_buf[19] << 8) | u_buf[20];
			v_sensor_C_Tot = dec_to_hex(tmp);
			tmp = (u_buf[21] << 24) | (u_buf[22] << 16) |
				(u_buf[23] << 8) | u_buf[24];
			v_sensor_D_Tot = dec_to_hex(tmp);
			break;
#endif
		case 0x3C:
		case 0x3F:
			if (u_buf[2] != 0xBB)
				goto no_cmd;

			v_data_real_time =
				date_to_sec(dec_to_hex(u_buf[6]),
						dec_to_hex(u_buf[7]),
						dec_to_hex(u_buf[8]));
			if (v_data_real_time == MAX_DAY_TO_SEC)
				goto no_cmd;

			v_fuel_consum_2_correct =
				u_buf[0] * 100 + (u_buf[1] & 0x7F) ;
		       v_fuel_consum_2_corrected =
                            u_buf[1] & 0x80;
			v_fuel_consum_1_correct =
				u_buf[41] * 100 + (u_buf[42] & 0x7F) ;
                    v_fuel_consum_1_corrected =
                            u_buf[42] & 0x80;
                            
			tmp = u_buf[43];
			v_sensor_A_status = (tmp & 0x10) ? ((tmp & 0x01) ? 2 : 1) : 0;
			v_sensor_B_status = (tmp & 0x20) ? ((tmp & 0x02) ? 2 : 1) : 0;
			v_sensor_C_status = (tmp & 0x40) ? ((tmp & 0x04) ? 2 : 1) : 0;
			v_sensor_D_status = (tmp & 0x80) ? ((tmp & 0x08) ? 2 : 1) : 0;
			
			tmp = (u_buf[9] << 24) | (u_buf[10] << 16) |
				(u_buf[11] << 8) | u_buf[12];
			v_sensor_A_Trip = dec_to_hex(tmp);
			tmp = (u_buf[13] << 24) | (u_buf[14] << 16) |
				(u_buf[15] << 8) | u_buf[16];
			v_sensor_A_Tot = dec_to_hex(tmp);
			tmp = (u_buf[17] << 24) | (u_buf[18] << 16) |
				(u_buf[19] << 8) | u_buf[20];
			v_sensor_B_Trip = dec_to_hex(tmp);
			tmp = (u_buf[21] << 24) | (u_buf[22] << 16) |
				(u_buf[23] << 8) | u_buf[24];
			v_sensor_B_Tot = dec_to_hex(tmp);
			tmp = (u_buf[25] << 24) | (u_buf[26] << 16) |
				(u_buf[27] << 8) | u_buf[28];
			v_sensor_C_Trip = dec_to_hex(tmp);
			tmp = (u_buf[29] << 24) | (u_buf[30] << 16) |
				(u_buf[31] << 8) | u_buf[32];
			v_sensor_C_Tot = dec_to_hex(tmp);
			tmp = (u_buf[33] << 24) | (u_buf[34] << 16) |
				(u_buf[35] << 8) | u_buf[36];
			v_sensor_D_Trip = dec_to_hex(tmp);
			tmp = (u_buf[37] << 24) | (u_buf[38] << 16) |
				(u_buf[39] << 8) | u_buf[40];
			v_sensor_D_Tot = dec_to_hex(tmp);
			break;
		case 0x3D:
			if (!(u_buf[0] == 0x0 && u_buf[1] == 0x0 &&
					u_buf[2] == 0xBB))
				goto no_cmd;

			v_data_real_time =
				date_to_sec(dec_to_hex(u_buf[6]),
						dec_to_hex(u_buf[7]),
						dec_to_hex(u_buf[8]));
			if (v_data_real_time == MAX_DAY_TO_SEC)
				goto no_cmd;

			tmp = u_buf[9];
			v_sensor_A_status = (tmp & 0x10) ? ((tmp & 0x01) ? 2 : 1) : 0;
			v_sensor_B_status = (tmp & 0x20) ? ((tmp & 0x02) ? 2 : 1) : 0;
			v_sensor_C_status = (tmp & 0x40) ? ((tmp & 0x04) ? 2 : 1) : 0;
			v_sensor_D_status = (tmp & 0x80) ? ((tmp & 0x08) ? 2 : 1) : 0;

			v_fuel_consum_1_travel_time =
				(u_buf[10] << 16) | (u_buf[11] << 8) | u_buf[12];   //yfeng mod
			tmp = (u_buf[13] << 24) | (u_buf[14] << 16) |
				(u_buf[15] << 8) | u_buf[16];
			v_fuel_consum_1_travel_consum = dec_to_hex(tmp);
			v_fuel_consum_2_travel_time =
				(u_buf[17] << 16) | (u_buf[18] << 8) | u_buf[19];  //yfeng mod
			tmp = (u_buf[20] << 24) | (u_buf[21] << 16) |
				(u_buf[22] << 8) | u_buf[23];
			v_fuel_consum_2_travel_consum = dec_to_hex(tmp);
			v_fuel_consum_1_trip = u_buf[24] | (u_buf[25] << 8)
				| (u_buf[26] << 16) | (u_buf[27] << 24);
			v_fuel_consum_1_tot = u_buf[29] | (u_buf[30] << 8)
				| (u_buf[31] << 16) | (u_buf[32] << 24);
			v_fuel_consum_2_trip = u_buf[34] | (u_buf[35] << 8)
				| (u_buf[36] << 16) | (u_buf[37] << 24);
			v_fuel_consum_2_tot = u_buf[39] | (u_buf[40] << 8)
				| (u_buf[41] << 16) | (u_buf[42] << 24);
			break;
		case 0x40:
			v_sensor_work_mode = u_buf[0];
			break;
		case 0xF0:
			if (u_buf[0] > 1)
				goto no_cmd;

			v_menu_sleep = u_buf[0];
			break;
		default:
			goto no_cmd;
	}

	if (is_cmd) {
		send_OK();
		recv_cmd |= (1 << u_cmd_index);
	} else {
		cmd_data_available |= (1 << u_cmd_index);
	}
no_cmd:
	return;
}

static void parse_uart_raw_data(void)
{
	u_raw_data *data;
	data = (u_raw_data *)u_raw_buf;
	// preamble error
	if (data->preamble1 != UART_PREAMBLE1 || data->preamble2 != UART_PREAMBLE2)
		return;

	// checksum error
	if (u_raw_buf[u_raw_data_len + 2] != checksum(u_raw_data_len + 2, u_raw_buf))
		return;

	if (data->response == UART_RES_M) {
		recv_cmd_id[0] = data->id_l;
		recv_cmd_id[1] = data->id_h;
	}

	u_cmd = data->cmd;
	u_data_len = data->cmd_len;
	u_cmd_index = get_cmd_index(u_cmd);
	if (u_cmd_index >= LAST_UART_CMD
			|| (data->len - (u_data_len & 0x7F)) != 6)
		return;

	if (!(u_data_len == 0x02 || u_data_len == 0x82
				|| u_data_len == g_uart_cmds[u_cmd_index].length
				|| u_data_len == (0x7F & g_uart_cmds[u_cmd_index].length)
				|| g_uart_cmds[u_cmd_index].length == 0xFF)) 
		return;

	u_buf = &(data->cmd_data);
	parse_uart_data();
}

// use TIM3 as auto loop timer, 100us
void al_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM2, 0xFF);

	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

static void al_timer_start(uint16_t arr)
{
	TIM_SetCounter(TIM3, 0);
	TIM_SetAutoreload(TIM3, arr);
	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

static void al_timer_stop(void)
{
	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
}

void start_cmd_loop(uint16_t arr, uint8_t cmd_index, uint8_t* param, uint8_t param_len)
{
	if (cmd_index >= LAST_UART_CMD)
		return;

	send_cmd(cmd_index, param, param_len);

	loop_cmd_index = cmd_index;
	loop_cmd_param = param;
	loop_cmd_param_len = param_len;
	al_timer_start(arr);
}

void stop_cmd_loop(void)
{
	al_timer_stop();
	loop_cmd_index = LAST_UART_CMD;
	loop_cmd_param = NULL;
	loop_cmd_param_len = 0;
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		/* Pin PD.02 toggling with frequency = 10KHz */
		//GPIO_WriteBit(GPIOD, GPIO_Pin_2, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_2)));
		//LED1=!LED1;
		send_cmd(loop_cmd_index, NULL, 0);
	}
}

// use TIM2 as timeout timer, 100us
void timo_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM2, 0xFF);

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void timo_timer_start(uint16_t arr)
{
	TIM_SetCounter(TIM2, 0);
	TIM_SetAutoreload(TIM2, arr);
	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void timo_timer_stop(void)
{
	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM2, DISABLE);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		// reset the uart state to UART_IDLE
		u_state = UART_IDLE;
		LED0 = 1;
		timo_timer_stop();
	}
}

void USART1_IRQHandler(void)
{
	uint8_t value;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		value = USART_ReceiveData(USART1);
		switch (u_state) {
			case UART_IDLE:
				if (value == UART_PREAMBLE1) {
					u_buf_offset = 0;
					u_raw_buf[u_buf_offset++] = value;
					u_state = UART_PREA;
					LED0 = 0;
					timo_timer_start(5000); //500ms
				}
				break;
			case UART_PREA:
				if (value == UART_PREAMBLE2) {
					u_raw_buf[u_buf_offset++] = value;
					u_state = UART_RCV1;
				} else if (value == UART_PREAMBLE1) {
					u_buf_offset = 0;
					u_raw_buf[u_buf_offset++] = value;
				} else {
					u_buf_offset = 0;
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
				}
				break;
			case UART_RCV1:
				u_raw_data_len = value;
				u_raw_buf[u_buf_offset++] = value;
				u_state = UART_RCV2;
				break;
			case UART_RCV2:
				u_raw_buf[u_buf_offset++] = value;

				if (u_buf_offset == MAX_U_BUF_LEN) {
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
				}

				if (u_buf_offset == u_raw_data_len + 3) {
					parse_uart_raw_data();
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
					break;
				}
				break;
			default:
				u_buf_offset = 0;
				u_state = UART_IDLE;
				LED0 = 1;
				timo_timer_stop();
				break;
		}
	}
}
#if 0
void USART1_IRQHandler(void)
{
	uint8_t value;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		value = USART_ReceiveData(USART1);
		switch (u_state) {
			case UART_IDLE:
				u_cmd_index = get_cmd_index(value);
				if (u_cmd_index >= LAST_UART_CMD)
					break;

				u_cmd = value;
				u_cmd_dft_len = g_uart_cmds[u_cmd_index].length;
				u_state = UART_RCV1;
				LED0 = 0;
				timo_timer_start(5000); //500ms
				break;

			case UART_RCV1:
				if (!(value == 0x02 || value == 0x82
						|| value == u_cmd_dft_len
						|| value == (u_cmd_dft_len & 0x7F)
						|| u_cmd_dft_len == 0xFF)) {
					u_cmd_index = get_cmd_index(value);
					if (u_cmd_index >= LAST_UART_CMD) {
						u_state = UART_IDLE;
						LED0 = 1;
						break;
					}
					u_cmd = value;
					u_cmd_dft_len =
						g_uart_cmds[u_cmd_index].length;
					u_state = UART_RCV1;
					break;
				}
				u_data_len = value;
				u_buf_offset = 0;
				u_state = UART_RCV2;
				if ((u_data_len & 0x7F) == 0) {
					parse_uart_data();
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
				}
				break;

			case UART_RCV2:
				u_buf[u_buf_offset++] = value;

				if (u_buf_offset == MAX_U_BUF_LEN) {
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
				}

				if (u_buf_offset == (u_data_len & 0x7F)) {
					parse_uart_data();
					u_state = UART_IDLE;
					LED0 = 1;
					timo_timer_stop();
					break;
				}
				break;

			default:
				u_state = UART_IDLE;
		}
	}

}
#endif

static void gen_file_name(uint8_t *car_plate, uint8_t *cur_date, TCHAR *filename)
{
	int i, offset = 0;

	filename[offset++] = '0';
	filename[offset++] = ':';
	filename[offset++] = '/';

	for (i = 0; i < CAR_PLATE_LEN; i++) {
		if (car_plate[i] == 0)
			break;

		filename[offset++] = car_plate[i];
	}

	filename[offset++] = '_';
	/*
	filename[offset++] = '2';
	filename[offset++] = '0';
	*/

	for (i = 0; i < DATE_BUF_LEN; i++) {
		filename[offset++] = ((cur_date[i] & 0xF0) >> 4) + 48;
		filename[offset++] = (cur_date[i] & 0x0F) + 48;
	}

	filename[offset++] = '.';
	filename[offset++] = 't';
	filename[offset++] = 'x';
	filename[offset++] = 't';
}

// 0: sucess
// 1: error
int query_store_history(void)
{
	uint32_t i, j, bw, max_retry = 5;
	uint8_t query_id[2], *cur_ptr, res;
	FATFS fs;
	FIL file;
	TCHAR filename[32];

	// get current date
	send_cmd(GET_DATE_TIME, NULL, 0);
	if (wait_for_cmd(GET_DATE_TIME))
		return 1;

	send_cmd(QUERY_DATA_TOT_CUR_ID, NULL, 0);
	if (wait_for_cmd(QUERY_DATA_TOT_CUR_ID))
		return 1;

	if (data_tot_num == 0)
		return 0;

	cur_ptr = data_write_buf;
	gen_file_name(car_plate, cur_date, filename);

	res = f_mount(0, &fs);
	if (res)
		return 1;

	res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
	if (res)
		goto err_out1;

	for (i = 0; i < data_tot_num; i++) {
		cur_query_id = cur_data_id - i;
		query_id[0] = cur_query_id & 0xFF;
		query_id[1] = ((cur_query_id & 0xFF00) >> 8);

		for (j = 0; j < max_retry; j++) {
			send_cmd(QUERY_DATA_BY_ID, query_id, 2);
			if (wait_for_cmd(QUERY_DATA_BY_ID))
				continue;
			else
				break;
		}
		if (j == max_retry)
			goto out;

		memcpy(cur_ptr, history_data, HISTORY_DATA_MAX_LEN);
		cur_ptr += HISTORY_DATA_MAX_LEN;

		if ((cur_ptr - data_write_buf) > FS_SECTOR_SIZE) {
			res = f_write(&file, data_write_buf,
					FS_SECTOR_SIZE, &bw);
			if (res)
				goto err_out;

			memcpy(data_write_buf, data_write_buf + FS_SECTOR_SIZE,
					cur_ptr - data_write_buf - FS_SECTOR_SIZE);
			cur_ptr = cur_ptr - FS_SECTOR_SIZE;
		}
	}

out:
	f_write(&file, data_write_buf, cur_ptr - data_write_buf, &bw);
	f_close(&file);
	f_mount(0, NULL);

	return i == data_tot_num ? 0 : 1;

err_out:
	f_close(&file);
err_out1:
	f_mount(0, NULL);
	return 1;
}
