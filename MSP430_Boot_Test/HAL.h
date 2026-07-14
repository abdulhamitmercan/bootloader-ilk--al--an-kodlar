#ifndef HAL_H_
#define HAL_H_

#include <msp430.h>
#include <stdint.h>

/*
 * MSP430F67751 Bootloader HAL
 *
 * UART:
 *   P3.0 = UCA0RXD
 *   P3.1 = UCA0TXD
 *
 * Baud:
 *   9600
 *
 * Clock:
 *   DCO ~25MHz
 */

static inline void SetVCoreUp(unsigned int level)
{
    PMMCTL0_H = 0xA5;

    __delay_cycles(5000);

    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;

    __delay_cycles(5000);

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);

    PMMCTL0_L = PMMCOREV0 * level;

    __delay_cycles(5000);

    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;

    PMMCTL0_H = 0x00;
}

static inline void HAL_CLK_FREQ(void)
{
    UCSCTL3 |= SELREF_2;
    UCSCTL4 |= SELA_2;

    __bis_SR_register(SCG0);
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_7;
    UCSCTL2 = FLLD_1 | 761;
    __bic_SR_register(SCG0);

    __delay_cycles(781250);

    do
    {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);
        SFRIFG1 &= ~OFIFG;
    }
    while (SFRIFG1 & OFIFG);
}

static inline void HAL_BOOT_INIT(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    SetVCoreUp(3);

    __delay_cycles(5000);

    HAL_CLK_FREQ();


}

#endif /* HAL_H_ */