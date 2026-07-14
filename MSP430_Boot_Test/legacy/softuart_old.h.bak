
#ifndef SOFTUART_H_
#define SOFTUART_H_

#include "stdbool.h"
#include "stdint.h"

#define BYTE uint8_t

// CPU

// LED
#define RX_LED BIT0
#define TX_LED BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

// SOFTUART
#define	TXD	BIT1 // TXD on P1.1
#define	RXD	BIT2 // RXD on P1.2

#define BAUD_RATE 9600
#define	BIT_TIME	(1000000 / 9600) // 9600 Baud, SMCLK=1MHz (1MHz/9600)=104
#define	HALF_BIT_TIME	(BIT_TIME / 2) // Time for half a bit.

// MODBUS
#define MODBUS_TIMEOUT (BIT_TIME * 12)

// ASCII values for the commands
#define	TEST_SPEED	0x31
#define	M_A3	0x32
//#define STREAM 0x33
//#define STOP 0x34
#define	M_TEMP	0x35
#define	M_VCC	0x36

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

extern bool isReceiving; // Status for when the device is receiving
extern bool hasReceived; // Lets the program know when a byte is received
bool isTimeout;




bool softuart_getc(uint8_t *c);
void softuart_putc(const uint8_t c);

void softuart_puts(const uint8_t *str);
uint8_t softuart_read(uint8_t *buf, uint8_t bufSize);
uint8_t softuart_read2(uint8_t *buf, uint8_t bufSize);
void softuart_write(const uint8_t *buf, uint8_t bufSize);



#endif /* SOFTUART_H_ */
