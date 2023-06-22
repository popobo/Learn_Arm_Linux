#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)
#define REGISTER_LENGTH 4

static void led_release(struct device *dev)
{
    printk("led device released!\r\n");
}

/*
 * 设备资源信息，也就是LED0所使用的所有寄存器
 */
static struct resource led_resource[] = {
    [0] = {
        .start = CCM_CCGR1_BASE,
        .end = (CCM_CCGR1_BASE + REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = SW_MUX_GPIO1_IO03_BASE,
        .end = (SW_PAD_GPIO1_IO03_BASE + REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = SW_PAD_GPIO1_IO03_BASE,
        .end = (SW_PAD_GPIO1_IO03_BASE + REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
    [3] = {
        .start = GPIO1_DR_BASE,
        .end = (GPIO1_DR_BASE + REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    },
    [4] = {
        .start = GPIO1_GDIR_BASE,
        .end = (GPIO1_GDIR_BASE + REGISTER_LENGTH - 1),
        .flags = IORESOURCE_MEM,
    }};

static struct platform_device led_device = {
    .name = "imx6ul-led",
    .id = -1,
    .dev = {
        .release = &led_release,
    },
    .num_resources = ARRAY_SIZE(led_resource),
    .resource = led_resource,
};

/*
 * @description : 设备模块加载
 * @param : 无
 * @return : 无
 */
static int __init led_device_init(void)
{
    return platform_device_register(&led_device);
}

/*
 * @description : 设备模块注销
 * @param : 无
 * @return : 无
 */

static void __exit led_device_exit(void)
{
    platform_device_unregister(&led_device);
}

module_init(led_device_init);
module_exit(led_device_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
