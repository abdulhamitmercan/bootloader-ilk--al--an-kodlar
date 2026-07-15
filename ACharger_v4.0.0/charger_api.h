#ifndef CHARGER_API_H
#define CHARGER_API_H
#pragma once
extern void WrapperMqttSetUnlockConn(const char* v); // wrapper 
extern void WrapperMqttSetChargeState(const char* v); // wrapper 

#include "ctrl_api.h"
#include "rfid_api.h"
#include "ocpp_api.h"
#include "ui_api.h"
#include "board_network.h"
#include "sdcard_api.h"
//#include "charger_api.h"
#include "comm_api.h"
#include "timeLib.h"
#include <MicroOcpp/Core/Connection.h>
#include "task_api.h"
//
#define INTERNET_ERR_TIMEOUT (uint32_t)(6000) // 3000 = 5dk
static uint8_t  chargingActive = 0;
static uint32_t chargeStartMs  = 0;
static float recoveryModeEnergy = 0.0f;
static uint32_t savedElapsedMs = 0;
static bool recoveryLoadedOnce = false;
static uint32_t lastWriteMs = 0;
static bool bootRecoveryLatched = false;
static bool bootRecovery = false;

static float acOptLastGoodEnergy = 0.0f;
static float acOptPendingEnergy = 0.0f;
static unsigned long acOptLastCheckMs = 0;
static uint8_t acOptInit = 0;
static uint8_t acMeterFaultCounter = 0;
static uint8_t lastChargingState = 0;
uint16_t ACmeter_stop= 0;
float candidateEnergy = 0 ;
uint16_t measureType = 0;
#define MODE_TAK_CALISTIR       0
#define MODE_RFID_TAK_CALISTIR  1
#define MODE_OCPP               2

uint16_t activeRunMode =  MODE_OCPP ;
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
    uint8_t actuator;
    int16_t temp;
    uint8_t leakage;
    uint8_t mspState;
    uint8_t rgb;
} ChargerSt1Data;

typedef struct {
    uint8_t controlPilot;
    uint8_t relayStatus;
    uint8_t pwmStatus;
} ChargerSt2Data;

typedef struct {
    uint16_t hw;
    uint8_t sw;
} ChargerVersionData;

typedef struct {
    uint16_t raw;
} ChargerSt3Data;


typedef struct {
    uint16_t voltage;
    uint8_t current;
} ChargerLineData;


struct ChargerData {
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

    ChargerSt1Data st1;
    ChargerSt2Data st2;
    ChargerSt3Data st3;

    ChargerLineData l1;
    ChargerLineData l2;
    ChargerLineData l3;

    ChargerVersionData version;
    
};

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
String rfidTag = "";
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



uint32_t isInternetAlive() {
    return 0 == disconnected;
}

void ocppUpdate(t_rfidParams *rfid, uint16_t rfidResponse, OcppUpdateCallback callback) {
    if (rfid->detected) {
        rfid->request = RFID_REQUEST_STARTED;
    }
    // Call the callback function
    callback(&ocppData, rfid->detected, rfid->tag);

    rfid->response = rfidResponse;
    rfid->detected = 0;
}

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

    // Initialize READ control veriable  
    charger->st1.actuator = 0;
    charger->st1.temp = 0;
    charger->st1.leakage = 0;
    charger->st1.mspState = 0;
    charger->st1.rgb = 0;

    charger->st2.controlPilot = 0;
    charger->st2.relayStatus = 0;
    charger->st2.pwmStatus = 0;

    charger->st3.raw = 0;//reserved

    charger->l1.voltage = 0;
    charger->l1.current = 0;
    charger->l2.voltage = 0;
    charger->l2.current = 0;
    charger->l3.voltage = 0;
    charger->l3.current = 0;

    charger->version.hw = 0;
    charger->version.sw = 0;
    
}


void charger_init(ChargerData *charger) {
    initChargerParameters(charger);
}

