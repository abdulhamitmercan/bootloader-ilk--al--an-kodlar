#include "config.h"


#include "charger_api.h"

#include "timeLib.h"
#include "config.h"
#if RUN_MODE == 3
#include <MicroOcpp/Core/Connection.h>
#endif

#include <otadrive_esp.h>
#include <Update.h>
#include <CRC32.h>
#include "FS.h"
#include "SPIFFS.h"
#include "conf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_task_wdt.h"

#include <stdlib.h>


#include "mqtt_api.h"
#include "timer.h"
#include "otaUpdate.h"
#include "dynamicLogo.h"
#include "mspBootloader.h"
// Mutex global
SemaphoreHandle_t xMutex;
SemaphoreHandle_t xMutex2;

uint16_t startOcppUpdate = 0;
int err = 0;
uint8_t uartTestVal = 0;
uint16_t unlockConn = 0;
uint8_t resetcountVal = 0;
uint16_t resetVal = 0;
uint16_t mqttunlock = 0;
uint16_t mqttstopcharge = 0;
uint16_t mqttstopchargeold = 0;
uint16_t mqttUnlockRspUartTest = 0;
uint16_t mqttocppActive = 0;
uint8_t status = 0;
volatile uint32_t ocppTimer = 0;
uint16_t buzzer = 0;
uint16_t clearSession = 0;
uint16_t loopCounter = 0;
uint16_t mqttOtaActive = 0;
uint8_t uartActive = 1;
const int WDT_TIMEOUT = 15;
t_EVSE confParams;
int param=0;
static bool prevTran = false;
uint8_t MSPResetTick = 0;
String readRecovery;
static float recoverStopValue = 0.0f;

bool internetAvailableAtBoot = false;

static constexpr char FW_VER_COMPILED[] = "v@4.0.4";

bool recoveryFakeActive = false;
unsigned long recoveryStartTime = 0;
const unsigned long RECOVERY_FAKE_DURATION = 13000UL; // 13 sn

bool vans = true;

esp_timer_handle_t recoveryTimer;
static bool recoveryLock = false;
static bool resetLock = false;
static bool readState = true; 

uint16_t ota_rescue = 0;
void recoveryCheckTask(void* arg) {

    forceHitConnectorStatusOnce(200,comm.rx);
    static uint32_t recoveryTaskStart = millis();
    esp_reset_reason_t reason = esp_reset_reason();
    //printResetReason();

    //Serial.print("girdi-1\n");
    
    if (reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT ||reason == ESP_RST_TASK_WDT ||reason == ESP_RST_WDT  ) {
      if(vans){
          if (initialConnectorStatus == 1 && initialMspState == "C2") {
              Nvsmem::loadString("recovery", "Unblocked");
             // Serial.print("1.if girdi\n");
          } else {
              Nvsmem::loadString("recovery", "blocked");
             // Serial.print("1.5 if girdi\n");
          }
          vans = false ;
      }
    }
        
    //Serial.print("girdi\n");

    readRecovery = Nvsmem::loadString("recovery");

    if (readRecovery == "Unblocked") {

        if (initialConnectorStatus == 1 && initialMspState == "C2") {
            Nvsmem::loadString("recovery", "Unblocked");
           // Serial.print("2.if girdi\n");
        } else {
            Nvsmem::loadString("recovery", "blocked");
           // Serial.print("2.5 if girdi\n");
        }
    }   
/*   
 Serial.print("initialConnectorStatus=");
Serial.println(initialConnectorStatus);



readRecovery = Nvsmem::loadString("recovery");
Serial.print("readRecovery=[");
Serial.print(readRecovery);
Serial.println("]");   */

 
    
}

void startRecoveryTimer() {

    esp_timer_create_args_t timer_args = {
        .callback = &recoveryCheckTask,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "recoveryTimer"
    };

    esp_timer_create(&timer_args, &recoveryTimer);

    // 1 saniye 1000000
    esp_timer_start_periodic(recoveryTimer, 1000000);

    Serial.println("Recovery timer started");
}


