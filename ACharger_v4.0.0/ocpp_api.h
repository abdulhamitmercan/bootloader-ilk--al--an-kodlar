#ifndef OCPP_API_H
#define OCPP_API_H

#include "stdio.h"
#include <MicroOcpp.h>
#include "MicroOcpp/Platform.h"
#include <MicroOcpp/Core/Connection.h>
#include <MicroOcpp/Core/Context.h>
#include <MicroOcpp/Model/Boot/BootService.h>
#include <MicroOcpp/Model/Model.h>
#include <MicroOcpp/Core/Configuration.h>
//#include <MicroOcpp/Core/SimpleRequestFactory.h>
//#include "tests/catch2/catch.hpp"
//#include "tests/helpers/testHelper.h"



#include "config.h"
#include "conf2.h"

#include "broker.h"

#include "sdcard_api.h"

#include "uiScreen.h"
// Set all possible Inputs and outputs
std::array<bool, 1024> checkpoints {false};
size_t ncheck = 0;
#include "esp_system.h"
String readRecoverymode;
int recoveryMode = false;
#define BASE_TIME "2023-01-01T00:00:00.000Z"
#define SCPROFILE "[2,\"testmsg\",\"SetChargingProfile\",{\"connectorId\":0,\"csChargingProfiles\":{\"chargingProfileId\":0,\"stackLevel\":0,\"chargingProfilePurpose\":\"ChargePointMaxProfile\",\"chargingProfileKind\":\"Absolute\",\"chargingSchedule\":{\"duration\":1000000,\"startSchedule\":\"2023-01-01T00:00:00.000Z\",\"chargingRateUnit\":\"W\",\"chargingSchedulePeriod\":[{\"startPeriod\":0,\"limit\":16,\"numberPhases\":3}]}}}]"

typedef enum {
    AC_EVSEOcppStatus_NO_PROCESS_OCPP = 0,
    AC_EVSEOcppStatus_BEGIN_TRANSACTION = 1,
    AC_EVSEOcppStatus_TRANSACTION_INITIATED = 2,
    AC_EVSEOcppStatus_NO_TRANSACTION_INITIATED = 3,
    AC_EVSEOcppStatus_END_TRANSACTION = 4,
    AC_EVSEOcppStatus_CANNOT_END_TRANSACTION = 5
} AC_EVSE_OcppStatus;

typedef enum {
    AC_EVSEChargeStatus_NO_CHARGING = 0,
    AC_EVSEChargeStatus_CHARGING = 1,
} AC_EVSE_ChargeStatus;

typedef enum {
    AC_EVSEConnectorStatus_NOT_PLUGGED = 0,
    AC_EVSEConnectorStatus_PLUGGED = 1,
} AC_EVSE_ConnectorStatus;

typedef struct {
    uint16_t ocppStatus;
    uint16_t chargeStatus;
    uint16_t connector;
    uint32_t activeTran;
} AC_EVSE_Status;

typedef struct {
    AC_EVSE_Status status;
    float power;
    float energy;
    uint8_t unlockConnReq;
} AC_EVSE;

typedef void (*OcppUpdateCallback)(AC_EVSE*, bool, String);

int transactionId = 0;
const char *transactionIdc;
time_t ocppStartTime = 0;
AC_EVSE ocppData;
static bool stopTransactionInProgress = false;

bool isPowerLossDetected() {
    esp_reset_reason_t r = esp_reset_reason();

    // POWERON veya BROWNOUT ise power loss kabul edilir
    if (r == ESP_RST_POWERON || r == ESP_RST_BROWNOUT) {
        return true;
    }
    return false;
}
void handleRemoteStopTran() {
    setOnSendConf("RemoteStopTransaction", [] (JsonObject payload) -> void {

        String transactionIdStr = String(transactionId);

        if(transactionId == 0){ 
            JsonObject response;
            response["status"] = "Rejected"; // set status to "Rejected"

            // Send the rejection response manually using sendRequest (or setOnSendConf if you need to handle it on the response side)
            sendRequest("StopTransaction", [] () -> std::unique_ptr<DynamicJsonDocument> {
                size_t capacity = JSON_OBJECT_SIZE(1);  // Calculate the required capacity
                auto res = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(capacity));
                JsonObject request = res->to<JsonObject>();
                request["status"] = "Rejected";  // Set the rejection status in the response
                return res;  // Return the rejection response
            }, [] (JsonObject response) -> void {
            });

            return; 
        }
        else{
            endTransaction(transactionIdStr.c_str(), "Remote"); // end transaction and send StopTransaction
            if (!strcmp(payload["status"], "Rejected")) {
                // the OCPP lib rejected the RemoteStopTransaction command.
                // customer wishes to stop the running transaction in any case
                endTransaction_authorized(transactionIdc, "Remote"); // end transaction and send StopTransaction
            }
        }
    });   
}

