#ifndef HAL_UART_H_
#define HAL_UART_H_

#include "msp430.h"
#include "stdint.h"
#include "stdbool.h"


typedef struct {
    unsigned int rxCntr; // receive occured counter
    unsigned int txCntr; // transmit complete counter
} t_uartObj;

#define UARTX       (t_uartObj)(uart3)


#define RX_SIZE 128
#define TX_SIZE 8

#define UART_TXD BIT1  // P4.2 UART TXD ile eþleþtirilmiþ pin
#define UART_RXD BIT2  // P4.1 UART RXD ile eþleþtirilmiþ pin

#define BYTE uint8_t


#define BAUD_RATE   9600


typedef struct {
    uint8_t rxBuff[RX_SIZE];
    uint8_t txBuff[TX_SIZE];
} t_HAL_UART_OBJ;




t_HAL_UART_OBJ UART_OBJ;





static void inline HAL_UART_initObj(t_HAL_UART_OBJ* obj){
    int i = 0;
    for ( i = 0; i < TX_SIZE; i++ ) {
        obj->txBuff[i] = 0;
    }

    for ( i = 0; i < RX_SIZE; i++ ) {
        obj->rxBuff[i] = 0;
    }
}



static void inline HAL_UART_INIT(){


   // Setup P3.0 UCA0RXD, P3.1 UCA0TXD
    P4SEL0 |= UART_RXD | UART_TXD;                   // Set P3.0, P3.1 to non-IO
    P4DIR |= UART_RXD | UART_TXD;                   // Enable UCA0RXD, UCA0TXD

    // Setup eUSCI_A0
    UCA3CTLW0 |= UCSWRST;                   // **Put state machine in reset**
    UCA3CTLW0 |= UCPEN;
    UCA3CTLW0 |= UCPAR;
    UCA3CTLW0 |= UCSSEL_2;                  // CLK = ACLK
    //UCA0BRW_L = 0x03;                       // 32kHz/9600=3.41 (see User's Guide)
    //UCA0BRW_H = 0x00;                       //
    UCA3BRW = 2604;
    UCA3MCTLW = 0x5300;                     // Modulation UCBRSx=0x53, UCBRFx=0
    UCA3CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
    UCA3IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
    UCA3IE |= UCTXCPTIE;
    //UCA3IE |= UCTXIE;


    HAL_UART_initObj(&UART_OBJ);
}


static void inline HAL_UART_PUTC(const uint8_t c)
{
    while (!(UCA3IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
    UCA3TXBUF = c;
}

static void inline HAL_UART_PUTS(const uint8_t *str)
{
    if (*str != 0)
        HAL_UART_PUTC(*str++);
    while (*str != 0)
        HAL_UART_PUTC(*str++);
}

static void inline HAL_UART_WRITE(const uint8_t *buf, uint8_t bufSize)
{
    uint8_t idx = 0;
    while(bufSize != idx)
    {
        HAL_UART_PUTC(buf[idx++]);
    }
}




#endif /* HAL_UART_H_ */
