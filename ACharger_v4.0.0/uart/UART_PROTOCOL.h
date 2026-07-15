#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H


#include <Arduino.h>


#include "rtMem.h"





#define UART_HEADER             (uint8_t)('b')
#define UART_PARITY             (uint8_t)('k')



//-- COMMAND_H
enum {
    SET_DATA = 0x00, //
    SET_DATA_RESP,
    READ_DATA,
    READ_DATA_RES,
};

//-- COMMAND_L
enum {
    MODE = 0x00, //
    RUN_CTRL, // set
    ETOTAL,
    ETOTAL_CHARGING_COMPLETE, // read
    CHARGING_TIME, // read
    PRMS, // read
    ERR_STATUS, // read
    CHARGE_FINISHED, // read
    CLEAR_CHARGE, //set
    END_TRANSACTION__SEND, // set

    CHARGING_STATUS = 10, // read
    CONNECTOR_STATUS,
    CHARGING_TIME_HOURS, // read
    MAX_POWER,
    UNLOCK_CONNECTOR,
    CONTROL_REG,

    UX_ST1,
    UX_ST2,
    UX_ST3,
    UX_L1,
    UX_L2,
    UX_L3,
    UX_VERSION,
};

typedef struct {
    uint8_t header;
    uint8_t commandH;
    uint8_t commandL;
    uint8_t reserved[2];
    uint16_t data;
    uint8_t parity;
} t_uartFrame;

t_uartFrame uf;


unsigned int requestOccured = 0;

void decodeUartFrame(t_uartFrame *uf, uint8_t* buffer){
    uf->header = buffer[0];
    uf->parity = buffer[7];

    if(uf->header == UART_HEADER && uf->parity == UART_PARITY){
        uf->commandH = buffer[1];
        uf->commandL = buffer[2];
        uf->data = (buffer[5] << 8) | (buffer[6]);
    }

}




void handleSET_DATA(t_uartFrame *uf, uint8_t *rspData){
    switch(uf->commandL){
        case MODE:
            break;

        case RUN_CTRL:
            setCommand_RunCtrl(uf->data);
            break;

        case CLEAR_CHARGE:
            setCommand_ClearChargeSession(uf->data);
            break;

        case END_TRANSACTION__SEND:
            setCommand_EndTransaction(uf->data);
            break;
    }
    *(rspData+0) = 'b';
    *(rspData+1) = 'u';
    *(rspData+2) = 'r';
    *(rspData+3) = 'a';
    *(rspData+4) = 'k';

}

void handleSET_DATA_RESP(t_uartFrame *uf, uint8_t *rspData){

    int temp = 0;

    temp = (*(rspData+5)<<8 & 0xFF00) | (*(rspData+6) & 0xFF);

    switch(uf->commandL){
        case MODE:
            break;

        case CLEAR_CHARGE:
            *(rspData+2) = CLEAR_CHARGE;
            setCommand_ClearChargeSession(temp);
            break;

        case UNLOCK_CONNECTOR:
            *(rspData+2) = UNLOCK_CONNECTOR;
            setUnlockConnectorRsp(temp);
            break;
    }

}


void responseData2UartFrame(uint8_t *rspData, int in){
    *(rspData+0) = UART_HEADER;
    *(rspData+1) = READ_DATA_RES;
    //*(rspData+2) = 0;
    *(rspData+3) = 0;
    *(rspData+4) = 0;
    *(rspData+5) = in>>8 & 0xFF;
    *(rspData+6) = in & 0xFF;
    *(rspData+7) = UART_PARITY;
}

