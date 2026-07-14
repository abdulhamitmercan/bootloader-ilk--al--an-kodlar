#ifndef ACMETER_H_
#define ACMETER_H_

#include "PetitModbus.h"
#include <stdint.h>

// Constants
#define ACMETER_SLAVE_ADDRESS   (1)
#define NUMBER_OF_TXDATA        (2)
#define UART_BUFFER_SIZE        (20)

// Data Structures
typedef struct {
    unsigned int txCounter;
    unsigned int rxCounter;
    unsigned int txCompleteCounter;
    uint8_t txBuffer[UART_BUFFER_SIZE];
    uint8_t rxBuffer[UART_BUFFER_SIZE];
} UartData;

typedef struct {
    float power;
    float energy;
} AcMeterData;

typedef struct {
    uint16_t control;
    uint16_t index;
    uint16_t rxReady;
    uint16_t timeout;
} AcMeterControl;

// Global Variables
AcMeterControl acMeterControl;
AcMeterData acMeterData;
UartData uart3;

// Function to read active power from the meter
static inline void acMeterReadActivePower(void) {
    uint8_t data[4] = {0, 0x1C, 0, 2};
    modbus_readHoldingRegister(ACMETER_SLAVE_ADDRESS, 4, data);
}

// Function to read active energy from the meter
static inline void acMeterReadActiveEnergy(void) {
    uint8_t data[4] = {0x01, 0x00, 0, 2};
    modbus_readHoldingRegister(ACMETER_SLAVE_ADDRESS, 4, data);
}

// Function to process transmit actions
static inline unsigned int acMeterProcessTransmit(void) {
    static unsigned int idx = 0;
    idx = (idx + 1) % NUMBER_OF_TXDATA;

    switch (idx) {
        case 0:
            acMeterReadActivePower();
            break;
        case 1:
            acMeterReadActiveEnergy();
            break;
    }

    return idx;
}

// Function to update AC meter data
static inline void acMeterUpdateData(AcMeterData *meter, const uint32_t dataBuf[2]) {
    meter->power = *(float*)(&dataBuf[0]);
    meter->energy = *(float*)(&dataBuf[1]);
}

// Function to handle RX ISR
static inline void acMeterHandleRxISR(void) {
    uint8_t data = UCA3RXBUF;
    ReceiveInterrupt(data);
}

// Function to handle TX ISR
static inline void acMeterHandleTxISR(UartData* uart, const uint8_t *txBuffer, uint16_t txSize) {
    if (uart->txCounter < txSize) {
        UCA3TXBUF = txBuffer[uart->txCounter];
        uart->txCounter++;
    } else {
        UCA3IE &= ~UCTXIE;
    }
}

// Function to handle TX complete ISR
static inline void acMeterHandleTxCompleteISR(UartData* uart, uint16_t txSize) {
    uart->txCompleteCounter++;
    if (uart->txCompleteCounter == txSize) {
        RS485_DisableTransmit();
        uart->txCompleteCounter = 0;
    }
}

#endif /* ACMETER_H_ */
