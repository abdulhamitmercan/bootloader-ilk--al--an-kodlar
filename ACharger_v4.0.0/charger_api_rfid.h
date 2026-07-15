#ifndef CHARGER_API_H
#define CHARGER_API_H

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
    uint16_t errStatus;
    uint16_t unlockConnRsp;
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

TagQueue tagQueue;


void startCharge(ChargerData *charger){
    charger->runControl = 1;
}

void stopCharge(ChargerData *charger){
    charger->runControl = 0;
}

void runCtrl(t_rfidParams *rfid, uint16_t rfidResponse) {
    if (rfid->detected) {
        rfid->request = RFID_REQUEST_STARTED;
    }

    rfid->response = rfidResponse;
    rfid->detected = 0;
}

String rfidTag = "";
static void initChargerParameters(ChargerData *charger) {
    // Initialize charge and plug status
    charger->chargeStatus = CHARGER_NOT_CHARGING;
    charger->plugStatus = CONNECTOR_NOT_PLUGGED;

    // Initialize time data
    charger->time.hours = 0;
    charger->time.minutes = 0;
    charger->time.seconds = 0;

    // Initialize meter data
    charger->meter.energy = 0.0f;
    charger->meter.power = 0.0f;

    // Initialize RFID data
    charger->rfid.request = 0;
    charger->rfid.response = 0;

    // Initialize charge finished status
    charger->chargeFinished = 0;

    // Initialize error status
    charger->errStatus = 0;

    // Initialize unlock connection response
    charger->unlockConnRsp = 0;

    // Initialize run control status
    charger->runControl = 0;

    // Initialize OCPP status
    charger->ocppStatus = 0;
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

static char *rfidString = "RFID Kart ya da QR Kodu Okutun";
void handleIdleState(ControlData *control, t_rfidParams *rfid) {
    control->status = P1_IDLE;
    control->rfidHandle = 0;
    control->buzzerOn = 0;
    control->clearSession = 0;
    rfid1.status = 0;
    rfid_FinishRequest(&rfid1);
}

void handlePlugInState(ControlData *control, t_rfidParams *rfid) {
    control->status = P2_PLUG_IN;
    control->rfidHandle = 1;
    control->clearSession = 1;
    chargerData.runControl = 0;
    control->buzzerOn = 0;
    rfid1.status = 0;

    uint16_t permission = checkMainRfid(rfid1.tag.c_str());
    if(permission){
        rfidManageEnable = 1;
    }
    rfid_resetParams(&rfid1);
    rfid_FinishRequest(&rfid1);
    rfidString = "RFID Kart ya da QR Kodu Okutun";
}

void handleScanRfidState(ControlData *control, t_rfidParams *rfid) {
    static uint16_t screenCntrP3 = 0;
    static unsigned long lastRfidTime = 0;
    const unsigned long rfidWaitTime = 5000;
    rfid_api_update(rfid);

    if (isRfidProcessOngoing(rfid)) {
            lastRfidTime = millis();
            rfidString = "RFID Okundu Cevap Bekleniyor...";
            control->buzzerOn = 1;
        }
    else {
        if (millis() - lastRfidTime > rfidWaitTime) {
            rfidString = "RFID Kart ya da QR Kodu Okutun";
            control->buzzerOn = 0;
        } else {
            //rfidString = "RFID Okundu Cevap Bekleniyor...";
            //control->buzzerOn = 1;
        }
    }


    write_message_P3_SCAN_RFID(rfidString);
    control->status = P3_SCAN_RFID;
    control->rfidHandle = 1;
    control->clearSession = 0;
    int permission = chargePermission(rfid1.tag.c_str(), tagQueue);
    if (rfidString) { 
        if (permission) {
            chargerData.runControl = 1;
        } else {
            chargerData.runControl = 0;
        }
    }
    
}

void handleChargingState(ControlData *control, t_rfidParams *rfid) {
    static uint16_t screenCntrP3 = 0;
    timeLib_clearCounter(&control->counter);
    rfid_api_update(&rfid1);
    if (screenCntrP3 == 0) {
        /*write_Screen_Energy(chargerData.meter.energy);
        write_Screen_Power(chargerData.meter.power);
        write_Screen_Chargetime(chargerData.time.hours, chargerData.time.minutes);*/
    }
    screenCntrP3 = (screenCntrP3 + 1) % 50;
    control->status = P4_CHARGING;
    control->rfidHandle = 1;
    control->buzzerOn = 0;
    control->clearSession = 0;
    rfid_FinishRequest(&rfid1);
    rfid1.status = 0;

    const char * rfidString = isRfidProcessOngoing(&rfid1) ? "RFID Okundu Cevap Bekleniyor..." : "RFID Kart ya da QR Kodu Okutun";
    int permission = chargePermission(rfid1.tag.c_str(), tagQueue);
    if (rfidString) { 
        if (permission) {
            chargerData.runControl = 0;
        } else {
            chargerData.runControl = 1;
        }
    }
}

void handleChargeEndedState(ControlData *control, t_rfidParams *rfid) {
    write_message_P5_CHARGE_ENDED(timeLib_isTooLong(&control->counter, 50));
    control->status = P5_CHARGE_ENDED;
    control->rfidHandle = 0;
    control->buzzerOn = 0;
    control->clearSession = 0;
    chargerData.runControl = 0;
    rfid_FinishRequest(&rfid1);
    rfid1.status = 0;

}

void handleErrorState(ControlData *control, t_rfidParams *rfid) {
    write_message_P6_ERR();
    control->status = P6_ERR;
    control->rfidHandle = 0;
    control->buzzerOn = 0;
    control->clearSession = 0;
    chargerData.runControl = 0;
    rfid_FinishRequest(&rfid1);
    rfid1.status = 0;
}

void charger_update(ControlData *control, unsigned int currentState, t_rfidParams *rfid) {
    switch(currentState) {
        case P1_IDLE:
            handleIdleState(control, rfid);
            break;
        case P2_PLUG_IN:
            handlePlugInState(control, rfid);
            break;
        case P3_SCAN_RFID:
            handleScanRfidState(control, rfid);
            break;
        case P4_CHARGING:
            handleChargingState(control, rfid);
            break;
        case P5_CHARGE_ENDED:
            handleChargeEndedState(control, rfid);
            break;
        case P6_ERR:
            handleErrorState(control, rfid);
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
        rfid_resetParams(rfid);
        rfid_FinishRequest(&rfid1);
        break;
      case P3_to_P4:
        rfidTag = rfid->tag;
        rfid_resetParams(rfid);
        break;
      case P4_to_P5:
        //modem.sendSMS("+905353452058", String("SARJ OZETI\n") + String("rfid: ") + rfidTag + String(" Enerji: ") + chargerData.meter.energy + String(" kWh"));
        rfid_resetParams(rfid);
        rfidTag = "";
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



void handleRfidManageMode(t_rfidParams *rfid, uint16_t *en){
  if (*en) {
       static uint16_t idx = 0;
        const char* msg = "RFID EKLE-SIL";
        write_message_RFID_MANAGE(msg);
        lv_timer_handler();

      if (rfid->tag != "") {
        const char* tag = rfid->tag.c_str();

        if (!checkMainRfid(tag)) {
            Serial.println(tag);
            if (isTagInQueue(tagQueue, tag)) {
                removeTagFromQueue(tagQueue, tag);
                Serial.println("Tag removed from queue");
                const char* msg1 = "rfid kart silindi";
                addScreenMessage(msg1, 5000);
            } else {
                enqueueTag(tagQueue, tag);
                Serial.println("Tag added to queue");
                const char* msg2 = "rfid kart eklendi";
                addScreenMessage(msg2, 5000);
            }
        } 
        else {
            *en = 0;
            Serial.println("Main RFID tag detected. Suspending Task1 and resuming Task2...");
            write_Screen_RfidUpdate("");
            change_screen4_1();
            loadTagsFromEEPROM(tagQueue);
            xTaskNotifyGive(Task2);  
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        }
      }

       rfid_resetParams(rfid);
    }
    updateScreenMessages();

}

void main2RfidManage(t_rfidParams *rfid, uint16_t *en){
    if(*en){
      change_screen1_4();
      rfid_resetParams(rfid);
      loadTagsFromEEPROM(tagQueue);
      xTaskNotifyGive(Task1);
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

#endif
