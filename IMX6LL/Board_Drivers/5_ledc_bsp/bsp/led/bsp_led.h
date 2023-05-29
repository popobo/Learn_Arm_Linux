#pragma once

#include "imx6ul.h"


#define LED0	0

/* 函数声明 */
void led_init(void);
void led_switch(int led, int status);
