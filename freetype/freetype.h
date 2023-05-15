#pragma once

#include <inttypes.h>
#include <stddef.h>

int32_t freetype_init(const char* face_path, int32_t font_size);

int32_t freetype_draw_char(int32_t x, int32_t y, const wchar_t *str);

int32_t freetype_draw_char_angle(int32_t x, int32_t y, int32_t angle, const wchar_t *str);

void freetype_set_font_size(int32_t font_size);

int32_t free_type_display_string(const wchar_t* wstr, int32_t lcd_x, int32_t lcd_y);