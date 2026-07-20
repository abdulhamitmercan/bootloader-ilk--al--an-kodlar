#ifndef UI_SCREEN_H
#define UI_SCREEN_H
#include "ui.h"
#include "ui_helpers.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>   
#include <lvgl.h>
#include <math.h> 

#include "config.h"

extern void WrapperMqttSetChargeingTime(const char* v); // wrapper 
extern void WrapperMqttSetPRMS(const char* v); // wrapper 
extern void WrapperMqttSetETOTAL(const char* v); // wrapper 
extern void WrapperMqttSetESP_ID(const char* v);
extern void WrapperMqttSetGSM_SIGNAL(const char* v);
extern const char*WrapperMqttGetUI_COMPANY_NAME(void);
extern const char*WrapperMqttGetUI_LG_TXT(void);
extern const char*WrapperMqttGetUI_COMPANY_TEL_NUM(void);
extern const char*WrapperMqttGetUI_COMPANY_URL(void);
extern const char*WrapperMqttGetUI_PLAYSTORE_URL(void);
extern const char*WrapperMqttGetUI_APPSTORE_URL(void);
extern const char*WrapperMqttGetOTA_VER(void); 

extern const char*WrapperMqttGetSELECT_COLOR(void);
extern const char*WrapperMqttGetGRADIENT_OR_COLOR(void);

extern const char* WrapperMqttGetMSP_SW_VER(void);
int csq = 99;
int dbm =99; 
int csqToDbm(int csq ) {
  if (csq < 0 || csq == 99) return 0x7FFF;
  if (csq > 31) csq = 31;// 31 parçali  kuantalıyor sim 800 o sebepten -53 dbi dan daha iyi olsa bile gösteremiyoruz
  return -113 + 2 * csq;
}

// Eşikler: ≤-95 Kritik, ≤-85 Riskli, ≤-75 Yeterli, ≤-65 Iyi, >-65 Harika
inline const char* rssiLevelLabelFromDbmGSM(int dbm) {
  if (dbm == 0x7FFF) return "Bilinmiyor";
  if (dbm <= -95)    return "Kritik";
  if (dbm <= -85)    return "Riskli";
  if (dbm <= -75)    return "Yeterli";
  if (dbm <= -65)    return "Iyi";
  return "Harika";
}

inline const char* rssiLevelLabelFromDbmWifi(int dbm) {
  if (dbm == 0x7FFF) return "Bilinmiyor"; // ölçüm yok / geçersiz
  if (dbm <= -85)    return "Kritik";
  if (dbm <= -75)    return "Riskli";
  if (dbm <= -67)    return "Yeterli";   // VoIP/akıcı video için tipik alt sınır
  if (dbm <= -55)    return "Iyi";
  return "Harika";
}

