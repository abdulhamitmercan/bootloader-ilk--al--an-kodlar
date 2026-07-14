#ifndef UARTPROTOCOL_H_
#define UARTPROTOCOL_H_

#include "rtMem.h"

#define UART_HEADER             (uint8_t)('b')
#define UART_PARITY             (uint8_t)('k')



//-- COMMAND_H
enum {
    SET_DATA = 0x00, //
    SET_DATA_RESPONSE,
    READ_DATA,
    READ_DATA_RESPONSE,
};

//-- COMMAND_L
enum {
    MODE = 0x00, //
    RUN_CTRL = 0x01,// set
    DEVICE_ID,
    ENERGY, // read
    CHARGING_TIME_MIN_SEC, // read
    POWER, // read
    ERR_STATUS, // read
    EV_CHARGE_TERMINATION, // read
    BUZZER_CMD, // set
    CLEAR_SESSION, // set

    CHARGING_STATUS = 0xA, // read
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


void responseData2UartFrame(uint8_t *rspData, int in){
    *(rspData+0) = UART_HEADER;
    *(rspData+1) = READ_DATA_RESPONSE;
    //*(rspData+2) = 0;
    *(rspData+3) = 0;
    *(rspData+4) = 0;
    *(rspData+5) = in>>8 & 0xFF;
    *(rspData+6) = in & 0xFF;
    *(rspData+7) = UART_PARITY;
}

void responseData2UartFrame2(uint8_t *rspData, int in){
    *(rspData+0) = UART_HEADER;
    *(rspData+1) = SET_DATA_RESPONSE;
    //*(rspData+2) = 0;
    *(rspData+3) = 0;
    *(rspData+4) = 0;
    *(rspData+5) = in>>8 & 0xFF;
    *(rspData+6) = in & 0xFF;
    *(rspData+7) = UART_PARITY;
}

int buzzerRsp = 0;
int unlockConnRsp = 0;
void handleSET_DATA(t_uartFrame *uf, uint8_t *rspData){
    int temp = 0;
    switch(uf->commandL){
        case MODE:
            break;

        case RUN_CTRL:
            *(rspData+2) = RUN_CTRL;
            setRunCtrl(uf->data);
            temp = 0;
            break;

        case BUZZER_CMD:
            *(rspData+2) = BUZZER_CMD;
            setBuzzer(uf->data);
            temp = buzzerRsp;
            break;

        case CLEAR_SESSION:
            *(rspData+2) = CLEAR_SESSION;
            setClearSession(uf->data);
            temp = 0;
            break;

        case MAX_POWER:
            *(rspData+2) = MAX_POWER;
            setMaxPower(uf->data);
            temp = 0;
            //getState_Energy(temp);

            break;

        case UNLOCK_CONNECTOR:
            *(rspData+2) = UNLOCK_CONNECTOR;
            setUnlockConnector(uf->data);
            temp = unlockConnRsp;
            break;

        case CONTROL_REG:
            *(rspData+2) = CONTROL_REG;
            setControlReg(uf->data);
            temp = 0;
            break;
    }

    responseData2UartFrame2(rspData, temp);

}



void handleREAD_DATA(t_uartFrame *uf, uint8_t *rspData){

    int temp = 0;
    switch(uf->commandL){
        case MODE:
            *(rspData+2) = MODE;
            break;

        case ENERGY:
            *(rspData+2) = ENERGY;
            getState_Energy(temp);
            break;

        case CHARGING_TIME_MIN_SEC:
            *(rspData+2) = CHARGING_TIME_MIN_SEC;
            getState_ChargingTimeMinSec(temp);
            break;

        case CHARGING_TIME_HOURS:
            *(rspData+2) = CHARGING_TIME_HOURS;
            getState_ChargingTimeHours(temp);
            break;

        case POWER:
            *(rspData+2) = POWER;
            getState_Power(temp);
            break;

        case DEVICE_ID:
            *(rspData+2) = DEVICE_ID;
            getState_DeviceId(temp);
            break;

        case ERR_STATUS:
            *(rspData+2) = ERR_STATUS;
            getState_ErrStatus(temp);
            break;

        case EV_CHARGE_TERMINATION:
            *(rspData+2) = EV_CHARGE_TERMINATION;
            getState_EVChargeTermination(temp);
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


void handleUartFrame(t_uartFrame *uf, uint8_t *rspData){
    switch(uf->commandH){
        case SET_DATA: handleSET_DATA(uf, rspData); break;
        case READ_DATA: handleREAD_DATA(uf, rspData); break;

    }

}


static void inline uartReqRsp(uint8_t* rxBuf, uint8_t* txBuf){
    static t_uartFrame uf;
    decodeUartFrame(&uf, rxBuf);
    handleUartFrame(&uf, txBuf);
}


#endif /* UARTPROTOCOL_H_ */
