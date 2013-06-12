#include "sys.h"
#include "led.h"
#include "uart_timer.h"
#include "v_menu.h"

#define UART_IDLE 0
#define UART_RCV1 1
#define UART_RCV2 2
#define CMD_DATA_END 0xFF
#define MAX_U_BUF_LEN 128
#define CAR_PLATE_LEN 8
#define HISTORY_DATA_MAX_LEN 44
#define HISTORY_DATA_BLOCKS 100

static u8 u_state = UART_IDLE;
static u8 u_cmd;
static int u_cmd_index;
static u8 u_cmd_dft_len;
static u8 u_data_len;
static u8 u_buf[MAX_U_BUF_LEN];
static u8 u_buf_offset;

// used for SD storage
static u16 data_tot_num;
static u16 cur_data_id;
static u16 cur_query_id;
static u8 history_data[HISTORY_DATA_MAX_LEN];
static u8 history_data_blocks[HISTORY_DATA_MAX_LEN * HISTORY_DATA_BLOCKS];
static u8 tot_blocks;
static u16 car_plate[CAR_PLATE_LEN];

// for start_cmd_loop
static int loop_cmd_index;
static u8* loop_cmd_param;
static int loop_cmd_param_len;

u32 cmd_data_available;

// support cmds
uart_cmd_t g_uart_cmds[] = 
{
	{0x01, 0x81, {0x01, 0x01, 0x01, CMD_DATA_END}},
	{0x06, 0xFF, {0x06, 0x80, CMD_DATA_END}},
	{0x07, 0x82, {0x07, 0x80, CMD_DATA_END}},
	{0x08, 0x82, {0x08, 0x80, CMD_DATA_END}},
	{0x09, 0x81, {0x09, 0x80, CMD_DATA_END}},
	{0x0C, 0x02, {0x0C, 0x00, CMD_DATA_END}},
	{0x0D, 0x02, {0x0D, 0x02, 0x47, 0x54, CMD_DATA_END}},
	{0x0E, 0x81, {0x0E, 0x80, CMD_DATA_END}},
	{0x11, 0x81, {0x11, 0x80, CMD_DATA_END}},
	{0x18, 0x81, {0x18, 0x80, CMD_DATA_END}},
	{0x19, 0x81, {0x19, 0x80, CMD_DATA_END}},
	{0x1A, 0x81, {0x1A, 0x80, CMD_DATA_END}},
	{0x1B, 0x81, {0x1B, 0x80, CMD_DATA_END}},
	{0x31, 0x84, {0x31, 0x80, CMD_DATA_END}},
	{0x3A, 0xAC, {0x3A, 0x82, CMD_DATA_END}},
	//{0x3B, 0xAC, {0x3B, 0x82, 0x00, 0x00, CMD_DATA_END}},
	{0x3C, 0xAC, {0x3C, 0x82, 0x00, 0x00, CMD_DATA_END}},
	{0x3D, 0xAC, {0x3D, 0x82, 0x00, 0x00, CMD_DATA_END}},
	{0x3F, 0xAC, {0x3F, 0x82, 0x00, 0x00, CMD_DATA_END}},
	{0x40, 0x81, {0x40, 0x80, CMD_DATA_END}},
	{0, 0, {CMD_DATA_END}}
};

void send_cmd(int cmd_index, u8* param, int param_len)
{
	int i;

	if (cmd_index < 0 || cmd_index >= LAST_UART_CMD)
		return;

	cmd_data_available &= ~(1 << cmd_index);

	for (i = 0; i < MAX_CMD_DATA_LEN; i++) {
		if (g_uart_cmds[cmd_index].cmd_data[i] == CMD_DATA_END)
			break;

		USART_SendData(USART1, g_uart_cmds[cmd_index].cmd_data[i]);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	}

	if (param != 0) {
		for (i = 0; i < param_len; i++) {
			USART_SendData(USART1, param[i]);
			while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
		}
	}
}

