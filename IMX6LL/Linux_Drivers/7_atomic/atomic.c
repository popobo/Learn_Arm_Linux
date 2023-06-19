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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_CNT 1          /* 设备号个数 */
#define NEWCHRLED_NAME "gpioled" /* 设备名字 */
#define LEDOFF 0                 /* 关灯 */
#define LEDON 1                  /* 开灯 */

/* gpioled设备结构体 */
struct gpioled_dev
{
    dev_t devid;            /* 设备号 */
    struct cdev cdev;       /* cdev */
    struct class *class;    /* 类 */
    struct device *device;  /* 设备 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    struct device_node *nd; /* 设备节点 */
    int led_gpio;           /* led所使用的gpio编号 */
    atomic_t lock; /* 原子变量 */
};

static struct gpioled_dev gpioled;

static int led_open(struct inode *inode, struct file *fp)
{
    /* 通过判断原子变量的值来检查LED有没有被其他应用使用 */
    /* atomic_dec_and_test 从 v 减 1，如果结果为 0 就返回真，否则返回假*/
    if (!atomic_dec_and_test(&gpioled.lock))
    {
        /* 结果不为0 */
        atomic_inc(&gpioled.lock);
        return -EBUSY;
    }

    fp->private_data = (void *)&gpioled; /* 设置私有数据 */
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
    struct gpioled_dev *dev = fp->private_data;

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

    gpio_set_value(dev->led_gpio, led_state);
    return 0;
}

static int led_release(struct inode *ip, struct file *fp)
{
    struct gpioled_dev *dev = fp->private_data;

    /* 关闭驱动文件的时候恢复原子变量 */
    atomic_inc(&dev->lock);
    return 0;
}

static struct file_operations gpioled_fops =
    {
        .owner = THIS_MODULE,
        .open = led_open,
        .release = led_release,
        .read = led_read,
        .write = led_write,
};

static int __init led_init(void)
{
    int ret = 0;

    /* 初始化原子变量为1 */
    atomic_set(&gpioled.lock, 1);

    /* 设置LED所使用的GPIO */
    /* 1.获取设备节点: gpioled */
    gpioled.nd = of_find_node_by_path("/gpioled");
    if (gpioled.nd == NULL)
    {
        printk("gpioled node cant not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("gpioled node has been found!\r\n");
    }

    /* 2.获取设备树中的gpio属性，得到led所使用的LED编号 */
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd, "led-gpio", 0);
    if (gpioled.led_gpio < 0)
    {
        printk("can't get led-gpio");
        return -EINVAL;
    }
    printk("led-gpio num = %d\r\n", gpioled.led_gpio);

    /* 3.设置GPIO1_IO03为输出，并且输出高电平，默认关闭LED灯 */
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if (ret < 0)
    {
        printk("can't set gpio!\r\n");
    }

    /* 注册字符设备驱动 */
    /* 1.创建设备号 */
    if (gpioled.major)
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&gpioled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.minor);
    }

    printk("gpioled.major: %d, gpioled.minor: %d\r\n", gpioled.major, gpioled.minor);

    /* 2.初始化cdev */
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    /* 3.添加一个cdev */
    cdev_add(&gpioled.cdev, gpioled.devid, NEWCHRLED_CNT);

    /* 4.创建类 */
    gpioled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        return PTR_ERR(gpioled.class);
    }

    /* 5.创建设备 */
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, NEWCHRLED_NAME);
    if (IS_ERR(gpioled.device))
    {
        return PTR_ERR(gpioled.device);
    }

    return 0;
}

static void __exit led_exit(void)
{
    /* 注销字符设备 */
    cdev_del(&gpioled.cdev); /* 删除cdev */
    unregister_chrdev_region(gpioled.devid, NEWCHRLED_CNT);

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