inline void assignUint16_(const char* s, uint16_t& dst) {
  if (!s) return;
  char* end = nullptr;
  unsigned long v = strtoul(s, &end, 10);  
  if (end != s && v <= 65535UL) {
    dst = static_cast<uint16_t>(v);
  }
}
/*
static inline void updateChargerData(ChargerData *charger, uint16_t rxBuf[],int reset) {

    if(reset==0){
    charger->rfid.response = rxBuf[rfidRsp];
    charger->time.minutes = (rxBuf[time_sm] >> 8) & 0x00FF;
    charger->time.seconds = (rxBuf[time_sm]) & 0x00FF;
    charger->time.hours = rxBuf[time_h];
    charger->plugStatus = rxBuf[connector];
    charger->chargeStatus = rxBuf[charging];
    charger->meter.energy = rxBuf[energy];
    charger->meter.power = rxBuf[power];
    charger->chargeFinished = rxBuf[chargeFinished];

    charger->version = rxBuf[version];
      
    chargerDataOld = *charger;

    }

    if(reset==1){

    chargerDataAfterResetNew.time.minutes= (rxBuf[time_sm] >> 8) & 0x00FF;
    chargerDataAfterResetNew.time.seconds = (rxBuf[time_sm]) & 0x00FF;
    chargerDataAfterResetNew.time.hours= rxBuf[time_h];
    chargerDataAfterResetNew.meter.energy = rxBuf[energy];
    chargerDataAfterResetNew.meter.power= rxBuf[power];
    
    chargerDataAfterResetOld.updateForNextCycle(chargerDataAfterResetNew);
    ChargerData delta = chargerDataAfterResetNew - chargerDataAfterResetOld;
    chargerDataNew = chargerDataOld + delta;
    chargerDataNew.meter.power = chargerDataAfterResetNew.meter.power ;

    charger->time.minutes =chargerDataNew.time.minutes ;  
    charger->time.seconds = chargerDataNew.time.seconds;
    charger->time.hours = chargerDataNew.time.hours;
    charger->meter.energy = chargerDataNew.meter.energy ;
    charger->meter.power = chargerDataNew.meter.power;
    charger->rfid.response = rxBuf[rfidRsp];
    charger->plugStatus = rxBuf[connector];
    charger->chargeStatus = rxBuf[charging];
    charger->chargeFinished = rxBuf[chargeFinished];

    chargerDataAfterResetOld = chargerDataAfterResetNew;
    chargerDataOld = chargerDataNew;

    }

    
     if (charger->chargeStatus== CHARGER_CHARGING) {

        if (!chargingActive) {
            chargingActive = 1;
            chargeStartMs = millis();

            charger->time.hours   = 0;
            charger->time.minutes = 0;
        }

        uint32_t elapsedMin = (millis() - chargeStartMs) / 60000;

        charger->time.hours   = elapsedMin / 60;
        charger->time.minutes = elapsedMin % 60;
    }
    else {
        chargingActive = 0;
    }  
}*/
static inline float optimizeAcMeterEnergy5s(float candidateEnergy,
                                            ChargerData *charger,
                                            uint8_t recoveryNow)
{
    unsigned long now = millis();
    uint8_t chargingNow = (charger->chargeStatus == CHARGER_CHARGING);

    if (!chargingNow) {
        acOptLastGoodEnergy = 0.0f;
        acOptPendingEnergy = 0.0f;
        acOptLastCheckMs = 0;
        acOptInit = 0;
        acMeterFaultCounter = 0;
        lastChargingState = 0;
        ACmeter_stop = 0;
        return 0.0f;
    }

    // normal modda yeni şarj başladıysa 0'dan başlat
    if (chargingNow && !lastChargingState && !recoveryNow) {
        acOptLastGoodEnergy = 0.0f;
        acOptPendingEnergy = 0.0f;
        acOptLastCheckMs = now;
        acOptInit = 1;
        acMeterFaultCounter = 0;
        lastChargingState = chargingNow;
        ACmeter_stop = 0;
        return 0.0f;
    }

    // recovery modda ilk değer yüksek gelebilir, direkt kabul et
    if (acOptInit == 0 || (chargingNow && !lastChargingState && recoveryNow)) {
        acOptInit = 1;
        acOptLastGoodEnergy = candidateEnergy;
        acOptPendingEnergy = candidateEnergy;
        acOptLastCheckMs = now;
        acMeterFaultCounter = 0;
        lastChargingState = chargingNow;
        ACmeter_stop = 0;
        return candidateEnergy;
    }

    lastChargingState = chargingNow;
    acOptPendingEnergy = candidateEnergy;

    if (now - acOptLastCheckMs < 5000UL) {
        return acOptLastGoodEnergy;
    }

    float dtHour = (now - acOptLastCheckMs) / 3600000.0f;
    float deltaEnergy = acOptPendingEnergy - acOptLastGoodEnergy;
    float expectedWh = charger->meter.power * dtHour;
    float maxAllowedWh = (expectedWh * 5.0f) + 20.0f;

    bool energyFault = false;

    if (deltaEnergy < -20.0f) energyFault = true;
    if (deltaEnergy > maxAllowedWh) energyFault = true;
    if (fabs(deltaEnergy) < 1.0f && charger->meter.power > 500.0f) energyFault = true;

    if (energyFault) {
        acMeterFaultCounter++;

        if (acMeterFaultCounter >= 6) {
            ACmeter_stop = 1;   // 30 sn sürekli hata
        }

        acOptLastCheckMs = now;
        return acOptLastGoodEnergy;
    }

    acMeterFaultCounter = 0;
    ACmeter_stop = 0;

    acOptLastGoodEnergy = acOptPendingEnergy;
    acOptLastCheckMs = now;

    return acOptLastGoodEnergy;
}