// 0: cmd sucessfully return
// 1: timeout
// 2: error
int wait_for_cmd(int cmd_index)
{
	int timeout = 5000;

	if (cmd_index < 0 || cmd_index >= LAST_UART_CMD)
		return 2;

	while (!(cmd_data_available & (1 << cmd_index))) {
		if (timeout == 0)
			return 1;

		timeout--;
	}

	return 0;
}

// return -1 if not a cmd, else return the cmd index.
static int get_cmd_index(u8 value)
{
	int i, ret = -1;

	for (i = 0; i < LAST_UART_CMD; i++) {
		if (g_uart_cmds[i].cmd == 0)
			break;

		if (g_uart_cmds[i].cmd == value) {
			ret = i;
			break;
		}
	}

	return ret;
}

static u32 dec_to_hex(u32 dec)
{
	u32 hex = 0, mask = 0xF, p = 1, i;

	for (i = 0; i < 8; i ++) {
		hex += ((dec & (mask << (i * 4))) >> (i * 4)) * p;
		p *= 10;
	}

	return hex;
}

static void parse_uart_data(void)
{
	u32 i, tmp;

	if ((u_data_len == 0x02 || u_data_len == 0x82) &&
			(u_buf[0] == 'N' && u_buf[1] == 'O'))
		goto no_cmd;

	switch (u_cmd) {
		case 0x01:
		case 0x0C:
		case 0x0D:
			break;
		case 0x06:
			tmp = 0;
			for (i = 0; i < u_data_len; i++) {
				if (u_buf[i] > 0x80) {
					if (i < (u_data_len - 1))
						// need to do something to convert to unicode
						car_plate[tmp++] =
							(u_buf[i] << 8) + u_buf[i + 1];
					else
						goto no_cmd;

					i++;
				} else {
					car_plate[tmp++] = u_buf[i];
				}
			}
			break;
		case 0x07:
			v_fuel_consum_1_correct =
				u_buf[0] * 1000 + (u_buf[1] & 0x7F) * 10;
			break;
		case 0x08:
			v_fuel_consum_2_correct =
				u_buf[0] * 1000 + (u_buf[1] & 0x7F) * 10;
			break;
		case 0x09:
			if (u_buf[0] > 60)
				goto no_cmd;
			v_sensor_flow_smaple_time = u_buf[0];
			break;
		case 0x0E:
			if (u_buf[0] > 1)
				goto no_cmd;
			v_menu_language = u_buf[0];
			break;
		case 0x11:
			if (u_buf[0] > 1)
				goto no_cmd;
			v_menu_display_format = u_buf[0];
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
		case 0x31:
			data_tot_num = (u_buf[1] << 8) | u_buf[0];
			cur_data_id = (u_buf[3] << 8) | u_buf[2];
			break;
		case 0x3A:
			if (u_data_len != HISTORY_DATA_MAX_LEN)
				goto no_cmd;
			//memcpy(history_data, u_buf, HISTORY_DATA_MAX_LEN);
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

			v_fuel_consum_1_correct =
				u_buf[0] * 1000 + (u_buf[1] & 0x7F) * 10;
			v_fuel_consum_2_correct =
				u_buf[41] * 1000 + (u_buf[42] & 0x7F) * 10;

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
			if (u_buf[0] == 0x0 && u_buf[1] == 0x0 &&
					u_buf[2] == 0xBB)
				goto no_cmd;

			v_fuel_consum_1_travel_time =
				(u_buf[10] << 24) | (u_buf[11] << 16) | u_buf[12];
			tmp = (u_buf[13] << 24) | (u_buf[14] << 16) |
				(u_buf[15] << 8) | u_buf[16];
			v_fuel_consum_1_travel_consum = dec_to_hex(tmp);
			v_fuel_consum_2_travel_time =
				(u_buf[17] << 24) | (u_buf[18] << 16) | u_buf[19];
			tmp = (u_buf[20] << 24) | (u_buf[21] << 16) |
				(u_buf[22] << 8) | u_buf[23];
			v_fuel_consum_2_travel_consum = dec_to_hex(tmp);
			break;
		case 0x40:
			if (u_buf[0] > 0x08)
				goto no_cmd;

			v_sensor_work_mode = u_buf[0];
			break;
		default:
			goto no_cmd;
	}

	cmd_data_available |= (1 << u_cmd_index);
no_cmd:
	return;
}

