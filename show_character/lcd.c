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
static __u8 * fb_base = NULL;

int lcd_init()
{
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

    return 0;
}

void lcd_put_pixel(int32_t x, int32_t y, uint32_t color)
{
    assert(fb_base != NULL);
    assert(fd_fb > 0);

    uint8_t * pen_8 = fb_base + y * line_width + x * pixel_width;
    uint16_t * pen_16 = (uint16_t *)pen_8;
    uint32_t * pen_32 = (uint32_t *)pen_32;

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