#include "font_8x16.h"
#include "lcd.h"
#include <string.h>

int main()
{
    lcd_init();
    lcd_clean();
    char *str = "hello world!";
    for (int32_t i = 0; i < strlen(str); ++i)
    {
        lcd_put_ascii(lcd_width() / 2 + i * 8, lcd_height() / 2, str[i]);
    }
    lcd_destory();
}