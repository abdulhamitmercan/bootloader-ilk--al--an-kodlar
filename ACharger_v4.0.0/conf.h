#ifndef CONF_H
#define CONF_H

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
    in->measuretype = AC_EVSEMeasuretype_Calculate;
    in->reset = AC_EVSEResetcontrol_Resetoff;
}

uint8_t setConfiguration(t_EVSE *in){
	uint8_t result = 0;

    result = (in->reset & 0x01)<<3 | (in->measuretype & 0x01)<<2
                         | (in->mode & 0x01)<<1 | (in->model & 0x01);

    return result;
}


#endif