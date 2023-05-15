#include "font_8x16.h"
#include "lcd.h"
#include "freetype.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    // if (argc < 2)
	// {
	// 	printf("Usage : %s <font_file> [font_size]\n", argv[0]);
	// 	return -1;
	// }

    int32_t font_size = 24;

    if (argc == 3)
		font_size = strtoul(argv[2], NULL, 0);

    lcd_init();
    freetype_init("simsun.ttc", font_size);
    lcd_clean();
    char *str = "hello world!";
    for (int32_t i = 0; i < strlen(str); ++i)
    {
        lcd_put_ascii(lcd_width() / 2 + i * 8, lcd_height() / 2, str[i]);
    }

    freetype_set_font_size(50);
    // wchar_t *chinese_str = L"我";
    // freetype_draw_char(lcd_width() / 3, lcd_height() / 3, chinese_str);
    // freetype_draw_char_angle(lcd_width()  / 3, 0, 90, chinese_str);

    wchar_t chinese_str2[] = L"你abc\n好efg";
    free_type_display_multiline(lcd_width() / 3, lcd_height() / 3, 30, 10, chinese_str2);

    free_type_display_multiline(lcd_width() / 2, lcd_height() / 3, 0, 10, chinese_str2);

    lcd_destory();
}