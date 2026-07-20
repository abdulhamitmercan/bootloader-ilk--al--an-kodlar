#ifndef MQTT_PROTOCOL_H
#define MQTT_PROTOCOL_H

#include <PubSubClient.h>
#include"nvsMem.h"
// [topic]message "topic  broker.h's topic"  ;meeesage = c[CmdType][MessageType][MessageType's data] v 
class MqttFrameHandler;

// --- Komut tipleri 
enum CmdTypeData : uint8_t { 
  CMD_SET = 1, 
  CMD_READ = 2, 
  CMD_LOG = 3 };

// --- Mesaj tipleri 
enum MessageTypeData : uint8_t {
  mqttESP_ID = 0,//
  mqttOCPP_URL = 1,//
  mqttOCPP_ID = 2,//
  mqttQR_ID = 3,//
  mqttMAX_POWER = 4,//
  mqttWIFI_SSID = 5,//
  mqttWIFI_PASS = 6,//
  mqttCONNECTION_TYPE = 7,//
  mqttUNLOCK_CONN = 8, //

 // ui3
 
  mqttUI_COMPANY_TEL_NUM = 9,//
  mqttUI_COMPANY_URL=10,//
  mqttUI_APPSTORE_URL = 11,//
  mqttUI_PLAYSTORE_URL = 12,//
  mqttUI_LOGO_OR_WRITING = 13,//
  mqttUI_COMPANY_NAME = 14,//
  
  

  // MSP config
  mqttCONF_PARAMS_RESET = 15,//
  mqttCONF_PARAMS_MEASURETYPE = 16,//
  mqttCONF_PARAMS_MODE = 17,//
  mqttCONF_PARAMS_MODEL = 18,//

  mqttESP_MODE = 19,

  // runtime variable
  mqttESP_RESET = 20,//
  mqttSTOP_CHARGE = 21,//
  mqttCHARGING_TIME = 22,//
  mqttPRMS = 23,//
  mqttETOTAL = 24,//
  mqttCHARGE_STATE = 25,//
  mqttUART_ACTIVE = 26,//
  mqttERROR = 27,

  // LOG tipleri
  mqttLOG_VERBOSE = 28,
  mqttLOG_DEBUG   = 29,
  mqttLOG_INFO    = 30,

  //mqtt ota conf 
  mqttOTA_UPDATE_ACTIVE = 31,//
  mqttOTA_KEY =32,//
  mqttOTA_VER=33, //

  mqttGSM_SIGNAL=34,// 

  mqttGRADIENT_OR_COLOR =35,//
  mqttSELECT_COLOR =36,//
  mqttCMSPRT_NET =37,
  mqttOCPP_ACTIVE = 38,

  //ocpp credentels
  mqttC_P_VENDOR = 39,
  mqttC_P_MODEL = 40,
  mqttC_P_SER_NUM = 41 ,
  mqttMET_SER_NUM = 42 ,
  mqttMET_TYPE = 43,
  mqttBOX_SER_NUM = 44,
  mqttOCPP_CONN =45,

// msp output read
  mqttMSP_ACTUATR = 46,
  mqttMSP_TEMP = 47,
  mqttMSP_LEAK = 48,
  mqttMSP_STATE = 49,
  mqttMSP_RGB_OUT = 50,
  mqttMSP_CP_VOLT = 51,
  mqttMSP_RLY_OUT = 52,
  mqttMSP_PWM_EN = 53,
  mqttMSP_L1_VOLT = 54,
  mqttMSP_L1_CUR = 55,
  mqttMSP_L2_VOLT = 56,
  mqttMSP_L2_CUR = 57,
  mqttMSP_L3_VOLT = 58,
  mqttMSP_L3_CUR = 59,
  mqttMSP_HW_VER = 60,
  mqttMSP_SW_VER = 61,
   
  mqttSCAN_LIST = 62,
  mqttLOGO_UPDATE = 63,
  mqttLOGO_NAME = 64,
  mqttACTIVE_LOGO = 65,

  mqttRFID_UPDATE = 66,
  mqttACTIVE_RFID = 67,
  mqttESP_HW_VER = 68
    //sonuncu 68,  68 dahil dolu sonraki sayıdan devam et
};

// ---------------- DataValue ----------------
class DataValue {
private:
  // DEFAULTS
  mutable String mqttESP_ID      = "esp1";//The Linux communication ID must be changed from its default and be unique. If multiple devices share the same ESP_ID, Linux may connect to the same device at the same time.
  mutable String mqttOCPP_URL    = "ws://iottest.oncharge.xyz";//estabilish ocpp url
  mutable String mqttOCPP_ID     = "onc1";//estabilish ocpp box id
  mutable String mqttQR_ID       = "ABDULLEAO";//estabilish ocppqr code on screen
  mutable String mqttMAX_POWER   = "22";//estabilish max power on ac charger 
  mutable String mqttWIFI_SSID   = "Mercan";//estabilish wifi ssid
  mutable String mqttWIFI_PASS   = "10203040";//estabilish wifi password
  mutable String mqttCONN_TYPE   = "0"; // change connection tye COMMSUPPORTED->0/GSM->1/WiFi->2/ETH->3
  mutable String mqttUI_TEL_NUM  = "";// change company tel number  from ui
  mutable String mqttUI_COMP_URL = "www.Bluetechcharge.com"; //change company url  from ui
  mutable String mqttUI_APP_URL  = "https://www.bluetechcharge.com/";// change appstore url from ui
  mutable String mqttUI_PLAY_URL = "https://www.bluetechcharge.com/";// change playstore url from ui
  mutable String mqttUI_LG_TXT   = "1"; // change text 0 logo 1 from ui
  mutable String mqttUI_CMP_NAME = "BLUETECH";// change company name from ui

  // MSP confparams 
  char mqttCONF_RESET[8]         = "0"; //this reset software reset on MSP enable 1 disable 0
  mutable String mqttCONF_MEASTP = "0"; //  calculeted msp 0 read ac meter 1
  mutable String mqttCONF_MODE   = "0";// product  mode 0 test mode 1
  mutable String mqttCONF_MODEL  = "0"; //product with no cable 0 with cable 1

  mutable String mqttESP_MODE = "2"; // rfid charge ->0 / plug&charge->1 / ocpp->2
  char mqttUNLOCK_CONN[16]    = "";// open actuator->1
  char mqttESP_RESET[8]       = "0"; // esp software reset ->1
  char mqttSTOP_CHARGE[8]     = "0"; // ocpp finishing  charge or pulug&drive stop charge ->1
  
  char mqttCHARGING_TIME[32]  = "";// charging time valu
  char mqttPRMS[32]           = "";// charging power valu
  char mqttETOTAL[32]         = "";// charging energy valu
  char mqttCHARGE_STATE[32]   = "";// charge circle state
  char mqttUART_ACTIVE[32]    = "0";// uart control 
  char mqttERROR[32]          = "";// errrors
  
  //mqtt ota conf
  mutable String mqttOTA_ACTIVE  = "0";// ota disable 0 enable 1
  mutable String mqttOTA_KEY     = "29c019d1-c198-47ad-82f9-07dcadb8fad0";// OTAdrive APIkey for this product
  mutable String mqttOTA_VER     = "v001"; /// this app version
  
