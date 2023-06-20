#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
#include <sys/ioctl.h>

#define CLOSE_CMD (_IO(0xEF, 0x1))           /* 关闭定时器 */
#define OPEN_CMD (_IO(0xEF, 0x2))            /* 打开定时器 */
#define SETPERIOD_CMD (_IOW(0xEF, 0x3, int)) /* 设置定时器周期 */

int main(int argc, char *argv[])
{
    int fd = 0;
    int ret = 0;
    char *filename = NULL;
    unsigned int cmd = 0;
    unsigned int arg = 0;
    unsigned char str[128] = {};

    if (argc != 2)
    {
        printf("Error Usage!\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\n", filename);
        return -1;
    }

    while (1)
    {
        printf("Input cmd:");
        ret = scanf("%d", &cmd);
        if (ret != 1)
        {
            gets(str); /* 防止卡死 */
        }

        switch (cmd)
        {
        case 1:
            cmd = CLOSE_CMD;
            break;
        case 2:
            cmd = OPEN_CMD;
            break;
        case 3:
            cmd = SETPERIOD_CMD;
            printf("Input timer period:");
            ret = scanf("%d", &arg);
            if (ret != 1)
            {
                gets(str);
            }
            break;
        default:
            break;
        }
        ret = ioctl(fd, cmd, &arg);
        if (ret < 0)
        {
            break;
        }
    }

    close(fd);
}
