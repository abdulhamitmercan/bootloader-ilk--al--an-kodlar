#include <msp430.h>
#include <stdint.h>
#include "boot_config.h"
#include "flash_driver.h"
#include "boot_uart.h"
#include "app_jump.h"

typedef void (*AppEntry)(void);

static uint16_t read_word(uint32_t address)
{
    uint16_t lo = flash_read8(address);
    uint16_t hi = flash_read8(address + 1UL);
    return (uint16_t)(lo | (uint16_t)(hi << 8));
}

bool application_is_valid(void)
{
    uint16_t entry = read_word(APP_VECTOR_RESET);
    if (boot_state_get() != BOOT_STATE_VALID)
        return false;
    if ((entry == 0xFFFFU) || (entry < (uint16_t)APP_LOW_START) ||
        (entry >= (uint16_t)APP_LOW_END))
        return false;
    return true;
}

void application_jump(void)
{
    uint16_t i;
    uint16_t entry;
    AppEntry start;

    __disable_interrupt();
    UCA0IE = 0U;
    boot_uart_flush_tx();

    if (!application_is_valid())
        return;

    for (i = 0U; i < VECTOR_TABLE_SIZE; i++)
    {
        *((volatile uint8_t *)(RAM_VECTOR_START + (uint32_t)i)) =
            flash_read8(APP_VECTOR_START + (uint32_t)i);
    }

    SYSCTL |= SYSRIVECT;
    entry = read_word(APP_VECTOR_RESET);
    start = (AppEntry)entry;
    start();

    while (1) { }
}