  char mqttGSM_SIGNAL[32] = ""; // GSM signal level value

  mutable String mqttGRAD_COLR   = "0";// Select screen gradiant color 0 or only one color 1
  mutable String mqttSLCT_COLR   = "0x4DB748";// select only one color's color
  char mqttCMSPRT_NET[16]        ="NONE";// GSM/WiFi/ETH for select which mqttCONN_TYPE  COMMSUPPORT() use now
  mutable String mqttOCPP_ACTIVE = "1";//select ocpp activate 1 deactive 0

  mutable String mqttC_P_VENDOR  = "BLUETECH"; //ocpp charge poin vendor 
  mutable String mqttC_P_MODEL   = "BT110-22kW"; //ocpp charge poin model
  mutable String mqttC_P_SER_NUM = "BT110S-DCDA0C59AA58"; //ocpp charge poin serial number "BT110S-[MAC in tesri]" DEFAULT ONLY READ !!!!!>MAC = 3C:71:BF:9A:2D:8E -> BT110S-8E2D9A
  mutable String mqttMET_SER_NUM = "IVY Metering EM519033"; //ocpp meter serial number ac meter->IVY Metering EM519033 ;local measure->Internal 
  mutable String mqttMET_TYPE    = "0598"; // ocpp meter type ac meter -> 0598 local measure->BT110-METER-001
  mutable String mqttBOX_SER_NUM = "BT110"; //Ocpp box serial number 
  
  char mqttOCPP_CONN[32] = ""; //Ocpp connected or disconnected
        
  char mqttMSP_ACTUATR[16] = "0";//read actuator output pin and relay position 1 open, 0 close
  char mqttMSP_TEMP[16]    = "9"; // read msp powerboard temperature 
  char mqttMSP_LEAK[16]    = ""; // read msp powerboard lekage current if there is a lekage current 1 else 0
  char mqttMSP_STATE[16]   = "0"; // read msp powerboard charge cycle 
  char mqttMSP_RGB_OUT[16] = "8"; //read led output pin and
  char mqttMSP_CP_VOLT[16] = ""; // read msp control pilot voltage 
  char mqttMSP_RLY_OUT[16] = "1"; //read realay output pin and  relay position 1 open, 0 close
  char mqttMSP_PWM_EN[16]  = "9";  // Read MSP power board PWM status: 1 if PWM is active, 0 if inactive
  char mqttMSP_L1_VOLT[16] = "9"; // read Line 1 Voltage 
  char mqttMSP_L1_CUR[16]  = "9"; // read Line 1 Current   
  char mqttMSP_L2_VOLT[16] = ""; // read Line 2 Voltage 
  char mqttMSP_L2_CUR[16]  = ""; // read Line 2 Current
  char mqttMSP_L3_VOLT[16] = ""; // read Line 3 Voltage 
  char mqttMSP_L3_CUR[16]  = ""; // read Line 3 Current
  mutable String mqttMSP_HW_VER  = "V0.0"; // read MSP Hardware Version
  mutable String mqttMSP_SW_VER = "v1.0"; // read MSP Software Version
  mutable String mqttLOGO_UPDATE = "0"; // ui logo download 1 no download 0
  mutable String mqttLOGO_NAME = "bluetech_logo"; // which logo download  acrapol_logo,celikler_logo,ctg_logo,sepas_logo
  mutable String mqttACTIVE_LOGO = "bluetech_logo"; // current logo 
  mutable String mqttESP_HW_VER  = "V1.3"; // ESP Hardware Version

  char mqttSCAN_LIST[2048]  = ""; //Read Scanning Wifi ssid list on esp evroment 

  mutable String mqttRFID_UPDATE = "0";
  mutable String mqttACTIVE_RFID = "24D7B502";

public:

    const char* getESP_ID() const { mqttESP_ID = Nvsmem::getOrInit("mqttESP_ID", mqttESP_ID); return mqttESP_ID.c_str(); }
    void setESP_ID(const char* d) { mqttESP_ID = d ? d : ""; Nvsmem::loadString("mqttESP_ID", mqttESP_ID); }

    const char* getOCPP_URL() const { mqttOCPP_URL = Nvsmem::getOrInit("mqttOCPP_URL", mqttOCPP_URL); return mqttOCPP_URL.c_str(); }
    void setOCPP_URL(const char* d) { mqttOCPP_URL = d ? d : ""; Nvsmem::loadString("mqttOCPP_URL", mqttOCPP_URL); }

    const char* getOCPP_ID() const { mqttOCPP_ID = Nvsmem::getOrInit("mqttOCPP_ID", mqttOCPP_ID); return mqttOCPP_ID.c_str(); }
    void setOCPP_ID(const char* d) { mqttOCPP_ID = d ? d : ""; Nvsmem::loadString("mqttOCPP_ID", mqttOCPP_ID); }

    const char* getQR_ID() const { mqttQR_ID = Nvsmem::getOrInit("mqttQR_ID", mqttQR_ID); return mqttQR_ID.c_str(); }
    void setQR_ID(const char* d) { mqttQR_ID = d ? d : ""; Nvsmem::loadString("mqttQR_ID", mqttQR_ID); }

    const char* getMAX_POWER() const { mqttMAX_POWER = Nvsmem::getOrInit("mqttMAX_POWER", mqttMAX_POWER); return mqttMAX_POWER.c_str(); }
    void setMAX_POWER(const char* d) { mqttMAX_POWER = d ? d : ""; Nvsmem::loadString("mqttMAX_POWER", mqttMAX_POWER); }

    const char* getWIFI_SSID() const { mqttWIFI_SSID = Nvsmem::getOrInit("mqttWIFI_SSID", mqttWIFI_SSID); return mqttWIFI_SSID.c_str(); }
    void setWIFI_SSID(const char* d) { mqttWIFI_SSID = d ? d : ""; Nvsmem::loadString("mqttWIFI_SSID", mqttWIFI_SSID); }

    const char* getWIFI_PASS() const { mqttWIFI_PASS = Nvsmem::getOrInit("mqttWIFI_PASS", mqttWIFI_PASS); return mqttWIFI_PASS.c_str(); }
    void setWIFI_PASS(const char* d) { mqttWIFI_PASS = d ? d : ""; Nvsmem::loadString("mqttWIFI_PASS", mqttWIFI_PASS); }

    const char* getCONNECTION_TYPE() const { mqttCONN_TYPE = Nvsmem::getOrInit("mqttCONN_TYPE", mqttCONN_TYPE); return mqttCONN_TYPE.c_str(); }
    void setCONNECTION_TYPE(const char* d) { mqttCONN_TYPE = d ? d : ""; Nvsmem::loadString("mqttCONN_TYPE", mqttCONN_TYPE); }

    const char* getUI_COMPANY_TEL_NUM() const  { mqttUI_TEL_NUM = Nvsmem::getOrInit("mqttUI_TEL_NUM", mqttUI_TEL_NUM); return mqttUI_TEL_NUM.c_str(); }
    void setUI_COMPANY_TEL_NUM(const char* d)  { mqttUI_TEL_NUM = d ? d : ""; Nvsmem::loadString("mqttUI_TEL_NUM", mqttUI_TEL_NUM); }
    
