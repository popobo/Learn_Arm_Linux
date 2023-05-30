#include "bsp_keyfilter.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_beep.h"

/*
 * @description		: 按键初始化
 * @param			: 无
 * @return 			: 无
 */
void filterkey_init(void)
{
    /* 1.初始化IO */
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);

    /* 2.初始化GPIO中断  
     *bit 16:0 HYS关闭
	 *bit [15:14]: 11 默认22K上拉
	 *bit [13]: 1 pull功能
	 *bit [12]: 1 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 000 关闭输出
	 *bit [0]: 0 低转换率
	 */
    gpio_pin_config_t key_config;
    key_config.direction = kGPIO_DigitalInput;
    key_config.interruptMode = kGPIO_IntFallingEdge;
    key_config.outputLogic = 1;
    gpio_init(GPIO1, 18, &key_config);

    /* 3.使能GOIO中断，并且注册中断处理函数 */
    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn, (system_irq_handler_t)gpio1_16_31_irqhandler, NULL);

    gpio_enableint(GPIO1, 18); /* 使能GPIO1_IO18的中断功能 */
    filtertimer_init(66000000 / 100); /* 初始化定时器，10ms */
}

/**
* @brief  初始化用于消抖的定时器，默认关闭定时器
* @param  
* @retval 
*/
void filtertimer_init(uint32_t value)
{
    EPIT1->CR = 0; /* 先清零 */
    EPIT1->CR = (1 << 24 | 1 << 3 | 1 << 2 | 1 << 1);
    EPIT1->LR = value; /* 计数值 */
    EPIT1->CMPR = 0; /* 比较寄存器为0 */

    /* 使能EPIT1中断并注册中断处理函数 */
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, (system_irq_handler_t)filtertimer_irqhandler, NULL);
}

/**
* @brief  关闭定时器
* @param  
* @retval 
*/
void filtertimer_stop(void)
{
    EPIT1->CR &= ~(1 << 0); /* 关闭定时器 */
}

/**
* @brief  重启定时器
* @param  value EPIT1定时器计数值
* @retval 
*/
void filtertimer_restart(uint32_t value)
{
    EPIT1->CR &= ~(1 << 0); /* 先关闭定时器 */
    EPIT1->LR = value; /* 计数值 */
    EPIT1->CR |= (1 << 0); /* 打开定时器 */
}

/**
* @brief  定时器中断处理函数
* @param  
* @retval 
*/
void filtertimer_irqhandler(void)
{
    static uint8_t state = OFF;
    if(EPIT1->SR & (1 << 0)) //判断比较事件是否发生
    {
        filtertimer_stop(); //关闭定时器
        if(gpio_pinread(GPIO1, 18) == 0) //KEY0
        {
            state = !state;
            beep_switch(state);
        }
    }
    EPIT1->SR |= (1 << 0); //写1清除中断标志位
}

/**
* @brief  GPIO中断处理函数
* @param  
* @retval 
*/
void gpio1_16_31_irqhandler(void)
{
    /* 开启定时器 */
    filtertimer_restart(66000000 / 100);

    /* 清除中断标志位 */
    gpio_clearintflags(GPIO1, 18);
}