static inline int16_t signExtend9Bit(uint16_t value) {
    value &= 0x01FF;
    if (value & 0x0100) {
        return (int16_t)(value | 0xFE00);
    }
    return (int16_t)value;
}


static inline void updateChargerData(ChargerData *charger, uint16_t rxBuf[],int reset) {

   
    charger->rfid.response = rxBuf[rfidRsp];
    charger->time.seconds = (rxBuf[time_sm]) & 0x00FF;
    charger->plugStatus = rxBuf[connector];
    charger->chargeStatus = rxBuf[charging];
    charger->meter.power = rxBuf[power];

    charger->chargeFinished = rxBuf[chargeFinished];// arabadan c2 den çıkınca plug çıkınca 0 oluyor
    charger->st1.actuator = (rxBuf[uxSt1] >> 0) & 0x01;    // ST1
    charger->st1.temp     = signExtend9Bit((rxBuf[uxSt1] >> 1) & 0x01FF);
    charger->st1.leakage  = (rxBuf[uxSt1] >> 10) & 0x01;
    charger->st1.mspState = (rxBuf[uxSt1] >> 11) & 0x0F;
    charger->st1.rgb      = (rxBuf[uxSt1] >> 15) & 0x01;
    charger->st2.controlPilot = (rxBuf[uxSt2] >> 0) & 0xFF;  // ST2
    charger->st2.relayStatus  = (rxBuf[uxSt2] >> 8) & 0x01;
    charger->st2.pwmStatus    = (rxBuf[uxSt2] >> 12) & 0x01;
    charger->st3.raw = rxBuf[uxSt3];  // ST3
    charger->l1.voltage = (rxBuf[uxL1] >> 0) & 0x01FF; // L1
    charger->l1.current = (rxBuf[uxL1] >> 9) & 0x7F;
    charger->l2.voltage = (rxBuf[uxL2] >> 0) & 0x01FF;// L2
    charger->l2.current = (rxBuf[uxL2] >> 9) & 0x7F;
    charger->l3.voltage = (rxBuf[uxL3] >> 0) & 0x01FF; // L3
    charger->l3.current = (rxBuf[uxL3] >> 9) & 0x7F;
    charger->version.hw = (rxBuf[uxVersion] >> 0) & 0x01FF; // VERSION
    charger->version.sw = (rxBuf[uxVersion] >> 9) & 0x7F;


    if (!bootRecoveryLatched) {
    bootRecovery = recoveryMode;   // cihaz açılışında recoveryMode true ise bunu kilitle
    bootRecoveryLatched = true;
    }

    if (charger->chargeStatus == CHARGER_CHARGING) {

        if (bootRecovery && !recoveryLoadedOnce) {
            savedElapsedMs = readChargeTime();  
            recoveryLoadedOnce = true;
        }

        if (!chargingActive) {
            chargingActive = 1;
            chargeStartMs = millis();
            lastWriteMs = millis();
        }

        uint32_t totalElapsedMs = savedElapsedMs + (millis() - chargeStartMs);
        uint32_t elapsedMin = totalElapsedMs / 60000;

        charger->time.hours   = elapsedMin / 60;
        charger->time.minutes = elapsedMin % 60;

        if (millis() - lastWriteMs >= 60000) {
            lastWriteMs = millis();
            writeChargeTime(totalElapsedMs);
        }

    } else {

        chargingActive = 0;
        chargeStartMs = 0;
        lastWriteMs = 0;

        //  hemen savedElapsedMs sıfırlama sorun yarattı
        // sadece transaction tamamen bittiyse sıfırla
        if (ocppData.status.activeTran == 0 ) {
            savedElapsedMs = 0;
            recoveryLoadedOnce = false;
            bootRecoveryLatched = false;
            bootRecovery = false;
            //writeChargeTime(0);
        }
    }



      assignUint16_(mqttDataValue.getCONF_PARAMS_MEASURETYPE(), measureType);
    
    if(reset==0){

        if(recoveryMode==false) {
            if (measureType == 1) {candidateEnergy =optimizeAcMeterEnergy5s(rxBuf[energy],charger,recoveryMode); charger->meter.energy = candidateEnergy;
            } else {charger->meter.energy = rxBuf[energy];}
           
        }else{
            if (measureType == 1) {
                recoveryModeEnergy = readEnergy() / 10;
                candidateEnergy =optimizeAcMeterEnergy5s(recoveryModeEnergy,charger,recoveryMode);
                charger->meter.energy = candidateEnergy;
            }else {
                recoveryModeEnergy =readEnergy();
                charger->meter.energy = (recoveryModeEnergy/10); }
        
        }

      //charger->meter.energy = rxBuf[energy];
      chargerDataOld.meter.energy=charger->meter.energy;
    }

    if(reset==1){


    chargerDataAfterResetNew.meter.energy = rxBuf[energy];
    
    if (chargerDataAfterResetNew.meter.energy < chargerDataAfterResetOld.meter.energy)  chargerDataAfterResetOld.meter.energy  = 0;
    chargerDataNew.meter.energy=chargerDataOld.meter.energy +chargerDataAfterResetNew.meter.energy - chargerDataAfterResetOld.meter.energy;

    if (measureType == 1) {
        candidateEnergy =optimizeAcMeterEnergy5s(chargerDataNew.meter.energy ,charger,recoveryMode); 
        charger->meter.energy = candidateEnergy;
        chargerDataOld.meter.energy=charger->meter.energy ;

     }else{
        charger->meter.energy = chargerDataNew.meter.energy ;
        chargerDataOld.meter.energy=chargerDataNew.meter.energy;
      } 
    
    
    chargerDataAfterResetOld.meter.energy =  chargerDataAfterResetNew.meter.energy;

    }

/* Serial.print("reset=");
Serial.print(reset);

Serial.print(" rxE=");
Serial.print(rxBuf[energy]);

Serial.print(" old=");
Serial.print(chargerDataOld.meter.energy);

Serial.print(" rstOld=");
Serial.print(chargerDataAfterResetOld.meter.energy);

Serial.print(" rstNew=");
Serial.print(chargerDataAfterResetNew.meter.energy);

Serial.print(" out=");
Serial.println(charger->meter.energy); */

}
static inline void updateChargerDataState(ChargerData *charger, uint16_t rxBuf[]){charger->st1.mspState = (rxBuf[uxSt1] >> 11) & 0x0F;}
void handleIdleState(ControlData *control, t_rfidParams *rfid) {
    control->status = P1_IDLE;
    control->rfidHandle = 0;
    control->clearSession = 0;
    control->buzzerOn = 0;
     if (activeRunMode == MODE_TAK_CALISTIR) {
        return;
    }  if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        rfid1.status = 0;
        rfid_FinishRequest(&rfid1);
        return;
    }

    if (activeRunMode == MODE_OCPP) {
        rfid_FinishRequest(rfid);
        return;
    }
    
}

