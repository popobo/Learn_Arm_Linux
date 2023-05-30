#include "bsp_exit.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_beep.h"
#include "core_ca7.h"
#include <stddef.h>

/*
 * @description			: 初始化外部中断
 * @param				: 无
 * @return 				: 无
 */
void exit_init(void)
{
	gpio_pin_config_t key_config;

	/* 1、设置IO复用 */
	IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18,0);			/* 复用为GPIO1_IO18 */
	IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18,0xF080);

	/* 2、初始化GPIO为中断模式 */
	key_config.direction = kGPIO_DigitalInput;
	key_config.interruptMode = kGPIO_IntFallingEdge;
	key_config.outputLogic = 1;
	gpio_init(GPIO1, 18, &key_config);

	GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);				/* 使能GIC中对应的中断 */
	system_register_irq_handler(GPIO1_Combined_16_31_IRQn, (system_irq_handler_t)gpio1_io18_irq_handler, NULL);	/* 注册中断服务函数 */
	gpio_enable_int(GPIO1, 18);								/* 使能GPIO1_IO18的中断功能 */
}

/*
 * @description			: GPIO1_IO18最终的中断处理函数
 * @param				: 无
 * @return 				: 无
 */
void gpio1_io18_irq_handler(void)
{ 
	static unsigned char state = 0;

	delay(10);
	if(gpio_pinread(GPIO1, 18) == 0)	/* 按键按下了  */
	{
		state = !state;
		beep_switch(state);
	}
	
	gpio_clear_int_flags(GPIO1, 18); /* 清除中断标志位 */
}