    const char* getUI_COMPANY_URL() const  { mqttUI_COMP_URL = Nvsmem::getOrInit("mqttUI_COMP_URL", mqttUI_COMP_URL); return mqttUI_COMP_URL.c_str(); }
    void setUI_COMPANY_URL(const char* d)  { mqttUI_COMP_URL = d ? d : ""; Nvsmem::loadString("mqttUI_COMP_URL", mqttUI_COMP_URL); }

    const char* getUI_APPSTORE_URL() const { mqttUI_APP_URL = Nvsmem::getOrInit("mqttUI_APP_URL", mqttUI_APP_URL); return mqttUI_APP_URL.c_str(); }
    void setUI_APPSTORE_URL(const char* d) { mqttUI_APP_URL = d ? d : ""; Nvsmem::loadString("mqttUI_APP_URL", mqttUI_APP_URL); }

    const char* getUI_PLAYSTORE_URL() const { mqttUI_PLAY_URL = Nvsmem::getOrInit("mqttUI_PLAY_URL", mqttUI_PLAY_URL); return mqttUI_PLAY_URL.c_str(); }
    void setUI_PLAYSTORE_URL(const char* d) { mqttUI_PLAY_URL = d ? d : ""; Nvsmem::loadString("mqttUI_PLAY_URL", mqttUI_PLAY_URL); }

    const char* getUI_LG_TXT() const { mqttUI_LG_TXT = Nvsmem::getOrInit("mqttUI_LG_TXT", mqttUI_LG_TXT); return mqttUI_LG_TXT.c_str(); }
    void setUI_LG_TXT(const char* d) { mqttUI_LG_TXT = d ? d : ""; Nvsmem::loadString("mqttUI_LG_TXT", mqttUI_LG_TXT); }

    const char* getUI_COMPANY_NAME() const { mqttUI_CMP_NAME = Nvsmem::getOrInit("mqttUI_CMP_NAME", mqttUI_CMP_NAME); return mqttUI_CMP_NAME.c_str(); }
    void setUI_COMPANY_NAME(const char* d) { mqttUI_CMP_NAME = d ? d : ""; Nvsmem::loadString("mqttUI_CMP_NAME", mqttUI_CMP_NAME); }

  // confparams deframe 
    const char* getCONF_PARAMS_RESET() const  { return mqttCONF_RESET; }
    void setCONF_PARAMS_RESET(const char* d)  { strncpy(mqttCONF_RESET, d ? d : "", sizeof(mqttCONF_RESET) - 1); mqttCONF_RESET[sizeof(mqttCONF_RESET) - 1] = '\0'; }

    const char* getCONF_PARAMS_MEASURETYPE() const { mqttCONF_MEASTP = Nvsmem::getOrInit("mqttCONF_MEASTP", mqttCONF_MEASTP); return mqttCONF_MEASTP.c_str(); }
    void setCONF_PARAMS_MEASURETYPE(const char* d) { mqttCONF_MEASTP = d ? d : ""; Nvsmem::loadString("mqttCONF_MEASTP", mqttCONF_MEASTP); }

    const char* getCONF_PARAMS_MODE() const  { mqttCONF_MODE = Nvsmem::getOrInit("mqttCONF_MODE", mqttCONF_MODE); return mqttCONF_MODE.c_str(); }
    void setCONF_PARAMS_MODE(const char* d)  { mqttCONF_MODE = d ? d : ""; Nvsmem::loadString("mqttCONF_MODE", mqttCONF_MODE); }

    const char* getCONF_PARAMS_MODEL() const { mqttCONF_MODEL = Nvsmem::getOrInit("mqttCONF_MODEL", mqttCONF_MODEL); return mqttCONF_MODEL.c_str(); }
    void setCONF_PARAMS_MODEL(const char* d) { mqttCONF_MODEL = d ? d : ""; Nvsmem::loadString("mqttCONF_MODEL", mqttCONF_MODEL); }

    const char* getESP_MODE() const { mqttESP_MODE = Nvsmem::getOrInit("mqttESP_MODE", mqttESP_MODE); return mqttESP_MODE.c_str(); }
    void setESP_MODE(const char* d) { mqttESP_MODE = d ? d : ""; Nvsmem::loadString("mqttESP_MODE", mqttESP_MODE); }

    const char* getUNLOCK_CONN() const  { return mqttUNLOCK_CONN; }
    void setUNLOCK_CONN(const char* d)  { strncpy(mqttUNLOCK_CONN, d ? d : "", sizeof(mqttUNLOCK_CONN) - 1); mqttUNLOCK_CONN[sizeof(mqttUNLOCK_CONN) - 1] = '\0'; }

    const char* getESP_RESET() const { return mqttESP_RESET; }
    void setESP_RESET(const char* d) { strncpy(mqttESP_RESET, d ? d : "", sizeof(mqttESP_RESET) - 1); mqttESP_RESET[sizeof(mqttESP_RESET) - 1] = '\0'; }

    const char* getSTOP_CHARGE() const { return mqttSTOP_CHARGE; }
    void setSTOP_CHARGE(const char* d) { strncpy(mqttSTOP_CHARGE, d ? d : "", sizeof(mqttSTOP_CHARGE) - 1); mqttSTOP_CHARGE[sizeof(mqttSTOP_CHARGE) - 1] = '\0'; }

    const char* getCHARGING_TIME() const { return mqttCHARGING_TIME; }
    void setCHARGING_TIME(const char* d) { strncpy(mqttCHARGING_TIME, d ? d : "", sizeof(mqttCHARGING_TIME) - 1); mqttCHARGING_TIME[sizeof(mqttCHARGING_TIME) - 1] = '\0'; }

    const char* getPRMS() const { return mqttPRMS; }
    void setPRMS(const char* d) { strncpy(mqttPRMS, d ? d : "", sizeof(mqttPRMS) - 1); mqttPRMS[sizeof(mqttPRMS) - 1] = '\0'; }

    const char* getETOTAL() const { return mqttETOTAL; }
    void setETOTAL(const char* d) { strncpy(mqttETOTAL, d ? d : "", sizeof(mqttETOTAL) - 1); mqttETOTAL[sizeof(mqttETOTAL) - 1] = '\0'; }

    const char* getCHARGE_STATE() const { return mqttCHARGE_STATE; }
    void setCHARGE_STATE(const char* d) { strncpy(mqttCHARGE_STATE, d ? d : "", sizeof(mqttCHARGE_STATE) - 1); mqttCHARGE_STATE[sizeof(mqttCHARGE_STATE) - 1] = '\0'; }

    const char* getUART_ACTIVE() const { return mqttUART_ACTIVE; }
    void setUART_ACTIVE(const char* d) { strncpy(mqttUART_ACTIVE, d ? d : "", sizeof(mqttUART_ACTIVE) - 1); mqttUART_ACTIVE[sizeof(mqttUART_ACTIVE) - 1] = '\0'; }

    const char* getERROR() const { return mqttERROR; }
    void setERROR(const char* d) { strncpy(mqttERROR, d ? d : "", sizeof(mqttERROR) - 1); mqttERROR[sizeof(mqttERROR) - 1] = '\0'; }

