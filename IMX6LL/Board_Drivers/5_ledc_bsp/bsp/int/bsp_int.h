#pragma once

#include "imx6ul.h"

typedef void (*system_irq_handler_t)(uint32_t giccIar, void* param);

typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler;
    void* userParam;
} sys_irq_handle_t;

void int_init(void);
void system_irqtable_init(void);
void system_register_irq_handler(IRQn_Type irq,
                                system_irq_handler_t handler,
                                void *userParam);
void system_irq_handler(uint32_t giccIar);
void default_irq_handler(uint32_t giccIar, void* userParam);