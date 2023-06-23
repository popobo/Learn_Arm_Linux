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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define MISCBEEP_NAME "misc_beep" /* 名字 */
#define MISCBEEP_MINOR 144        /* 子设备号 */
#define BEEPOFF 0                 /* 关蜂鸣器 */
#define BEEPON 1                  /* 开蜂鸣器 */

/* misc_beep设备结构体 */
struct misc_beep_dev
{
    dev_t devid;            /* 设备号 */
    struct cdev cdev;       /* cdev */
    struct class *class;    /* 类 */
    struct device *device;  /* 设备 */
    struct device_node *nd; /* 设备节点 */
    int beep_gpio;          /* beep所使用的GPIO编号 */
};

static struct misc_beep_dev misc_beep;

/*
 * @description : 打开设备
 * @param – inode : 传递给驱动的 inode
 * @param - filp : 设备文件， file 结构体有个叫做 private_data 的成员变量
 * 一般在 open 的时候将 private_data 指向设备结构体。
 * @return : 0 成功;其他 失败
 */
static int misc_beep_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &misc_beep; /* 设置私有数据 */
    return 0;
}

/*
 * @description : 向设备写数据
 * @param - filp : 设备文件，表示打开的文件描述符
 * @param - buf : 要写给设备写入的数据
 * @param - cnt : 要写入的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t misc_beep_write(struct file *filp,
                               const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue;
    unsigned char databuf[1];
    unsigned char beep_stat;
    struct misc_beep_dev *dev = filp->private_data;
    retvalue = copy_from_user(databuf, buf, cnt);
    if (retvalue < 0)
    {
        printk("kernel write failed!\r\n");
        return -EFAULT;
    }
    beep_stat = databuf[0]; /* 获取状态值 */
    if (beep_stat == BEEPON)
    {
        gpio_set_value(dev->beep_gpio, 0); /* 打开蜂鸣器 */
    }
    else if (beep_stat == BEEPOFF)
    {
        gpio_set_value(dev->beep_gpio, 1); /* 关闭蜂鸣器 */
    }
    return 0;
}

/* 设备操作函数 */
static struct file_operations misc_beep_fops = {
    .owner = THIS_MODULE,
    .open = misc_beep_open,
    .write = misc_beep_write,
};

/* MISC 设备结构体 */
static struct miscdevice beep_miscdev = {
    .minor = MISCBEEP_MINOR,
    .name = MISCBEEP_NAME,
    .fops = &misc_beep_fops,
};

/*
 * @description : flatform 驱动的 probe 函数，当驱动与
 * 设备匹配以后此函数就会执行
 * @param - dev : platform 设备
 * @return : 0，成功;其他负值,失败
 */
static int misc_beep_probe(struct platform_device *dev)
{
    int ret = 0;
    printk("beep driver and device was matched!\r\n");
    /* 设置 BEEP 所使用的 GPIO */
    /* 1、获取设备节点： beep */
    misc_beep.nd = of_find_node_by_path("/gpiobeep");
    if (misc_beep.nd == NULL)
    {
        printk("beep node not find!\r\n");
        return -EINVAL;
    }
    /* 2、 获取设备树中的 gpio 属性，得到 BEEP 所使用的 BEEP 编号 */
    misc_beep.beep_gpio = of_get_named_gpio(misc_beep.nd, "beep-gpio",
                                            0);
    if (misc_beep.beep_gpio < 0)
    {
        printk("can't get beep-gpio");
        return -EINVAL;
    }
    gpio_request(misc_beep.beep_gpio, "beep");
    ret = gpio_direction_output(misc_beep.beep_gpio, 1);
    if (ret < 0)
    {
        printk("can't set gpio!\r\n");
    }
    /* 一般情况下会注册对应的字符设备，但是这里我们使用 MISC 设备
     * 所以我们不需要自己注册字符设备驱动，只需要注册 misc 设备驱动即可
     */
    ret = misc_register(&beep_miscdev);
    if (ret < 0)
    {
        printk("misc device register failed!\r\n");
        return -EFAULT;
    }
    return 0;
}

/*
 * @description : remove 函数，移除 platform 驱动的时候此函数会执行
 * @param - dev : platform 设备
 * @return : 0，成功;其他负值,失败
 */
static int misc_beep_remove(struct platform_device *dev)
{
    /* 注销设备的时候关闭蜂鸣器 */
    gpio_set_value(misc_beep.beep_gpio, 1);
    gpio_free(misc_beep.beep_gpio);
    /* 注销 misc 设备驱动 */
    misc_deregister(&beep_miscdev);
    return 0;
}

/* 匹配列表 */
static const struct of_device_id beep_of_match[] = {
    {.compatible = "atkalpha-beep"},
    {/* Sentinel */}};

/* platform 驱动结构体 */
static struct platform_driver beep_driver = {
    .driver = {
        .name = "imx6ul-beep",           /* 驱动名字 */
        .of_match_table = beep_of_match, /* 设备树匹配表 */
    },
    .probe = misc_beep_probe,
    .remove = misc_beep_remove,
};

/*
 * @description : 驱动入口函数
 * @param : 无
 * @return : 无
 */
static int __init misc_beep_init(void)
{
    return platform_driver_register(&beep_driver);
}
/*
 * @description : 驱动出口函数
 * @param : 无
 * @return : 无
 */
static void __exit misc_beep_exit(void)
{
    platform_driver_unregister(&beep_driver);
}

module_init(misc_beep_init);
module_exit(misc_beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