void handleRemoteStartTran() {
    setOnSendConf("RemoteStartTransaction", [] (JsonObject payload) -> void {
        beginTransaction_authorized(transactionIdc);
    }); 
}

bool rebootRequired = false;
bool configRejected = false;

unsigned long custom_timer_cb() {
    return millis();
}

void microocppCallbacks() {
    setConnectorPluggedInput([c = &checkpoints[ncheck++]] () -> bool { *c = true; return ocppData.status.connector; });
    setEnergyMeterInput([c = &checkpoints[ncheck++]] () -> float { *c = true;return ocppData.energy;});
    setPowerMeterInput([c = &checkpoints[ncheck++]] () -> float { *c = true; return ocppData.power; });

    setOnReceiveRequest("StatusNotification", [c = &checkpoints[ncheck++]] (JsonObject) {*c = true; });
    setOnSendConf("StatusNotification", [c = &checkpoints[ncheck++]] (JsonObject) {*c = true; });

    /*sendRequest("DataTransfer", [c = &checkpoints[ncheck++]] () {
        *c = true;
        auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1)));
        doc->to<JsonObject>();
        return doc;
    }, [c = &checkpoints[ncheck++]] (JsonObject) {*c = true; });*/

    setRequestHandler("DataTransfer", [c = &checkpoints[ncheck++]] (JsonObject) {*c = true;}, [c = &checkpoints[ncheck++]] () {
        *c = true;
        auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1)));
        doc->to<JsonObject>();
        return doc;
    });

    setRequestHandler("ChangeConfiguration", 
        [c = &checkpoints[ncheck++], &rebootRequired, &configRejected](JsonObject payload) {
            *c = true;
            
            rebootRequired = false;
            configRejected = false;

            const char* key = payload["key"];
            const char* value = payload["value"];

            if (strcmp(key, "bt110Qr") == 0) {
                mqttDataValue.setQR_ID(value);
                char buffer[128] = {0};
                strncpy(buffer, value, sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';
                qrCode_id(buffer);
            } 
            else if (strcmp(key, "bt110ChargeBoxUrl") == 0) {
              String fullUrl = value;

              // Sadece ws:// veya wss:// ile başlıyorsa geçerli
              if (!(fullUrl.startsWith("ws://") || fullUrl.startsWith("wss://"))) {
                  configRejected = true;
                  return;
              }

              int protoLen = fullUrl.startsWith("wss://") ? 6 : 5;

              int lastSlashIndex = fullUrl.lastIndexOf('/');
              if (lastSlashIndex < protoLen + 1 || lastSlashIndex >= fullUrl.length() - 1) {
                  configRejected = true;
                  return;
              }

              String backendUrl = fullUrl.substring(0, lastSlashIndex);
              String chargeBoxId = fullUrl.substring(lastSlashIndex + 1);

              if (chargeBoxId.length() == 0) {
                  configRejected = true;
                  return;
              }

              mqttDataValue.setOCPP_URL(backendUrl.c_str()); 
              mqttDataValue.setOCPP_ID(chargeBoxId.c_str());
              rebootRequired = true;
          }

            else if (strcmp(key, "bt110ChargeBoxId") == 0) {
                mqttDataValue.setOCPP_ID(value);
                rebootRequired = true;
            }
            else if (strcmp(key, "bt110GsmApn") == 0) {
                // örnek: config işlemi
            }
        }, 
        [c = &checkpoints[ncheck++], &rebootRequired, &configRejected]() {
            *c = true;

            auto doc = std::unique_ptr<DynamicJsonDocument>(new DynamicJsonDocument(JSON_OBJECT_SIZE(1)));
            JsonObject response = doc->to<JsonObject>();

                return doc;
            if (configRejected) {
                response["status"] = "Rejected";
            } else if (rebootRequired) {
                response["status"] = "RebootRequired";
            } else {
                response["status"] = "Accepted";
            }

            return doc;
        }
    );

    setOnUnlockConnectorInOut([c = &checkpoints[ncheck++]] () -> UnlockConnectorResult {
      *c = true; 
      ocppData.unlockConnReq = 1;
      return UnlockConnectorResult_Unlocked;
      });

}

