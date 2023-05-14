#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

const char* LCD_DEVICE = "/dev/fb0";
static struct fb_var_screeninfo var_scfo;

static __u32 pixel_width = 0;
static __u32 line_width = 0;
static __u32 screen_size = 0;
static __u8 * fb_base = NULL;

void lcd_put_pixel(__s32 x, __s32 y, __u32 color);

int main(int argc, char **argv)
{
    int fd_fb = open(LCD_DEVICE, O_RDWR);
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

    memset(fb_base, 0, screen_size);

    for(int i = 0; i < 100; i++)
    {
        lcd_put_pixel(var_scfo.xres / 2 + i, var_scfo.yres / 2, 0xFF0000);
    }

    munmap(fb_base, screen_size);
    close(fd_fb);

    return 0;
}

void lcd_put_pixel(__s32 x, __s32 y, __u32 color)
{
    __u8 * pen_8 = fb_base + y * line_width + x * pixel_width;
    __u16 * pen_16 = (__u16 *)pen_8;
    __u32 * pen_32 = (__u32 *)pen_32;

    __u32 red = 0;
    __u32 green = 0;
    __u32 blue = 0;

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