static char *rfidString = "RFID Kart ya da QR Kodu Okutun";
static int connectionCounter = 0;
void handlePlugInState(ControlData *control, t_rfidParams *rfid) {
    control->status = P2_PLUG_IN;
    control->buzzerOn = 0;

    if (activeRunMode == MODE_TAK_CALISTIR) {
        control->rfidHandle = 0;
        chargerData.ocppStatus = 4;
        chargerData.runControl = 0;
        control->clearSession = 1;
        return;
    }

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        control->status = P2_PLUG_IN;
        control->rfidHandle = 1;
        control->clearSession = 1;
        chargerData.runControl = 0;
        control->buzzerOn = 0;
        rfid1.status = 0;

        uint16_t permission = checkMainRfid(rfid1.tag.c_str());
        if (permission) {
            rfidManageEnable = 1;
        }

        rfid_resetParams(&rfid1);
        rfid_FinishRequest(&rfid1);
        rfidString = "RFID Kart ya da QR Kodu Okutun";
        return;
    }
    if (activeRunMode == MODE_OCPP) {
        control->rfidHandle = 0;
        control->clearSession = 0;
        rfid->status = 0;
        rfid_FinishRequest(rfid);

        if (isInternetAlive()) {
            connectionCounter = 0;
        } else {
            connectionCounter++;
            if (connectionCounter > INTERNET_ERR_TIMEOUT) {
                ESP.restart();
            }
        }

        rfidString = "RFID Kart ya da QR Kodu Okutun";
        runPermission = 0;
        return;
    }
}

