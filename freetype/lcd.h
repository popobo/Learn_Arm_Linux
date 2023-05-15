#pragma once

#include <inttypes.h>

int lcd_init();

void lcd_destory();

void lcd_put_pixel(int32_t x, int32_t y, uint32_t color);

void lcd_put_ascii(int32_t x, int32_t y, uint8_t c);

/// @brief put a chinese character in lcd, GB2312
/// @param x 
/// @param y 
/// @param str 
void lcd_put_chinese(int32_t x, int32_t y, char* str);

void lcd_clean();

int32_t lcd_width();

int32_t lcd_height();

int32_t lcd_is_init();