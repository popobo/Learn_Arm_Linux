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
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define TIMER_CNT 1                    /* 设备号个数 */
#define TIMER_NAME "timer"             /* 名字 */
#define CLOSE_CMD (_IO(0xEF, 0x1))     /* 关闭定时器 */
#define OPEN_CMD (_IO(0xEF, 0x2))      /* 打开定时器 */
#define SETPERIOD_CMD (_IO(0xEF, 0x3)) /* 设置定时器周期 */
#define LEDON 1                        /* 开灯 */
#define LEDOFF 0                       /* 关灯 */

/* timer设备结构体 */
struct timer_dev
{
    dev_t devid;             /* 设备号 */
    struct cdev cdev;        /* cdev */
    struct class *class;     /* 类 */
    struct device *device;   /* 设备 */
    int major;               /* 主设备号 */
    int minor;               /* 次设备号 */
    struct device_node *nd;  /* 设备节点 */
    int led_gpio;            /* led所使用的gpio编号 */
    int timer_period;        /* 定时周期，单位为ms */
    struct timer_list timer; /* 定义一个定时器 */
};

struct timer_dev timerdev; /* timer设备 */

static int led_init(struct timer_dev* dev)
{
    int ret = 0;
    dev->nd = of_find_node_by_path("/gpioled");
    if (dev->nd == NULL)
    {
        ret = -EINVAL;
        printk("fail to of_find_node_by_path\r\n");
        goto fail_find_node;
    }

    dev->led_gpio = of_get_named_gpio(dev->nd, "led-gpio", 0);
    if (dev->led_gpio < 0)
    {
        ret = -EINVAL;
        printk("fail to of_get_named_gpio\r\n");
        goto fail_get_gpio;
    }

    /* 初始化led所使用的IO */
    ret = gpio_request(dev->led_gpio, "led");
    if (ret < 0)
    {
        ret = -EBUSY;
        printk("fail to gpio_request %d\r\n", dev->led_gpio);
        goto fail_request;
    }

    ret = gpio_direction_output(dev->led_gpio, 1);
    if (ret < 0)
    {
        ret = -EBUSY;
        printk("fail to gpio_request\r\n");
        goto fail_direction;
    }

    return 0;

fail_direction:
    gpio_free(dev->led_gpio);
fail_request:
fail_find_node:
fail_get_gpio:
    return ret;
}

static int led_exit(struct timer_dev* dev)
{
    gpio_free(dev->led_gpio);
    return 0;
}

static int timer_open(struct inode *nd, struct file *fp)
{
    fp->private_data = (void *)&timerdev; /* 设置私有数据 */

    return 0;
}

static int timer_release(struct inode *ip, struct file *fp)
{
    return 0;
}

/* 设备操作函数 */
static struct file_operations timer_fops = {
    .owner = THIS_MODULE,
    .open = timer_open,
    .release = timer_release,
};

/* 定时器回调函数 */
void timer_function(unsigned long arg)
{
    struct timer_dev* dev = (struct timer_dev*) arg;
    static int status = 1;
    
    status = !status;
    gpio_set_value(dev->led_gpio, status);

    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(500));
}

static int __init timer_init(void)
{
    int ret = 0;

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (timerdev.major)
    { /* 定义了设备号 */
        timerdev.devid = MKDEV(timerdev.major, 0);
        ret = register_chrdev_region(timerdev.devid, TIMER_CNT, TIMER_NAME);
    }
    else
    { /* 没有定义设备号 */
        ret = alloc_chrdev_region(&timerdev.devid, 0, TIMER_CNT, TIMER_NAME);
        timerdev.major = MAJOR(timerdev.devid); /* 获取分配号的主设备号 */
        timerdev.minor = MINOR(timerdev.devid); /* 获取分配号的次设备号 */
    }
    if (ret < 0)
        goto fail_devid;

    /* 2、初始化 cdev */
    timerdev.cdev.owner = THIS_MODULE;
    cdev_init(&timerdev.cdev, &timer_fops);
    /* 3、添加一个 cdev */
    ret = cdev_add(&timerdev.cdev, timerdev.devid, TIMER_CNT);
    if (ret < 0)
        goto fail_cdev_add;
    /* 4、创建类 */
    timerdev.class = class_create(THIS_MODULE, TIMER_NAME);
    if (IS_ERR(timerdev.class))
    {
        ret = PTR_ERR(timerdev.class);
        goto fail_class_create;
    }
    /* 5、创建设备 */
    timerdev.device = device_create(timerdev.class, NULL, timerdev.devid,
                                  NULL, TIMER_NAME);
    if (IS_ERR(timerdev.device))
    {
        ret = PTR_ERR(timerdev.device);
        goto fail_device_create;
    }

    /* 初始化led */
    led_init(&timerdev);

    /* 6、初始化timer，设定定时器处理函数，还未设置周期，所以不会激活定时器 */
    init_timer(&timerdev.timer);
    timerdev.timer.function = timer_function;
    timerdev.timer.expires = jiffies + msecs_to_jiffies(500);
    timerdev.timer.data = (unsigned long)&timerdev;
    add_timer(&timerdev.timer);
    
    return 0;

fail_device_create:
    class_destroy(timerdev.class);
fail_class_create:
    cdev_del(&timerdev.cdev);
fail_cdev_add:
    unregister_chrdev_region(timerdev.devid, TIMER_CNT);
fail_devid:
    return ret;
}

static void __exit timer_exit(void)
{
    gpio_set_value(timerdev.led_gpio, 1);

    /* 删除定时器 */
    del_timer_sync(&timerdev.timer);
#if 0
    del_timer(&timerdev.tiemr);
#endif
    /* 注销字符设备驱动 */
    cdev_del(&timerdev.cdev);
    unregister_chrdev_region(timerdev.devid, TIMER_CNT);
    device_destroy(timerdev.class, timerdev.devid);
    class_destroy(timerdev.class);

    led_exit(&timerdev);
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
