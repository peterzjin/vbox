#include "sys.h"
#include "led.h"

#define UART_IDLE 0
#define UART_RCV1 1
#define UART_RCV2 2

static u8 uart_state;
static u8 uart_cmd;
static u8 uart_data_len;
static u8 uart_buf[128];
static u8 uart_buf_offset;

// just for debug
u8 cmd_data;
u8 cmd_available;

static int is_cmd(u8 value)
{
	return 1;
}

static int is_cmd_length(u8 cmd, u8 length)
{
	return 1;
}

static void parse_uart_data(void)
{
	cmd_data = uart_buf[0];
	cmd_available = 1;
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

void al_timer_start(u16 arr)
{
	TIM_SetCounter(TIM3, 0);
	TIM_SetAutoreload(TIM3, arr);
	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void al_timer_stop(void)
{
	TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_Trigger, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
}


void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		/* Pin PD.02 toggling with frequency = 10KHz */
		//GPIO_WriteBit(GPIOD, GPIO_Pin_2, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_2)));
		LED1=!LED1;
	}
}

void USART1_IRQHandler(void)
{
	u8 value;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		value = USART_ReceiveData(USART1);
		switch (uart_state) {
			case UART_IDLE:
				if (!is_cmd(value))
					break;
				uart_cmd = value;
				uart_state = UART_RCV1;
				break;

			case UART_RCV1:
				if (!is_cmd_length(uart_buf[0], value)) {
					if (is_cmd(value)) {
						uart_cmd = value;
						break;
					}
					uart_state = UART_IDLE;
					break;
				}
				uart_data_len = value;
				uart_buf_offset = 0;
				uart_state = UART_RCV2;
				if ((uart_data_len & 0x3f) == 0) {
					parse_uart_data();
					uart_state = UART_IDLE;
				}
				break;

			case UART_RCV2:
				uart_buf[uart_buf_offset++] = value;

				if (uart_buf_offset == 128)
					uart_state = UART_IDLE;

				if (uart_buf_offset == (uart_data_len & 0x3f)) {
					parse_uart_data();
					uart_state = UART_IDLE;
					break;
				}
				break;

			default:
				uart_state = UART_IDLE;
		}
	}

}
