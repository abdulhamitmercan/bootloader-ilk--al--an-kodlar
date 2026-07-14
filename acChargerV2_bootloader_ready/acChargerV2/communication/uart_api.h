#ifndef UART_API_H_
#define UART_API_H_

#include "rtMem.h"
#include "softuart.h"
#include "uartProtocol.h"

#define RECV_SIZE 8
#define SEND_SIZE 8

typedef struct {
    uint8_t recvBuff[RECV_SIZE];
    uint8_t sendBuff[SEND_SIZE];
    unsigned int rxCompletedFlg;
} t_uartParams;

t_uartParams comm;

static inline void uartUpdate(uint8_t* rxBuf, uint8_t* txBuf, uint16_t txSize){
    uartReqRsp(rxBuf, txBuf);
    softuart_write((const uint8_t*) txBuf, SEND_SIZE);
}

static inline unsigned int handleUartRxISR(uint8_t* rxBuf, uint16_t rxSize){
    static unsigned int cnt = 0;
    RXRegister = UCA0RXBUF;
    hasReceived = true;
    cnt = softuart_read2((uint8_t*) rxBuf, rxSize);
    hasReceived = false;

    if(cnt == 8){
        // all frame elements sent
        cnt = 0;
        return 1;
    }
    else{
        return 0;
    }

}

#endif /* UART_API_H_ */
