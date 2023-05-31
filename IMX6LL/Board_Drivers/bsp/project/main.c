#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_exit.h"
#include "bsp_int.h"
#include "bsp_epit_timer.h"
#include "bsp_keyfilter.h"
#include "bsp_uart.h"
#include "stdio.h"

/*
 * @description	: mian函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
    
    int_init();         //保证中断初始化最先调用
    imx6u_clk_init();   // 初始化系统时钟
    delay_init();       /* 初始化延时 */
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/
    beep_init();
    filterkey_init();
    uart_init();        /* 初始化串口，波特率115200 */

    uint8_t state = OFF;
    uint32_t a = 0;
    uint32_t b = 0;
	while(1)
	{
        printf("输入两个整数，使用空格隔开：");
        scanf("%d %d", &a, &b);
        printf("\r\n 数据 %d + %d = %d \r\n", a, b, a + b);

        state = !state;
        led_switch(LED0, state);
    }

	return 0;
}