void handleScanRfidState(ControlData *control, t_rfidParams *rfid) {
    static uint16_t screenCntrP3 = 0;
   static unsigned long lastRfidTime = 0;
    const unsigned long rfidWaitTime = 5000;

    if (activeRunMode == MODE_TAK_CALISTIR) {
        chargerData.ocppStatus = 0;
        chargerData.runControl = 1;

        control->status = P3_SCAN_RFID;
        control->rfidHandle = 0;
        control->clearSession = 1;

        write_Screen_Energy(chargerData.meter.energy);
        write_Screen_Power(chargerData.meter.power);
        write_Screen_Chargetime(chargerData.time.hours, chargerData.time.minutes);
        return;
    }

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        rfid_api_update(rfid);

        if (isRfidProcessOngoing(rfid)) {
            lastRfidTime = millis();
            rfidString = "RFID Okundu Cevap Bekleniyor...";
            control->buzzerOn = 1;
        } else {
            if (millis() - lastRfidTime > rfidWaitTime) {
                rfidString = "RFID Kart ya da QR Kodu Okutun";
                control->buzzerOn = 0;
            }
        }

        write_message_P3_SCAN_RFID(rfidString);

        control->status = P3_SCAN_RFID;
        control->rfidHandle = 1;
        control->clearSession = 0;

        int permission = chargePermission(rfid1.tag.c_str(), tagQueue);
        chargerData.runControl = permission ? 1 : 0;
        return;
    }

    if (activeRunMode == MODE_OCPP) {
        if (isInternetAlive()) {
            rfid_api_update(rfid);
            runPermission = 1;

            if (isRfidProcessOngoing(rfid)) {
                lastRfidTime = millis();
                rfidString = "RFID Okundu Cevap Bekleniyor...";
                control->buzzerOn = 1;
            } else {
                if (millis() - lastRfidTime > rfidWaitTime) {
                    rfidString = "RFID Kart ya da QR Kodu Okutun";
                    control->buzzerOn = 0;
                } else {
                    rfidString = "RFID Okundu Cevap Bekleniyor...";
                    control->buzzerOn = 1;
                }
            }

            write_message_P3_SCAN_RFID(rfidString);

            control->status = P3_SCAN_RFID;
            control->rfidHandle = 1;
            control->clearSession = 1;

            if (ocppData.status.activeTran == 0 && readEnergy() > 1.0f) {
                writeEnergy(0.0f);
                writeChargeTime(0);
                Serial.print("delete fonk in charger api");
            }

            connectionCounter = 0;
        } else {
            runPermission = 0;
            rfidString = "Internet baglantisi yok. Sarj baslatilamaz...";
            control->buzzerOn = 0;
            control->status = P3_SCAN_RFID;
            control->rfidHandle = 0;
            control->clearSession = 0;
            connectionCounter++;
        }

        return;
    }
}

