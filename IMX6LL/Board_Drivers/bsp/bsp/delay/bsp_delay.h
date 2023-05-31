#pragma once

#include "imx6ul.h"

void delay_init(void);
void delayus(volatile uint32_t us);
void delayms(volatile uint32_t ms);
void delay(volatile uint32_t n);
void gpt1_irqhandler(void);