void handleREAD_DATA(t_uartFrame *uf, uint8_t *rspData){

    int temp = 0;
    switch(uf->commandL){
        case MODE:
            *(rspData+2) = MODE;
            break;

        case ETOTAL_CHARGING_COMPLETE:
            *(rspData+2) = ETOTAL_CHARGING_COMPLETE;
            getState_Etotal_ChargingComplete(temp);
            break;

        case CHARGING_TIME:
            *(rspData+2) = CHARGING_TIME;
            getState_ChargingTime(temp);
            break;

        case CHARGING_TIME_HOURS:
            *(rspData+2) = CHARGING_TIME_HOURS;
            getState_ChargingTimeHours(temp);
            break;

        case PRMS:
            *(rspData+2) = PRMS;
            getState_Prms(temp);
            break;

        case ETOTAL:
            *(rspData+2) = ETOTAL;
            getState_Etotal(temp);
            break;

        case ERR_STATUS:
            *(rspData+2) = ERR_STATUS;
            getState_ErrStatus(temp);
            break;

        case CHARGE_FINISHED:
            *(rspData+2) = CHARGE_FINISHED;
            getState_ChargeFinished(temp);
            break;

        case CHARGING_STATUS:
            *(rspData+2) = CHARGING_STATUS;
            getState_ChargingStatus(temp);
            break;

        case CONNECTOR_STATUS:
            *(rspData+2) = CONNECTOR_STATUS;
            getState_ConnectorStatus(temp);
            break;

        case UX_ST1:
            *(rspData+2) = UX_ST1;
            getState_uxST1(temp);
            break;

        case UX_ST2:
            *(rspData+2) = UX_ST2;
            getState_uxST2(temp);
            break;

        case UX_ST3:
            *(rspData+2) = UX_ST3;
            getState_uxST3(temp);
            break;

        case UX_L1:
            *(rspData+2) = UX_L1;
            getState_uxL1(temp);
            break;

        case UX_L2:
            *(rspData+2) = UX_L2;
            getState_uxL2(temp);
            break;

        case UX_L3:
            *(rspData+2) = UX_L3;
            getState_uxL3(temp);
            break;

        case UX_VERSION:
            *(rspData+2) = UX_VERSION;
            getState_uxVersion(temp);
            break;
    }

    responseData2UartFrame(rspData, temp);
}



void handleREAD_DATA_RES(t_uartFrame *uf, uint8_t *rspData){

    int temp = 0;

    temp = (*(rspData+5)<<8 & 0xFF00) | (*(rspData+6) & 0xFF);

    switch(uf->commandL){
        case MODE:
            *(rspData+2) = MODE;
            break;

        case ETOTAL_CHARGING_COMPLETE:
            *(rspData+2) = ETOTAL_CHARGING_COMPLETE;
            setState_Etotal_ChargingComplete(temp);
            break;

        case CHARGING_TIME:
            *(rspData+2) = CHARGING_TIME;
            setState_ChargingTime(temp);
            break;

        case CHARGING_TIME_HOURS:
            *(rspData+2) = CHARGING_TIME_HOURS;
            setState_ChargingTimeHours(temp);
            break;

        case PRMS:
            *(rspData+2) = PRMS;
            setState_Prms(temp);
            break;

        case ETOTAL:
            *(rspData+2) = ETOTAL;
            setState_Etotal(temp);
            break;

        case ERR_STATUS:
            *(rspData+2) = ERR_STATUS;
            setState_ErrStatus(temp);
            break;

        case CHARGE_FINISHED:
            *(rspData+2) = CHARGE_FINISHED;
            setState_ChargeFinished(temp);
            break;

        case CHARGING_STATUS:
            *(rspData+2) = CHARGING_STATUS;
            setState_ChargingStatus(temp);
            break;

        case CONNECTOR_STATUS:
            *(rspData+2) = CONNECTOR_STATUS;
            setState_ConnectorStatus(temp);
            break;

        case UX_ST1:
            *(rspData+2) = UX_ST1;
            setState_uxST1(temp);
            break;

        case UX_ST2:
            *(rspData+2) = UX_ST2;
            setState_uxST2(temp);
            break;

        case UX_ST3:
            *(rspData+2) = UX_ST3;
            setState_uxST3(temp);
            break;

        case UX_L1:
            *(rspData+2) = UX_L1;
            setState_uxL1(temp);
            break;

        case UX_L2:
            *(rspData+2) = UX_L2;
            setState_uxL2(temp);
            break;

        case UX_L3:
            *(rspData+2) = UX_L3;
            setState_uxL3(temp);
            break;

        case UX_VERSION:
            *(rspData+2) = UX_VERSION;
            setState_uxVersion(temp);
            break;

    }

    //responseData2UartFrame(rspData, temp);
}



void handleUartFrame(t_uartFrame *uf, uint8_t *rspData){
    switch(uf->commandH){
        case SET_DATA: handleSET_DATA(uf, rspData); break;
        case SET_DATA_RESP: handleSET_DATA_RESP(uf, rspData); break;
        case READ_DATA: handleREAD_DATA(uf, rspData); break;
        case READ_DATA_RES: handleREAD_DATA_RES(uf, rspData); break;
    }

}

#endif