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
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/poll.h>

#define IMX6UIRQ_CNT 1          /* 设备号个数 */
#define IMX6UIRQ_NAME "unblockio" /* 名字 */
#define KEY0VALUE 0X01          /* KEY0 按键值 */
#define INVAKEY 0XFF            /* 无效的按键值 */
#define KEY_NUM 1               /* 按键数量 */

/* 中断IO描述结构体 */
struct irq_keydesc
{
    int gpio;                            /* gpio */
    int irq_num;                         /* 中断号 */
    unsigned char value;                 /* 按键对应值 */
    char name[10];                       /* 名字 */
    irqreturn_t (*handler)(int, void *); /* 中断服务函数 */
};

/* imx6uirq设备结构体 */
struct imx6uirq_dev
{
    dev_t devid;                              /* 设备号 */
    struct cdev cdev;                         /* cdev */
    struct class *class;                      /* 类 */
    struct device *device;                    /* 设备 */
    int major;                                /* 主设备号 */
    int minor;                                /* 次设备号 */
    struct device_node *nd;                   /* 设备节点 */
    atomic_t key_value;                       /* 有效的按键值 */
    atomic_t release_key;                     /* 标记是否完成一次完整的按键 */
    struct timer_list timer;                  /* 定义一个定时器 */
    struct irq_keydesc irq_key_desc[KEY_NUM]; /* 按键描述数组 */
    unsigned char cur_key_num;                /* 当前的按键号 */
    wait_queue_head_t r_wait;                 /* 读等待队列头 */
};

static struct imx6uirq_dev imx6uirq;

static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)dev_id;

    dev->cur_key_num = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10)); /* 通过10ms后定时器读取IO值消抖 */
    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long arg)
{
    unsigned char value = 0;
    unsigned char num = 0;
    struct irq_keydesc *keydesc = NULL;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)arg;

    num = dev->cur_key_num;
    keydesc = &dev->irq_key_desc[num];
    value = gpio_get_value(keydesc->gpio); /* 读取IO值 */
    if (value == 0)                        /* 按键按下 */
    {
        atomic_set(&dev->key_value, keydesc->value);
    }
    else /* 按键松开 */
    {
        atomic_set(&dev->key_value, 0x80 | keydesc->value);
        atomic_set(&dev->release_key, 1); /* 标记松开按键 */
    }

    /* 唤醒进程 */
    if (atomic_read(&dev->release_key)) /* 完成一次按键过程 */
    {
        /* wake_up(&dev->r_wait); */
        /* wake_up_interruptible只能唤醒处于TASK_INTERRUPTIBLE的进程 */
        wake_up_interruptible(&dev->r_wait);
    }
}

