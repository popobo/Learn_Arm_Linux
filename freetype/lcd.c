#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "lcd.h"
#include "font_8x16.h"

static const char* LCD_DEVICE = "/dev/fb0";
static struct fb_var_screeninfo var_scfo;

static uint32_t pixel_width = 0;
static uint32_t line_width = 0;
static uint32_t screen_size = 0;

static int fd_fb = 0;
static __u8* fb_base = NULL;

static int fd_hzk16 = 0;
static uint8_t* hzk_mem = NULL;

static int32_t is_init = 0;

int32_t lcd_is_init()
{
    return is_init;
}

int lcd_init()
{
    if (is_init) return 0;

    fd_fb = open(LCD_DEVICE, O_RDWR);
    if (fd_fb < 0)
    {
        printf("can't open %s\n", LCD_DEVICE);
        return -1;
    }
    
    if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var_scfo))
    {
        printf("can't get fb_var_screeninfo\n");
        return -1;
    }
    
    pixel_width = var_scfo.bits_per_pixel / 8;
    line_width = var_scfo.xres * pixel_width;
    screen_size = var_scfo.xres * var_scfo.yres * pixel_width;
    fb_base = (__u8 *)mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);

    if (fb_base == (__u8 *)-1)
    {
        printf("can not mmap\n");
        return -1;
    }

    fd_hzk16 = open("HZK16", O_RDONLY);
    if (fd_hzk16 < 0)
    {
        printf("can't open HZK16\n");
        return -1;
    }

    struct stat hzk_stat;
    if (fstat(fd_hzk16, &hzk_stat))
    {
        printf("can't get fstat\n");
        return -1;
    }

    hzk_mem = (uint8_t *)mmap(NULL, hzk_stat.st_size, PROT_READ, MAP_SHARED, fd_hzk16, 0);
    if(hzk_mem == (uint8_t *)-1)
    {
        printf("can't mmap for hzk16");
        return -1;
    }

    is_init = 1;

    return 0;
}

void lcd_put_pixel(int32_t x, int32_t y, uint32_t color)
{
    assert(fb_base != NULL);
    assert(fd_fb > 0);

    uint8_t * pen_8 = fb_base + y * line_width + x * pixel_width;
    uint16_t * pen_16 = (uint16_t *)pen_8;
    uint32_t * pen_32 = (uint32_t *)pen_32;

    assert(pen_8 <= fb_base + var_scfo.xres * var_scfo.yres * pixel_width);

    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;

    switch (var_scfo.bits_per_pixel)
    {
        case 8:
        {
            *pen_8 = color;
            break;
        }
        case 16:
        {
            /*565*/
            red = (color >> 16) & 0xff;
            green = (color >> 8) & 0xff;
            blue = (color >> 0) & 0xff;
            color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
            *pen_16 = color;
            break;
        }
        case 32:
        {
            *pen_32 = color;
            break;
        }
        default:
        {
            printf("can't support %d bpp\n", var_scfo.bits_per_pixel);
            break;
        }
    }
}

void lcd_destory()
{
    assert(fb_base != NULL);
    assert(fd_fb > 0);
    munmap(fb_base, screen_size);
    close(fd_fb);
}

void lcd_put_ascii(int32_t x, int32_t y, uint8_t c)
{
    uint8_t *dots = (uint8_t *)&fontdata_8x16[c * 16];
    
    for(int32_t i = 0; i < 16; i++)
    {
        uint8_t byte = dots[i];
        for (int32_t b = 7; b >= 0; b--)
        {
            if (byte & (1 << b))
            {
                lcd_put_pixel(x + 7 - b, y + i, 0xffffff);
            }
            else
            {
                lcd_put_pixel(x + 7 - b, y + i, 0);
            }
        }
    }
}

void lcd_put_chinese(int32_t x, int32_t y, char* str)
{
    uint32_t area = str[0] - 0xA1;
    uint32_t where = str[1] - 0xA1;
    uint8_t* dots = hzk_mem + (area * 94 + where) * 32;
    for(int32_t i = 0; i < 16; ++i)
    {
        /*
        uint16_t bytes = ((uint16_t *)dots)[i]; // little end issue 
        for(int32_t b = 15; b >= 0; --b)
        {
            lcd_put_pixel(x + 15 - b, y + i, (bytes & (1 << b)) ? 0xFFFFFF : 0);
        }
        */

        for (int32_t j = 0; j < 2; j++)
		{
			uint8_t byte = dots[i*2 + j];
			for (int32_t b = 7; b >=0; b--)
			{
                lcd_put_pixel(x+j*8+7-b, y+i, (byte & (1<<b)) ? 0xFFFFFF : 0);
			}
		}
    }
}

void lcd_clean()
{
    assert(fb_base != NULL);
    memset(fb_base, 0, screen_size);
}

int32_t lcd_width()
{
    return var_scfo.xres;
}

int32_t lcd_height()
{
    return var_scfo.yres;
}