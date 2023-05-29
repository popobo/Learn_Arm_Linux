#include "main.h"

/**
* @brief  使能IMX6U所有外设时钟
* @param  
* @retval 
*/

void clk_enable(void)
{
    CCM_CCGR0 = 0xffffffff;
    CCM_CCGR1 = 0xffffffff;
    CCM_CCGR2 = 0xffffffff;
    CCM_CCGR3 = 0xffffffff;
    CCM_CCGR4 = 0xffffffff;
    CCM_CCGR5 = 0xffffffff;
    CCM_CCGR6 = 0xffffffff;
}

/**
* @brief  初始化LED对应的GPIO
* @param  
* @retval 
*/

void led_init(void)
{
    /* 1.初始化IO复用，复用位GPIO1_IO03 */
    SW_MUX_GPIO1_IO03 = 0x5;

    /* 2.配置GPIO1_IO03的IO电气属性  
     *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
     */
    SW_PAD_GPIO1_IO03 = 0x10b0;
    
    /* 3.初始化GPIO */
    GPIO1_GDIR = 0x00000008; /* GPIO1_IO03设置为输出 */
    
    /* 4.设置GPIO1_IO03的输出为低电平，打开LED0 */
    GPIO1_DR = 0x0;
}

/**
* @brief  打开LED0
* @param  
* @retval 
*/

void led_on(void)
{
    GPIO1_DR &= ~(1 << 3);
}

void led_off(void)
{
	GPIO1_DR |= (1<<3);
}

void delay_short(volatile unsigned int n)
{
	while(n--){}
}

void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0x7ff);
	}
}

int main(void)
{
	clk_enable();		/* 使能所有的时钟		 	*/
	led_init();			/* 初始化led 			*/

	while(1)			/* 死循环 				*/
	{	
		led_off();		/* 关闭LED   			*/
		delay(500);		/* 延时大约500ms 		*/

		led_on();		/* 打开LED		 	*/
		delay(500);		/* 延时大约500ms 		*/
	}

	return 0;
}

