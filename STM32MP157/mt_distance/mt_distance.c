
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/input.h>

#include <tslib.h>

#define TS_NO_BLOCK 1
#define TS_BLOCK 0
#define MAX_PRESSED_POINT 20

int distance(struct ts_sample_mt *point1, struct ts_sample_mt *point2)
{
	int x = point1->x - point2->x;
	int y = point1->y - point2->y;

	return x*x + y*y;
}

int main(int argc, char** argv)
{
    char* tsdevice = "/dev/input/event0";
    if (argc == 2)
    {
        tsdevice = argv[1];
    }

    struct tsdev* tsd = ts_setup(tsdevice, TS_BLOCK);
    if (!tsd)
    {
        perror("ts_setup");
        return errno;
    }

    struct input_absinfo slot;
    if(ioctl(ts_fd(tsd),EVIOCGABS(ABS_MT_SLOT), &slot))
    {
        perror("ioctl EVIOGABS");
        ts_close(tsd);
        return errno;
    }

    int32_t read_samples = 1;
    int32_t max_slots = slot.maximum + 1 - slot.minimum;
    struct ts_sample_mt** samp_mt = malloc(read_samples * sizeof(struct ts_sample_mt *));
    struct ts_sample_mt** last_samp_mt = malloc(read_samples * sizeof(struct ts_sample_mt *));
    if (!samp_mt || !last_samp_mt)
    {
        ts_close(tsd);
        return -ENOMEM;
    }
    for (int32_t i = 0; i < read_samples; ++i)
    {
        samp_mt[i] = calloc(max_slots, sizeof(struct ts_sample_mt));
        last_samp_mt[i] = calloc(max_slots, sizeof(struct ts_sample_mt));
        if (!samp_mt[i] || !last_samp_mt[i])
        {
            free(samp_mt);
            ts_close(tsd);
            return errno;
        }
    }

    int ret;
    int32_t touch_cnt;
    while(1)
    {
        ret = ts_read_mt(tsd, samp_mt, max_slots, read_samples);
        if (ret < 0)
        {
            perror("ts_read_mt");
            ts_close(tsd);
            return -1;
        }

        for (int32_t i = 0; i < ret; ++i)
        {
            for (int32_t j = 0; j < max_slots; ++j)
            {
                if (!samp_mt[i][j].valid)
                {
                    continue;
                }
                
                memcpy(&last_samp_mt[i][j], &samp_mt[i][j], sizeof(struct ts_sample_mt));
            }
        }

        touch_cnt = 0;
        int32_t point_pressed[MAX_PRESSED_POINT];
        for (int32_t i = 0; i < ret; ++i)
        {
            for (int32_t j = 0; j < max_slots; ++j)
            {
                if (last_samp_mt[i][j].valid && last_samp_mt[i][j].tracking_id != -1)
                {
                    point_pressed[touch_cnt++] = j;
                }
            }
        }

        if (touch_cnt == 2)
        {
            printf("distance: %08d\n", distance(&last_samp_mt[0][point_pressed[0]], &last_samp_mt[0][point_pressed[1]]));
        }
    }

    return 0;
}