void ocpp_isActiveTransaction(AC_EVSE *ocppData){
    auto tx = getTransaction(); 
    if (tx) { 
      transactionId = tx->getTransactionId(); 
      transactionIdc = getTransactionIdTag();
      ocppData->status.activeTran = uint32_t(tx->isActive());
    }
    else{
        ocppData->status.activeTran = 0;
    }
}
// Abdulhamit's init
void ocpp_init(const String backendUrl, const String chargeBoxId,const String chargePointVendor,const String chargePointModel,const String firmwareVersion,const String chargePointSerialNumber,const String meterSerialNumber,const String meterType,const String chargeBoxSerialNumber) {

    ocppData.unlockConnReq = 0;
    MicroOcpp::LoopbackConnection loopback;

    static String iccidS, imsiS;

    bool gotIccid = false;
    bool gotImsi  = false;

    if (netType == NET_TYPE_GSM || (netType == NET_TYPE_ALL && comsupportedNetwork == NET_TYPE_GSM)) {
      gotIccid = GetICCID(SerialAT, iccidS, 2500);
      gotImsi  = GetIMSI(SerialAT, imsiS, 2500);
    }

    const char *iccid = gotIccid ? iccidS.c_str() : nullptr;
    const char *imsi  = gotImsi  ? imsiS.c_str()  : nullptr;

    const char* url = (backendUrl.length() ? backendUrl.c_str() : OCPP_BACKEND_URL);
    const char* id  = (chargeBoxId.length()  ? chargeBoxId.c_str()  : OCPP_CHARGE_BOX_ID);

    mocpp_initialize(url, id, OCPP_CHARGE_POINT_MODEL, OCPP_CHARGE_POINT_VENDOR);

    auto *ctx = getOcppContext();
    if (ctx) {
        auto &model = ctx->getModel();
        MicroOcpp::BootService *boot = model.getBootService();

        if (boot) {
            StaticJsonDocument<512> creds;

            creds["chargePointVendor"]       = chargePointVendor.length()       ? chargePointVendor.c_str()       : nullptr;
            creds["chargePointModel"]        = chargePointModel.length()        ? chargePointModel.c_str()        : nullptr;
            creds["firmwareVersion"]         = firmwareVersion.length()         ? firmwareVersion.c_str()         : nullptr;
            creds["chargePointSerialNumber"] = chargePointSerialNumber.length() ? chargePointSerialNumber.c_str() : nullptr;
            creds["meterSerialNumber"]       = meterSerialNumber.length()       ? meterSerialNumber.c_str()       : nullptr;
            creds["meterType"]               = meterType.length()               ? meterType.c_str()               : nullptr;
            creds["chargeBoxSerialNumber"]   = chargeBoxSerialNumber.length()   ? chargeBoxSerialNumber.c_str()   : nullptr;

            creds["iccid"] = iccid;
            creds["imsi"]  = imsi;

             char out[512];
             serializeJson(creds, out, sizeof(out));
             boot->setChargePointCredentials(out);
        }
    }
   
    
    microocppCallbacks();
    handleRemoteStopTran();
    handleRemoteStartTran();

   static float lossStopValue = 0.0f;

    lossStopValue = readEnergy();

    ocpp_isActiveTransaction(&ocppData);
    if(ocppData.status.activeTran == 0){
        
    }
    else{
        if(isPowerLossDetected()){
            if (auto tx = getTransaction()) {
                if (tx) {
                  tx->setMeterStop(lossStopValue); 
                }
            }
            endTransaction(nullptr, "PowerLoss");
        }else{
          esp_reset_reason_t reason = esp_reset_reason();
          //Serial.print("girdi-1\n");

          if (reason == ESP_RST_PANIC ||reason == ESP_RST_INT_WDT ||reason == ESP_RST_TASK_WDT ||reason == ESP_RST_WDT  ){  Nvsmem::loadString("recoverymode", "Activate");}
       
          readRecoverymode = Nvsmem::loadString("recoverymode");
          
          if(readRecoverymode == "Activate") {
            recoveryMode = true;
            
            }else {
                if (auto tx = getTransaction()) {
                    if (tx) {
                    tx->setMeterStop(lossStopValue); 
                    }
                }
                endTransaction(nullptr, "Local"); 
            }
              

           /*  if (auto tx = getTransaction()) {
                if (tx) {
                  tx->setMeterStop(lossStopValue); 
                }
            }
            endTransaction(nullptr, "Local");*/

         /* setEnergyMeterInput([c = &checkpoints[ncheck++]] () -> float { *c = true; return  lossStopValue; });
             //endTransaction(nullptr, "EVDisconnected");
              stopTransaction(); */
        }
    } 
}

