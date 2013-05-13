#ifndef __UART_TIMER_H
#define __UART_TIMER_H

extern u8 cmd_data;
extern u8 cmd_available;

void al_timer_init(void);
void al_timer_start(u16 arr);
void al_timer_stop(void);

#endif