// use TIM3 as auto loop timer, 100us
void al_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 5000;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

static void al_timer_start(u16 arr)
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

void start_cmd_loop(u16 arr, int cmd_index, u8* param, int param_len)
{
	if (cmd_index < 0 || cmd_index >= LAST_UART_CMD)
		return;

	loop_cmd_index = cmd_index;
	loop_cmd_param = param;
	loop_cmd_param_len = param_len;
	al_timer_start(arr);
}

void stop_cmd_loop(void)
{
	al_timer_stop();
	loop_cmd_index = -1;
	loop_cmd_param = 0;
	loop_cmd_param_len = 0;
}

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		/* Pin PD.02 toggling with frequency = 10KHz */
		//GPIO_WriteBit(GPIOD, GPIO_Pin_2, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_2)));
		LED1=!LED1;
		send_cmd(loop_cmd_index, 0, 0);
	}
}

// use TIM2 as timeout timer, 100us
void timo_timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 5000;
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void timo_timer_start(u16 arr)
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
	}
}

void USART1_IRQHandler(void)
{
	u8 value;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		value = USART_ReceiveData(USART1);
		switch (u_state) {
			case UART_IDLE:
				u_cmd_index = get_cmd_index(value);
				if (u_cmd_index < 0)
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
						|| u_cmd_dft_len == 0xFF)) {
					u_cmd_index = get_cmd_index(value);
					if (u_cmd_index < 0) {
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
#if 0
static u16 data_tot_num;
static u16 cur_data_id;
static u16 cur_query_id;
static u8 history_data[HISTORY_DATA_MAX_LEN];
static u16 car_plate[CAR_PLATE_LEN];

// 0: sucess
// 1: error
int query_store_history(void)
{
	u16 i;
	u8 query_id[2], cur_date[3];
	u8* cur_ptr;
	FIL file;

	send_cmd(QUERY_DATA_TOT_CUR_ID, NULL, 0);
	if (wait_for_cmd(QUERY_DATA_TOT_CUR_ID))
		return 1;

	if (data_tot_num == 0)
		return 0;

	memcpy(history_data_blocks, history_data, HISTORY_DATA_MAX_LEN);
	memcpy(cur_date, history_data + 5, 3);
	tot_blocks = 1;
	cur_ptr = history_data_blocks + HISTORY_DATA_MAX_LEN;
	f_open();
	for (i = 1; i < data_tot_num; i++) {
		cur_query_id = cur_data_id - i;
		query_id[0] = cur_query_id & 0xFF;
		query_id[1] = ((cur_query_id & 0xFF00) >> 8);

		send_cmd(QUERY_DATA_BY_ID, query_id, 2);
		if (wait_for_cmd(QUERY_DATA_TOT_CUR_ID))
			return 1;

		if (tot_blocks == HISTORY_DATA_BLOCKS) {
			f_write();
			tot_blocks = 0;
			cur_ptr = history_data_blocks;
		}
		if (memcmp(history_data + 5, cur_date, 3)) {
			f_write();
			f_close();
			f_open();
			tot_blocks = 0;
			cur_ptr = history_data_blocks;
			memcpy(cur_ptr, history_data, HISTORY_DATA_MAX_LEN);
			memcpy(cur_date, history_data + 5, 3);
		} else {
			memcpy(cur_ptr, history_data, HISTORY_DATA_MAX_LEN);
		}

		tot_blocks++;
		cur_ptr += HISTORY_DATA_MAX_LEN;
	}
	f_write();
	f_close();

	return 0;
}
#endif
