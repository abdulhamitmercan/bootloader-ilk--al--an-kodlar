#ifndef MQTT_API_H
#define MQTT_API_H
#include "broker.h"

uint16_t isOcppActive =1;  // kW varsayılan
uint16_t maxPower = 22;

inline void assignUint16(const char* s, uint16_t& dst) {
  if (!s) return;
  char* end = nullptr;
  unsigned long v = strtoul(s, &end, 10);  // "22" / "22kW" / " 22 " -> 22
  if (end != s && v <= 65535UL) {
    dst = static_cast<uint16_t>(v);
  }
}

inline uint16_t toUint16OrKeep(const char* s, uint16_t keep) {
  if (!s) return keep;
  char* end = nullptr;
  unsigned long v = strtoul(s, &end, 10);
  return (end != s && v <= 65535UL) ? (uint16_t)v : keep;
}

static String mspStateToString(uint16_t state) {
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

static void updateMqttMspValues(const ChargerData *charger) {
  if (!charger) return;

  String sActuator = String(charger->st1.actuator);
  String sTemp     = String(charger->st1.temp) + "°C";
  String sLeak     = String(charger->st1.leakage);
  String sState    = mspStateToString(charger->st1.mspState);
  String sRgb      = String(charger->st1.rgb);

  float cpVolt = charger->st2.controlPilot / 10.0f;
  String sCpVolt = String(cpVolt, 1) + "V";
  String sRlyOut   = String(charger->st2.relayStatus);
  String sPwmEn    = String(charger->st2.pwmStatus);

  String sL1Volt   = String(charger->l1.voltage) + "V";
  String sL1Cur    = String(charger->l1.current) + "A";

  String sL2Volt   = String(charger->l2.voltage) + "V";
  String sL2Cur    = String(charger->l2.current) + "A";

  String sL3Volt   = String(charger->l3.voltage) + "V";
  String sL3Cur    = String(charger->l3.current) + "A";

  int hw = charger->version.hw; String sHWVer = "HW v" + String(hw / 10) + "." + String(hw % 10);
  int sw = charger->version.sw; String sSWVer = "SW v" + String(sw / 10) + "." + String(sw % 10);

  mqttDataValue.setMSP_ACTUATR(sActuator.c_str());
  mqttDataValue.setMSP_TEMP(sTemp.c_str());
  if(charger->st1.leakage) mqttDataValue.setMSP_LEAK("1");
  mqttDataValue.setMSP_STATE(sState.c_str());
  mqttDataValue.setMSP_RGB_OUT(sRgb.c_str());

  mqttDataValue.setMSP_CP_VOLT(sCpVolt.c_str());
  mqttDataValue.setMSP_RLY_OUT(sRlyOut.c_str());
  mqttDataValue.setMSP_PWM_EN(sPwmEn.c_str());

  mqttDataValue.setMSP_L1_VOLT(sL1Volt.c_str());
  mqttDataValue.setMSP_L1_CUR(sL1Cur.c_str());

  mqttDataValue.setMSP_L2_VOLT(sL2Volt.c_str());
  mqttDataValue.setMSP_L2_CUR(sL2Cur.c_str());

  mqttDataValue.setMSP_L3_VOLT(sL3Volt.c_str());
  mqttDataValue.setMSP_L3_CUR(sL3Cur.c_str());

  mqttDataValue.setMSP_HW_VER(sHWVer.c_str());
  mqttDataValue.setMSP_SW_VER(sSWVer.c_str());  


   // case mqttSCAN_LIST:           return abdl.getSCAN_LIST();
}

static void printUartDebugValues(const ChargerData *charger) {
    

    static unsigned long lastPrint = 0;
    const unsigned long printInterval = 20000;

    if (millis() - lastPrint < printInterval) {
        return;
    }
    lastPrint = millis();

    Serial.println("\n========== UART / MSP DEBUG ==========");

    Serial.println("----- ST1 -----");
    Serial.print("st1.actuator      : ");
    Serial.println(charger->st1.actuator);

    Serial.print("st1.temp          : ");
    Serial.println(charger->st1.temp);

    Serial.print("st1.leakage       : ");
    Serial.println(charger->st1.leakage);

    Serial.print("st1.mspState      : ");
    Serial.println(charger->st1.mspState);

    Serial.print("st1.rgb           : ");
    Serial.println(charger->st1.rgb);

    Serial.println("----- ST2 -----");
    Serial.print("st2.controlPilot  : ");
    Serial.println(charger->st2.controlPilot);

    Serial.print("st2.relayStatus   : ");
    Serial.println(charger->st2.relayStatus);

    Serial.print("st2.pwmStatus     : ");
    Serial.println(charger->st2.pwmStatus);

    Serial.println("----- ST3 -----");
    Serial.print("st3.raw           : ");
    Serial.println(charger->st3.raw);

    Serial.println("----- L1 -----");
    Serial.print("l1.voltage        : ");
    Serial.println(charger->l1.voltage);

    Serial.print("l1.current        : ");
    Serial.println(charger->l1.current);

    Serial.println("----- L2 -----");
    Serial.print("l2.voltage        : ");
    Serial.println(charger->l2.voltage);

    Serial.print("l2.current        : ");
    Serial.println(charger->l2.current);

    Serial.println("----- L3 -----");
    Serial.print("l3.voltage        : ");
    Serial.println(charger->l3.voltage);

    Serial.print("l3.current        : ");
    Serial.println(charger->l3.current);

    Serial.println("----- VERSION -----");
    Serial.print("version.hw        : ");
    Serial.println(charger->version.hw);

    Serial.print("version.sw        : ");
    Serial.println(charger->version.sw);

  
}

void generateAndSetSerialFromMac() {
  uint64_t mac64 = ESP.getEfuseMac();
  uint8_t mac[6];

  // 6 byte'a ayır
  for (int i = 0; i < 6; i++) {
    mac[i] = (uint8_t)(mac64 >> ((5 - i) * 8));
  }

  // --- Format: BT110S-<son 3 byte ters> ---
  // Örn: MAC = 3C:71:BF:9A:2D:8E -> BT110S-8E2D9A
  char buf[32];
  snprintf(buf, sizeof(buf),
           "BT110S-%02X%02X%02X",
           mac[0], mac[1], mac[2]);

  
  mqttDataValue.setC_P_SER_NUM(buf);
}

void noInternetMode() {
  mqttDataValue.setUI_COMPANY_TEL_NUM(" ");
  mqttDataValue.setUI_COMPANY_URL("www.Bluetechcharge.com");
  mqttDataValue.setUI_APPSTORE_URL("https://www.bluetechcharge.com/");
  mqttDataValue.setUI_PLAYSTORE_URL("https://www.bluetechcharge.com/");
  mqttDataValue.setCONF_PARAMS_MODEL("1");
}

void confParamsInit(t_EVSE *confParams){
  confParams->reset = 0;
  confParams->measuretype = 0;
  confParams->mode = 0;
  confParams->model = 0;
  
  assignUint16(mqttDataValue.getOCPP_ACTIVE(), isOcppActive);
  assignUint16(mqttDataValue.getMAX_POWER(), maxPower);
  confParams->measuretype = toUint16OrKeep(mqttDataValue.getCONF_PARAMS_MEASURETYPE(), confParams->measuretype);
  confParams->mode = toUint16OrKeep(mqttDataValue.getCONF_PARAMS_MODE(), confParams->mode);
  confParams->model = toUint16OrKeep(mqttDataValue.getCONF_PARAMS_MODEL(), confParams->model);
  mainTag = mqttDataValue.getACTIVE_RFID();

}

void printResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    
    switch (reason) {
        case ESP_RST_POWERON:   Serial.println("Power on reset"); break;
        case ESP_RST_SW:        Serial.println("Software reset (ESP.restart)"); break;
        case ESP_RST_PANIC:     Serial.println("Panic / Exception (crash)"); break;
        case ESP_RST_INT_WDT:   Serial.println("Interrupt WDT reset"); break;
        case ESP_RST_TASK_WDT:  Serial.println("Task WDT reset"); break;
        case ESP_RST_WDT:       Serial.println("Other watchdog reset"); break;
        case ESP_RST_BROWNOUT:  Serial.println("Brownout (voltaj düştü)"); break;
        case ESP_RST_DEEPSLEEP: Serial.println("Wake from deep sleep"); break;
        default:                Serial.println("Unknown reset reason"); break;
    }
}