    const char* getOTA_UPDATE_ACTIVE() const { mqttOTA_ACTIVE = Nvsmem::getOrInit("mqttOTA_ACTIVE", mqttOTA_ACTIVE); return mqttOTA_ACTIVE.c_str(); }
    void setOTA_UPDATE_ACTIVE(const char* d) { mqttOTA_ACTIVE = d ? d : ""; Nvsmem::loadString("mqttOTA_ACTIVE", mqttOTA_ACTIVE); }

    const char* getOTA_KEY() const   { mqttOTA_KEY = Nvsmem::getOrInit("mqttOTA_KEY", mqttOTA_KEY); return mqttOTA_KEY.c_str(); }
    void setOTA_KEY(const char* d)   { mqttOTA_KEY = d ? d : ""; Nvsmem::loadString("mqttOTA_KEY", mqttOTA_KEY); }

    const char* getOTA_VER() const   { mqttOTA_VER = Nvsmem::getOrInit("mqttOTA_VER", mqttOTA_VER); return mqttOTA_VER.c_str(); }
    void setOTA_VER(const char* d)   { mqttOTA_VER = d ? d : ""; Nvsmem::loadString("mqttOTA_VER", mqttOTA_VER); }

    const char* getGSM_SIGNAL() const  { return mqttGSM_SIGNAL; }
    void setGSM_SIGNAL (const char* d)  { strncpy(mqttGSM_SIGNAL, d ? d : "", sizeof(mqttGSM_SIGNAL) - 1); mqttGSM_SIGNAL[sizeof(mqttGSM_SIGNAL) - 1] = '\0'; }

    const char* getGRADIENT_OR_COLOR() const   { mqttGRAD_COLR = Nvsmem::getOrInit("mqttGRAD_COLR", mqttGRAD_COLR); return mqttGRAD_COLR.c_str(); }
    void setGRADIENT_OR_COLOR(const char* d)   { mqttGRAD_COLR = d ? d : ""; Nvsmem::loadString("mqttGRAD_COLR", mqttGRAD_COLR); }

    const char* getSELECT_COLOR() const   { mqttSLCT_COLR = Nvsmem::getOrInit("mqttSLCT_COLR", mqttSLCT_COLR); return mqttSLCT_COLR.c_str(); }
    void setSELECT_COLOR(const char* d)   { mqttSLCT_COLR = d ? d : ""; Nvsmem::loadString("mqttSLCT_COLR", mqttSLCT_COLR ); }
    
    const char* getCOMMSUPPORT_NETWORK() const { return mqttCMSPRT_NET; }  
    void setCOMMSUPPORT_NETWORK(const char* d) { strncpy(mqttCMSPRT_NET, d ? d : "", sizeof(mqttCMSPRT_NET) - 1); mqttCMSPRT_NET[sizeof(mqttCMSPRT_NET) - 1] = '\0'; }  

    const char* getOCPP_ACTIVE() const  { mqttOCPP_ACTIVE = Nvsmem::getOrInit("mqttOCPP_ACTIVE", mqttOCPP_ACTIVE); return mqttOCPP_ACTIVE.c_str(); }
    void setOCPP_ACTIVE (const char* d)  { mqttOCPP_ACTIVE  = d ? d : ""; Nvsmem::loadString("mqttOCPP_ACTIVE", mqttOCPP_ACTIVE); }

   // ocpp boot credentals
    const char* getC_P_VENDOR() const  { mqttC_P_VENDOR = Nvsmem::getOrInit("mqttC_P_VENDOR", mqttC_P_VENDOR); return mqttC_P_VENDOR.c_str(); }
    void setC_P_VENDOR (const char* d)  {  mqttC_P_VENDOR = d ? d : ""; Nvsmem::loadString("mqttC_P_VENDOR",  mqttC_P_VENDOR); }

    const char* getC_P_MODEL() const  { mqttC_P_MODEL = Nvsmem::getOrInit("mqttC_P_MODEL", mqttC_P_MODEL); return mqttC_P_MODEL.c_str(); }
    void setC_P_MODEL (const char* d)  { mqttC_P_MODEL  = d ? d : ""; Nvsmem::loadString("mqttC_P_MODEL", mqttC_P_MODEL); }

    const char* getC_P_SER_NUM() const  { mqttC_P_SER_NUM = Nvsmem::getOrInit("mqttC_P_SER_NUM", mqttC_P_SER_NUM); return mqttC_P_SER_NUM.c_str(); }
    void setC_P_SER_NUM(const char* d)  { mqttC_P_SER_NUM = d ? d : ""; Nvsmem::loadString("mqttC_P_SER_NUM", mqttC_P_SER_NUM); }

    const char* getMET_SER_NUM() const  { mqttMET_SER_NUM = Nvsmem::getOrInit("mqttMET_SER_NUM", mqttMET_SER_NUM); return mqttMET_SER_NUM.c_str(); }
    void setMET_SER_NUM (const char* d)  { mqttMET_SER_NUM = d ? d : ""; Nvsmem::loadString("mqttMET_SER_NUM", mqttMET_SER_NUM); }

    const char* getMET_TYPE() const  { mqttMET_TYPE = Nvsmem::getOrInit("mqttMET_TYPE", mqttMET_TYPE); return mqttMET_TYPE.c_str(); }
    void setMET_TYPE(const char* d)  { mqttMET_TYPE = d ? d : ""; Nvsmem::loadString("mqttMET_TYPE", mqttMET_TYPE); }

    const char* getBOX_SER_NUM() const  { mqttBOX_SER_NUM = Nvsmem::getOrInit("mqttBOX_SER_NUM", mqttBOX_SER_NUM); return mqttBOX_SER_NUM.c_str(); }
    void setBOX_SER_NUM(const char* d)  { mqttBOX_SER_NUM = d ? d : ""; Nvsmem::loadString("mqttBOX_SER_NUM", mqttBOX_SER_NUM); }

    const char* getOCPP_CONN() const  { return mqttOCPP_CONN; }
    void setOCPP_CONN  (const char* d)  { strncpy(mqttOCPP_CONN, d ? d : "", sizeof(mqttOCPP_CONN) - 1); mqttOCPP_CONN[sizeof(mqttOCPP_CONN) - 1] = '\0'; }

    const char* getMSP_ACTUATR() const  { return mqttMSP_ACTUATR; }
    void setMSP_ACTUATR  (const char* d)  { strncpy(mqttMSP_ACTUATR, d ? d : "", sizeof(mqttMSP_ACTUATR) - 1); mqttMSP_ACTUATR[sizeof(mqttMSP_ACTUATR) - 1] = '\0'; }
    
    const char* getMSP_TEMP() const  { return mqttMSP_TEMP; }
    void setMSP_TEMP  (const char* d)  { strncpy(mqttMSP_TEMP, d ? d : "", sizeof(mqttMSP_TEMP) - 1); mqttMSP_TEMP[sizeof(mqttMSP_TEMP) - 1] = '\0'; }

    const char* getMSP_LEAK() const  { return mqttMSP_LEAK; }
    void setMSP_LEAK  (const char* d)  { strncpy(mqttMSP_LEAK, d ? d : "", sizeof(mqttMSP_LEAK) - 1); mqttMSP_LEAK[sizeof(mqttMSP_LEAK) - 1] = '\0'; }

