#ifndef COMM_API_H
#define COMM_API_H

#include <stdint.h>
#include <Arduino.h>
#include "uart/UART_PROTOCOL.h"
#include "uart/softuart.h"
#include "uart/uartTxCmd.h"

#define SerialMAIN Serial2

#define RECV_SIZE 8
#define SEND_SIZE 8

#define bloked 0
#define Unblocked 1

static uint8_t txCount = 0;
static uint8_t txBuff[SEND_SIZE]; 
uint8_t recvBuff[RECV_SIZE] = { 0 };
uint8_t cnt = 0;
uint8_t controlRecoveryMode=9;
uint8_t initialConnectorStatus;
String initialMspState = "UNKNOWN";
typedef struct {
    uint16_t chargeCtrl;
    uint16_t ocppStatus;
    uint16_t rfidRead;
} t_txVars;

enum{
    energy = 0,
    power,
    charging,
    connector,
    chargeFinished,
    time_sm,
    time_h,
    rfidRsp,
    errStatus,
    unlockConnRsp,
    uxSt1,
    uxSt2,
    uxSt3,
    uxL1,
    uxL2,
    uxL3,
    uxVersion,

    commRxCount
};

typedef struct {
  uint16_t rx[commRxCount];
  uint16_t tx[10];
} t_commBuffer;
t_commBuffer comm = {0};


void comm_init(){
  SerialMAIN.begin(9600, SERIAL_8N1, 18, 17);
  SerialMAIN.setTimeout(10);
  SerialMAIN.flush();
}


void sendUartBytes(uint8_t* buf, uint16_t numOfBytes){
  for(int i=0; i<numOfBytes; i++){
      SerialMAIN.write(buf[i]);
  }
}

void handleUartRx(char rxVal){
  RXRegister = rxVal;
  hasReceived = true;
  uint8_t cnt = softuart_read2((uint8_t*) &recvBuff, RECV_SIZE);
  hasReceived = false;
  if (cnt == 8) {
    
    decodeUartFrame(&uf, recvBuff);
    handleUartFrame(&uf, recvBuff);
    SerialMAIN.flush();
  }
}




static void sendChargetx(uint16_t chargeCtrl) {
    switch (chargeCtrl) {
        case 1:
            sendStartCharging(txBuff);
            break;
        case 0:
            sendStopCharging(txBuff);
            break;
        case 8:
            sendClearChargeSession(txBuff);
            break;
        default:
            
            break;
    }
}

static void sendTransactionStatus(uint16_t ocppStatus) {
    if (ocppStatus == 1) {
        sendEndTransaction(txBuff);
    } else {
        sendNotEndTransaction(txBuff);
    }
}


void comm_txUpdate(uint16_t runCtrl, uint16_t ocppEndTran, uint16_t rfidRead, uint8_t confParams, uint16_t unlockConn, uint16_t esptest,uint16_t maxPower) {
    switch (txCount) {
        case 1:
            sendChargetx(runCtrl);
            break;
        case 2:
            sendReadChargeFinished(txBuff);
            break;
        case 3:
            sendReadPrms(txBuff);
            break;
        case 4:
            sendReadEnergyComplete(txBuff);
            break;
        case 5:
            sendTransactionStatus(ocppEndTran);
            break;
        case 6:
            sendReadConnectorStatus(txBuff);
            break;
        case 7:
            sendReadChargingStatus(txBuff);
            break;
        case 8:
            sendReadChargingTimeHours(txBuff);
            break;
        case 0:
            sendReadChargingTime(txBuff);
            break;
        case 9:
            sendSetBuzzer(txBuff, rfidRead);
            break;

        case 10:
          sendReadErrorStatus(txBuff, esptest);
          break;

        case 11:
          sendSetUnlockConnector(txBuff, unlockConn);
          break;

        case 12:
            sendSetConfiguration(txBuff, confParams);
            break;
        case 13:
            sendSetMaxPower(txBuff, maxPower);
            break;
        case 14:
            sendReadUxSt1(txBuff);
            break;
        case 15:
            sendReadUxSt2(txBuff);
            break;
        case 16:
            sendReadUxSt3(txBuff);
            break;
        case 17:
            sendReadUxL1(txBuff);
            break;
        case 18:
            sendReadUxL2(txBuff);
            break;
        case 19:
            sendReadUxL3(txBuff);
            break;
        case 20:
            sendReadVersion(txBuff);
            break;
        default:
          break;

    }
    sendUartBytes(txBuff, SEND_SIZE);

    txCount++;
    txCount = txCount % 21;
}






static inline void getStates(uint16_t *out){
    getState_Etotal_ChargingComplete(*(out+energy));
    getState_Prms(*(out+power));
    getState_ChargingStatus(*(out+charging));
    getState_ConnectorStatus(*(out+connector));
    getState_ChargeFinished(*(out+chargeFinished));
    getState_ChargingTime(*(out+time_sm));
    getState_ChargingTimeHours(*(out+time_h));
    getCommand_ClearChargeSession(*(out+rfidRsp));
    getState_ErrStatus(*(out+errStatus));
    getUnlockConnectorRsp(*(out+unlockConnRsp));

    getState_uxST1(*(out + uxSt1));
    getState_uxST2(*(out + uxSt2));
    getState_uxST3(*(out + uxSt3));
    getState_uxL1(*(out + uxL1));
    getState_uxL2(*(out + uxL2));
    getState_uxL3(*(out + uxL3));
    getState_uxVersion(*(out + uxVersion));
}
static String mspStateToString1(uint16_t state) {
  switch (state) {
    case 0: return "A1";
    case 1: return "A2";
    case 2: return "B1";
    case 3: return "B2";
    case 4: return "C1";
    case 5: return "C2";
    default: return "UNKNOWN";
  }
}
static void forceHitConnectorStatusOnce(uint32_t timeoutMs ,uint16_t *out) {

  uint8_t buf[8];
  sendReadConnectorStatus(buf);
  sendUartBytes(buf, 8);

   sendReadUxSt1(buf);
  sendUartBytes(buf, 8);
  
  uint32_t t0 = millis();
  uint32_t rxCnt = 0;

  while (millis() - t0 < timeoutMs) {
    int av = SerialMAIN.available();
   
    while (SerialMAIN.available() > 0) {
      char b = (char)SerialMAIN.read();
      rxCnt++;
      handleUartRx(b);
    }
    delay(1);
  }
  int v = 0;
 // uint16_t st1 = *(out + uxSt1);
  getState_ConnectorStatus(v);
  getState_uxST1(*(out + uxSt1));

  uint16_t st1Raw = *(out + uxSt1);
  uint16_t mspState = (st1Raw >> 11) & 0x0F;

  
  //Serial.print("initialMspState=");
  //Serial.println(initialMspState);
  initialMspState = mspStateToString1(mspState);
  initialConnectorStatus = v;
}

#endif  // COMM_API_H
