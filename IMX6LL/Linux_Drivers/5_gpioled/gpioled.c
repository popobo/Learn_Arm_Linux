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
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define NEWCHRLED_CNT 1            /* 设备号个数 */
#define NEWCHRLED_NAME "dtsled" /* 设备名字 */
#define LEDOFF 0                   /* 关灯 */
#define LEDON 1                    /* 开灯 */
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

/* dtsled设备结构体 */
struct dtsled_dev
{
    dev_t devid;            /* 设备号 */
    struct cdev cdev;       /* cdev */
    struct class *class;    /* 类 */
    struct device *device;  /* 设备 */
    int major;              /* 主设备号 */
    int minor;              /* 次设备号 */
    struct device_node *nd; /* 设备节点 */
};

static struct dtsled_dev dtsled;

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
    fp->private_data = (void *)&dtsled; /* 设置私有数据 */
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

static struct file_operations dtsled_fops =
    {
        .owner = THIS_MODULE,
        .open = led_open,
        .release = led_release,
        .read = led_read,
        .write = led_write,
};

static int __init led_init(void)
{
    u32 val = 0;
    int ret = 0;
    u32 reg_data[14] = {};
    const char *str = NULL;
    struct property *proper;

    /* 获取设备树中的属性数据 */
    /* 1.获取设备节点：alphaled */
    dtsled.nd = of_find_node_by_path("/alphaled");
    if (dtsled.nd == NULL)
    {
        printk("alphaled node not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("alphaled node not found!\r\n");
    }

    /* 2.获取compatible内容数据 */
    proper = of_find_property(dtsled.nd, "compatible", NULL);
    if (proper == NULL)
    {
        printk("of_find_property failed!\r\n");
    }
    else 
    {
        printk("compatible = %s\r\n", (char *)proper->value);
    }
    /* 3.获取status属性内容 */
    ret = of_property_read_string(dtsled.nd, "status", &str);
    if (ret < 0)
    {
        printk("status read failed!\r\n");
    }
    else
    {
        printk("status = %s\r\n", str);
    }

    /* 4.获取reg属性内容 */
    ret = of_property_read_u32_array(dtsled.nd, "reg", reg_data, 10);
    if (ret < 0)
    {
        printk("reg property read failed!\r\n");
    }
    else
    {
        u8 i = 0;
        printk("reg data:\r\n");
        for (i = 0; i < 10; i++)
            printk("%#X ", reg_data[i]);
        printk("\r\n");
    }

/* 初始化 LED */
#if 0
    /* 1、寄存器地址映射 */
        IMX6U_CCM_CCGR1 = ioremap(regdata[0], regdata[1])
        SW_MUX_GPIO1_IO03 = ioremap(regdata[2], regdata[3
        SW_PAD_GPIO1_IO03 = ioremap(regdata[4], regdata[5
        GPIO1_DR = ioremap(regdata[6], regdata[7]);
        GPIO1_GDIR = ioremap(regdata[8], regdata[9]);
#else
    IMX6U_CCM_CCGR1 = of_iomap(dtsled.nd, 0);
    SW_MUX_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
    SW_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
    GPIO1_DR = of_iomap(dtsled.nd, 3);
    GPIO1_GDIR = of_iomap(dtsled.nd, 4);
#endif

    /* 2、使能 GPIO1 时钟 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26); /* 清楚以前的设置 */
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
    
    /* 注册字符设备驱动 */
    /* 1.创建设备号 */
    if (dtsled.major)
    {
        dtsled.devid = MKDEV(dtsled.major, 0);
        register_chrdev_region(dtsled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&dtsled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.minor);
    }

    printk("dtsled.major: %d, dtsled.minor: %d\r\n", dtsled.major, dtsled.minor);

    /* 2.初始化cdev */
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);

    /* 3.添加一个cdev */
    cdev_add(&dtsled.cdev, dtsled.devid, NEWCHRLED_CNT);

    /* 4.创建类 */
    dtsled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if (IS_ERR(dtsled.class))
    {
        return PTR_ERR(dtsled.class);
    }

    /* 5.创建设备 */
    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, NEWCHRLED_NAME);
    if (IS_ERR(dtsled.device))
    {
        return PTR_ERR(dtsled.device);
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

    /* 注销字符设备 */
    cdev_del(&dtsled.cdev); /* 删除cdev */
    unregister_chrdev_region(dtsled.devid, NEWCHRLED_CNT);

    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
}

module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");