static int single_key_io_init(struct imx6uirq_dev *dev, int index)
{
    int ret = 0;

    if (index >= KEY_NUM)
        return -EFAULT;

    /* 设置key所使用的GPIO */
    /* 1.获取设备节点: /key */
    dev->nd = of_find_node_by_path("/key");
    if (dev->nd == NULL)
    {
        ret = -EINVAL;
        printk("fail to of_find_node_by_path\r\n");
        goto fail_find_node;
    }

    /* 2.获取设备树中的gpio属性，得到key所使用的GPIO编号 */
    dev->irq_key_desc[index].gpio = of_get_named_gpio(dev->nd, "key-gpio", index);
    if (dev->irq_key_desc[index].gpio < 0)
    {
        ret = -EINVAL;
        printk("fail to of_get_named_gpio\r\n");
        goto fail_get_gpio;
    }

    memset(imx6uirq.irq_key_desc[index].name, 0, sizeof(imx6uirq.irq_key_desc[index].name));
    sprintf(imx6uirq.irq_key_desc[index].name, "KEY%d", index);

    /* 初始化key所使用的IO */
    ret = gpio_request(dev->irq_key_desc[index].gpio, imx6uirq.irq_key_desc[index].name);
    if (ret < 0)
    {
        ret = -EBUSY;
        printk("fail to gpio_request %d\r\n", dev->irq_key_desc[index].gpio);
        goto fail_request;
    }

    /* 3.设置GPIO1_IO18为输入  */
    ret = gpio_direction_input(dev->irq_key_desc[index].gpio);
    if (ret < 0)
    {
        ret = -EBUSY;
        printk("fail to gpio_request\r\n");
        goto fail_direction;
    }

    dev->irq_key_desc[index].irq_num = irq_of_parse_and_map(dev->nd, index);

    printk("key%d:gpio=%d, irq_num=%d\r\n", index, dev->irq_key_desc[index].gpio, dev->irq_key_desc[index].irq_num);

    /* 申请中断 */
    if (index == 0)
    {
        dev->irq_key_desc[index].handler = key0_handler;
        dev->irq_key_desc[index].value = KEY0VALUE;
    }

    ret = request_irq(dev->irq_key_desc[index].irq_num,
                      dev->irq_key_desc[index].handler,
                      IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                      dev->irq_key_desc[index].name, &imx6uirq);

    if (ret < 0)
    {
        printk("irq %d request failed!\r\n", dev->irq_key_desc[index].irq_num);
        goto fail_irq;
    }

    return 0;

fail_irq:
fail_direction:
    gpio_free(dev->irq_key_desc[index].gpio);
fail_request:
fail_find_node:
fail_get_gpio:
    return ret;
}

static int key_io_init(struct imx6uirq_dev *dev)
{
    int ret = 0;
    int i = 0;
    for (i = 0; i < KEY_NUM; ++i)
    {
        ret = single_key_io_init(dev, i);
        if (ret < 0)
        {
            return ret;
        }
    }
    return 0;
}

static int imx6uirq_open(struct inode *nd, struct file *fp)
{
    fp->private_data = (void *)&imx6uirq;
    return 0;
}

static int imx6uirq_read(struct file *fp, char __user *buf, size_t cnt, loff_t *off)
{
    int ret = 0;
    unsigned char key_value = 0;
    unsigned char release_key = 0;
    struct imx6uirq_dev *dev = (struct imx6uirq_dev *)fp->private_data;
    DECLARE_WAITQUEUE(wait, current);

    if (fp->f_flags | O_NONBLOCK) /* 非阻塞访问 */
    {
        if (atomic_read(&dev->release_key) == 0) /* 没有按键按下 */
        {
            return -EAGAIN;
        }
    }
    else
    {

    #if 0
        ret = wait_event_interruptible(dev->r_wait, atomic_read(&dev->release_key));
        if (ret)
        {
            goto wait_error;
        }
    #endif
        /* 定义一个等待队列 */
        if (atomic_read(&dev->release_key) == 0) /* 没有按键按下 */
        {
            add_wait_queue(&dev->r_wait, &wait);     /* 添加等待队列头 */
            __set_current_state(TASK_INTERRUPTIBLE); /* 设置任务（进程）状态 */
            schedule();                              /* 进行一次任务切换 */
            if (signal_pending(current))             /* 判断是否为信号引起的唤醒 */
            {
                ret = -ERESTARTSYS;
                goto wait_error;
            }
            __set_current_state(TASK_RUNNING);      /* 设置为运行态 */
            remove_wait_queue(&dev->r_wait, &wait); /* 将等待队列移除 */
        }
    }

    key_value = atomic_read(&dev->key_value);
    release_key = atomic_read(&dev->release_key);
    if (release_key) /* 有按键按下 */
    {
        if (key_value & 0x80) /* 判断是否是有效值 */
        {
            key_value &= ~0x80;
            ret = copy_to_user(buf, &key_value, sizeof(key_value));
        }
        else
        {
            goto data_error;
        }
        atomic_set(&dev->release_key, 0); /* 按下标志清0 */
    }
    else
    {
        goto data_error;
    }

    return ret;
wait_error:
    set_current_state(TASK_RUNNING); /* 设置任务为运行态 */
    remove_wait_queue(&dev->r_wait, &wait);

data_error:
    return -EINVAL;
}

