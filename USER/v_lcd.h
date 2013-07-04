#ifndef __V_LCD_H__
#define __V_LCD_H__

void v_lcd_init(void);
void v_lcd_backlight(uint8_t open);
void v_menu_show_str(uint8_t line, char *str);
void v_menu_show_inverse(uint8_t v_location);
void v_menu_clear_inverse(void);
#endif