void handleChargingState(ControlData *control, t_rfidParams *rfid) {
    static uint16_t screenCntrP3 = 0;
    static unsigned long lastUpdate = 0;
    static const unsigned long updateInterval = 60000UL;

    timeLib_clearCounter(&control->counter);

    if (activeRunMode == MODE_TAK_CALISTIR) {
        screenCntrP3 = (screenCntrP3 + 1) % 50;

        control->status = P4_CHARGING;
        control->rfidHandle = 0;
        control->clearSession = 0;
        return;
    }

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        rfid_api_update(&rfid1);

        screenCntrP3 = (screenCntrP3 + 1) % 50;

        control->status = P4_CHARGING;
        control->rfidHandle = 1;
        control->buzzerOn = 0;
        control->clearSession = 0;

        rfid_FinishRequest(&rfid1);
        rfid1.status = 0;
        
        const char * rfidString = isRfidProcessOngoing(&rfid1) ? "RFID Okundu Cevap Bekleniyor..." : "RFID Kart ya da QR Kodu Okutun";
        int permission = chargePermission(rfid1.tag.c_str(), tagQueue);

        if (permission) {
            chargerData.runControl = 0;
        } else {
            chargerData.runControl = 1;
        }

        return;
    }

    if (activeRunMode == MODE_OCPP) {
        rfid_FinishRequest(rfid);

        screenCntrP3 = (screenCntrP3 + 1) % 50;

        control->status = P4_CHARGING;
        control->rfidHandle = 1;
        control->clearSession = 0;
        control->buzzerOn = 0;
        rfid->status = 0;

        if (millis() - lastUpdate >= updateInterval) {
            lastUpdate = millis();
            if (recoveryMode == false) {
                writeEnergy(ocppData.energy);
            }
        }

        runPermission = 1;
        return;
    }
}

void handleChargeEndedState(ControlData *control, t_rfidParams *rfid) {
    write_message_P5_CHARGE_ENDED(timeLib_isTooLong(&control->counter, 50));

    control->status = P5_CHARGE_ENDED;
    control->rfidHandle = 0;
    control->buzzerOn = 0;
    control->clearSession = 0;
    chargerData.runControl = 0;

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        rfid_FinishRequest(&rfid1);
        rfid1.status = 0;
        return;
    }

    if (activeRunMode == MODE_OCPP) {
        rfid->status = 0;
        rfid_FinishRequest(rfid);
        runPermission = 0;
        return;
    }

    if (activeRunMode == MODE_TAK_CALISTIR) {
        return;
    }
}

void handleErrorState(ControlData *control, t_rfidParams *rfid) {
 write_message_P6_ERR();

    control->status = P6_ERR;
    control->rfidHandle = 0;
    control->buzzerOn = 0;
    control->clearSession = 0;
    chargerData.runControl = 0;

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        rfid_FinishRequest(&rfid1);
        rfid1.status = 0;
        return;
    }

    if (activeRunMode == MODE_OCPP) {
        runPermission = 0;
        return;
    }

    if (activeRunMode == MODE_TAK_CALISTIR) {
        return;
    }
}

void charger_update(ControlData *control, unsigned int currentState, t_rfidParams *rfid) {
    switch (currentState) {
        case P1_IDLE:
            handleIdleState(control, rfid);
            WrapperMqttSetChargeState("P1_IDLE");
            break;
        case P2_PLUG_IN:
            handlePlugInState(control, rfid);
            WrapperMqttSetChargeState("P2_PLUG_IN");
            break;
        case P3_SCAN_RFID:
            handleScanRfidState(control, rfid);
            WrapperMqttSetChargeState("P3_SCAN_RFID");
            break;
        case P4_CHARGING:
            handleChargingState(control, rfid);
            WrapperMqttSetChargeState("P4_CHARGING");
            break;
        case P5_CHARGE_ENDED:
            handleChargeEndedState(control, rfid);
            WrapperMqttSetChargeState("P5_CHARGE_ENDED");
            break;
        case P6_ERR:
            handleErrorState(control, rfid);
            WrapperMqttSetChargeState("P6_ERR");
            break;
        default:
            break;
    }
}

