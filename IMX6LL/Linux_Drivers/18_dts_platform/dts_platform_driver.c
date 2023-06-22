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

#define LED_DEV_CNT 1               /* 设备号长度 */
#define LED_DEV_NAME "dts_plat_led" /* 设备名字 */
#define LED_OFF 0
#define LED_ON 1

/* led_dev设备结构体 */
struct led_dev_dev
{
    dev_t devid;              /* 设备号 */
    struct cdev cdev;         /* cdev */
    struct class *class;      /* 类 */
    struct device *device;    /* 设备 */
    int major;                /* 主设备号 */
    struct device_node *node; /* LED设备节点 */
    int led0;                 /* LED灯GPIO标号 */
};

struct led_dev_dev led_dev; /* led设备 */

/*
 * @description : LED 打开/关闭
 * @param - sta : LEDON(0) 打开 LED， LEDOFF(1) 关闭 LED
 * @return : 无
 */

void led0_switch(struct led_dev_dev *dev, u8 status)
{
    if (LED_ON == status)
    {
        gpio_set_value(dev->led0, 0);
    }
    else if (LED_OFF == status)
    {
        gpio_set_value(dev->led0, 1);
    }
}

/*
 * @description : 打开设备
 * @param – inode : 传递给驱动的 inode
 * @param - filp : 设备文件， file 结构体有个叫做 private_data 的成员变量
 * 一般在 open 的时候将 private_data 指向设备结构体。
 * @return : 0 成功;其他 失败
 */
static int led_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &led_dev; /* 设置私有数据 */
    return 0;
}

/*
 * @description : 向设备写数据
 * @param – filp : 设备文件，表示打开的文件描述符
 * @param - buf : 要写给设备写入的数据
 * @param - cnt : 要写入的数据长度
 * @param - offt : 相对于文件首地址的偏移
 * @return : 写入的字节数，如果为负值，表示写入失败
 */
static ssize_t led_write(struct file *fp, const char __user *buf, size_t cnt, loff_t *off)
{
    int ret = 0;
    unsigned char data_buf[1];
    unsigned char led_status = 0;
    struct led_dev_dev *dev = (struct led_dev_dev *)fp->private_data;
    BUG_ON(cnt != 1);
    ret = copy_from_user(data_buf, buf, cnt);
    if (ret < 0)
    {
        return -EFAULT;
    }

    led_status = data_buf[0];
    if (led_status != LED_ON && led_status != LED_OFF)
    {
        return -EFAULT;
    }

    led0_switch(dev, led_status);

    return 0;
}

/* 设备操作函数 */
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};

/**
 * @brief
 * @param
 * @retval
 */

static int led_probe(struct platform_device *dev)
{
    printk("led drive and device has matched!\r\n");

    /* 注册字符设备驱动 */
    /*1、创建设备号 */
    if (led_dev.major)
    { /* 定义了设备号 */
        led_dev.devid = MKDEV(led_dev.major, 0);
        register_chrdev_region(led_dev.devid, LED_DEV_CNT,
                               LED_DEV_NAME);
    }
    else
    { /* 没有定义设备号 */
        alloc_chrdev_region(&led_dev.devid, 0, LED_DEV_CNT,
                            LED_DEV_NAME);
        led_dev.major = MAJOR(led_dev.devid);
    }

    /* 2、初始化 cdev */
    led_dev.cdev.owner = THIS_MODULE;
    cdev_init(&led_dev.cdev, &led_fops);
    /* 3、添加一个 cdev */
    cdev_add(&led_dev.cdev, led_dev.devid, LED_DEV_CNT);
    /* 4、创建类 */
    led_dev.class = class_create(THIS_MODULE, LED_DEV_NAME);
    if (IS_ERR(led_dev.class))
    {
        return PTR_ERR(led_dev.class);
    }
    /* 5、创建设备 */
    led_dev.device = device_create(led_dev.class, NULL, led_dev.devid, NULL, LED_DEV_NAME);
    if (IS_ERR(led_dev.device))
    {
        return PTR_ERR(led_dev.device);
    }

    /* 5、初始化 IO */
    led_dev.node = of_find_node_by_path("/gpioled");
    if (led_dev.node == NULL)
    {
        printk("gpioled node nost find!\r\n");
        return -EINVAL;
    }
    led_dev.led0 = of_get_ named_gpio(led_dev.node, "led-gpio", 0);
    if (led_dev.led0 < 0)
    {
        printk("can't get led-gpio\r\n");
        return -EINVAL;
    }

    gpio_request(led_dev.led0, "led0");
    gpio_direction_output(led_dev.led0, 1);

    return 0;
}

/*
 * @description :移除 platform 驱动的时候此函数会执行
 * @param - dev : platform 设备
 * @return : 0，成功;其他负值,失败
 */
static int led_remove(struct platform_device *dev)
{
    gpio_set_value(led_dev.led0, 1); /* 卸载驱动的时候关闭 LED */

    cdev_del(&led_dev.cdev); /* 删除 cdev */
    unregister_chrdev_region(led_dev.devid, LED_DEV_CNT);
    device_destroy(led_dev.class, led_dev.devid);
    class_destroy(led_dev.class);
    return 0;
}

/* 匹配列表 */
static const struct of_device_id led_of_match[] = {
    {.compatible = "atkalpha-gpioled"},
    {/* Sentinel */}};

/* platform驱动结构体 */
static struct platform_driver led_driver = {
    .driver = {
        .name = "imx6ul-led",           /* 驱动名字，用于和设备匹配 */
        .of_match_table = led_of_match, /* 设备树匹配表 */
    },
    .probe = led_probe,
    .remove = led_remove,
};

/*
 * @description : 驱动模块加载函数
 * @param : 无
 * @return : 无
 */
static int __init led_driver_init(void)
{
    return platform_driver_register(&led_driver);
}

/*
 * @description : 驱动模块卸载函数
 * @param : 无
 * @return : 无
 */
static void __exit led_driver_exit(void)
{
    return platform_driver_unregister(&led_driver);
}

module_init(led_driver_init);
module_exit(led_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
