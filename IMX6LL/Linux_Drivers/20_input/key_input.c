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
#include <linux/input.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define KEY_INPUT_CNT 1            /* 设备号个数 */
#define KEY_INPUT_NAME "key_input" /* 名字 */
#define KEY0VALUE 0X01             /* KEY0 按键值 */
#define INVAKEY 0XFF               /* 无效的按键值 */
#define KEY_NUM 1                  /* 按键数量 */

/* 中断IO描述结构体 */
struct irq_key_desc
{
    int gpio;                            /* gpio */
    int irq_num;                         /* 中断号 */
    unsigned char value;                 /* 按键对应值 */
    char name[10];                       /* 名字 */
    irqreturn_t (*handler)(int, void *); /* 中断服务函数 */
};

/* key_input设备结构体 */
struct key_input_dev
{
    dev_t devid;                               /* 设备号 */
    struct cdev cdev;                          /* cdev */
    struct class *class;                       /* 类 */
    struct device *device;                     /* 设备 */
    int major;                                 /* 主设备号 */
    int minor;                                 /* 次设备号 */
    struct device_node *nd;                    /* 设备节点 */
    atomic_t release_key;                      /* 标记是否完成一次完整的按键 */
    struct timer_list timer;                   /* 定义一个定时器 */
    struct irq_key_desc irq_key_desc[KEY_NUM]; /* 按键描述数组 */
    unsigned char cur_key_num;                 /* 当前的按键号 */
    struct input_dev *input_dev;               /* input结构体 */
};

static struct key_input_dev key_input;

static irqreturn_t key0_handler(int irq, void *dev_id)
{
    struct key_input_dev *dev = (struct key_input_dev *)dev_id;

    dev->cur_key_num = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10)); /* 通过10ms后定时器读取IO值消抖 */
    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_function(unsigned long arg)
{
    unsigned char value = 0;
    unsigned char num = 0;
    struct irq_key_desc *key_desc = NULL;
    struct key_input_dev *dev = (struct key_input_dev *)arg;
    printk("%s : %d\n", __FILE__, __LINE__);
    num = dev->cur_key_num;
    key_desc = &dev->irq_key_desc[num];
    value = gpio_get_value(key_desc->gpio); /* 读取IO值 */
    if (value == 0)                         /* 按键按下 */
    {
        /* 上报按键值 */
        // input_event(dev->inputdev, EV_KEY, keydesc->value, 1);
        input_report_key(dev->input_dev, key_desc->value, 1);
        input_sync(dev->input_dev);
        printk("%s : %d\n", __FILE__, __LINE__);
    }
    else /* 按键松开 */
    {
        // input_event(dev->inputdev, EV_KEY, keydesc->value, 0);
        input_report_key(dev->input_dev, key_desc->value, 0);
        input_sync(dev->input_dev);
        printk("%s : %d\n", __FILE__, __LINE__);
    }
}

static int single_key_io_init(struct key_input_dev *dev, int index)
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

    memset(key_input.irq_key_desc[index].name, 0, sizeof(key_input.irq_key_desc[index].name));
    sprintf(key_input.irq_key_desc[index].name, "KEY%d", index);

    /* 初始化key所使用的IO */
    ret = gpio_request(dev->irq_key_desc[index].gpio, key_input.irq_key_desc[index].name);
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
        dev->irq_key_desc[index].value = KEY_0;
    }

    ret = request_irq(dev->irq_key_desc[index].irq_num,
                      dev->irq_key_desc[index].handler,
                      IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                      dev->irq_key_desc[index].name, &key_input);

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

static int key_io_init(struct key_input_dev *dev)
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

static int __init key_input_init(void)
{
    int ret = 0;
    printk("key_input_init\n");
    /* 初始化按键 */
    atomic_set(&key_input.release_key, 0);
    /* 初始化key io */
    ret = key_io_init(&key_input);
    if (ret < 0)
    {
        return ret;
    }

    /* 创建定时器 */
    init_timer(&key_input.timer);
    key_input.timer.function = timer_function;

    /* 申请input_dev */
    key_input.input_dev = input_allocate_device();
    key_input.input_dev->name = KEY_INPUT_NAME;

#if 0    
    __set_bit(EV_KEY, key_input.input_dev->evbit); /* 按键事件 */
    __set_bit(EV_REP, key_input.input_dev->evbit); /* 重复事件 */
    
    /* 初始化input_dev，设置产生哪些按键 */
    __set_bit(KEY_0, key_input.input_dev->keybit);
#endif


    key_input.input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    key_input.input_dev->keybit[BIT_WORD(KEY_0)] |= BIT_MASK(KEY_0);

#if 0
    key_input.input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    input_set_capability(key_input.input_dev, EV_KEY, KEY_0);
#endif

    /* 注册输入设备 */
    ret = input_register_device(key_input.input_dev);
    if (ret)
    {
        printk("register input device failed!\r\n");
        return ret;
    }

    return 0;
}

static void __exit key_input_exit(void)
{
    int i = 0;
    printk("key_input_exit\n");
    del_timer_sync(&key_input.timer);

    /* 释放中断 */
    for (i = 0; i < KEY_NUM; ++i)
    {
        free_irq(key_input.irq_key_desc[i].irq_num, &key_input);
        gpio_free(key_input.irq_key_desc[i].gpio);
    }

    /* 释放input_dev */
    input_unregister_device(key_input.input_dev);
    input_free_device(key_input.input_dev);
}

module_init(key_input_init);
module_exit(key_input_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
