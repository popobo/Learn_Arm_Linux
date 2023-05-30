#include "bsp_gpio.h"

void gpio_init(GPIO_Type* base, int pin, gpio_pin_config_t* config)
{
    if(kGPIO_DigitalInput == config->outputLogic)
    {
        base->GDIR &= ~(1 << pin);
    }
    else if(kGPIO_DigitalOutput == config->outputLogic)
    {
        base->GDIR |= 1 << pin;
        gpio_pinwrite(base, pin, config->outputLogic); //默认输出电平
    }
}

int gpio_pinread(GPIO_Type* base, int pin)
{
    return (((base->DR) >> pin) & 0x1);
}

void gpio_pinwrite(GPIO_Type* base, int pin, int value)
{
    if(value == 0U)
    {
        base->DR &= ~(1U << pin); //输出低电平
    }
    else
    {
        base->DR |= (1U << pin); //输出高电平
    }
}

/*
 * @description  			: 设置GPIO的中断配置功能
 * @param - base 			: 要配置的IO所在的GPIO组。
 * @param - pin  			: 要配置的GPIO脚号。
 * @param - pinInterruptMode: 中断模式，参考枚举类型gpio_interrupt_mode_t
 * @return		 			: 无
 */
void gpio_int_config(GPIO_Type* base, uint32_t pin, gpio_interrupt_mode_t pin_int_mode)
{
	volatile uint32_t *icr;
	uint32_t icrShift;

	icrShift = pin;
	
	base->EDGE_SEL &= ~(1U << pin);

	if(pin < 16) 	/* 低16位 */
	{
		icr = &(base->ICR1);
	}
	else			/* 高16位 */
	{
		icr = &(base->ICR2);
		icrShift -= 16;
	}
	switch(pin_int_mode)
	{
		case(kGPIO_IntLowLevel):
			*icr &= ~(3U << (2 * icrShift));
			break;
		case(kGPIO_IntHighLevel):
			*icr = (*icr & (~(3U << (2 * icrShift)))) | (1U << (2 * icrShift));
			break;
		case(kGPIO_IntRisingEdge):
			*icr = (*icr & (~(3U << (2 * icrShift)))) | (2U << (2 * icrShift));
			break;
		case(kGPIO_IntFallingEdge):
			*icr |= (3U << (2 * icrShift));
			break;
		case(kGPIO_IntRisingOrFallingEdge):
			base->EDGE_SEL |= (1U << pin);
			break;
		default:
			break;
	}
}

/*
 * @description  			: 使能GPIO的中断功能
 * @param - base 			: 要使能的IO所在的GPIO组。
 * @param - pin  			: 要使能的GPIO在组内的编号。
 * @return		 			: 无
 */
void gpio_enable_int(GPIO_Type* base, uint32_t pin)
{ 
    base->IMR |= (1 << pin);
}

/*
 * @description  			: 禁止GPIO的中断功能
 * @param - base 			: 要禁止的IO所在的GPIO组。
 * @param - pin  			: 要禁止的GPIO在组内的编号。
 * @return		 			: 无
 */
void gpio_disable_int(GPIO_Type* base, uint32_t pin)
{ 
    base->IMR &= ~(1 << pin);
}

/*
 * @description  			: 清除中断标志位(写1清除)
 * @param - base 			: 要清除的IO所在的GPIO组。
 * @param - pin  			: 要清除的GPIO掩码。
 * @return		 			: 无
 */
void gpio_clear_int_flags(GPIO_Type* base, uint32_t pin)
{
    base->ISR |= (1 << pin);
}
