#include <linux/input.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

/* ./01_get_input_info /dev/input/event0 [noblock]*/
int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("Usage: %s <dev> [noblock]", argv[0]);
        return -1;
    }

    int fd;

    if (argc == 3 && !strcmp(argv[2], "noblock"))
    {
        fd = open(argv[1], O_RDWR | O_NONBLOCK);
    }
    else
    {
        fd = open(argv[1], O_RDWR);
    }

    char* ev_name[] = {
        "EV_SYN",
        "EV_KEY",
        "EV_REL",
        "EV_ABS",
        "EV_MSC",
        "EV_SW",
        "NULL",
        "NULL",
        "NULL",
        "NULL",
        "NULL",
        "EV_LED",
        "EV_SND",
        "NULL",
        "EV_REP",
        "EV_FF", 
        "EV_PWR"
    };

    if (fd < 0)
    {
        printf("open %s error\n", argv[1]);
        return -1;
    }

    struct input_id id;
    int err = ioctl(fd, EVIOCGID, &id);
    if (err == 0)
    {
        printf(" bustype = 0x%x\n", id.bustype);
        printf(" vendor = 0x%x\n", id.vendor);
        printf(" product = 0x%x\n", id.product);
        printf(" version = 0x%x\n", id.version);
    }

    unsigned int evbit[2];
    int len = ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), &evbit);
    if (len > 0 && len <= sizeof(evbit))
    {
        printf("support ev type: ");
        for (int32_t i = 0; i < len; i++)
        {
            unsigned byte = ((unsigned char*)evbit)[i];
            for (int32_t bit = 0; bit < 8; bit++)
            {
                if(byte & (1 << bit))
                {
                    printf("%s ", ev_name[i * 8 + bit]);
                }        
            }
        }
        printf("\n");
    }

    struct input_event in_ev;
    size_t read_len;
    while(1)
    {
        read_len = read(fd, &in_ev, sizeof(in_ev));
        if (read_len == sizeof(in_ev))
        {
            printf("get event: type = 0x%x, code = 0x%x, value = 0x%x\n", in_ev.type, in_ev.code, in_ev.value);
        }
        else
        {
            printf("read err: %d\n", read_len);
        }
    }

    return 0;
}