#pragma once

#include "imx6ul.h"

void filterkey_init(void);
void filtertimer_init(uint32_t value);
void filtertimer_stop(void);
void filtertimer_restart(uint32_t value);
void filtertimer_irqhandler(void);
void gpio1_16_31_irqhandler(void);
