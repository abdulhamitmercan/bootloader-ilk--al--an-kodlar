#ifndef UI_API_H
#define UI_API_H


#include <Arduino.h>


#include <Wire.h>
#include <SPI.h>
#include "uiScreen.h"
#include "ctrl_api.h"
#include "config.h"

#define USE_UI

#if defined USE_UI
#include <lvgl.h>
#include "ui.h"
#endif

#include <Arduino_GFX_Library.h>
#define TFT_BL 2
#define GFX_BL DF_GFX_BL
#define DEVICE_ID "onc1-1"
#define Display_50
#if defined (Display_50)         //5.0INCH 800x480 


Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
  40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
  45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
  5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
  8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);
Arduino_RPi_DPI_RGBPanel *lcd = new Arduino_RPi_DPI_RGBPanel(
  bus,
  800 /* width */, 0 /* hsync_polarity */, 210 /* hsync_front_porch */, 1 /* hsync_pulse_width */, 43 /* hsync_back_porch */,
  480 /* height */, 0 /* vsync_polarity */, 22 /* vsync_front_porch */, 1/* vsync_pulse_width */, 12 /* vsync_back_porch */,
  0 /* pclk_active_neg */, 7000000 /* prefer_speed */, true /* auto_flush */);

#endif

char device_Id[] = DEVICE_ID;
#ifdef USE_UI
/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
//static lv_color_t disp_draw_buf[800 * 480 / 10];   //notice here!!!  5,7inch: lv_color_t disp_draw_buf[800*480/10]            4.3inch: lv_color_t disp_draw_buf[480*272/10]
static lv_color_t buf1[800 * 480 / 10];
static lv_color_t buf2[800 * 480 / 10];
//lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 10);
//lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 10);
//static lv_color_t disp_draw_buf;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  lcd->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  lcd->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}







static inline void ui_init(char *qrCode){
  #if defined(Display_50) || defined(Display_70)
  //IO Port Pins
  //pinMode(38, OUTPUT);
  //digitalWrite(38, LOW);
  //pinMode(17, OUTPUT);
  //digitalWrite(17, LOW);
  //pinMode(18, OUTPUT);
  //digitalWrite(18, LOW);
  pinMode(42, OUTPUT);
  digitalWrite(42, LOW);
#elif defined(Display_43)
  pinMode(20, OUTPUT);
  digitalWrite(20, LOW);
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
  pinMode(35, OUTPUT);
  digitalWrite(35, LOW);
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  pinMode(0, OUTPUT);//TOUCH-CS
#endif

  // Init Display
  lcd->begin();
  lcd->fillScreen(BLACK);
  lcd->setTextSize(2);
  delay(200);

#ifdef USE_UI
  lv_init();

  delay(100);

  screenWidth = lcd->width();
  screenHeight = lcd->height();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 10);
  //lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / 10);
  //  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, 480 * 272 / 10);
  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  lv_indev_drv_register(&indev_drv);
#endif

#ifdef TFT_BL
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif

#ifdef USE_UI
  ui_init();//ui from Squareline or GUI Guider
  qrCode_id(qrCode);
  write_Screen_Systeminitialization();
  lv_timer_handler();
#else
  lcd->fillScreen(RED);
  delay(800);
  lcd->fillScreen(BLUE);
  delay(800);
  lcd->fillScreen(YELLOW);
  delay(800);
  lcd->fillScreen(GREEN);
  delay(800);
#endif
  //Serial.println( F("Setup done") );

}

static inline void ui_applyPageChanges(unsigned int *pageTransition){
  switch(*pageTransition){

      case P2_to_P3:
#if (RUN_MODE == 2 || RUN_MODE == 1)
        change_screen1_2();
#else
        change_screen1_3();    
#endif
        break;

      case P2_to_P4:
        change_screen1_3();
        break;

      case P3_to_P2:
        change_screen2_1();
        break;

      case P3_to_P4:
        change_screen2_3();
        break;

      case P4_to_P5:
        change_screen3_4();
        break;

      case P5_to_P2:
        change_screen4_1();
        break; 

      case P2_to_P6:
        change_screen1_4();
        break;

      case P3_to_P6:
        change_screen2_4();
        break;

      case P4_to_P6:
        change_screen3_4();
        break;

      case P6_to_P2:
        change_screen4_1();
        break;
    }
}



void write_message_P4_CHARGING(double energy, double power, int hours, int minute){
  write_Screen_Energy(energy);
  write_Screen_Power(power);
  write_Screen_Chargetime(hours, minute);
}

void write_message_P5_CHARGE_ENDED(uint16_t timeout){
  if (timeout){
    write_Screen_Soketcikart();
  }
  else{
    write_Screen_Sarjsonaerdi();
  }
}

void write_message_P6_ERR(){
  const char* messageErr = "HATA";
  write_Screen_message(messageErr);
}

void write_message_RFID_MANAGE(const char* msg){
  write_Screen_message(msg);
}

#endif


#endif