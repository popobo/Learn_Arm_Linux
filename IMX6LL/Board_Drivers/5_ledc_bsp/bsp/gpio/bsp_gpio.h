#pragma once

#include "imx6ul.h"

typedef enum _gpio_pin_direction
{
    kGPIO_DigitalInput = 0U,
    kGPIO_DigitalOutput = 1U,
} gpio_pin_direction_t;

/*
 * GPIO中断触发类型枚举
 */
typedef enum _gpio_interrupt_mode
{
    kGPIO_NoIntMode = 0U, 				/* 无中断功能 */
    kGPIO_IntLowLevel = 1U, 			/* 低电平触发	*/
    kGPIO_IntHighLevel = 2U, 			/* 高电平触发 */
    kGPIO_IntRisingEdge = 3U, 			/* 上升沿触发	*/
    kGPIO_IntFallingEdge = 4U, 			/* 下降沿触发 */
    kGPIO_IntRisingOrFallingEdge = 5U, 	/* 上升沿和下降沿都触发 */
} gpio_interrupt_mode_t;

/*
 * GPIO配置结构体
 */	
typedef struct _gpio_pin_config
{
    gpio_pin_direction_t direction; //输入还是输出
    uint8_t outputLogic; //输出的默认电平
    gpio_interrupt_mode_t interruptMode; //中断方式
} gpio_pin_config_t;

void gpio_init(GPIO_Type* base, int32_t pin, gpio_pin_config_t* config);
int gpio_pinread(GPIO_Type* base, int32_t pin);
void gpio_pinwrite(GPIO_Type* base, int32_t pin, int32_t value);
void gpio_int_config(GPIO_Type* base, uint32_t pin, gpio_interrupt_mode_t pinInterruptMode);
void gpio_enable_int(GPIO_Type* base, uint32_t pin);
void gpio_disable_int(GPIO_Type* base, uint32_t pin);
void gpio_clear_int_flags(GPIO_Type* base, uint32_t pin);
