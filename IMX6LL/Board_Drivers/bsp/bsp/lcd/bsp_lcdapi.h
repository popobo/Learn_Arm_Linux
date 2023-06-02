#ifndef BSP_LCDAPI_H
#define BSP_LCDAPI_H

#include "imx6ul.h"
#include "bsp_lcd.h"

/* 函数声明 */
void lcd_drawline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void lcd_showchar(unsigned     short x,uint16_t y,uint8_t num,uint8_t size, uint8_t mode);
uint32_t lcd_pow(uint8_t m,uint8_t n);
void lcd_shownum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,uint8_t size);
void lcd_showxnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint8_t mode);
void lcd_show_string(uint16_t x,uint16_t y,
uint16_t width, uint16_t height, uint8_t size, const char *p);

#endif

