#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_exit.h"
#include "bsp_int.h"
#include "bsp_epit_timer.h"

/*
 * @description	: mian函数
 * @param 		: 无
 * @return 		: 无
 */
int main(void)
{
    
    int_init(); //保证中断初始化最先调用
    imx6u_clk_init(); // 初始化系统时钟
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/
    beep_init();
    key_init();
    epit1_init(0, 66000000/2);	/* 初始化EPIT1定时器，1分频
								 * 计数值为:66000000/2，也就是
								 * 定时周期为500ms。
								 */

	while(1)
	{
        delay(500);
    }

	return 0;
}
