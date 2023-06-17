#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define CHRDEVBASE_MAJOR 200 /* main device number */
#define CHRDEVBASE_NAME "chrdevbase" /* device name */

#define BUF_SIZE 128

static char read_buf[BUF_SIZE];
static char write_buf[BUF_SIZE];
static char kernel_data[] = { "kernel data!" };

/**
* @brief  打开设备
* @param  inode 传递给设备的inode
* @param  file 设备文件，file结构体有个叫做private_data的成员变量，一般在open的时候将private_data指向设备结构体
* @retval 0成功，其他失败 
*/
static int chrdevbase_open(struct inode* inodep, struct file* filep)
{
    return 0;
}

/**
* @brief  从设备读取数据
* @param  filep 要打开的设备文件
* @param  buf 返回给用户空间的数据缓冲区
* @param  cnt 要读取的数据长度
* @param  offt 相对于文件首地址的偏移
* @retval 读取的字节数，负值表示读取失败
*/
static ssize_t chrdevbase_read(struct file* filep, char __user* buf, size_t cnt, loff_t* offt)
{
    int ret_val = 0;
    
    /* 向用户空间发送数据 */
    memcpy(read_buf, kernel_data, sizeof(kernel_data) / sizeof(kernel_data[0]));
    ret_val = copy_to_user(buf, read_buf, cnt);
    if (ret_val == 0)
    {
        printk("kernel send data ok!\r\n");
    }
    else
    {
        printk("kernel send data failed!\r\n");
    }

    return 0;
}

/**
* @brief  向设备写数据
* @param  filep 设备文件，表示打开的文件描述符
* @param  buf 要给设备写入的数据
* @param  cnt 要写入的数据长度
* @param  offt 相对于文件首地址的偏移
* @retval 写入的字节数，负值表示写入失败
*/
static ssize_t chrdevbase_write(struct file* filep, const char __user* buf, size_t cnt, loff_t* offt)
{
    int ret_val = 0;
    /* 接收用户空间传递给内核的数据并打印出来 */
    ret_val = copy_from_user(write_buf, buf, cnt);
    if (ret_val == 0)
    {
        printk("kernel receive data ok!\r\n");
    }
    else
    {
        printk("kernel receive data failed!\r\n");
    }

    return 0;
}

/**
* @brief  关闭/释放设备
* @param  flip 要关闭的设备文件(文件描述符)
* @retval 0成功，其他失败
*/
static int chrdevbase_release(struct inode* inodep, struct file* filep)
{
    return 0;   
}

static struct file_operations chrdevbase_fops = 
{
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
    .release = chrdevbase_release,
};


/**
* @brief  驱动入口函数
* @param  无
* @retval 0成功，其他失败
*/
static int __init chrdevbase_init(void)
{
    int ret_val = 0;
    /* 注册字符设备驱动 */
    ret_val = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if (ret_val < 0)
    {
        printk("chrdevbase driver register failed!\r\n");
    }

    printk("chrdevbase_init()\r\n");
    
    return 0;
}

static void __exit chrdevbase_exit(void)
{
    /* 注销字符驱动设备 */
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase_exit()\r\n");
}

/*
 * 将上面两个函数指定为驱动的入口和出口函数 
 */
module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

/* 
 * LICENSE和作者信息
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("bo");