unsigned int imx6uirq_poll(struct file* fp, struct poll_table_struct* wait)
{
    unsigned int mask = 0;
    struct imx6uirq_dev* dev = (struct imx6uirq_dev*)fp->private_data;
    poll_wait(fp, &dev->r_wait, wait);

    if (atomic_read(&dev->release_key)) /* 按键按下 */
    {
        mask = POLLIN | POLLRDNORM; /* 返回PLLIN */
    }
    return mask;
}

static struct file_operations imx6uirq_fops =
    {
        .owner = THIS_MODULE,
        .open = imx6uirq_open,
        .read = imx6uirq_read,
        .poll = imx6uirq_poll,
};

static int __init imx6uirq_init(void)
{
    int ret = 0;

    /* 注册字符设备驱动 */
    /* 1、创建设备号 */
    if (imx6uirq.major)
    { /* 定义了设备号 */
        imx6uirq.devid = MKDEV(imx6uirq.major, 0);
        ret = register_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
    }
    else
    { /* 没有定义设备号 */
        ret = alloc_chrdev_region(&imx6uirq.devid, 0, IMX6UIRQ_CNT, IMX6UIRQ_NAME);
        imx6uirq.major = MAJOR(imx6uirq.devid); /* 获取分配号的主设备号 */
        imx6uirq.minor = MINOR(imx6uirq.devid); /* 获取分配号的次设备号 */
    }
    if (ret < 0)
        goto fail_devid;

    /* 2、初始化 cdev */
    imx6uirq.cdev.owner = THIS_MODULE;
    cdev_init(&imx6uirq.cdev, &imx6uirq_fops);
    /* 3、添加一个 cdev */
    ret = cdev_add(&imx6uirq.cdev, imx6uirq.devid, IMX6UIRQ_CNT);
    if (ret < 0)
        goto fail_cdev_add;
    /* 4、创建类 */
    imx6uirq.class = class_create(THIS_MODULE, IMX6UIRQ_NAME);
    if (IS_ERR(imx6uirq.class))
    {
        ret = PTR_ERR(imx6uirq.class);
        goto fail_class_create;
    }
    /* 5、创建设备 */
    imx6uirq.device = device_create(imx6uirq.class, NULL, imx6uirq.devid,
                                    NULL, IMX6UIRQ_NAME);
    if (IS_ERR(imx6uirq.device))
    {
        ret = PTR_ERR(imx6uirq.device);
        goto fail_device_create;
    }

    /* 初始化按键 */
    atomic_set(&imx6uirq.key_value, INVAKEY);
    atomic_set(&imx6uirq.release_key, 0);
    /* 初始化key io */
    ret = key_io_init(&imx6uirq);
    if (ret < 0)
    {
        goto fail_io_init;
    }

    /* 创建定时器 */
    init_timer(&imx6uirq.timer);
    imx6uirq.timer.function = timer_function;

    /* 初始化等待队列头 */
    init_waitqueue_head(&imx6uirq.r_wait);

    return 0;

fail_io_init:
fail_device_create:
    class_destroy(imx6uirq.class);
fail_class_create:
    cdev_del(&imx6uirq.cdev);
fail_cdev_add:
    unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
fail_devid:
    return ret;
}

static void __exit imx6uirq_exit(void)
{
    int i = 0;
    del_timer_sync(&imx6uirq.timer);

    /* 释放中断 */
    for (i = 0; i < KEY_NUM; ++i)
    {
        free_irq(imx6uirq.irq_key_desc[i].irq_num, &imx6uirq);
        gpio_free(imx6uirq.irq_key_desc[i].gpio);
    }
    cdev_del(&imx6uirq.cdev);
    unregister_chrdev_region(imx6uirq.devid, IMX6UIRQ_CNT);
    device_destroy(imx6uirq.class, imx6uirq.devid);
    class_destroy(imx6uirq.class);
}

module_init(imx6uirq_init);
module_exit(imx6uirq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
