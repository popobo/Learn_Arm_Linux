#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <poll.h>
#include <signal.h>
#include <linux/ioctl.h>

#define KEY0VALUE 0X01           /* KEY0 按键值 */
#define INVAKEY 0XFF             /* 无效的按键值 */

static int fd = 0;

static void sigio_signal_func(int sig_num)
{
    int err = 0;
    unsigned int key_value = 0;
    err = read(fd, &key_value, sizeof(key_value));
    if (err < 0)
    {
        printf("read error\n");
    }
    else
    {
        printf("sigio signal = %d! key value = %d\r\n", sig_num, key_value);
    }
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *filename;
    int flags = 0;


    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    filename = argv[1];
    /* 打开驱动 */
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", argv[1]);
        return -1;
    }

    signal(SIGIO, sigio_signal_func);

    fcntl(fd, F_SETOWN, getpid()); /* 将当前进程的进程号告诉给内核 */
    flags = fcntl(fd, F_GETFD); /* 获取当前进程状态 */
    fcntl(fd, F_SETFL, flags | FASYNC); /* 设置进程启用异步通知功能 */

    while(1)
    {
        sleep(2);
    }

    ret = close(fd); /* 关闭驱动 */
    if (ret < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
