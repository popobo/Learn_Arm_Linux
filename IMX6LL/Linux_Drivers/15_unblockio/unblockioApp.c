#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <poll.h>

#define KEY0VALUE 0X01           /* KEY0 按键值 */
#define INVAKEY 0XFF             /* 无效的按键值 */

int main(int argc, char *argv[])
{
    int fd, ret;
    char *filename;
    unsigned char databuf[1];
    struct pollfd fds;
    struct timeval timeout;

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

    fds.fd = fd;
    fds.events = POLLIN;

    while(1)
    {
        ret = poll(&fds, 1, 500);
        if (ret)
        {
            ret = read(fd, databuf, sizeof(databuf));
            if (ret < 0)
            {
                /* 读取错误 */
                printf("read error %d\n", ret);
            }
            else
            {
                if (databuf[0] == KEY0VALUE)
                {
                    printf("key value = %d\r\n", databuf[0]);
                }
            }
        }
        else if (ret == 0) /* 超时 */
        {
            printf("time out\r\n");
        }
        else if (ret < 0)
        {
            printf("other error %d\n", ret);
        }
    }

    ret = close(fd); /* 关闭驱动 */
    if (ret < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