    const char* getMSP_STATE() const  { return mqttMSP_STATE; }
    void setMSP_STATE  (const char* d)  { strncpy(mqttMSP_STATE, d ? d : "", sizeof(mqttMSP_STATE) - 1); mqttMSP_STATE[sizeof(mqttMSP_STATE) - 1] = '\0'; }

    const char* getMSP_RGB_OUT() const  { return mqttMSP_RGB_OUT; }
    void setMSP_RGB_OUT  (const char* d)  { strncpy(mqttMSP_RGB_OUT, d ? d : "", sizeof(mqttMSP_RGB_OUT) - 1); mqttMSP_RGB_OUT[sizeof(mqttMSP_RGB_OUT) - 1] = '\0'; }

    const char* getMSP_CP_VOLT() const  { return mqttMSP_CP_VOLT; }
    void setMSP_CP_VOLT  (const char* d)  { strncpy(mqttMSP_CP_VOLT, d ? d : "", sizeof(mqttMSP_CP_VOLT) - 1); mqttMSP_CP_VOLT[sizeof(mqttMSP_CP_VOLT) - 1] = '\0'; }

    const char* getMSP_RLY_OUT() const  { return mqttMSP_RLY_OUT; }
    void setMSP_RLY_OUT  (const char* d)  { strncpy(mqttMSP_RLY_OUT, d ? d : "", sizeof(mqttMSP_RLY_OUT) - 1); mqttMSP_RLY_OUT[sizeof(mqttMSP_RLY_OUT) - 1] = '\0'; }

    const char* getMSP_PWM_EN() const  { return mqttMSP_PWM_EN; }
    void setMSP_PWM_EN  (const char* d)  { strncpy(mqttMSP_PWM_EN, d ? d : "", sizeof(mqttMSP_PWM_EN) - 1); mqttMSP_PWM_EN[sizeof(mqttMSP_PWM_EN) - 1] = '\0'; }

    const char* getMSP_L1_VOLT() const  { return mqttMSP_L1_VOLT; }
    void setMSP_L1_VOLT  (const char* d)  { strncpy(mqttMSP_L1_VOLT, d ? d : "", sizeof(mqttMSP_L1_VOLT) - 1); mqttMSP_L1_VOLT[sizeof(mqttMSP_L1_VOLT) - 1] = '\0'; }

    const char* getMSP_L1_CUR() const  { return mqttMSP_L1_CUR; }
    void setMSP_L1_CUR  (const char* d)  { strncpy(mqttMSP_L1_CUR, d ? d : "", sizeof(mqttMSP_L1_CUR) - 1); mqttMSP_L1_CUR[sizeof(mqttMSP_L1_CUR) - 1] = '\0'; }

    const char* getMSP_L2_VOLT() const  { return mqttMSP_L2_VOLT; }
    void setMSP_L2_VOLT  (const char* d)  { strncpy(mqttMSP_L2_VOLT, d ? d : "", sizeof(mqttMSP_L2_VOLT) - 1); mqttMSP_L2_VOLT[sizeof(mqttMSP_L2_VOLT) - 1] = '\0'; }

    const char* getMSP_L2_CUR() const  { return mqttMSP_L2_CUR; }
    void setMSP_L2_CUR  (const char* d)  { strncpy(mqttMSP_L2_CUR, d ? d : "", sizeof(mqttMSP_L2_CUR) - 1); mqttMSP_L2_CUR[sizeof(mqttMSP_L2_CUR) - 1] = '\0'; }

    const char* getMSP_L3_VOLT() const  { return mqttMSP_L3_VOLT; }
    void setMSP_L3_VOLT  (const char* d)  { strncpy(mqttMSP_L3_VOLT, d ? d : "", sizeof(mqttMSP_L3_VOLT) - 1); mqttMSP_L3_VOLT[sizeof(mqttMSP_L3_VOLT) - 1] = '\0'; }

    const char* getMSP_L3_CUR() const  { return mqttMSP_L3_CUR; }
    void setMSP_L3_CUR  (const char* d)  { strncpy(mqttMSP_L3_CUR, d ? d : "", sizeof(mqttMSP_L3_CUR) - 1); mqttMSP_L3_CUR[sizeof(mqttMSP_L3_CUR) - 1] = '\0'; }

    const char* getMSP_HW_VER() const  { mqttMSP_HW_VER = Nvsmem::getOrInit("mqttMSP_HW_VER", mqttMSP_HW_VER); return mqttMSP_HW_VER.c_str(); }
    void setMSP_HW_VER  (const char* d)  { mqttMSP_HW_VER = d ? d : ""; Nvsmem::loadString("mqttMSP_HW_VER", mqttMSP_HW_VER); }

    const char* getMSP_SW_VER() const { mqttMSP_SW_VER = Nvsmem::getOrInit("mqttMSP_SW_VER", mqttMSP_SW_VER); return mqttMSP_SW_VER.c_str(); }
    void setMSP_SW_VER(const char* d) { mqttMSP_SW_VER = d ? d : ""; Nvsmem::loadString("mqttMSP_SW_VER", mqttMSP_SW_VER); }

    const char* getSCAN_LIST() const  { return mqttSCAN_LIST; }
    void setSCAN_LIST  (const char* d)  { strncpy(mqttSCAN_LIST, d ? d : "", sizeof(mqttSCAN_LIST) - 1); mqttSCAN_LIST[sizeof(mqttSCAN_LIST) - 1] = '\0'; }    

    const char* getLOGO_UPDATE() const { mqttLOGO_UPDATE = Nvsmem::getOrInit("logoupd", mqttLOGO_UPDATE); return mqttLOGO_UPDATE.c_str(); }
    void setLOGO_UPDATE(const char* d) { mqttLOGO_UPDATE = d ? d : ""; Nvsmem::loadString("logoupd", mqttLOGO_UPDATE); }

    const char* getLOGO_NAME() const { mqttLOGO_NAME = Nvsmem::getOrInit("logoname", mqttLOGO_NAME); return mqttLOGO_NAME.c_str(); }
    void setLOGO_NAME(const char* d) { mqttLOGO_NAME = d ? d : ""; Nvsmem::loadString("logoname", mqttLOGO_NAME); }

    const char* getACTIVE_LOGO() const { mqttACTIVE_LOGO = Nvsmem::getOrInit("activelogo", mqttACTIVE_LOGO); return mqttACTIVE_LOGO.c_str(); }
    void setACTIVE_LOGO(const char* d) { mqttACTIVE_LOGO = d ? d : ""; Nvsmem::loadString("activelogo", mqttACTIVE_LOGO); }

    const char* getRFID_UPDATE() const { mqttRFID_UPDATE = Nvsmem::getOrInit("mqttRFID_UPDATE", mqttRFID_UPDATE); return mqttRFID_UPDATE.c_str(); }
    void setRFID_UPDATE(const char* d) { mqttRFID_UPDATE = d ? d : ""; Nvsmem::loadString("mqttRFID_UPDATE", mqttRFID_UPDATE); }