/*// burak's init
void ocpp_init(const String backendUrl, const String chargeBoxId) {
    ocppData.unlockConnReq = 0;
    MicroOcpp::LoopbackConnection loopback;

 
  const char* url = (backendUrl.length() ? backendUrl.c_str() : OCPP_BACKEND_URL);
  const char* id  = (chargeBoxId.length()  ? chargeBoxId.c_str()  : OCPP_CHARGE_BOX_ID);
   
  mocpp_initialize(url, id, OCPP_CHARGE_POINT_MODEL, OCPP_CHARGE_POINT_VENDOR);
   /* int i;
      for (i = 0; i < 20000; ++i) {
          Serial.print('.'); delay(500);
      }*/
    /*
    mocpp_initialize(OCPP_BACKEND_URL, config.chargeBoxId, OCPP_CHARGE_POINT_MODEL, OCPP_CHARGE_POINT_VENDOR);
    mocpp_initialize(OCPP_BACKEND_URL, OCPP_CHARGE_BOX_ID, OCPP_CHARGE_POINT_MODEL, OCPP_CHARGE_POINT_VENDOR);
    */

/*
    auto context = getOcppContext();
    auto& model = context->getModel();

    model.getClock().setTime(BASE_TIME);
*/
    
    /*mocpp_set_timer(custom_timer_cb);
    model.getClock().setTime(BASE_TIME);*/
/*




    microocppCallbacks();
    handleRemoteStopTran();
    handleRemoteStartTran();

   static float lossStopValue = 0.0f;

    lossStopValue = readEnergy();

    ocpp_isActiveTransaction(&ocppData);
    if(ocppData.status.activeTran == 0){
    }
    else{
      
            if (auto tx = getTransaction()) {
                if (tx) {
                    tx->setMeterStop(lossStopValue); 
                }
            }
            endTransaction(nullptr, "PowerLoss");       
    } 
}*/

void ocpp_rfidProcess(AC_EVSE *ocppData, uint16_t rfidTriggered, String rfidTag) {
    if (rfidTriggered) {
        if (!getTransaction()) {
            // no transaction running or preparing. Begin a new transaction
            ocppData->status.ocppStatus = AC_EVSEOcppStatus_BEGIN_TRANSACTION;
            // Begin Transaction. The OCPP lib will prepare transaction by checking the Authorization
            // and listen to the ConnectorPlugged Input. When the Authorization succeeds and an EV
            // is plugged, the OCPP lib will send the StartTransaction
            auto ret = beginTransaction(rfidTag.c_str());

            if (ret) {
                ocppData->status.ocppStatus = AC_EVSEOcppStatus_TRANSACTION_INITIATED;
            } else {
                ocppData->status.ocppStatus = AC_EVSEOcppStatus_NO_TRANSACTION_INITIATED;
            }

        } else {
            // Transaction already initiated. Check if to stop current Tx by RFID card
            if (rfidTag.equals(getTransactionIdTag())) {
                // card matches -> user can stop Tx
                ocppData->status.ocppStatus = AC_EVSEOcppStatus_END_TRANSACTION;
                endTransaction(rfidTag.c_str());   
            } else {
                ocppData->status.ocppStatus = AC_EVSEOcppStatus_CANNOT_END_TRANSACTION;
            }
        } 
    }
}



uint16_t runPermission = 0;
void ocpp_update(AC_EVSE *ocppData) {
    mocpp_loop();

    // Energize EV plug if OCPP transaction is up and running
    if (ocppPermitsCharge() && (runPermission == 1)) {
        // OCPP set up and transaction running. Energize the EV plug here
        ocppData->status.chargeStatus = AC_EVSEChargeStatus_CHARGING;
    } else {
        // No transaction running at the moment. De-energize EV plug
        ocppData->status.chargeStatus = AC_EVSEChargeStatus_NO_CHARGING;
    }
}

