#include "PetitModbus.h"
#include "PetitModbusPort.h"
#include "msp430.h"
#include "rs485.h"

// This port file for PIC microcontrollers!

// Modbus RTU Variables
volatile unsigned char   PetitReceiveBuffer[PETITMODBUS_RECEIVE_BUFFER_SIZE];   // Buffer to collect data from hardware
volatile unsigned char   PetitReceiveCounter=0;                                 // Collected data number

// UART Initialize for Microconrollers, yes you can use another phsycal layer!
void PetitModBus_UART_Initialise(void)
{
    RS485_Init();
}

// Timer Initialize for Petit Modbus, 1ms Timer will be good for us!
void PetitModBus_TIMER_Initialise(void)
{
    /*
    TA1CCR0 = 3276-1;//5000 - 1;                 // Set Timer Timeout Value
    TA1CCTL0 |= CCIE;                            // Enable Overflow Interrupt
    TA1CTL |= MC_1 + TASSEL_1 + TACLR ;
    */
}

// This is used for send one character
void PetitModBus_UART_Putch(unsigned char c)
{
    //HAL_UART_PUTC(c);
}

// This is used for send string, better to use DMA for it ;)
unsigned char PetitModBus_UART_String(unsigned char *s, unsigned int Length)
{
    unsigned short  DummyCounter;
    for(DummyCounter=0;DummyCounter<Length;DummyCounter++)
        PetitModBus_UART_Putch(s[DummyCounter]);
    
    return TRUE;
}

/*************************Interrupt Fonction Slave*****************************/
// Call this function into your UART Interrupt. Collect data from it!
// Better to use DMA
void ReceiveInterrupt(unsigned char Data)
{
    PetitReceiveBuffer[PetitReceiveCounter]   =Data;
    PetitReceiveCounter++;

    if(PetitReceiveCounter>PETITMODBUS_RECEIVE_BUFFER_SIZE)
        PetitReceiveCounter=0;


    PetitModbusTimerValue=0;
}

// Call this function into 1ms Interrupt or Event!
void PetitModBus_TimerValues(void)
{
    PetitModbusTimerValue++;
}
/******************************************************************************/

