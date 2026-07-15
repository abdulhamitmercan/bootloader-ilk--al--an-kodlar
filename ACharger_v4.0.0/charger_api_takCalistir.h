#ifndef CHARGER_API_TAKCALISTIR_H
#define CHARGER_API_TAKCALISTIR_H

#include "ctrl_api.h"
#include "rfid_api.h"
#include "ui_api.h"
#include "board_network.h"
#include "sdcard_api.h"
#include "comm_api.h"
#include "timeLib.h"
#include "task_api.h"

typedef enum {
    CHARGER_NOT_CHARGING = 0,
    CHARGER_CHARGING = 1
} ChargerChargeStatus;

typedef enum {
    CONNECTOR_NOT_PLUGGED = 0,
    CONNECTOR_PLUGGED = 1,
} ChargerConnectorStatus;

typedef struct {
    float energy;
    float power;
} ChargerMeterData;

typedef struct {
    uint16_t hours;
    uint16_t minutes;
    uint16_t seconds;
} ChargerTimeData;

typedef struct {
    uint16_t request;
    uint16_t response;
} ChargerRfidData;

typedef struct {
    uint16_t runControl;
    uint16_t chargeFinished;
    uint16_t chargeStatus;
    uint16_t plugStatus;
    uint16_t ocppStatus;
    ChargerTimeData time;
    ChargerMeterData meter;
    ChargerRfidData rfid;
} ChargerData;

typedef struct {
    uint16_t status;
    uint16_t counter;
    uint16_t rfidHandle;
    uint16_t clearSession;
    uint16_t buzzerOn;
} ControlData;

ControlData ctrl;
ChargerData chargerData;
ChargerData chargerDataNew;
ChargerData chargerDataOld;
ChargerData chargerDataAfterResetNew;
ChargerData chargerDataAfterResetOld;


static void initChargerParameters(ChargerData *charger) {
    charger->chargeStatus = CHARGER_NOT_CHARGING;
    charger->plugStatus = CONNECTOR_NOT_PLUGGED;
    charger->time.hours = 0;
    charger->time.minutes = 0;
    charger->time.seconds = 0;
    charger->meter.energy = 0.f;
    charger->meter.power = 0.f;
}

void charger_init(ChargerData *charger) {
    initChargerParameters(charger);
}

static inline void updateChargerData(ChargerData *charger, uint16_t rxBuf[], int reset) {
    charger->rfid.response = rxBuf[rfidRsp];
    charger->time.minutes = (rxBuf[time_sm] >> 8) & 0x00FF;
    charger->time.seconds = (rxBuf[time_sm]) & 0x00FF;
    charger->time.hours = rxBuf[time_h];
    charger->plugStatus = rxBuf[connector];
    charger->chargeStatus = rxBuf[charging];
    charger->meter.energy = rxBuf[energy];
    charger->meter.power = rxBuf[power];
    charger->chargeFinished = rxBuf[chargeFinished];
}

void handleIdleState(ControlData *control) {
    control->status = P1_IDLE;
    control->rfidHandle = 0;
    control->clearSession = 0;
}

void handlePlugInState(ControlData *control) {
    control->status = P2_PLUG_IN;
    control->rfidHandle = 0;
    chargerData.ocppStatus = 4;
    chargerData.runControl = 0;
    control->clearSession = 1;
}

void handleScanRfidState(ControlData *control) {
    static uint16_t screenCntrP3 = 0;
    chargerData.ocppStatus = 0;
    chargerData.runControl = 1;
    //write_message_P3_SCAN_RFID("Bekleyiniz... Sarj baslamasi bekleniyor...");
    control->status = P3_SCAN_RFID;
    control->rfidHandle = 0;
    control->clearSession = 1;
    write_Screen_Energy(chargerData.meter.energy);
    write_Screen_Power(chargerData.meter.power);
    write_Screen_Chargetime(chargerData.time.hours, chargerData.time.minutes);
    
}

void handleChargingState(ControlData *control) {
    static uint16_t screenCntrP3 = 0;
    timeLib_clearCounter(&control->counter);
    if (screenCntrP3 == 0) {
      
    }
    screenCntrP3 = (screenCntrP3 + 1) % 50;
    control->status = P4_CHARGING;
    control->rfidHandle = 0;
    control->clearSession = 0;
}

void handleChargeEndedState(ControlData *control) {
    write_message_P5_CHARGE_ENDED(timeLib_isTooLong(&control->counter, 50));
    control->status = P5_CHARGE_ENDED;
    control->rfidHandle = 0;

    chargerData.runControl = 0;
    control->clearSession = 0;

}

void handleErrorState(ControlData *control) {
    write_message_P6_ERR();
    control->status = P6_ERR;
    control->rfidHandle = 0;

    chargerData.runControl = 0;
    control->clearSession = 0;
}

void charger_update(ControlData *control, unsigned int currentState, t_rfidParams *rfid) {
    switch(currentState) {
        case P1_IDLE:
            handleIdleState(control);
            break;
        case P2_PLUG_IN:
            handlePlugInState(control);
            break;
        case P3_SCAN_RFID:
            handleScanRfidState(control);
            break;
        case P4_CHARGING:
            handleChargingState(control);
            break;
        case P5_CHARGE_ENDED:
            handleChargeEndedState(control);
            break;
        case P6_ERR:
            handleErrorState(control);
            break;
        default:
            break;
    }
}



static inline void charger_stateTransitions(t_rfidParams *rfid, unsigned int *pageTransition){
  switch(*pageTransition){
      case P2_to_P3:
        
        break;
      case P3_to_P2:
        
        break;
      case P3_to_P4:
        
        break;
      case P4_to_P5:
        
        break;
      case P5_to_P2:
        
        break; 

      case P2_to_P6:
        
        break;
      case P3_to_P6:
        
        break;
      case P4_to_P6:
        
        break;
    }

    *pageTransition = 0;
}

#endif
