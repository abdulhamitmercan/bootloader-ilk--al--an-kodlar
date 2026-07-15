#ifndef uartTxCmd_H
#define uartTxCmd_H


#include "UART_PROTOCOL.h"

enum {
    STOP_CHARGE = 0x02,
    START_CHARGE = 0x01, 

};

void sendStartCharging(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = RUN_CTRL;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = START_CHARGE;
    *(buf+7) = UART_PARITY;
}

void sendStopCharging(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = RUN_CTRL;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = STOP_CHARGE;
    *(buf+7) = UART_PARITY;
}

void sendReadEnergyIns(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = ETOTAL;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadEnergyComplete(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = ETOTAL_CHARGING_COMPLETE;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadChargingTime(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = CHARGING_TIME;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadChargingTimeHours(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = CHARGING_TIME_HOURS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadPrms(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = PRMS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadErr(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = ERR_STATUS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadChargeFinished(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = CHARGE_FINISHED;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendClearChargeSession(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = RUN_CTRL;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 8;
    *(buf+7) = UART_PARITY;
}


void sendEndTransaction(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = END_TRANSACTION__SEND;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 1;
    *(buf+7) = UART_PARITY;
}

void sendNotEndTransaction(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = END_TRANSACTION__SEND;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}


void sendReadConnectorStatus(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = CONNECTOR_STATUS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadChargingStatus(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = CHARGING_STATUS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendSetBuzzer(uint8_t *buf, uint16_t val){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = CLEAR_CHARGE;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = val;
    *(buf+7) = UART_PARITY;
}

void sendReadErrorStatus(uint8_t *buf, uint16_t val){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = ERR_STATUS;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = val;
    *(buf+7) = UART_PARITY;
}


void sendSetUnlockConnector(uint8_t *buf, uint16_t val){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = UNLOCK_CONNECTOR;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = val;
    *(buf+7) = UART_PARITY;
}

void sendSetConfiguration(uint8_t *buf, uint16_t val){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = CONTROL_REG;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = val;
    *(buf+7) = UART_PARITY;
}

void sendSetMaxPower(uint8_t *buf, uint16_t val){
    *(buf+0) = UART_HEADER;
    *(buf+1) = SET_DATA;
    *(buf+2) = MAX_POWER;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = val*10;
    *(buf+7) = UART_PARITY;
}


void sendReadUxSt1(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_ST1;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadUxSt2(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_ST2;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadUxSt3(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_ST3;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadUxL1(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_L1;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadUxL2(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_L2;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadUxL3(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_L3;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}

void sendReadVersion(uint8_t *buf){
    *(buf+0) = UART_HEADER;
    *(buf+1) = READ_DATA;
    *(buf+2) = UX_VERSION;
    *(buf+3) = 0;
    *(buf+4) = 0;
    *(buf+5) = 0;
    *(buf+6) = 0;
    *(buf+7) = UART_PARITY;
}


#endif 
