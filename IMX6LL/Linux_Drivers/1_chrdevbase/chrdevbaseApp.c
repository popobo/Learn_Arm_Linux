#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char user_data[] = { "user data!" };

int main(int argc, char* argv[])
{
    if (argc !=3 )
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    char* filename = argv[1];
    
    /* 打开驱动文件 */
    int fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("failed to open %s\r\n", filename);
        return -1;
    }

    int ret = 0;
    char read_buf[128] = {};
    char write_buf[128] = {};
    if (atoi(argv[2]) == 1) /* 从驱动中读取数据 */
    {
        ret = read(fd, read_buf, 50);
        if (ret < 0)
        {
            printf("read file %s failed!\r\n", filename);
        }
        else
        {
            printf("read data %s \r\n", read_buf);
        }
    }
    else if (atoi(argv[2]) == 2) /* 向驱动写数据 */
    {
        memcpy(write_buf, user_data, sizeof(user_data));
        ret = write(fd, write_buf, 50);
        if (ret < 0)
        {
            printf("write file %s failed!\r\n", filename);
        }
        else
        {
            printf("write data ok!\r\n");
        }
    }

    /* 关闭设备 */
    ret = close(fd);
    if (ret < 0)
    {
        printf("failed to close file %s\r\n", filename);
        return -1;
    }

    return 0;
}