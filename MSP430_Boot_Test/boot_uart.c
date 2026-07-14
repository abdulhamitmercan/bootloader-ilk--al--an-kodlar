#include <msp430.h>
#include "boot_uart.h"

static volatile uint8_t rxByte;
static volatile bool rxReady = false;

void boot_uart_init(void)
{
    /* P3.0=UCA0RXD, P3.1=UCA0TXD */
    P3SEL0 |= BIT0 | BIT1;
    P3DIR &= ~BIT0;
    P3DIR |= BIT1;

    UCA0CTLW0 = UCSWRST | UCSSEL__SMCLK;
    /* SMCLK ~= 25 MHz, 9600 baud. */
    UCA0BRW = 2604U;
    UCA0MCTLW = 0x5300U;
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IFG &= ~UCRXIFG;
    UCA0IE = UCRXIE;
}

bool boot_uart_getc(uint8_t *value)
{
    bool result = false;
    __disable_interrupt();
    if (rxReady)
    {
        *value = rxByte;
        rxReady = false;
        result = true;
    }
    __enable_interrupt();
    return result;
}

void boot_uart_putc(uint8_t value)
{
    while ((UCA0IFG & UCTXIFG) == 0U) { }
    UCA0TXBUF = value;
}

void boot_uart_write(const uint8_t *data, uint8_t length)
{
    uint8_t i;
    for (i = 0U; i < length; i++)
        boot_uart_putc(data[i]);
}

void boot_uart_flush_tx(void)
{
    while ((UCA0STATW & UCBUSY) != 0U) { }
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4))
    {
        case 2:
            rxByte = UCA0RXBUF;
            rxReady = true;
            break;
        default:
            break;
    }
}