    const char* getACTIVE_RFID() const { mqttACTIVE_RFID = Nvsmem::getOrInit("mqttACTIVE_RFID", mqttACTIVE_RFID); return mqttACTIVE_RFID.c_str(); }
    void setACTIVE_RFID(const char* d) { mqttACTIVE_RFID = d ? d : ""; Nvsmem::loadString("mqttACTIVE_RFID", mqttACTIVE_RFID); }

    const char* getESP_HW_VER() const  { mqttESP_HW_VER = Nvsmem::getOrInit("mqttESP_HW_VER", mqttESP_HW_VER); return mqttESP_HW_VER.c_str(); }
    void setESP_HW_VER  (const char* d)  { mqttESP_HW_VER = d ? d : ""; Nvsmem::loadString("mqttESP_HW_VER", mqttESP_HW_VER); }

};





inline DataValue mqttDataValue;


inline String logMsg = "";


inline void publishFrame(MqttFrameHandler& sendFrame,
                         PubSubClient& mqtt,
                         const char* topic,
                         uint8_t cmd, uint8_t msg,
                         const char* data)
{
  char buffer[320];
  sendFrame.setAll(cmd, msg, data ? data : "");
  int outLen = sendFrame.encode(buffer, sizeof(buffer));
  if (outLen > 0)
    mqtt.publish(topic, reinterpret_cast<const uint8_t*>(buffer), outLen);
}

// ---------------- SET / READ / LOG ----------------
inline bool protoSet(DataValue& abdl, uint8_t msg, const char* val) {
  switch (msg) {
    case mqttESP_ID:               abdl.setESP_ID(val); return true;
    case mqttOCPP_URL:             abdl.setOCPP_URL(val); return true;
    case mqttOCPP_ID:              abdl.setOCPP_ID(val); return true;
    case mqttQR_ID:                abdl.setQR_ID(val); return true;
    case mqttMAX_POWER:            abdl.setMAX_POWER(val); return true;
    case mqttWIFI_SSID:            abdl.setWIFI_SSID(val); return true;
    case mqttWIFI_PASS:            abdl.setWIFI_PASS(val); return true;
    case mqttCONNECTION_TYPE:      abdl.setCONNECTION_TYPE(val); return true;
    case mqttUNLOCK_CONN:          abdl.setUNLOCK_CONN(val); return true;

    case mqttUI_COMPANY_TEL_NUM:   abdl.setUI_COMPANY_TEL_NUM(val); return true;
    case mqttUI_COMPANY_URL:       abdl.setUI_COMPANY_URL(val); return true;
    case mqttUI_COMPANY_NAME:      abdl.setUI_COMPANY_NAME(val); return true;
    case mqttUI_APPSTORE_URL:      abdl.setUI_APPSTORE_URL(val); return true;
    case mqttUI_PLAYSTORE_URL:     abdl.setUI_PLAYSTORE_URL(val); return true;
    case mqttUI_LOGO_OR_WRITING:   abdl.setUI_LG_TXT(val); return true;
    case mqttOTA_UPDATE_ACTIVE:    abdl.setOTA_UPDATE_ACTIVE(val); return true;

    case mqttCONF_PARAMS_RESET:        abdl.setCONF_PARAMS_RESET(val); return true;
    case mqttCONF_PARAMS_MEASURETYPE:  abdl.setCONF_PARAMS_MEASURETYPE(val); return true;
    case mqttCONF_PARAMS_MODE:         abdl.setCONF_PARAMS_MODE(val); return true;
    case mqttCONF_PARAMS_MODEL:        abdl.setCONF_PARAMS_MODEL(val); return true;

    case mqttESP_MODE:             abdl.setESP_MODE(val); return true;
    case mqttESP_RESET:            abdl.setESP_RESET(val); return true;
    case mqttSTOP_CHARGE:          abdl.setSTOP_CHARGE(val); return true;
                                    
    case mqttOTA_KEY:          abdl.setOTA_KEY(val);return true;
    case mqttOTA_VER:          abdl.setOTA_VER(val);return true;
  //  case mqttGSM_SIGNAL:       abdl.setGSM_SIGNAL(val);return true;

    case mqttGRADIENT_OR_COLOR:  abdl.setGRADIENT_OR_COLOR(val);return true;
    case mqttSELECT_COLOR:       abdl.setSELECT_COLOR(val);return true;

   // case mqttCMSPRT_NET:         abdl.setCOMMSUPPORT_NETWORK(val);return true;
    case mqttOCPP_ACTIVE:        abdl.setOCPP_ACTIVE(val);return true;

    case mqttC_P_VENDOR:        abdl.setC_P_VENDOR(val);return true;
    case mqttC_P_MODEL:         abdl.setC_P_MODEL(val);return true;
    case mqttC_P_SER_NUM:       abdl.setC_P_SER_NUM(val);return true;
    case mqttMET_SER_NUM:       abdl.setMET_SER_NUM(val);return true;
    case mqttMET_TYPE:          abdl.setMET_TYPE(val);return true;
    case mqttBOX_SER_NUM:       abdl.setBOX_SER_NUM(val);return true;

   // case mqttOCPP_CONN:         abdl.setOCPP_CONN(val);return true;
   
 /* case mqttMSP_ACTUATR:       abdl.setMSP_ACTUATR(val);return true;
    case mqttMSP_TEMP:          abdl.setMSP_TEMP(val);return true;
    case mqttMSP_LEAK:          abdl.setMSP_LEAK(val);return true;
    case mqttMSP_STATE:         abdl.setMSP_STATE(val);return true;
    case mqttMSP_RGB_OUT:       abdl.setMSP_RGB_OUT(val);return true;
    case mqttMSP_CP_VOLT:       abdl.setMSP_CP_VOLT(val);return true;
    case mqttMSP_RLY_OUT:       abdl.setMSP_RLY_OUT(val);return true;
    case mqttMSP_PWM_EN:        abdl.setMSP_PWM_EN(val);return true;
    case mqttMSP_L1_VOLT:       abdl.setMSP_L1_VOLT(val);return true;
    case mqttMSP_L1_CUR:        abdl.setMSP_L1_CUR(val);return true;
    case mqttMSP_L2_VOLT:       abdl.setMSP_L2_VOLT(val);return true;
    case mqttMSP_L2_CUR:        abdl.setMSP_L2_CUR(val);return true;
    case mqttMSP_L3_VOLT:       abdl.setMSP_L3_VOLT(val);return true;
    case mqttMSP_L3_CUR:        abdl.setMSP_L3_CUR(val);return true;*/
    case mqttMSP_HW_VER:        abdl.setMSP_HW_VER(val);  return true;
    case mqttMSP_SW_VER:        abdl.setMSP_SW_VER(val);  return true;
    
    case mqttLOGO_UPDATE:       abdl.setLOGO_UPDATE(val); return true;
    case mqttLOGO_NAME:         abdl.setLOGO_NAME(val);   return true;
    case mqttACTIVE_LOGO:       abdl.setACTIVE_LOGO(val); return true;
    case mqttRFID_UPDATE:       abdl.setRFID_UPDATE(val); return true;
    case mqttACTIVE_RFID:       abdl.setACTIVE_RFID(val); return true;
    case mqttESP_HW_VER:        abdl.setESP_HW_VER(val);  return true;
      return false;

    default:
      return false;
  }
}

