#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/input.h>

/* 定义一个 input_event 变量，存放输入事件信息 */
static struct input_event input_event;

/*
 * @description : main 主程序
 * @param - argc : argv 数组元素个数
 * @param - argv : 具体参数
 * @return : 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd;
    int err = 0;
    char *filename;
    filename = argv[1];
    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }

    while (1)
    {
        err = read(fd, &input_event, sizeof(input_event));
        if (err > 0)
        {
            switch (input_event.type)
            {
            case EV_KEY:
                printf("EV_KEY\n");
                if (input_event.code < BTN_MISC) /* 键盘键值 */
                {
                    printf("key %d %s\r\n", input_event.code, input_event.value ? "press" : "release");
                }
                else
                {
                    printf("button %d %s\r\n", input_event.code, input_event.value ? "press" : "release");
                }
                break;

                /* 其他类型的事件，自行处理 */
            case EV_REL:
                printf("EV_REL\n");
                break;
            case EV_ABS:
                printf("EV_ABS\n");
                break;
            case EV_MSC:
                printf("EV_MSC\n");
                break;
            case EV_SW:
                printf("EV_SW\n");
                break;
            default:
                printf("default\n");
                break;
            }
        }
        else
        {
            printf("读取数据失败\r\n");
        }
    }

    return 0;
}