static inline void charger_stateTransitions_takCalistir(t_rfidParams *rfid, unsigned int *pageTransition) {
    switch(*pageTransition) {
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

static inline void charger_stateTransitions_rfidTakCalistir(t_rfidParams *rfid, unsigned int *pageTransition) {
    switch(*pageTransition) {
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

static inline void charger_stateTransitions_ocpp(t_rfidParams *rfid, unsigned int *pageTransition){
  switch(*pageTransition){
      case P2_to_P3:
        
        break;
      case P3_to_P2:
        ocppData.unlockConnReq = 1;
        break;
      case P3_to_P4:
        
        break;
      case P4_to_P5:
        
        break;
      case P5_to_P2:
        ocppData.unlockConnReq = 1;
        break; 

      case P4_to_P2:
        ocppData.unlockConnReq = 1;
        break; 

      case P6_to_P2:
        ocppData.unlockConnReq = 1;
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

static inline void charger_stateTransitions(t_rfidParams *rfid, unsigned int *pageTransition) {
    if (activeRunMode == MODE_TAK_CALISTIR) {
        charger_stateTransitions_takCalistir(rfid, pageTransition);
        return;
    }

    if (activeRunMode == MODE_RFID_TAK_CALISTIR) {
        charger_stateTransitions_rfidTakCalistir(rfid, pageTransition);
        return;
    }

    if (activeRunMode == MODE_OCPP) {
        charger_stateTransitions_ocpp(rfid, pageTransition);
        return;
    }

    *pageTransition = 0;
}

void handleUnlockConn() {
    
  static unsigned long previousMillis = 0;
  const long interval = 5000;
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    
      ocppData.unlockConnReq = 0;
      WrapperMqttSetUnlockConn("0");   
  
  }
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

void main2RfidManage2(t_rfidParams *rfid, uint16_t *en){
    if(*en){
      change_screen1_4();
      rfid_resetParams(rfid);
      loadTagsFromEEPROM(tagQueue);
      xTaskNotifyGive(Task1);
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
void main2RfidManage(t_rfidParams *rfid, uint16_t *en) {

    if (activeRunMode != MODE_RFID_TAK_CALISTIR) {
        *en = 0;
        return;
    }

    if (FSM.current_state != P2_PLUG_IN) {
        *en = 0;
        return;
    }

    if (*en) {
        change_screen1_4();
        rfid_resetParams(rfid);
        loadTagsFromEEPROM(tagQueue);

        xTaskNotifyGive(Task1);

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

uint16_t mqttRfidUpdate = 0;
uint8_t mainRfidUpdateScreenActive = 0;

void handleMainRfidUpdate(t_rfidParams *rfid) {

    assignUint16_(mqttDataValue.getRFID_UPDATE(), mqttRfidUpdate);

    if (mqttRfidUpdate != 1) {
        mainRfidUpdateScreenActive = 0;
        return;
    }

    if (!mainRfidUpdateScreenActive) {
        change_screen1_4();
        rfid_resetParams(rfid);
        mainRfidUpdateScreenActive = 1;
    }

    write_message_RFID_MANAGE("YENI ANA RFID OKUTUN");
    lv_timer_handler();

    if (rfid->tag != "") {

        mainTag = rfid->tag;

        mqttDataValue.setACTIVE_RFID(mainTag.c_str());
        mqttDataValue.setRFID_UPDATE("0");

        addScreenMessage("ana rfid guncellendi", 5000);

        Serial.print("Yeni Ana RFID: ");
        Serial.println(mainTag);

        rfid_resetParams(rfid);
        rfid_FinishRequest(rfid);

        mainRfidUpdateScreenActive = 0;

        write_Screen_RfidUpdate("");
        change_screen4_1();
    }

    updateScreenMessages();
}
#endif