inline const char* protoRead(const DataValue& abdl, uint8_t msg) {
  switch (msg) {
    case mqttESP_ID:               return abdl.getESP_ID();
    case mqttOCPP_URL:             return abdl.getOCPP_URL();
    case mqttOCPP_ID:              return abdl.getOCPP_ID();
    case mqttQR_ID:                return abdl.getQR_ID();
    case mqttMAX_POWER:            return abdl.getMAX_POWER();
    case mqttWIFI_SSID:            return abdl.getWIFI_SSID();
    case mqttWIFI_PASS:            return abdl.getWIFI_PASS();
    case mqttCONNECTION_TYPE:      return abdl.getCONNECTION_TYPE();
    case mqttUNLOCK_CONN:          return abdl.getUNLOCK_CONN();

    case mqttUI_COMPANY_TEL_NUM:   return abdl.getUI_COMPANY_TEL_NUM();
    case mqttUI_COMPANY_URL:       return abdl.getUI_COMPANY_URL();
    case mqttUI_COMPANY_NAME:      return abdl.getUI_COMPANY_NAME();
    case mqttUI_APPSTORE_URL:      return abdl.getUI_APPSTORE_URL();
    case mqttUI_PLAYSTORE_URL:     return abdl.getUI_PLAYSTORE_URL();
    case mqttUI_LOGO_OR_WRITING:   return abdl.getUI_LG_TXT();
    case mqttOTA_UPDATE_ACTIVE:    return abdl.getOTA_UPDATE_ACTIVE();

    case mqttCONF_PARAMS_RESET:        return abdl.getCONF_PARAMS_RESET();
    case mqttCONF_PARAMS_MEASURETYPE:  return abdl.getCONF_PARAMS_MEASURETYPE();
    case mqttCONF_PARAMS_MODE:         return abdl.getCONF_PARAMS_MODE();
    case mqttCONF_PARAMS_MODEL:        return abdl.getCONF_PARAMS_MODEL();

    case mqttESP_MODE:             return abdl.getESP_MODE();
    case mqttESP_RESET:            return abdl.getESP_RESET();
    case mqttSTOP_CHARGE:          return abdl.getSTOP_CHARGE();

    // Skip flash writes for this parameter; it changes regularly.
    case mqttCHARGING_TIME:        return abdl.getCHARGING_TIME();
    case mqttPRMS:                 return abdl.getPRMS() ;
    case mqttETOTAL:               return abdl.getETOTAL();
    case mqttCHARGE_STATE:         return abdl.getCHARGE_STATE() ;
    case mqttUART_ACTIVE:          return abdl.getUART_ACTIVE();
    case mqttERROR:                return abdl.getERROR();

    case mqttOTA_KEY:              return abdl.getOTA_KEY();
    case mqttOTA_VER:              return abdl.getOTA_VER();
    case mqttGSM_SIGNAL:           return abdl.getGSM_SIGNAL();

    case mqttGRADIENT_OR_COLOR:    return abdl.getGRADIENT_OR_COLOR();
    case mqttSELECT_COLOR:         return abdl.getSELECT_COLOR();

    case mqttCMSPRT_NET:           return abdl.getCOMMSUPPORT_NETWORK();
    case mqttOCPP_ACTIVE:          return abdl.getOCPP_ACTIVE();
    

    case mqttC_P_VENDOR:           return abdl.getC_P_VENDOR();
    case mqttC_P_MODEL:            return abdl.getC_P_MODEL();
    case mqttC_P_SER_NUM:          return abdl.getC_P_SER_NUM();
    case mqttMET_SER_NUM:          return abdl.getMET_SER_NUM();
    case mqttMET_TYPE:             return abdl.getMET_TYPE();
    case mqttBOX_SER_NUM:          return abdl.getBOX_SER_NUM();
 
    case mqttOCPP_CONN:            return abdl.getOCPP_CONN();
    
    case mqttMSP_ACTUATR:          return abdl.getMSP_ACTUATR();
    case mqttMSP_TEMP:             return abdl.getMSP_TEMP();
    case mqttMSP_LEAK:             return abdl.getMSP_LEAK();
    case mqttMSP_STATE:            return abdl.getMSP_STATE();
    case mqttMSP_RGB_OUT:          return abdl.getMSP_RGB_OUT();
    case mqttMSP_CP_VOLT:          return abdl.getMSP_CP_VOLT();
    case mqttMSP_RLY_OUT:          return abdl.getMSP_RLY_OUT();
    case mqttMSP_PWM_EN:           return abdl.getMSP_PWM_EN();
    case mqttMSP_L1_VOLT:          return abdl.getMSP_L1_VOLT();
    case mqttMSP_L1_CUR:           return abdl.getMSP_L1_CUR();
    case mqttMSP_L2_VOLT:          return abdl.getMSP_L2_VOLT();
    case mqttMSP_L2_CUR:           return abdl.getMSP_L2_CUR();
    case mqttMSP_L3_VOLT:          return abdl.getMSP_L3_VOLT();
    case mqttMSP_L3_CUR:           return abdl.getMSP_L3_CUR();
    case mqttMSP_HW_VER:           return abdl.getMSP_HW_VER();
    case mqttMSP_SW_VER:           return abdl.getMSP_SW_VER();

    case mqttSCAN_LIST:            return abdl.getSCAN_LIST();
    case mqttLOGO_UPDATE:          return abdl.getLOGO_UPDATE();
    case mqttLOGO_NAME:            return abdl.getLOGO_NAME();
    case mqttACTIVE_LOGO:          return abdl.getACTIVE_LOGO();

    case mqttRFID_UPDATE:          return abdl.getRFID_UPDATE();
    case mqttACTIVE_RFID:          return abdl.getACTIVE_RFID();
    case mqttESP_HW_VER:           return abdl.getESP_HW_VER();    
      return nullptr;

    default:
      return nullptr;
  }
}


inline void protoLog(uint8_t msg, const DataValue& /*abdl*/) {
  switch (msg) {
    case mqttLOG_VERBOSE: logMsg = "VERBOSEA GIRDIM"; break;
    case mqttLOG_DEBUG:   logMsg = "DEBUGE GIRDIM";   break;
    case mqttLOG_INFO:    logMsg = "INFOYA GIRDIM";   break;
    default:          logMsg = "LOG UNKNOWN";     break;
  }
}


inline void handleSetCmd(DataValue& mqttDataValue, uint8_t msg, const char* val,
                         MqttFrameHandler& sendFrame, PubSubClient& mqtt, const char* topic)
{
  const bool ok = protoSet(mqttDataValue, msg, val);
  publishFrame(sendFrame, mqtt, topic, CMD_SET, msg, ok ? "SET OK" : "SET UNSUPPORTED");
}

inline void handleReadCmd(const DataValue& mqttDataValue, uint8_t msg,
                          MqttFrameHandler& sendFrame, PubSubClient& mqtt, const char* topic)
{
  const char* resp = protoRead(mqttDataValue, msg);
  publishFrame(sendFrame, mqtt, topic, CMD_READ, msg, resp ? resp : "READ UNSUPPORTED");
}