void ocpp_update_WithRfid(AC_EVSE *ocppData, uint16_t rfidTriggered, String rfidTag) {
    ocpp_update(ocppData);
    ocpp_rfidProcess(ocppData, rfidTriggered, rfidTag);
}

void ocpp_update_WithoutRfid(AC_EVSE *ocppData) {
    ocpp_update(ocppData);
}

void ocppUpdateCallback(AC_EVSE* ocppData, bool detected, String tag) {
    ocpp_update_WithRfid(ocppData, detected, tag);
}

void xocppChargeFinishedfromEV(AC_EVSE* ocppData, uint16_t chargeFinished) {
    if (chargeFinished == 1) {
        String tranID = String(transactionId);
        stopTransaction([c = &checkpoints[ncheck++]](JsonObject) { *c = true; });
        ocppData->status.ocppStatus = AC_EVSEOcppStatus_END_TRANSACTION;
    }
}


void ocppChargeFinishedfromEV(AC_EVSE* ocppData, uint16_t chargeFinished) {
     static bool stopTransactionInProgress = false;

    if (chargeFinished == 1 && !stopTransactionInProgress) {
        const char* idTag = nullptr;
        const char* reason = "EVDisconnected";

        bool stopped = endTransaction(idTag, reason);

        if (stopped) {
            stopTransactionInProgress = true;
            ocppData->status.ocppStatus = AC_EVSEOcppStatus_END_TRANSACTION;
        }
    }

    // Burada status END_TRANSACTION ise ve işlem tamamlandıysa flag'i sıfırla
    if (ocppData->status.ocppStatus == AC_EVSEOcppStatus_END_TRANSACTION) {
        // Bu satır, durumu başka yerde resetliyorsan kaldır
        stopTransactionInProgress = false;
    } 
}

#if (FRACTION_DIGIT == 1)
void ocppDataUpdate(float energy, float power, uint16_t connStatus){
    ocppData.energy = energy * 10.0;  
    ocppData.power = power * 100.0;
    ocppData.status.connector = connStatus; 
}
#elif (FRACTION_DIGIT == 2)
void ocppDataUpdate(float energy, float power, uint16_t connStatus){
    ocppData.energy = energy * 10.0;  
    ocppData.power = power * 10.0;
    ocppData.status.connector = connStatus; 
}
#endif 

#include "board_network.h"
bool shouldCallOcppUpdate(bool gprsConnected) {
    static bool previous = false;
    static unsigned long allowUntil = 0;

    unsigned long now = millis();

    // GPRS 0 -> 1 geçişinde 20 saniyelik pencere aç
    if (!previous && gprsConnected) {
      SerialAT.println("AT");
      if (SerialAT.find("OK")) {
          allowUntil = now + 20000;
      }
      else{
        return false;
      }
    }

    previous = gprsConnected;

    // GPRS bağlıysa zaten çalıştır
    if (gprsConnected) return true;

    // GPRS bağlı değil ama 20 saniyelik pencere içindeysek yine çalıştır
    if (now < allowUntil) return true;

    return false;
}


bool xshouldCallOcppUpdate(bool gprsConnected) {
  static uint16_t gprsConnectedOld = 0;
  static unsigned long forceOcppStartTime = 0;
  static bool forceOcppEnabled = false;

  unsigned long now = millis();

  // GPRS ilk kez geldiğinde 20 saniyelik izin ver
  if (gprsConnectedOld == 0 && gprsConnected == 1) {
    forceOcppEnabled = true;
    forceOcppStartTime = now;
  }

  bool result = false;

  // Eğer GPRS bağlıysa normal çalıştır
  if (gprsConnected) {
    result = true;
  }
  // Veya GPRS gelmemiş olsa bile, ilk geldiği andan itibaren 20 saniye boyunca çalıştır
  else if (forceOcppEnabled && (now - forceOcppStartTime < 20000)) {
    result = true;
  }
  // 20 saniye dolduysa bu özel modu kapat
  else if (forceOcppEnabled && (now - forceOcppStartTime >= 20000)) {
    forceOcppEnabled = false;
    result = false;
  }

  gprsConnectedOld = gprsConnected;

  return result;
}
void stopTransactionWithMeter(float meterStopValue, const char *reason = "EVDisconnected") {

    if (auto tx = getTransaction()) {
        if (tx) {
            tx->setMeterStop(meterStopValue);
        }

        endTransaction(nullptr, reason);
    }
}
#endif /* OCPP_API_H */
