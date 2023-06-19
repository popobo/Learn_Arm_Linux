#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define KEY0VALUE 0XF0 /* 按键值 */
#define INVAKEY 0X00   /* 无效的按键值 */

int main(int argc, char *argv[])
{
    int fd, retvalue;
    char *filename;
    unsigned char databuf[1];
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

    while (1)
    {
        read(fd, databuf, sizeof(databuf));
        if (databuf[0] == KEY0VALUE)
        {
            printf("get key value\n");
        }
    }

    retvalue = close(fd); /* 关闭驱动 */
    if (retvalue < 0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        return -1;
    }
    return 0;
}