inline void handleLogCmd(uint8_t msg, const DataValue& mqttDataValue,
                         MqttFrameHandler& sendFrame, PubSubClient& mqtt, const char* topic)
{
  protoLog(msg, mqttDataValue);
  publishFrame(sendFrame, mqtt, topic, CMD_LOG, msg, logMsg.c_str());
}

// ---------------- Ana handler ----------------
inline void handleMqttProtocol(const uint8_t* payload, unsigned length,
                               MqttFrameHandler& receiveFrame,
                               MqttFrameHandler& sendFrame,
                               PubSubClient& mqtt,
                               const char* topicToServer)
{
  if (!receiveFrame.decode(payload, length)) {
    publishFrame(sendFrame, mqtt, topicToServer, 0xFF, 0, "ERR:Parse failed");
    return;
  }

  const uint8_t cmd = receiveFrame.getCmdType();
  const uint8_t msg = receiveFrame.getMsgType();

  if (cmd == CMD_SET) {

    handleSetCmd(mqttDataValue, msg, receiveFrame.getData(), sendFrame, mqtt, topicToServer);

  } else if (cmd == CMD_READ) {

    handleReadCmd(mqttDataValue, msg, sendFrame, mqtt, topicToServer);

  } else if (cmd == CMD_LOG) {

    handleLogCmd(msg, mqttDataValue, sendFrame, mqtt, topicToServer);

  } else {
    publishFrame(sendFrame, mqtt, topicToServer, 0xFF, 0, "UNKNOWN CMD");
  }
}

// wrapper funktion  işlenecek veri tanımlandı  MODULARİTE SIFIR 0 0 :(

extern void WrapperMqttSetESP_ID(const char* v);  // prototip
inline void WrapperMqttSetESP_ID(const char* v) { // tanım
    mqttDataValue.setESP_ID(v);
}


extern void WrapperMqttSetUnlockConn(const char* v);  // prototip
inline void WrapperMqttSetUnlockConn(const char* v) { // tanım
    mqttDataValue.setUNLOCK_CONN(v);
}

extern void WrapperMqttSetChargeState(const char* v);  // prototip
inline void WrapperMqttSetChargeState(const char* v) { // tanım
    mqttDataValue.setCHARGE_STATE(v);
}

extern void WrapperMqttSetChargeingTime(const char* v);  // prototip
inline void WrapperMqttSetChargeingTime(const char* v) { // tanım
    mqttDataValue.setCHARGING_TIME(v);
}

extern void WrapperMqttSetPRMS(const char* v);  // prototip
inline void WrapperMqttSetPRMS(const char* v) { // tanım
    mqttDataValue.setPRMS(v);
}

extern void WrapperMqttSetETOTAL(const char* v);  // prototip
inline void WrapperMqttSetETOTAL(const char* v) { // tanım
    mqttDataValue.setETOTAL(v);
}

extern void WrapperMqttSetGSM_SIGNAL(const char* v);  // prototip
inline void WrapperMqttSetGSM_SIGNAL(const char* v) { // tanım
    mqttDataValue.setGSM_SIGNAL(v);
}
  // prototip
extern void WrapperMqttSetCOMMSUPPORT_NETWORK(const char* v);
inline void WrapperMqttSetCOMMSUPPORT_NETWORK(const char* v) { // tanım
    mqttDataValue.setCOMMSUPPORT_NETWORK(v);
}

extern void WrapperMqttSetSCAN_LIST(const char* v);
inline void WrapperMqttSetSCAN_LIST(const char* v) { // tanım
    mqttDataValue.setSCAN_LIST(v);
}

extern inline const char* WrapperMqttGetWIFI_SSID(void) {
  return mqttDataValue.getWIFI_SSID();  
}

extern inline const char* WrapperMqttGetWIFI_PASS(void) {
  return mqttDataValue.getWIFI_PASS();  
}

extern inline const char* WrapperMqttGetCONNECTION_TYPE(void) {
  return mqttDataValue.getCONNECTION_TYPE() ;  
}

extern inline const char* WrapperMqttGetOTA_VER(void) {
  return mqttDataValue.getOTA_VER();  
}

extern inline const char* WrapperMqttGetUI_LG_TXT(void) {
  return mqttDataValue.getUI_LG_TXT();  
}
extern inline const char* WrapperMqttGetUI_COMPANY_NAME(void) {
  return mqttDataValue.getUI_COMPANY_NAME();  

}
extern inline const char* WrapperMqttGetUI_COMPANY_TEL_NUM(void) {
  return mqttDataValue.getUI_COMPANY_TEL_NUM();  
}
extern inline const char* WrapperMqttGetUI_COMPANY_URL(void) {
  return mqttDataValue.getUI_COMPANY_URL() ;  
}
extern inline const char* WrapperMqttGetUI_APPSTORE_URL(void) {
  return mqttDataValue.getUI_APPSTORE_URL();  
}
extern inline const char* WrapperMqttGetUI_PLAYSTORE_URL(void) {
  return mqttDataValue.getUI_PLAYSTORE_URL();  
}


extern inline const char* WrapperMqttGetGRADIENT_OR_COLOR(void) {
  return mqttDataValue.getGRADIENT_OR_COLOR();  
}
extern inline const char* WrapperMqttGetSELECT_COLOR(void) {
  return mqttDataValue.getSELECT_COLOR();  
}

extern inline const char* WrapperMqttGetESP_ID(void) {
  return mqttDataValue.getESP_ID();  
}

extern inline const char* WrapperMqttGetMSP_SW_VER(void) {
  return mqttDataValue.getMSP_SW_VER();  
}
#endif 

/*
 * Proje   : EVSE – MQTT / OCPP Haberleşme
 * Dosya   : mqttProtocol.h
 * Amaç    : MQTT çerçevelerini (SET/READ/LOG) çözümlemek/üretmek, DataValue
 *           üzerinden konfigürasyon/telemetri get/set işlemlerini yapmak.
 *
 * Yazar   : Abdulhamit Mercan
 * Tarih   : Ağustos 2025
 *
 * Protokol:
 * - Frame: 'c' + CMD(1B) + MSG(1B) + DATA(N) + 'v'
 * - CMD  : { 1=CMD_SET, 2=CMD_READ, 3=CMD_LOG }
 * - MSG  : MessageTypeData (mqttESP_ID, mqttOCPP_URL, … mqttLOG_VERBOSE/DEBUG/INFO)
 *
 * Özet    :
 * - DataValue: tüm konfig/telemetri alanlarının (String) saklayıcısı
 * - NVS entegrasyonu: nvsmem.h ile kalıcı değerleri okur/yazar (key ≤ 15 char)
 * - publishFrame(): MqttFrameHandler ile encode + MQTT publish
 * - handleMqttProtocol(): payload → decode → protoSet / protoRead / protoLog
 *
 * Notlar:
 * - Sık değişen runtime değerler (CHARGING_TIME, PRMS, ETOTAL, CHARGE_STATE,
 *   UART_ACTIVE, ERROR) flash’a yazılmaz; RAM’de tutulur.
 * - ESP_ID benzersiz olmalı (Linux tarafı için kritik).
 */


