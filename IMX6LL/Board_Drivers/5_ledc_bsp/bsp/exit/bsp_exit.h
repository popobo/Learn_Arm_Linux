#pragma once

#include "imx6ul.h"

/* 函数声明 */
void exit_init(void);						/* 中断初始化 */
void gpio1_io18_irq_handler(void); 			/* 中断处理函数 */