void stopRecoveryTimer() {

    esp_timer_stop(recoveryTimer);
    Serial.println("Recovery timer stopped");
}


void recreateTaskPinned(TaskHandle_t* taskHandle, TaskFunction_t taskCode, const char* name,
                        UBaseType_t priority, uint16_t stackSize, BaseType_t coreID) {
  BaseType_t res = xTaskCreatePinnedToCore(
      taskCode,
      name,
      stackSize,
      NULL,
      priority,
      taskHandle,
      coreID);  // Core 0 veya 1

  if (res != pdPASS) {
    printf("Task %s could not be recreated!\n", name);
    while (1); // Hatalı durum
  }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  if (xTask == Task1) {
    vTaskDelete(Task1);
    Task1 = NULL;
    recreateTaskPinned(&Task1, Task1code, "Task1", 1, 5000, 0); // Core 0
  } 
  else if (xTask == Task2) {
    vTaskDelete(Task2);
    Task2 = NULL;
    recreateTaskPinned(&Task2, Task2code, "Task2", 1, 10000, 0);
  } 
  else if (xTask == Task3) {
    vTaskDelete(Task3);
    Task3 = NULL;
    recreateTaskPinned(&Task3, Task3code, "Task3", 1, 10000, 0);
  } 
  else {
    vTaskDelete(xTask);
  }
}



