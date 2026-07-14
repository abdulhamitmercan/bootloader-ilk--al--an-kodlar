#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "HAL.h"
#include "boot_config.h"
#include "boot_uart.h"
#include "boot_protocol.h"
#include "app_jump.h"

int main(void)
{
    uint32_t wait = 0UL;
    uint8_t value;

    HAL_BOOT_INIT();
    boot_uart_init();
    boot_protocol_reset();
    __enable_interrupt();

    /* Give ESP time to start an update. */
    while (wait < BOOT_STARTUP_WAIT_LOOPS)
    {
        if (boot_uart_getc(&value))
        {
            boot_protocol_feed(value);
            if (boot_protocol_update_started())
                break;
        }
        wait++;
    }

    if (!boot_protocol_update_started() && application_is_valid())
        application_jump();

    /* Invalid app or active update: remain in bootloader. */
    while (1)
    {
        if (boot_uart_getc(&value))
            boot_protocol_feed(value);
    }
}