static void wdtSetup(uint32_t timeoutMs) {
  
  uint32_t timeoutSec = (timeoutMs + 999) / 1000;
  if (timeoutSec == 0) timeoutSec = 1;

 
  esp_task_wdt_deinit();

 
  esp_err_t err = esp_task_wdt_init(timeoutSec, true /* trigger_panic */);
  Serial.printf("[WDT] init=%d timeout=%lu s\n", (int)err, (unsigned long)timeoutSec);


  Serial.printf("[WDT] add current task=%d\n", (int)err);
}

static inline void wdtFeed() {
  esp_task_wdt_reset();
  vTaskDelay(1);
}

void mspHardReset1() {
  pinMode(19, OUTPUT);
  digitalWrite(19, HIGH);

  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    // 1 saniye bekle
  }
  
  digitalWrite(19, LOW);
 // mqttDataValue.setMSP_HARD_RESET("0");
   //uartActive = 1;
   //ACmeter_stop = 0;
} 
uint8_t mspResetActive = 0;
unsigned long mspResetStartMs = 0;

void mspHardReset() {
  pinMode(19, OUTPUT);

  digitalWrite(19, HIGH);   // reset bas
  mspResetStartMs = millis();
  mspResetActive = 1;

  Serial.println("[MSP RESET] START HIGH");
}

void mspHardResetLoop() {
  if (mspResetActive == 1) {
    if (millis() - mspResetStartMs >= 1000UL) {
      digitalWrite(19, LOW);   // reset bırak
      mspResetActive = 0;

      //uartActive = 1;
      //ACmeter_stop = 0;

      Serial.println("[MSP RESET] END LOW");
    }
  }
}

#endif