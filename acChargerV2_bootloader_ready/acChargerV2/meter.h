#ifndef METER_H_
#define METER_H_


#include "measurements.h"
#include <dspLib.h>
#include "IQmathLib.h"


// EMETER REGISTERS
#define REG_16BIT(address)  volatile unsigned int address

//-- Registers
REG_16BIT(ZCSTAT);              //-- ZCSTAT zero cross status register

/* ZCSTAT Control Bits */
#define ZCOUT                      (0x0001) /* ZC Sign, signal is pozitive or negative */
#define ZCSTART                    (0x0002) /* ZC start point */

typedef struct {
    unsigned short zcPoint; // zero cross
    unsigned int freqCount;
    unsigned int numOfSample;
} t_zcParams;


typedef struct {
    _iq energy;
    _iq power;
} t_meter;


typedef struct {
    float P;
    float E;
} t_meterF;

t_meter meter;
t_meterF meterF;

static inline unsigned short dsp_ZeroCrossDetector(_iq Vin){
    unsigned short sign = 0;
    static unsigned short status = 0;

    sign = ((Vin>_IQ(0.0)) ? 1 : 0);
    if(sign == 1){
        //HAL_SET_GPIO_LEDRED();
        ZCSTAT |= ZCOUT;
    }
    else{
        //HAL_RESET_GPIO_LEDRED();
        ZCSTAT &= ~ZCOUT;
        ZCSTAT &= ~ZCSTART;
    }

    if((ZCSTAT&ZCOUT) && ((ZCSTAT&ZCSTART)>>1 == 0)){
        // start of calculation
        ZCSTAT |= ZCSTART;
        status = 1;
    }
    else{
        status = 0;
    }
    return status;
}

static inline _iq measurements_calculatePower(t_rmsParams *Vin, t_rmsParams *Iin, _iq deltaT, unsigned int idx){
    _iq Vrms[3], Irms[3], Prms[3];
    _iq result = _IQ(0.0);

    Vrms[0] = dsp_CalculateActiveRms((Vin+0), deltaT, idx);
    (Vin+0)->maxVrms = Vrms[0];
    Vrms[1] = dsp_CalculateActiveRms((Vin+1), deltaT, idx);
    Vrms[2] = dsp_CalculateActiveRms((Vin+2), deltaT, idx);

    Irms[0] = dsp_CalculateActiveRms((Iin+0), deltaT, idx);
    Irms[1] = dsp_CalculateActiveRms((Iin+1), deltaT, idx);
    Irms[2] = dsp_CalculateActiveRms((Iin+2), deltaT, idx);

    Prms[0] = _IQmpy(Vrms[0], Irms[0]);
    Prms[1] = _IQmpy(Vrms[1], Irms[1]);
    Prms[2] = _IQmpy(Vrms[2], Irms[2]);

    result = Prms[0] + Prms[1] + Prms[2];
    return result;
}

static inline void measurements_calculateEnergy(_iq *energy, _iq power, _iq timeInterval){
    *energy = *energy + _IQmpy(power, timeInterval);
}

static inline void energyMeter_calculateEnergy(_iq *energy, float meterStop, float meterStart){
    *energy = _IQ(meterStop) - _IQ(meterStart);
}

static inline void energyMeter_calculateEnergyF(float *energy, float meterStop, float meterStart){
    *energy = meterStop - meterStart;
}

#endif /* METER_H_ */
