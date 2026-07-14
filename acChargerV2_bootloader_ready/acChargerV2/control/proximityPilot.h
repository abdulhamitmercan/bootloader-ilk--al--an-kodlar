 #ifndef PROXIMITYPILOT_H_
#define PROXIMITYPILOT_H_

#include "IQmathLib.h"
#include "utils/utils.h"
#define DUTY_20A                                    ((unsigned int)(33))
#define DUTY_32A                                    ((unsigned int)(53))
#define DUTY_63A                                    ((unsigned int)(89))
#define DUTY_ERR                                    ((unsigned int)(0))

#define isCable20A(i)                               (i>=_IQ(0.429) && i<_IQ(0.456))
#define isCable32A(i)                               (i>=_IQ(0.189) && i<_IQ(0.206))
#define isCable60A(i)                               (i>=_IQ(0.095) && i<_IQ(0.105))


#define VOLTAGE_1PH                                 (float)(0.23)
#define PHASE_NUMBER                                (float)(3.0)
#define MAX_CURRENT_CONSTANT                        _IQdiv(_IQ(1.0), (_IQmpy(_IQ(VOLTAGE_1PH), _IQ(PHASE_NUMBER))))

#define DUTY_RATIO_CONSTANT                         _IQdiv(_IQ(1.0), _IQ(0.6))

uint16_t maxPowerReq = 22;

typedef struct {
    t_mvngAvg avg;
    _iq qout;
    float out;
    unsigned int dutyRatio;
    unsigned int maxCurrent;
} t_ppVoltage;

t_ppVoltage ppLevel;

void pp_VoltageLevel(_iq voltage){
    ppLevel.avg.qoutVal = dsp_MovingAvg(ppLevel.avg.buffer, &ppLevel.avg.sum, ppLevel.avg.pos, LENGTH_OF_MOVIG_AVG, voltage);
    ppLevel.qout = ppLevel.avg.qoutVal;

    ppLevel.out = _IQtoF(ppLevel.qout);
    ppLevel.avg.pos++;
    if (ppLevel.avg.pos >= LENGTH_OF_MOVIG_AVG){
        ppLevel.avg.pos = 0;
    }

}

void pp_SetMaxCurrent(_iq voltage){
    static unsigned int dutyCycle = 0;
    static unsigned int dutyCycleOld = 0;

    if(isCable20A(voltage)){
        dutyCycle = DUTY_20A;
        ppLevel.maxCurrent = 2200;
    }
    else if(isCable32A(voltage)){
        dutyCycle = DUTY_32A;
        ppLevel.maxCurrent = 3520;
    }
    else if(isCable60A(voltage)){
        dutyCycle = DUTY_63A;
        ppLevel.maxCurrent = 6930;
    }
    else
    {
        dutyCycle = DUTY_32A;
        dutyCycle = dutyCycleOld;
    }

    dutyCycleOld = dutyCycle;
    ppLevel.dutyRatio = dutyCycle;
}

_iq maxPower2DutyRatio(_iq maxPower, _iq vrms){
    //_iq maxCurrent = _IQmpy(maxPower, MAX_CURRENT_CONSTANT);
    _iq maxCurrent = _IQmpy(maxPower, _IQdiv(_IQ(1.0), (_IQmpy(vrms, _IQ(PHASE_NUMBER)))));
    return _IQmpy(maxCurrent, DUTY_RATIO_CONSTANT);
}

void pp_SetMaxCurrent2(_iq voltage, uint16_t dutyReq){
    static unsigned int dutyCycle = 33;
    static unsigned int dutyCycleOld = 33;

    if(isCable20A(voltage)){
        dutyCycle = DUTY_20A;
        ppLevel.maxCurrent = 2200;
    }
    else if(isCable32A(voltage)){
        dutyCycle = DUTY_32A;
        ppLevel.maxCurrent = 3520;
    }
    else if(isCable60A(voltage)){
        dutyCycle = DUTY_63A;
        ppLevel.maxCurrent = 6930;
    }
    else
    {
        dutyCycle = dutyCycleOld;
    }


    dutyCycleOld = dutyCycle;

    if(dutyCycle < dutyReq){
        ppLevel.dutyRatio = dutyCycle;
    }
    else{
        ppLevel.dutyRatio = dutyReq;
    }

}


void pp_NoCable(int dutyRatio){
    ppLevel.dutyRatio = dutyRatio;
}


void pp_Handle(_iq voltage, uint16_t dutyReq){
    pp_VoltageLevel(voltage);
    //pp_SetMaxCurrent2(ppLevel.qout, dutyReq);

    if(charger.model == AC_EVSEModel_Cable){
        pp_NoCable(dutyReq);
    }
    else{
         pp_SetMaxCurrent2(ppLevel.qout, dutyReq);
       // pp_SetMaxCurrent(ppLevel.qout);
    }
}


#endif /* PROXIMITYPILOT_H_ */
