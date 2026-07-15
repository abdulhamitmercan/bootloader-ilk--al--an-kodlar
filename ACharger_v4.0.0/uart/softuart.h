
#ifndef SOFTUART_H
#define SOFTUART_H

#include "stdbool.h"
#include "stdint.h"
#include "Arduino.h"

#define BYTE uint8_t



#define BAUD_RATE 9600
#define	BIT_TIME	(1000000 / 9600) // 9600 Baud, SMCLK=1MHz (1MHz/9600)=104
#define	HALF_BIT_TIME	(BIT_TIME / 2) // Time for half a bit.



// ADC10

// initial struct
struct Settings
{
	uint32_t SOFTUART_TXD_PIN;
	uint32_t SOFTUART_RXD_PIN;
} settings;



uint8_t BitCnt; // Bit count, used when transmitting byte
uint16_t TXRegister; // Value sent over UART when Transmit() is called
uint16_t RXRegister; // Value recieved once hasRecieved is set

//uint8_t i; // 'for' loop variable


bool isReceiving; // Status for when the device is receiving
bool hasReceived; // Lets the program know when a byte is received


bool softuart_getc(uint8_t *c)
{
	if (!hasReceived)
	{
		return false;
	}
	*c = RXRegister;
	hasReceived = false;
	return true;
}

void softuart_putc(const uint8_t c)
{
	TXRegister = c;
	//while (!(UCA0IFG & UCTXIFG)) ;  // USCI_A0 TX buffer ready?
    //UCA0TXBUF = c;
}

void softuart_puts(const uint8_t *str)
{
	if (*str != 0)
		softuart_putc(*str++);
	while (*str != 0)
		softuart_putc(*str++);
}

void softuart_write(const uint8_t *buf, uint8_t bufSize)
{
	uint8_t idx = 0;
	while(bufSize != idx)
	{
		softuart_putc(buf[idx++]);
	}
}

uint8_t softuart_read(uint8_t *buf, uint8_t bufSize)
{
	uint8_t limit = 0;
	uint8_t idx = 0;
	uint8_t c;
	while((bufSize != idx) && (limit != 40))
	{
		if(hasReceived)
		{
			softuart_getc(&c);
			buf[idx] = c;
			idx++;
		}
		limit++;
	}
	return idx;
}

uint8_t softuart_read2(uint8_t *buf, uint8_t bufSize)
{
	static uint8_t idx = 0;
	uint8_t c;

	if(hasReceived)
	{
	    if(idx == bufSize){
            idx = 0;
        }
		softuart_getc(&c);
		if(idx == 0 && c == 'b'){
		    buf[idx] = c;
		    idx++;
		}
		else if(idx == bufSize-1 && c == 'k'){
		    buf[idx] = c;
		    idx++;
		}
		else if(idx > 0 && idx < bufSize){
		    buf[idx] = c;
		    idx++;
		}
		else{
		    buf[0] = 0;
		    buf[7] = 0;
		}


	}
	return idx;
}

#endif /* SOFTUART_H_ */
