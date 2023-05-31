#pragma once

#include "imx6ul.h"

void uart_init(void);
void uart_io_init(void);
void uart_disable(UART_Type *base);
void uart_enable(UART_Type *base);
void uart_softreset(UART_Type *base);
void uart_setbaudrate(UART_Type *base, uint32_t baudrate, uint32_t srcclock_hz);
void uart_putc(uint8_t c);
void uart_puts(char *str);
uint8_t uart_getc(void);
void raise(int32_t sig_nr);