uint8_t initializeModules() {
  //configurationInit();
//#if (RUN_MODE == 1 || RUN_MODE == 2)
  rfid_init();
  sd_init();
//#endif

//#if (RUN_MODE == 1)
    if(activeRunMode == MODE_RFID_TAK_CALISTIR){
      initTagQueue(tagQueue);
      loadTagsFromEEPROM(tagQueue);
    }

//#endif
   
   if (activeRunMode == MODE_OCPP) {startRecoveryTimer();}
   else { mspHardReset1(); mqttDataValue.setOCPP_ACTIVE("0");}
   resetLock = true;
  fsm_init();
  charger_init(&chargerData);
  charger_init(&chargerDataNew);
  charger_init(&chargerDataOld);
  charger_init(&chargerDataAfterResetNew);
  charger_init(&chargerDataAfterResetOld);
    
   
#if (MQTT_ACTIVE == 0)
  mqttDataValue.setQR_ID("10340085011");
#endif

  const char* constQr = mqttDataValue.getQR_ID();
  char OCPP_QR[100];
  strcpy(OCPP_QR, constQr);


  ui_init(OCPP_QR);
  

 
  assignUint16(mqttDataValue.getOTA_UPDATE_ACTIVE(), mqttOtaActive);
  if (mqttOtaActive == 1 || mqttOtaActive == 2 ) {
    stopRecoveryTimer();
    write_Screen_Uploading_Start();
    startLvglTask();
    ota_rescue = 1 ;
  }
   //readRecovery = Nvsmem::loadString("recovery");
   //if (readRecovery == "blocked")  mspHardReset1();
   internetAvailableAtBoot = netInit(WIFI_SSid, WIFI_Pass);

readRecovery = Nvsmem::loadString("recovery");

if (!internetAvailableAtBoot) {

    if (activeRunMode != MODE_OCPP) {

        noInternetMode();

    } else {

        if (readRecovery == "Unblocked") {

            String bootFailCountStr = Nvsmem::loadString("bootFailCount");
            uint16_t bootFailCount = bootFailCountStr.toInt();

            bootFailCount++;

            Nvsmem::loadString("bootFailCount", String(bootFailCount));

            Serial.print("bootFailCount=");
            Serial.println(bootFailCount);

            if (bootFailCount >= 10) {

                Serial.println("10 kez recovery + internet yok -> MSP reset");

                mspHardReset1();

                Nvsmem::loadString("bootFailCount", "0");
            }
        }

        ESP.restart();
        return 0;
    }

} else {

    // internet gldiyse sayaç temizler
    Nvsmem::loadString("bootFailCount", "0");
}
  if (internetAvailableAtBoot) {
    if(strcmp(mqttDataValue.getUI_LG_TXT(), "1") == 0) dynamicLogoInitFromMqttConfig();
    
    printResetReason();

      if (mqttOtaActive==1 ){
        startOtaUpdate();
      }

      if (mqttOtaActive == 2) {
          bool ok = startMspUpdate();
          Serial.println( ok ? "[MSP OTA] SUCCESS" : "[MSP OTA] FAIL" );

          return 1;
      }    

    #if (MQTT_ACTIVE == 1)
      mqtt_init();
    #else
      const char* a = "ws://iottest.oncharge.xyz";
      const char* b = "onc1";

      mqttDataValue.setOCPP_URL(a);
      mqttDataValue.setOCPP_ID(b);
    #endif
 }
/*
  const char* a = "ws://ocpp-uat.e4sarj.com.tr/ocpp";
  const char* b = "TT_B1";

  mqttDataValue.setOCPP_URL(a);
  mqttDataValue.setOCPP_ID(b);
*/
  if (activeRunMode == MODE_OCPP) {
    String urlS    = String(mqttDataValue.getOCPP_URL());
    String idS     = String(mqttDataValue.getOCPP_ID());
    String vendorS = String(mqttDataValue.getC_P_VENDOR());
    String modelS  = String(mqttDataValue.getC_P_MODEL());
    String cpSerS  = String(mqttDataValue.getC_P_SER_NUM());
    String metSerS = String(mqttDataValue.getMET_SER_NUM());
    String metTypeS= String(mqttDataValue.getMET_TYPE());
    String boxSerS = String(mqttDataValue.getBOX_SER_NUM());
    String firmwareVersionS =String(mqttDataValue.getOTA_VER());

    ocpp_init(urlS, idS,vendorS, modelS,firmwareVersionS,cpSerS,metSerS, metTypeS,boxSerS);
      
      readRecovery = Nvsmem::loadString("recovery");
      if ( recoveryMode == true){
        //SerialAT.println("3.if girdi\n");
        if (readRecovery == "blocked" ){
        Nvsmem::loadString("recoverymode","Deactivate");
        recoveryMode=false;
        recoverStopValue = readEnergy();
          if (auto tx = getTransaction()) {
            if (tx) {
                tx->setMeterStop(recoverStopValue); 
            }
          endTransaction(nullptr, "EVDisconnected");
          // SerialAT.println("4.if girdi\n");
          }

        }
      }

      if (readRecovery == "blocked" || recoveryMode == false ){stopRecoveryTimer(); recoveryLock = true; mspHardReset1();}
      resetLock = false;
   return 1;
  } 
//SerialAT.println("girdi4\n");

  return 1;
}
uint32_t tas4Flg = 0;

void setup() {
   assignUint16(mqttDataValue.getESP_MODE(), activeRunMode);
 
   Serial.begin(115200);
   
   loadWifiCredentials(); 

   comm_init();
   forceHitConnectorStatusOnce(1200,comm.rx);  
   

  if (activeRunMode != MODE_OCPP) noInternetMode();

  
generateAndSetSerialFromMac();

  xMutex = xSemaphoreCreateMutex();
  xMutex2 = xSemaphoreCreateMutex();
  if (xMutex == NULL) {
    while (1)
      ;  // Mutex oluşturulamadıysa takıl
  }
  if (xMutex2 == NULL) {
    while (1)
      ;  // Mutex oluşturulamadıysa takıl
  }
  
  if (initializeModules() != 1) {
    return;
  }
  gprsConnected = 1;
  change_screen4_1();

  initScreenNetIcon(internetAvailableAtBoot);
  

  setupTasks(Task1code, Task2code, Task3code);

  
 if (activeRunMode == MODE_OCPP) xTaskNotifyGive(Task2);


  startOcppUpdate = 1;
  rfid_FinishRequest(&rfid1);

  wdtSetup(20000);//20sn
  confParamsInit(&confParams);

  mqttDataValue.setOTA_VER(FW_VER_COMPILED);
  assignUint16(mqttDataValue.getOCPP_ACTIVE(), mqttocppActive);
  if(ota_rescue) ESP.restart(); 

}