static inline void bar_on(lv_obj_t* p, lv_color_t col) {
   
    lv_obj_set_style_bg_color(p, col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa  (p, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 255

}

static inline void bar_off(lv_obj_t* p) {

    
    lv_obj_set_style_bg_color(p, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa  (p, 160, LV_PART_MAIN | LV_STATE_DEFAULT); //102  yarı opak
   
          
}
static inline void bar_close(lv_obj_t* p){
     lv_obj_set_style_bg_opa  (p, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

}

static inline void arc_on(lv_obj_t* a, lv_color_t col)
{
    lv_obj_set_style_arc_color(a, col, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa  (a, LV_OPA_COVER,LV_PART_INDICATOR | LV_STATE_DEFAULT);
}

static inline void arc_off(lv_obj_t* a)
{

    lv_obj_set_style_arc_color(a, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(a, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static inline void arc_close(lv_obj_t* a)
{
    
 lv_obj_set_style_arc_opa(a, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}
void eth_icon_open(){
   lv_obj_set_style_bg_opa(ui_Container2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(ui_Container3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_border_opa(ui_Container1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(ui_Container1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_label_set_text(ui_Label10, "Ethernet");
  lv_label_set_text(ui_Label9, "");
   lv_label_set_text(ui_Label8, "");
   lv_label_set_text(ui_Label7, "");
}

void eth_icon_close(){
   lv_obj_set_style_bg_opa(ui_Container2,0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(ui_Container3,0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_border_opa(ui_Container1,0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(ui_Container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_label_set_text(ui_Label10, "");
   lv_label_set_text(ui_Label9, "dBm");

}
void wifi_icon_close(){
   arc_close(ui_Arc1);
   arc_close(ui_Arc2); 
   arc_close(ui_Arc3); 
   arc_close(ui_Arc4);

}

void GSM_icon_close(){
   bar_close(ui_Panel1);
   bar_close(ui_Panel2);
   bar_close(ui_Panel3);
   bar_close(ui_Panel4);



}


static inline lv_color_t levelColorFromDbm(int dbm) {

  

    if (netType==NET_TYPE_GSM||(netType==NET_TYPE_ALL&&comsupportedNetwork == NET_TYPE_GSM)){
        wifi_icon_close();
        eth_icon_close();  

        if (dbm == 0x7FFF) {return lv_color_hex(0x9E9E9E);} // Bilinmiyor (gri)
        else if (dbm <= -95) {
            // TEK diş kırmızı, diğerleri gri yarı opak
            bar_on (ui_Panel1, lv_color_hex(0xD32F2F)); // kritik: kırmızı diş
            bar_off(ui_Panel2);
            bar_off(ui_Panel3);
            bar_off(ui_Panel4);
            return lv_color_hex(0xD32F2F);             // yazı rengi kırmızı
        }
        else if (dbm <= -85) {
            
            bar_on (ui_Panel1, lv_color_hex(0xFFFFFF));  
            bar_off(ui_Panel2);
            bar_off(ui_Panel3);
            bar_off(ui_Panel4);
            return lv_color_hex(0xFB8C00);               // yazı turuncu
        }
        else if (dbm <= -75) {
            bar_on (ui_Panel1, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel2, lv_color_hex(0xFFFFFF));
            bar_off(ui_Panel3);
            bar_off(ui_Panel4);
            return lv_color_hex(0xFBC02D);               // yazı sarı
        }
        else if (dbm <= -65) {
            bar_on (ui_Panel1, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel2, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel3, lv_color_hex(0xFFFFFF));
            bar_off(ui_Panel4);
            return lv_color_hex(0x66FF00);               // yazı yeşil
        }
        else {
            bar_on (ui_Panel1, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel2, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel3, lv_color_hex(0xFFFFFF));
            bar_on (ui_Panel4, lv_color_hex(0xFFFFFF));
            return lv_color_hex(0x00004D);               // yazı mavi
        }
        
    }
    if (netType==NET_TYPE_WIFI||(netType==NET_TYPE_ALL&&comsupportedNetwork == NET_TYPE_WIFI)){
        GSM_icon_close();
        eth_icon_close();

        if (dbm == 0x7FFF) {return lv_color_hex(0x9E9E9E); }// Bilinmiyor (gri)

        // KRİTİK: tek diş kırmızı, diğerleri kapalı/yarı opak
        if (dbm <= -85) {
            arc_on (ui_Arc1, lv_color_hex(0xD32F2F)); // kırmızı
            arc_off(ui_Arc2);
            arc_off(ui_Arc3);
            arc_off(ui_Arc4);
            return lv_color_hex(0xD32F2F); // yazı kırmızı
        }

        // RİSKLİ: 1 diş beyaz
        if (dbm <= -75) {
            arc_on (ui_Arc1, lv_color_hex(0xFFFFFF));
            arc_off(ui_Arc2);
            arc_off(ui_Arc3);
            arc_off(ui_Arc4);
            return lv_color_hex(0xFB8C00); // yazı turuncu
        }

        // YETERLİ: 2 diş beyaz
        if (dbm <= -67) {
            arc_on (ui_Arc1, lv_color_hex(0xFFFFFF));
            arc_on (ui_Arc2, lv_color_hex(0xFFFFFF));
            arc_off(ui_Arc3);
            arc_off(ui_Arc4);
            return lv_color_hex(0xFBC02D); // yazı sarı
        }

        // İYİ: 3 diş beyaz
        if (dbm <= -55) {
            arc_on (ui_Arc1, lv_color_hex(0xFFFFFF));
            arc_on (ui_Arc2, lv_color_hex(0xFFFFFF));
            arc_on (ui_Arc3, lv_color_hex(0xFFFFFF));
            arc_off(ui_Arc4);
            return lv_color_hex(0x66FF00); // yazı yeşil
        }

        // HARİKA: 4 diş beyaz
        arc_on (ui_Arc1, lv_color_hex(0xFFFFFF));
        arc_on (ui_Arc2, lv_color_hex(0xFFFFFF));
        arc_on (ui_Arc3, lv_color_hex(0xFFFFFF));
        arc_on (ui_Arc4, lv_color_hex(0xFFFFFF));
        return lv_color_hex(0x00004D);     // yazı koyu mavi
        
  }

}
void screen_eth_upload(){

   GSM_icon_close();
   wifi_icon_close();
   eth_icon_open();

}
static inline const char* getDeviceMacCStr(void) {
    static char macStr[18];          // "AA:BB:CC:DD:EE:FF" + '\0'
    uint8_t mac[6] = {0};

    
    #ifdef ESP_MAC_WIFI_STA
      esp_read_mac(mac, ESP_MAC_WIFI_STA);
    #else
      esp_efuse_mac_get_default(mac);
    #endif

    snprintf(macStr, sizeof(macStr),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macStr;
}

void assignUint16_c(const char* s, uint16_t* dst) {
    if (!s || !dst) return;

    while (*s && isspace((unsigned char)*s)) ++s;

    errno = 0;
    char* end = NULL;
    unsigned long v = strtoul(s, &end, 0); 

    if (end != s && errno != ERANGE && v <= 65535UL) {
        *dst = (uint16_t)v;
    }
}


static int lv_color_from_string(const char* s, lv_color_t* out_color, lv_opa_t* out_opa)
{
    if (!s || !out_color) return 0;

    while (*s && isspace((unsigned char)*s)) ++s;   // baştaki boşlukları at
    if (*s == '#') ++s;                              // # önekini at

    errno = 0;
    char* end = NULL;
    unsigned long v = strtoul(s, &end, 0);           // "0x4DB748 " gibi ifadeleri okur
    if (end == s || errno == ERANGE) return 0;

    // kaç hex hanesi var, alfa var mı?
    const char* p = s;
    if (p[0]=='0' && (p[1]=='x' || p[1]=='X')) p += 2;
    int hex_digits = 0;
    while (isxdigit((unsigned char)p[hex_digits])) hex_digits++;

    uint32_t rgb = (uint32_t)v & 0x00FFFFFFu;
    lv_opa_t opa = LV_OPA_COVER;
    if (hex_digits >= 8) {                  // AARRGGBB ise alfa uygula
        opa = (lv_opa_t)((v >> 24) & 0xFF);
    }

    *out_color = lv_color_hex(rgb);
    if (out_opa) *out_opa = opa;
    return 1;
}

void mqtt_screen_update(){

   const char* mac = getDeviceMacCStr();
   lv_label_set_text(ui_Label69, mac);
   WrapperMqttSetESP_ID(mac);

   uint16_t lg16 = 0;
   assignUint16_c(WrapperMqttGetUI_LG_TXT(), &lg16);        
   int lg = (int)lg16; 
   if(lg==0){
   lv_label_set_text(ui_Label1, WrapperMqttGetUI_COMPANY_NAME());
   lv_label_set_text(ui_Label21, WrapperMqttGetUI_COMPANY_NAME());
   lv_label_set_text(ui_Label41, WrapperMqttGetUI_COMPANY_NAME());
   lv_label_set_text(ui_Label61,WrapperMqttGetUI_COMPANY_NAME());

   lv_obj_set_style_opa(ui_Image1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image16, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image31, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image46, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   }
   if(lg==1){
   lv_label_set_text(ui_Label1, "");
   lv_label_set_text(ui_Label21, "");
   lv_label_set_text(ui_Label41, "");
   lv_label_set_text(ui_Label61,"");

   lv_obj_set_style_opa(ui_Image1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image16, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image31, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_opa(ui_Image46, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   }
  lv_label_set_text(ui_Label5, WrapperMqttGetUI_COMPANY_URL());
  lv_label_set_text(ui_Label6, WrapperMqttGetUI_COMPANY_TEL_NUM());

    lv_obj_t *gr1 = lv_qrcode_create(ui_Screen1, 100, lv_color_hex3(0x000), lv_color_hex3(0xeef));//playstore
    lv_qrcode_update(gr1, WrapperMqttGetUI_PLAYSTORE_URL() , strlen(WrapperMqttGetUI_PLAYSTORE_URL()));
    lv_obj_set_width(gr1, LV_SIZE_CONTENT);
    lv_obj_set_height(gr1, LV_SIZE_CONTENT);
    lv_obj_set_x(gr1, 300);
    lv_obj_set_y(gr1, 120);
    lv_obj_set_align(gr1, LV_ALIGN_CENTER);
    lv_obj_add_flag(gr1, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(gr1, LV_OBJ_FLAG_SCROLLABLE);
    lv_img_set_zoom(gr1, 256);

    lv_obj_t *gr2 = lv_qrcode_create(ui_Screen1, 100, lv_color_hex3(0x000), lv_color_hex3(0xeef)); // appstore
    lv_qrcode_update(gr2,WrapperMqttGetUI_APPSTORE_URL(), strlen(WrapperMqttGetUI_APPSTORE_URL()));
    lv_obj_set_x(gr2, -315);
    lv_obj_set_y(gr2, 120);
    lv_obj_set_align(gr2, LV_ALIGN_CENTER);
    lv_obj_add_flag(gr2, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(gr2, LV_OBJ_FLAG_SCROLLABLE);
 
     uint16_t selctionparam16 =99;
   assignUint16_c(WrapperMqttGetGRADIENT_OR_COLOR(), &selctionparam16 );        // önce uint16_t olarak al
   int selctionparam = (int)selctionparam16; 


   

   
   const char* a = WrapperMqttGetSELECT_COLOR();   
   lv_color_t col;
   lv_opa_t opa;
   
   if(selctionparam ==0){

    lv_obj_set_style_bg_grad_color(ui_Screen1, lv_color_hex(0x10779E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Screen1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Screen1, 238, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen1, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_grad_color(ui_Screen2, lv_color_hex(0x10779E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Screen2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Screen2, 238, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen2, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_grad_color(ui_Screen3, lv_color_hex(0x10779E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Screen3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Screen3, 238, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen3, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_grad_color(ui_Screen4, lv_color_hex(0x10779E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_main_stop(ui_Screen4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_stop(ui_Screen4, 238, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen4, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);


   }
   if(selctionparam == 1){
        if (lv_color_from_string(a, &col, &opa)) {
        
        lv_obj_set_style_bg_color(ui_Screen1, col, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_Screen2, col, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_Screen3, col, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(ui_Screen4, col, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_border_color(ui_Panel1, col, LV_PART_MAIN | LV_STATE_DEFAULT);// panel 
        lv_obj_set_style_border_color(ui_Panel2, col, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(ui_Panel3, col, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(ui_Panel4, col, LV_PART_MAIN | LV_STATE_DEFAULT);

        }

   }

}


void intToString(int num, char *str) {
    int i = 0;
    //sayı sıfırsa
    if(num == 0) {
        str[i++] = '0';
        str[i] = '\0';
    } else {
      // sayı sıfırdan farklı ise sayıyı 10 a bölerek sırayla diziye ekleriz ama bu işlem kalana göre yapılır o yüzden sayı ters çevrilir
        while (num != 0) {
            int rem = num % 10;
            str[i++] = rem + '0';//0 ın ascii değeri üzerine diğer sayıların değerini ekleriz
            num /= 10;
        }
        str[i] = '\0';// sonuna string bitiş değişkeni
        
       
        int len = i;
        // dizinin yarısına kadar gider ve yansıtır diziyi
        for (int j = 0; j < len / 2; j++) {
            char temp = str[j];
            str[j] = str[len - j - 1];
            str[len - j - 1] = temp;
        }
    }
}

void intToStringNegative(int value, char* out) {
  char buf[16];
  int i = 0;
  bool isNeg = (value < 0);

  unsigned int v = isNeg ? (unsigned int)(-value) : (unsigned int)value;

  // rakamları ters sırada yaz
  do {
    buf[i++] = (char)('0' + (v % 10));
    v /= 10;
  } while (v > 0 && i < (int)sizeof(buf) - 1);

  if (isNeg && i < (int)sizeof(buf) - 1) {
    buf[i++] = '-';
  }

  // ters çevir ve out'a kopyala
  int j = 0;
  while (i > 0) {
    out[j++] = buf[--i];
  }
  out[j] = '\0'; // null terminator
}

#if (FRACTION_DIGIT == 1)

void doubleToString(double num, char *str) {
    int intPart = (int)num; // int kısım
    double fracPart = num - intPart; //ondalık  kısım

    
    intToString(intPart, str);

   
    while (*str) // stringin sonuna gelene kadar ilerle . koy sonuna
        str++; 
    *str++ = '.'; // Noktayı ekle

    // Ondalık kısmı stringe dönüştürür
    int frac = (int)(fracPart * 10); // Ondalık kısmı 10'a çarpıp integer'a çevir
    intToString(frac, str); 
}

#elif (FRACTION_DIGIT == 2)

void doubleToString(double num, char *str) {
    // Sayıyı 2 ondalık basamaklı tam sayıya çevir: örn. 1.256 → 125, 1.254 → 125
    int scaled = (int)(num * 100); // floor gibi davranır (truncation)

    int intPart = scaled / 100;
    int fracPart = scaled % 100;

    intToString(intPart, str);

    while (*str) str++;

    *str++ = '.';

    // Eğer fracPart 1 basamaklıysa başına 0 ekle: örn. 1.20 → "20", 1.02 → "02"
    if (fracPart < 10) {
        *str++ = '0';
    }

    intToString(fracPart, str);
}
#endif


void appendString(char *dest, const char *src) {
  // hedef dosyanın adresine giderek adresinin sonuna kadar gider eklenecek dizi o adrese eklenir sondan
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
}
void change_screen1_2()
{
    _ui_screen_delete(&ui_Screen1);
    _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);  
}
void change_screen1_3()
{
   _ui_screen_delete(&ui_Screen1);
   _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen3_screen_init); 
}
void change_screen1_4 ()
{
    _ui_screen_delete(&ui_Screen1);
    _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen4_screen_init); 

}    
void change_screen2_1 ()
{
     _ui_screen_delete(&ui_Screen2);
     _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen1_screen_init);  
}
void change_screen2_3 ()
{
     _ui_screen_delete(&ui_Screen2);
     _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen3_screen_init);  
}
void change_screen2_4 ()
{
     _ui_screen_delete(&ui_Screen2);
     _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen4_screen_init);  
}
void change_screen3_1 ()
{
     _ui_screen_delete(&ui_Screen3);
     _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen1_screen_init);  
}
void change_screen3_2 ()
{
     _ui_screen_delete(&ui_Screen3);
     _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);  
}
void change_screen3_4 ()
{
     _ui_screen_delete(&ui_Screen3);
     _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen4_screen_init);  
}
void change_screen4_1 ()
{
     _ui_screen_delete(&ui_Screen4);
     _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen1_screen_init);  
}
void change_screen4_2 ()
{
     _ui_screen_delete(&ui_Screen4);
     _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen2_screen_init);  
}
void change_screen4_3 ()
{
     _ui_screen_delete(&ui_Screen4);
     _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen3_screen_init);  
}

void change_screen_Errorpage ()
{
    _ui_screen_delete(&ui_Screen1);
    _ui_screen_delete(&ui_Screen2);
    _ui_screen_delete(&ui_Screen3);
    _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Screen4_screen_init);  
}


#if (FRACTION_DIGIT == 1)

void write_Screen_Power(double uiPower)
{

   uiPower=uiPower/10;

     char uiStringPower1[64] = "";
     char uiStringPower2[64] = ""; 
    
    doubleToString(uiPower, uiStringPower2); 
    
    char *ptr = uiStringPower1;
    appendString(ptr,"GÜÇ:");
    appendString(ptr, uiStringPower2);// appendstring "ekleme fonksiyonu yukarda yapılmıştır anlaşılır biçimde de anlatılmıştır printf in versiyonlarının kullanımı sıkıntıya yol açabilir dikkat edilmelidir appendstring  fonksiyonu bunun için yapıldı
    appendString(ptr, "kW");
    
     lv_label_set_text(ui_Label42,uiStringPower1);
     WrapperMqttSetPRMS(uiStringPower1); 
    
}


#elif (FRACTION_DIGIT == 2)

void write_Screen_Power(double uiPower)
{

   uiPower=uiPower/100;

     char uiStringPower1[64] = "";
     char uiStringPower2[64] = ""; 
    
    doubleToString(uiPower, uiStringPower2); 
    
    char *ptr = uiStringPower1;
    appendString(ptr,"GÜÇ:");
    appendString(ptr, uiStringPower2);// appendstring "ekleme fonksiyonu yukarda yapılmıştır anlaşılır biçimde de anlatılmıştır printf in versiyonlarının kullanımı sıkıntıya yol açabilir dikkat edilmelidir appendstring  fonksiyonu bunun için yapıldı
    appendString(ptr, "kW");
    
     lv_label_set_text(ui_Label42,uiStringPower1);//ekrana yazma 
     WrapperMqttSetPRMS(uiStringPower1);  // broker a yazma 
     
 
    
}

#endif


void xwrite_Screen_Power(double uiPower)
{
#if (FRACTION_DIGIT == 1)
    uiPower = uiPower / 10.0;
#elif (FRACTION_DIGIT == 2)
    uiPower = uiPower / 100.0;
#endif

    char uiStringPower1[32];  // güvenli boyut

    // fractional digit'e göre format
#if (FRACTION_DIGIT == 1)
    snprintf(uiStringPower1, sizeof(uiStringPower1), "GÜÇ:%.1f kW", uiPower);
#else
    snprintf(uiStringPower1, sizeof(uiStringPower1), "GÜÇ:%.2f kW", uiPower);
#endif

    lv_label_set_text(ui_Label42, uiStringPower1);
    WrapperMqttSetPRMS(uiStringPower1);
}

void xwrite_Screen_Energy(double uiEnergy)
{
    char uiStringEnergy1[32];   // biraz büyük olmalı

    uiEnergy = uiEnergy / 100;
    snprintf(uiStringEnergy1, sizeof(uiStringEnergy1), "ENERJİ:%.2f kWh", uiEnergy);

    lv_label_set_text(ui_Label43, uiStringEnergy1);
    WrapperMqttSetETOTAL(uiStringEnergy1);
}


void write_Screen_Energy(double uiEnergy)
{   
    char uiStringEnergy1[64] = ""; 
    char uiStringEnerg2[64] = "";
    uiEnergy = uiEnergy / 100; 
    doubleToString(uiEnergy, uiStringEnerg2); 
    char *ptr = uiStringEnergy1;
    appendString(ptr,"ENERJİ:");
    appendString(ptr, uiStringEnerg2);
    appendString(ptr, "kWh");
     lv_label_set_text(ui_Label43, uiStringEnergy1);//ekrana yazma
    
    WrapperMqttSetETOTAL(uiStringEnergy1); // broker a yaz 
  
}

void xwrite_Screen_Chargetime(int uiTimeh, int uiTimem)
{
    char uiStringTime[32];

    snprintf(uiStringTime, sizeof(uiStringTime), "ZAMAN:%02d.%02d", uiTimeh, uiTimem);

    lv_label_set_text(ui_Label44, uiStringTime);
    WrapperMqttSetChargeingTime(uiStringTime);
}


void write_Screen_Chargetime(int uiTimeh, int uiTimem) {

    
    char uiStringTime[64] = ""; // Diziyi saklamak için alan
    
    char timeh_str[3] = ""; // Saat için dize
    char timem_str[3] = ""; // Dakika için dize
    
    // Saat ve dakikayı dizilere çevirme
    intToString(uiTimeh, timeh_str);
    intToString(uiTimem, timem_str);
    
    // "ZAMAN:0" sabit dizisini, saat dizisini, "." ve dakika dizisini birleştirme
    char *ptr = uiStringTime;
    appendString(ptr, "  ZAMAN:");
    appendString(ptr, timeh_str);
    appendString(ptr, ".");
    appendString(ptr, timem_str);
    
    // Elde edilen sonucu ekrana yazdırma
    lv_label_set_text(ui_Label44, uiStringTime);
     //  zanmanı broker a yazdıh
    WrapperMqttSetChargeingTime(uiStringTime); 
   
    
    // Dakikaları ve saniyeleri kontrol eden ekstra kod parçaları buraya eklenebilir
    // Örneğin, uiTimem ve uiTimes değerlerine göre farklı formatlamalar yapılabilir
}


void write_Screen_message(const char *Message)
{

    lv_label_set_text(ui_Label62,Message);
    lv_label_set_text(ui_Label69,"");

}

void write_Screen_Errormessage(const char *Error)
{
     change_screen_Errorpage ();
    lv_label_set_text(ui_Label62,Error);
    lv_label_set_text(ui_Label69,"");
}
    
void write_Screen_Systeminitialization() {
  
    lv_label_set_text(ui_Label62,"SİSTEM YÜKLENİYOR...");
    mqtt_screen_update();

}

void write_Screen_RfidUpdate(const char* msg) {

    
    lv_label_set_text(ui_Label63, msg);
 
}


void write_Screen_Soketcikart(){

    lv_label_set_text(ui_Label62,"LÜTFEN SOKETİ ÇIKARTINIZ");
    lv_label_set_text(ui_Label69, "");


  
}
void write_Screen_Sarjsonaerdi(){
 const char init [30]="SARJ SONA ER";
   
    lv_label_set_text(ui_Label62,"ŞARJ SONA ERDİ");
    lv_label_set_text(ui_Label69, "");

  
}
void write_Screen_rfidcevap(){

    lv_label_set_text(ui_Label22,"Okuma işlemi başarılı Cevap Bekleniyor");
}


void write_message_P3_SCAN_RFID(const char *rfidString){
  lv_label_set_text(ui_Label22, rfidString);
}

static inline void Screen_Signal_level(int siglevel) {
  

    if (netType==NET_TYPE_GSM||(netType==NET_TYPE_ALL&&comsupportedNetwork == NET_TYPE_GSM)) {
      int dbm = csqToDbm(siglevel);

      char bufDbm[16];
      intToStringNegative(dbm, bufDbm);
      lv_label_set_text(ui_Label8, bufDbm);

      // appendString(dstPtr, ...) mantığına göre ptr'yi sonuna konumlayalım
      char* ptr = bufDbm + strlen(bufDbm);
      appendString(ptr, "dBm");

      // MQTT'ye tüm stringi gönder; (ptr sondan başlıyorsa boş gider)
      WrapperMqttSetGSM_SIGNAL(bufDbm);

      const char* lvl = rssiLevelLabelFromDbmGSM(dbm);
      lv_label_set_text(ui_Label7, lvl);
      lv_obj_set_style_text_color(ui_Label7, levelColorFromDbm(dbm),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
     
    }

    if (netType==NET_TYPE_WIFI||(netType==NET_TYPE_ALL&&comsupportedNetwork == NET_TYPE_WIFI)) {
      char bufDbmW[16];
      intToStringNegative(siglevel, bufDbmW);
      lv_label_set_text(ui_Label8, bufDbmW);

      char* ptrw = bufDbmW + strlen(bufDbmW);
      appendString(ptrw, "dBm");
      WrapperMqttSetGSM_SIGNAL(bufDbmW); // Wi-Fi için de aynı string gidiyor

      const char* lvl1 = rssiLevelLabelFromDbmWifi(siglevel);
      lv_label_set_text(ui_Label7, lvl1);
      lv_obj_set_style_text_color(ui_Label7, levelColorFromDbm(siglevel),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    }

  

   
}
void initScreenNetIcon(int internet){
    if (internet == 1){

    switch (netType) {
        case NET_TYPE_ALL :{ 
            switch (comsupportedNetwork){
                case NET_TYPE_GSM:
                    csq = getCsqOnce();
                Screen_Signal_level(csq); 
                break;
                case NET_TYPE_WIFI:
                    dbm =getDbmOnce();
                    Screen_Signal_level(dbm);  
                break;
                case NET_TYPE_ETH:
                    screen_eth_upload(); 
                break;
            }
            break;  
        }

        case NET_TYPE_GSM:  
            csq = getCsqOnce();
            Screen_Signal_level(csq);  
            break;
        
        case NET_TYPE_WIFI:    
            dbm =getDbmOnce();
            Screen_Signal_level(dbm);  
            break;
        
        case NET_TYPE_ETH: 
            screen_eth_upload(); 
            break;
        default: break;
        }
    }else{ 
    eth_icon_close();
    wifi_icon_close();
    GSM_icon_close();
    lv_label_set_text(ui_Label7, "");
    lv_label_set_text(ui_Label8, "");
    lv_label_set_text(ui_Label9, "");
    }
}



void write_Screen_Uploading_Start() {
    lv_label_set_text(ui_Label62,"  PREPARING UPDATE...");
    lv_label_set_text(ui_Label69, "");

    }
static int abdulkorumacount = 0;
void write_Screen_DownloadStatus(size_t downloaded, size_t total, int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    char uiStringstatus[90] = "";

    double downloadedMB = (double)downloaded / 1000000.0;
    double totalMB = (double)total / 1000000.0;

    appendString(uiStringstatus, "Downloaded ");

    char buf1[16];
    doubleToString(downloadedMB, buf1);
    appendString(uiStringstatus, buf1);
    appendString(uiStringstatus, " MB / ");

    char buf2[16];
    doubleToString(totalMB, buf2);
    appendString(uiStringstatus, buf2);
    appendString(uiStringstatus, " MB");

    lv_label_set_text(ui_Label67, uiStringstatus);

    char percentText[12];
    snprintf(percentText, sizeof(percentText), "%d%%", percent);
    lv_label_set_text(ui_Label65, percentText);

    lv_obj_clear_flag(ui_Slider31, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Arc31, LV_OBJ_FLAG_HIDDEN);

    lv_arc_set_value(ui_Arc31, percent);

    if (percent >= 100) {
        lv_label_set_text(ui_Label68, "Software Writing...");
        lv_slider_set_value(ui_Slider31, 100, LV_ANIM_OFF);
    } else if (percent >= 99) {
        lv_label_set_text(ui_Label68, "Software Writing...");
        lv_slider_set_value(ui_Slider31, 50, LV_ANIM_OFF);
    } else {
        lv_slider_set_value(ui_Slider31, 0, LV_ANIM_OFF);
    }

    if (abdulkorumacount == 0) {
        lv_label_set_text(ui_Label64, "FIRMWARE UPDATE");
        lv_label_set_text(ui_Label68, "Waiting for Writing...");
        char verText[32] = "Version ";
        appendString(verText, WrapperMqttGetOTA_VER());
        lv_label_set_text(ui_Label66, verText);

        lv_label_set_text(ui_Label62, "");
        lv_label_set_text(ui_Label69, "");

        abdulkorumacount++;
    }
}
static int abdulkorumacount2 = 0;

void write_Screen_DownloadStatusmsp(size_t downloaded, size_t total, int percent) {
    char uiStringstatus[90] = "";

    double downloadedKB = downloaded / 1000.0;
    double totalKB      = total / 1000.0;

    appendString(uiStringstatus, " Downloaded ");

    char buf1[16];
    doubleToString(downloadedKB, buf1);
    appendString(uiStringstatus, buf1);
    appendString(uiStringstatus, " KB / ");

    char buf2[16];
    doubleToString(totalKB, buf2);
    appendString(uiStringstatus, buf2);
    appendString(uiStringstatus, " KB");

    lv_label_set_text(ui_Label67, uiStringstatus);

    char buf3[8];
    intToString(percent, buf3);

    char percentText[12] = "";
    appendString(percentText, buf3);
    appendString(percentText, "%");

    lv_label_set_text(ui_Label65, percentText);

    // Yüzde değerini sınırla
    if (percent < 0) {
        percent = 0;
    } else if (percent > 100) {
        percent = 100;
    }

    lv_obj_clear_flag(ui_Slider31, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(ui_Arc31, LV_OBJ_FLAG_HIDDEN);

    lv_arc_set_value(ui_Arc31, percent);

    // Slider sıfırda kalacaksa:
    lv_slider_set_value(ui_Slider31, 0, LV_ANIM_OFF);

    if (abdulkorumacount2 == 0) {
        lv_label_set_text(ui_Label64, "FIRMWARE UPDATE 2");
        lv_label_set_text(ui_Label68, "");

        char verText[32] = "Version ";
        appendString(verText, WrapperMqttGetMSP_SW_VER() );
        
        lv_label_set_text(ui_Label66, verText);
        lv_label_set_text(ui_Label62, "");
        lv_label_set_text(ui_Label69, "");

        abdulkorumacount2++;
    }
}

void write_Screen_WritingStatus(int percent) {

    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    lv_label_set_text(ui_Label68, "Software writing...");

    _ui_slider_set_property(ui_Slider31, _UI_SLIDER_PROPERTY_VALUE_WITH_ANIM, percent);
}

  void  qrCode_id(char* ui_Qr)
{   
     char uiString[50]="";
    sprintf(uiString,"%s",ui_Qr);
    lv_obj_t * qrCode;
    qrCode = lv_qrcode_create(ui_Screen2, 200, lv_color_hex3(0x000), lv_color_hex3(0xeef));
    lv_obj_set_pos(qrCode, 150, 3);
    lv_obj_set_align(qrCode, LV_ALIGN_CENTER);
    lv_qrcode_update(qrCode, uiString, strlen(uiString));
    lv_obj_clear_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
}


typedef struct {
    const char* message;
    unsigned long timestamp;
    unsigned long duration;
} ScreenMessage;

#define MAX_SCREEN_MESSAGES 2
ScreenMessage screenMessages[MAX_SCREEN_MESSAGES];

void addScreenMessage(const char* msg, unsigned long duration) {
    for (int i = 0; i < MAX_SCREEN_MESSAGES; i++) {
        if (screenMessages[i].message == NULL) {
            screenMessages[i].message = msg;
            screenMessages[i].timestamp = millis();
            screenMessages[i].duration = duration;
            break;
        }
    }
}

void updateScreenMessages() {
    unsigned long currentTime = millis();

    for (int i = 0; i < MAX_SCREEN_MESSAGES; i++) {
        if (screenMessages[i].message != NULL && currentTime - screenMessages[i].timestamp > screenMessages[i].duration) {
            screenMessages[i].message = NULL;
            write_Screen_RfidUpdate("");
        }
    }

    for (int i = 0; i < MAX_SCREEN_MESSAGES; i++) {
        if (screenMessages[i].message != NULL) {
            write_Screen_RfidUpdate(screenMessages[i].message);
        }
    }
}


#endif
