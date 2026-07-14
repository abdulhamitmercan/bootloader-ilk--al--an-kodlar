#ifndef BOOT_UART_H_
#define BOOT_UART_H_
#include <stdint.h>
#include <stdbool.h>
void boot_uart_init(void);
bool boot_uart_getc(uint8_t *value);
void boot_uart_putc(uint8_t value);
void boot_uart_write(const uint8_t *data, uint8_t length);
void boot_uart_flush_tx(void);
#endif
