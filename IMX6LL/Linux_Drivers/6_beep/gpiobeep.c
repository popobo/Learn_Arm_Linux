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

#define NEWCHRBEEP_CNT 1          /* 设备号个数 */
#define NEWCHRBEEP_NAME "gpiobeep" /* 设备名字 */
#define BEEPOFF 0                 /* 关灯 */
#define BEEPON 1                  /* 开灯 */

/* gpiobeep设备结构体 */
struct gpiobeep_dev
{
    dev_t devid;            /* 设备号 */
    struct cdev cdev;       /* cdev */
    struct class *class;    /* 类 */
    struct device *device;  /* 设备 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    struct device_node *nd; /* 设备节点 */
    int beep_gpio;           /* beep所使用的gpio编号 */
};

static struct gpiobeep_dev gpiobeep;

static int beep_open(struct inode *inode, struct file *fp)
{
    fp->private_data = (void *)&gpiobeep; /* 设置私有数据 */
    return 0;
}

static ssize_t beep_read(struct file *fp, char __user *buf, size_t cnt, loff_t *off)
{
    return 0;
}

static ssize_t beep_write(struct file *fp, const char __user *buf, size_t cnt, loff_t *off)
{
    int ret;
    u8 data_buf[1];
    u8 beep_state;
    struct gpiobeep_dev *dev = fp->private_data;

    ret = copy_from_user(data_buf, buf, cnt);
    if (ret < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }

    beep_state = data_buf[0];
    if (beep_state != BEEPON && beep_state != BEEPOFF)
    {
        printk("wrong beep state!\r\n");
        return -EPERM;
    }

    gpio_set_value(dev->beep_gpio, beep_state);
    return 0;
}

static int beep_release(struct inode *ip, struct file *fp)
{
    return 0;
}

static struct file_operations gpiobeep_fops =
    {
        .owner = THIS_MODULE,
        .open = beep_open,
        .release = beep_release,
        .read = beep_read,
        .write = beep_write,
};

static int __init beep_init(void)
{
    int ret = 0;
    /* 设置beep所使用的GPIO */
    /* 1.获取设备节点: gpiobeep */
    gpiobeep.nd = of_find_node_by_path("/gpiobeep");
    if (gpiobeep.nd == NULL)
    {
        printk("gpiobeep node cant not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("gpiobeep node has been found!\r\n");
    }

    /* 2.获取设备树中的gpio属性，得到beep所使用的GPIO编号 */
    gpiobeep.beep_gpio = of_get_named_gpio(gpiobeep.nd, "beep-gpio", 0);
    if (gpiobeep.beep_gpio < 0)
    {
        printk("can't get beep-gpio");
        return -EINVAL;
    }
    printk("beep-gpio num = %d\r\n", gpiobeep.beep_gpio);

    /* 3.设置GPIO1_IO03为输出，并且输出高电平，默认关闭beep */
    ret = gpio_direction_output(gpiobeep.beep_gpio, 1);
    if (ret < 0)
    {
        printk("can't set gpio!\r\n");
    }

    /* 注册字符设备驱动 */
    /* 1.创建设备号 */
    if (gpiobeep.major)
    {
        gpiobeep.devid = MKDEV(gpiobeep.major, 0);
        register_chrdev_region(gpiobeep.devid, NEWCHRBEEP_CNT, NEWCHRBEEP_NAME);
    }
    else
    {
        alloc_chrdev_region(&gpiobeep.devid, 0, NEWCHRBEEP_CNT, NEWCHRBEEP_NAME);
        gpiobeep.major = MAJOR(gpiobeep.devid);
        gpiobeep.minor = MINOR(gpiobeep.minor);
    }

    printk("gpiobeep.major: %d, gpiobeep.minor: %d\r\n", gpiobeep.major, gpiobeep.minor);

    /* 2.初始化cdev */
    gpiobeep.cdev.owner = THIS_MODULE;
    cdev_init(&gpiobeep.cdev, &gpiobeep_fops);

    /* 3.添加一个cdev */
    cdev_add(&gpiobeep.cdev, gpiobeep.devid, NEWCHRBEEP_CNT);

    /* 4.创建类 */
    gpiobeep.class = class_create(THIS_MODULE, NEWCHRBEEP_NAME);
    if (IS_ERR(gpiobeep.class))
    {
        return PTR_ERR(gpiobeep.class);
    }

    /* 5.创建设备 */
    gpiobeep.device = device_create(gpiobeep.class, NULL, gpiobeep.devid, NULL, NEWCHRBEEP_NAME);
    if (IS_ERR(gpiobeep.device))
    {
        return PTR_ERR(gpiobeep.device);
    }

    return 0;
}

static void __exit beep_exit(void)
{
    /* 注销字符设备 */
    cdev_del(&gpiobeep.cdev); /* 删除cdev */
    unregister_chrdev_region(gpiobeep.devid, NEWCHRBEEP_CNT);

    device_destroy(gpiobeep.class, gpiobeep.devid);
    class_destroy(gpiobeep.class);
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