uint16_t mqtt_first = 0;
uint16_t ocppUpdateCounter1 = 0;
uint16_t ocppUpdateCounter2 = 0;
uint16_t mqttRun = 0;
unsigned long task4Counter = 0;

unsigned long prevUpdateTime = 0;
unsigned long prevLvglTime = 0; 
const unsigned long updateInterval = 5000;   // 10 saniye
const unsigned long lvglInterval  = 5;    
void loop() {

  if (activeRunMode == MODE_OCPP){
    if (netType==NET_TYPE_WIFI||netType==NET_TYPE_ETH||(netType==NET_TYPE_ALL && comsupportedNetwork==NET_TYPE_WIFI)||(netType==NET_TYPE_ALL && comsupportedNetwork==NET_TYPE_ETH) && mqttocppActive) {
        
        ocppUpdate(&rfid1, chargerData.rfid.response, ocppUpdateCallback);
        mqttRun = 1;   
  } }



if (ocppTimer >= 5) {
  if (activeRunMode == MODE_OCPP){
    // GSM OCPP UPDATE
    if (netType == NET_TYPE_GSM || 
       (netType == NET_TYPE_ALL && comsupportedNetwork == NET_TYPE_GSM)) 
    {
        if (shouldCallOcppUpdate(gprsConnected) && mqttocppActive) {
            if (xSemaphoreTake(xMutex2, portMAX_DELAY) == pdTRUE) {
                ocppUpdate(&rfid1, chargerData.rfid.response, ocppUpdateCallback);
                xSemaphoreGive(xMutex2);
                mqttRun = 1;
            } else {
                mqttRun = 0;
                disconnected = 1;
            }
        } else {
            mqttRun = 0;
            disconnected = 1;
        }
    }
  }
    ocppTimer = 0;
}
/*
    // MQTT handling
#if (MQTT_ACTIVE == 1)
    if ((isInternetAlive() || mqtt_first == 0) && mqttRun) {
        if (xSemaphoreTake(xMutex2, portMAX_DELAY) == pdTRUE) {
            if (!mqtt.connected()) {
                mqttReconnectNonBlocking();
            } else {
                mqtt.loop();
            }
            xSemaphoreGive(xMutex2);
        }
        mqtt_first = 1;
    }
#endif

    ocppTimer = 0;
}
#endif
*/
//30:ED:A0:2F:CD:C8
#if (MQTT_ACTIVE == 1)
if(internetAvailableAtBoot){
  if (gprsRetryCount < GPRS_RETRY_LIMIT) {
    if (xSemaphoreTake(xMutex2, pdMS_TO_TICKS(200)) == pdTRUE) {
      if (!mqtt.connected()) {
        if(!mqttocppActive){
          GPRS_RETRY_LIMIT = 30;
        }
        
        mqttReconnectNonBlocking();
      } else {
          mqtt.loop();
          if(!mqttocppActive){
            GPRS_RETRY_LIMIT = 120;
          } 
        }
      xSemaphoreGive(xMutex2);
    }
  }
}
#endif


 
 


  if (activeRunMode == MODE_OCPP){
    if (ctrl.rfidHandle && rfidAvailable) {
      rfidRead(&rfid1, 100);
    }
  }  


//#if RUN_MODE == 1
 if (activeRunMode == MODE_RFID_TAK_CALISTIR){
      if (ctrl.rfidHandle ) {
        rfidRead(&rfid1, 100);
      }
      //if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        runCtrl(&rfid1, chargerData.rfid.response);
      //  xSemaphoreGive(xMutex);
      //}
 }
//#endif

 
  unsigned long now = millis();
 //if(netType==NET_TYPE_GSM||(netType==NET_TYPE_ALL && comsupportedNetwork==NET_TYPE_GSM)){ 
     if (now - prevLvglTime >= lvglInterval) {
        prevLvglTime = now;
            lv_timer_handler();           
      }
  //}
    if (FSM.current_state == P4_CHARGING) {

        if (now - prevUpdateTime >= updateInterval) {
            prevUpdateTime = now;

            write_Screen_Energy(chargerData.meter.energy);
            write_Screen_Power(chargerData.meter.power);
            write_Screen_Chargetime(chargerData.time.hours,
                                    chargerData.time.minutes);
        }
    }
    

}

