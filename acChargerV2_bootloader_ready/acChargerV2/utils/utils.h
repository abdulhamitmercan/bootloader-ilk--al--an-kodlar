#ifndef UTILS_H_
#define UTILS_H_


typedef enum {
    AC_EVSEModel_NoCable = 0,
    AC_EVSEModel_Cable = 1,
} AC_EVSE_Model;

typedef enum {
    AC_EVSEMode_Product = 0,
    AC_EVSEMode_Test = 1,
} AC_EVSE_Mode;

typedef enum {
    AC_EVSEMeasuretype_Calculate = 0,
    AC_EVSEMeasuretype_Acmeter = 1,
} AC_EVSE_Measuretype;

typedef enum {
    AC_EVSEResetcontrol_Resetoff = 0,
    AC_EVSEResetcontrol_Reseton = 1,
} AC_EVSE_Resettype;


typedef struct{
    uint8_t model;
    uint8_t mode;
    uint8_t measuretype;
    uint8_t reset;
} t_EVSE;

t_EVSE charger;

void initCharger(t_EVSE *in){
    in->model = AC_EVSEModel_NoCable;
    in->mode = AC_EVSEMode_Product;
    in->measuretype = AC_EVSEMeasuretype_Acmeter;
    in->reset = AC_EVSEResetcontrol_Resetoff;
}

void handleConfiguration(t_EVSE *in, uint8_t cmd){
    in->model = (cmd) & 0x01;
    in->mode = (cmd >> 1) & 0x01;
    in->measuretype = (cmd >> 2) & 0x01;
    in->reset = (cmd >> 3) & 0x01;
}

#endif
