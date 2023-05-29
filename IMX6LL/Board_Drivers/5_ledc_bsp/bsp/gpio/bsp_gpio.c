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