uint16_t c = 0;
uint16_t csqTime = 0;
void Task1code(void* pvParameters) {
  delay(15000);
  while (1) {

    

//isMqttTcpAlive()
if(internetAvailableAtBoot){
    switch (netType) {
      case NET_TYPE_ALL :{ 
          switch (comsupportedNetwork){
              case NET_TYPE_GSM:
                if (isOcppActive) gsmReconnect(isInternetAlive(), &gprsConnected); 
                else   gsmReconnect(isMqttTcpAlive(), &gprsConnected); 
              break;
              case NET_TYPE_WIFI:
                if (isOcppActive) wifiReconnect(isInternetAlive(), &gprsConnected,WIFI_SSid,WIFI_Pass);
                else   wifiReconnect(isMqttTcpAlive(), &gprsConnected,WIFI_SSid,WIFI_Pass);
              break;
              case NET_TYPE_ETH:
                if (isOcppActive) ethernetReconnect(isInternetAlive(), &gprsConnected);
                else  ethernetReconnect(isMqttTcpAlive(), &gprsConnected);
              break;
          }
        break;  
      }        
      case NET_TYPE_GSM: 
          if (isOcppActive) gsmReconnect(isInternetAlive(), &gprsConnected); 
          else  gsmReconnect(isMqttTcpAlive(), &gprsConnected);  
        break;

      case NET_TYPE_WIFI:
          if (isOcppActive) wifiReconnect(isInternetAlive(), &gprsConnected,WIFI_SSid,WIFI_Pass);
          else   wifiReconnect(isMqttTcpAlive(), &gprsConnected,WIFI_SSid,WIFI_Pass); 
      break;

      case NET_TYPE_ETH: 
          if (isOcppActive) ethernetReconnect(isInternetAlive(), &gprsConnected);
          else  ethernetReconnect(isMqttTcpAlive(), &gprsConnected);
      break;
      default: break;
    }
}

  
//#if RUN_MODE == 1
  if (activeRunMode == MODE_RFID_TAK_CALISTIR){
    //if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
     if (FSM.current_state == P2_PLUG_IN) handleRfidManageMode(&rfid1, &rfidManageEnable);
    //  xSemaphoreGive(xMutex);
    //}
  }
//#endif
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

uint16_t delCount = 0;
void mainUpdate() {
    static unsigned long MSPHardRestarTimerStart = 0;
    static uint8_t transectionlock =0;
    static uint32_t recoveryStart = 0;
    const unsigned long MSP_HARD_RESET_INTERVAL = 13000UL; // 13 sn
    unsigned long present = millis(); 
  //if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    confParams.reset = toUint16OrKeep(mqttDataValue.getCONF_PARAMS_RESET(), confParams.reset);
    uint8_t confVal = setConfiguration(&confParams);
    if(rfidAvailable) chargerData.rfid.request = rfid1.request;
    uartTestVal = (uartTestVal + 10) % 100;
    resetcountVal = (resetcountVal + 5) % 140;  //   BROKER MSP RESET
    status = chargerData.chargeStatus << 2 | chargerData.plugStatus << 1 | 0;

    fsm_update(status, &FSM);
    ui_applyPageChanges(&FSM.Out);
    charger_update(&ctrl, FSM.current_state, &rfid1);

    //if (netType==NET_TYPE_WIFI||netType==NET_TYPE_ETH||(netType==NET_TYPE_ALL && comsupportedNetwork==NET_TYPE_WIFI)||(netType==NET_TYPE_ALL && comsupportedNetwork==NET_TYPE_ETH)) lv_timer_handler();
    static uint32_t deleteSD = 1;
  if (activeRunMode == MODE_OCPP){
      bool nowTran = (ocppData.status.activeTran == 1);
      if (prevTran && !nowTran && deleteSD) {   
      if (readEnergy() > 1.0f){
          writeEnergy(0.0f);
          writeChargeTime(0);
          Serial.print("main in delete fonk");
      }
      } 
    
    prevTran = nowTran;
    
     

     readRecovery = Nvsmem::loadString("recovery");
    if ( recoveryMode == true ) {
          if(readRecovery == "Unblocked"){
            if (!recoveryStart) recoveryStart = millis();
            bool fakeEnd = (millis() - recoveryStart < 13000);
            //if (initialMspState != "C2") {deleteSD = 0; Serial.println("aracdurdurma stop basarılı\n"); Serial.println(readEnergy()); stopTransactionWithMeter(readEnergy());} 
            comm_txUpdate(1, fakeEnd, ctrl.buzzerOn, confVal, unlockConn, uartTestVal, maxPower);
            
            // Serial.println("aracdurdurma stop basarılı= "); Serial.println(mqttDataValue.getMSP_STATE());
          }else {
            comm_txUpdate(0, 1, ctrl.buzzerOn, confVal, unlockConn, uartTestVal, maxPower);
            if(!isInternetAlive()) transectionlock =1;
            if(transectionlock == 0 && ocppData.status.activeTran == 1){
                //Serial.println("transectionlock == 0 girdi\n");
                deleteSD = 0; 
                stopTransactionWithMeter(readEnergy());// fonk default u EVDisconnect
            }  

          }  
      
    } else if(recoveryLock == true) {
        recoveryStart = 0;
        deleteSD =1 ;
        readState = false; 
        comm_txUpdate(chargerData.runControl, ctrl.clearSession, ctrl.buzzerOn, confVal, unlockConn, uartTestVal, maxPower);
    }
   
    if(transectionlock == 1){
       // Serial.println("transectionlock == 1 girdi\n");
        if(isInternetAlive()){
         
         // Serial.println("transectionlock == 1 internetalive  girdi\n");
          if (ocppData.status.activeTran != 1)transectionlock = 0;
          deleteSD = 0; 
         // Serial.println("transectionlock == 1 fonk default u EVDisconnect\n");
          stopTransactionWithMeter(readEnergy());// fonk default u EVDisconnect
        }
    }
  }else {
    comm_txUpdate(chargerData.runControl, ctrl.clearSession, ctrl.buzzerOn, confVal, unlockConn, uartTestVal, maxPower);
  }       
     if (resetcountVal == 130){ mqttDataValue.setCONF_PARAMS_RESET("0");  /*MSP RESETİ SIFIRLA*/  }
     

    static uint8_t lastResetParam = 0;

    uint8_t currentResetParam =(strcmp(mqttDataValue.getCONF_PARAMS_RESET(), "1") == 0);
    if (currentResetParam && !lastResetParam) {
        mspHardReset();
    }
    lastResetParam = currentResetParam;

    // uart test
    getUnlockConnectorRsp(mqttUnlockRspUartTest);
    if (resetcountVal == 130) {
      if (!mqttUnlockRspUartTest) {
        mqttDataValue.setUART_ACTIVE("uart active");
        uartActive=1;
      } else {
        mqttDataValue.setUART_ACTIVE("uart broken");
        uartActive=0;
      }
      setUnlockConnectorRsp(1);
    }
    // uart test end


  if (activeRunMode == MODE_OCPP){
    // ocpp connectivity
    if(isInternetAlive()){
      mqttDataValue.setOCPP_CONN("OCPP Online");
    }else{
      mqttDataValue.setOCPP_CONN("OCPP Ofline");
    }
  }
   
   
    // ocpp connectivity end
    assignUint16(mqttDataValue.getESP_RESET(), resetVal);  // mqtt resetvalu
    if (resetVal == 1) ESP.restart();                      // esp reset with mqtt

    charger_stateTransitions(&rfid1, &FSM.Out);
    getStates(comm.rx);// hafızadan veri çeker
    
    if(MSPResetTick == 0 ){
       updateChargerData(&chargerData, comm.rx,0);
    }

    if (MSPResetTick == 1) {

       if(readState == true) updateChargerDataState(&chargerData, comm.rx);

        //  resetten sonra 10 sn dolduysa artık hep çalış
        if (present - MSPHardRestarTimerStart >= MSP_HARD_RESET_INTERVAL) {

         if(strcmp(mqttDataValue.getMSP_STATE(), "C2") == 0 || present - MSPHardRestarTimerStart >=60000UL || strcmp(mqttDataValue.getMSP_STATE(), "A1") == 0   ) {readState = false; updateChargerData(&chargerData, comm.rx, 1);}
      
        }

    }
  if (activeRunMode == MODE_OCPP){
    if (MSPResetTick == 2){ 
      updateChargerData(&chargerData, comm.rx, 1);
      recoveryLock = true; 
      if (readRecovery == "blocked" && ocppData.status.activeTran == 1 ){ transectionlock = 1;}

      }
   }

    if(uartActive == 0 && !(FSM.current_state == P4_CHARGING)){
      //mspHardReset();
      uartActive = 1;
    }

    if((uartActive == 0|| ACmeter_stop == 1 )&& FSM.current_state == P4_CHARGING && !recoveryFakeActive ){ 
      MSPHardRestarTimerStart = millis();
      mspHardReset();
      MSPResetTick = 1;
      uartActive = 1;
      ACmeter_stop = 0 ;
    }
    mspHardResetLoop();
  if (activeRunMode == MODE_OCPP){
    if (MSPResetTick != 0 && (FSM.current_state == P3_SCAN_RFID ||FSM.current_state == P2_PLUG_IN ) && ocppData.status.activeTran == 0){ MSPResetTick = 0; /*Serial.println("MSPResetTick -> 0");*/ }
  }else{
    if (MSPResetTick != 0  && FSM.current_state == P2_PLUG_IN ) MSPResetTick = 0; 
  }   
   
    static unsigned long plugInResetStart = 0;
    const unsigned long PLUGIN_RESET_WAIT = 6000UL; // 2 dk = 120.000 ms 10sn = 6000ms
   
    /*if (MSPResetTick == 1 && FSM.current_state == P2_PLUG_IN && ocppData.status.activeTran == 0) {
        // Zamanlayıcı daha önce başlatılmadıysa başlat
        if (plugInResetStart == 0) {
            plugInResetStart = millis();
        }

        // 2 dk dolduysa reset at
        if (millis() - plugInResetStart >= PLUGIN_RESET_WAIT) {
           ESP.restart();


        }

    }else {
        plugInResetStart = 0;
    } */


  static unsigned long lastMspUpdate = 0;
  if (millis() - lastMspUpdate >= 1000  ) {
      lastMspUpdate = millis();
      updateMqttMspValues(&chargerData);
  }

}

uint16_t resetCounter = 0;
void Task2code(void* pvParameters) {
  while (1) {
    //if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      wdtFeed();

  
        static unsigned long resetTimerStart = millis();
    if (activeRunMode == MODE_OCPP) {
        chargerData.ocppStatus = (FSM.current_state == P2_PLUG_IN ? 0 : ocppData.status.ocppStatus);
        
      if (recoveryMode && !recoveryFakeActive) {
      recoveryFakeActive = true;
      recoveryStartTime = millis();
      //SerialAT.println("5.if girdi\n");

      }

    static uint8_t readMSPHardActive = 0;
    static uint32_t recoveryTaskStop = 0;
      //Serial.println("5.if girdi\n");
    if (recoveryFakeActive ) {
          //Serial.println(readRecovery );
          //Serial.println("\n ");
        // 13 saniye 
        if (millis() - recoveryStartTime < RECOVERY_FAKE_DURATION &&  readRecovery == "Unblocked") {

            ocppDataUpdate(readEnergy(), 0, 1);
           // Serial.println("6.if girdi\n");
           recoveryTaskStop = 1;
           recoveryLock = false;
        } else {
            //Serial.println("else recoveryfacactive girdi\n");
            // if(strcmp(mqttDataValue.getMSP_STATE(), "C2") == 0 || millis() - recoveryStartTime>= 120000UL || chargerData.plugStatus == 0  ){    
            MSPResetTick = 2;
            if(recoveryLock == true ){
              //Serial.println("recovery deactivate girdi\n");Serial.println(readRecovery );Serial.println(" ! recoveryfakeactive girdi\n"); 
              recoveryFakeActive = false;   
              recoveryMode = false;
              Nvsmem::loadString("recoverymode","Deactivate");
              if (recoveryTaskStop == 1 ){stopRecoveryTimer();  recoveryTaskStop = 0;}
            }
        }

    }
 


    if (!recoveryFakeActive) {
        
        ocppChargeFinishedfromEV(&ocppData, chargerData.chargeFinished);
        ocppDataUpdate( chargerData.meter.energy,chargerData.meter.power,chargerData.plugStatus);
    }

   }
      //updateMqttMspValues(&chargerData);
      //printUartDebugValues(&chargerData);

       handleUnlockConn();
    
      assignUint16(mqttDataValue.getUNLOCK_CONN(), mqttunlock);
      unlockConn = (ocppData.unlockConnReq | mqttunlock);

      // mqtt stop charge fork
      assignUint16(mqttDataValue.getSTOP_CHARGE(), mqttstopcharge);
      if (!(mqttstopchargeold == mqttstopcharge)) {
        chargerData.runControl = mqttstopcharge;

        if (mqttstopcharge == chargerData.chargeStatus) {
          mqttDataValue.setSTOP_CHARGE("0");
          mqttstopchargeold = mqttstopcharge;
        }
      } else {
         if (activeRunMode == MODE_OCPP) chargerData.runControl = ocppData.status.chargeStatus;
      }
      // mqtt stop charge fork  end

      if (activeRunMode == MODE_OCPP) ocpp_isActiveTransaction(&ocppData);

      unsigned long now = millis();
      //const unsigned long RESET_INTERVAL = 8640000UL; // bir gün
        const unsigned long RESET_INTERVAL = 4320000UL;  // 12 saat

      if ((now - resetTimerStart >= RESET_INTERVAL) && FSM.current_state == P2_PLUG_IN) {
        resetCounter++;
        if (resetCounter > 600) {
          resetCounter = 0;
          ESP.restart();
        }
      }

      //xSemaphoreGive(xMutex);
    //}



//#if RUN_MODE == 1
if (activeRunMode == MODE_RFID_TAK_CALISTIR) {

    //if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
       if (FSM.current_state == P2_PLUG_IN) main2RfidManage(&rfid1, &rfidManageEnable);
         handleMainRfidUpdate(&rfid1);
    //  xSemaphoreGive(xMutex);
    //}
}
//#endif

    mainUpdate();


    if (activeRunMode == MODE_OCPP && runPermission == 0) {
      ocppData.status.chargeStatus = AC_EVSEChargeStatus_NO_CHARGING;
    }

    delay(100);
  }
}

void Task3code(void* pvParameters) {
  while (1) {
    if (SerialMAIN.available() > 0) {
      char outVal = SerialMAIN.read();
      if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        handleUartRx(outVal);
        xSemaphoreGive(xMutex);
      }
    }
    ocppTimer++;
    delay(1);
  }
}
