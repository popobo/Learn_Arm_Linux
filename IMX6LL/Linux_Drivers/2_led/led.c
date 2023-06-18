#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define LED_MAJOR 200  /* 主设备号 */
#define LED_NAME "led" /* 设备名字 */
#define LEDOFF 0       /* 关灯 */
#define LEDON 1        /* 开灯 */
/* 寄存器物理地址 */
#define CCM_CCGR1_BASE (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE (0X020E02F4)
#define GPIO1_DR_BASE (0X0209C000)
#define GPIO1_GDIR_BASE (0X0209C004)

static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

static void led_switch(u8 state)
{
    u32 val = 0;
    if (state == LEDON)
    {
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }
    else if (state == LEDOFF)
    {
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val, GPIO1_DR);
    }
}

static int led_open(struct inode *inode, struct file *fp)
{
    return 0;
}

static ssize_t led_read(struct file *fp, char __user *buf, size_t cnt, loff_t *off)
{
    return 0;
}

static ssize_t led_write(struct file *fp, const char __user *buf, size_t cnt, loff_t *off)
{
    int ret;
    u8 data_buf[1];
    u8 led_state;

    ret = copy_from_user(data_buf, buf, cnt);
    if (ret < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    led_state = data_buf[0];
    if (led_state != LEDON && led_state != LEDOFF)
    {
        printk("wrong led state!\r\n");
        return -EPERM;
    }

    led_switch(led_state);
    return 0;
}

static int led_release(struct inode *ip, struct file *fp)
{
    return 0;
}

static struct file_operations led_fops =
    {
        .owner = THIS_MODULE,
        .open = led_open,
        .release = led_release,
        .read = led_read,
        .write = led_write,
};

static int __init led_init(void)
{
    int retvalue = 0;
    u32 val = 0;
    /* 初始化 LED */
    /* 1、寄存器地址映射 */
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);
    /* 2、使能 GPIO1 时钟 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26); /* 清除以前的设置 */
    val |= (3 << 26);  /* 设置新值 */
    writel(val, IMX6U_CCM_CCGR1);
    /* 3、设置 GPIO1_IO03 的复用功能，将其复用为
     * GPIO1_IO03，最后设置 IO 属性。
     */
    writel(5, SW_MUX_GPIO1_IO03);
    /* 寄存器 SW_PAD_GPIO1_IO03 设置 IO 属性 */
    writel(0x10B0, SW_PAD_GPIO1_IO03);
    /* 4、设置 GPIO1_IO03 为输出功能 */
    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3); /* 清除以前的设置 */
    val |= (1 << 3);  /* 设置为输出 */

    writel(val, GPIO1_GDIR);
    /* 5、默认关闭 LED */
    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val, GPIO1_DR);
    /* 6、注册字符设备驱动 */
    retvalue = register_chrdev(LED_MAJOR, LED_NAME, &led_fops);
    if (retvalue < 0)
    {
        printk("register chrdev failed!\r\n");
        return -EIO;
    }
    return 0;
}

static void __exit led_exit(void)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
