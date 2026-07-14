#ifndef MEASUREMENTS_H_
#define MEASUREMENTS_H_

/* Select the global Q value and include the Qmath header file. */

#include "IQmathLib.h"

//----------------------- ADC and SD24 ==> VREF, MAX values   --------------------------------------------------------
#define OFFSET_VAL                                                       ((unsigned int)(12))
#define qVREF_ADC                                                        (_IQ(2.5))
#define qADC_MAX                                                         (_IQ(1023.0))
#define qADC_MAX_INV                                                     (_IQ(0.000978)) // 1/1023
#define qVREF_SD24                                                       (_IQ(1.17))
#define qSD24_MAX                                                        (_IQ(4096.0))
#define qSD24_MAX_INV                                                    (_IQ(0.000248200244))

//----------------------- ADC CHANNELS DIVIDER AND OFFSET VALUES -----------------------------------------------------
#define qDIVIDER_MCU_L1_Relay_Vout                                       (_IQ(471.0)) // A0
#define qDIVIDER_RATE_AIN_CP                                             (_IQ(1.0)) // A1
#define qDIVIDER_RATE_PP_ADC                                             (_IQ(1.0)) // A2
#define qDIVIDER_RATE_EMPTY                                              (_IQ(1.0)) // A3
#define qDIVIDER_MCU_L2_Relay_Vout                                       (_IQ(471.0)) // A4
#define qDIVIDER_MCU_L3_Relay_Vout                                       (_IQ(471.0)) // A5


#define qOFFSET_MCU_L1_Relay_Vout                                        (_IQ(0.0))
#define qOFFSET_AIN_CP                                                   (_IQ(0.0))
#define qOFFSET_PP_ADC                                                   (_IQ(0.0))
#define qOFFSET_EMPTY                                                    (_IQ(0.0))
#define qOFFSET_MCU_L2_Relay_Vout                                        (_IQ(0.0))
#define qOFFSET_MCU_L3_Relay_Vout                                        (_IQ(0.0))

//----------------------- SD24 CHANNELS DIVIDER AND OFFSET VALUES -----------------------------------------------------

#define qDIVIDER_RATE_IL1                                               (_IQ(-4.42477876106))
#define qDIVIDER_RATE_IL2                                               (_IQ(-4.42477876106))
#define qDIVIDER_RATE_IL3                                               (_IQ(-4.42477876106))
#define qDIVIDER_RATE_VL1                                               (_IQ(8.877924528302))
#define qDIVIDER_RATE_VL2                                               (_IQ(8.877924528302))
#define qDIVIDER_RATE_VN                                                (_IQ(8.877924528302))
#define qDIVIDER_RATE_VL3                                               (_IQ(8.877924528302))

/*
// (V / 1.01) (I / 1.01)
#define qDIVIDER_RATE_IL1                                               (_IQ(-4.38096907036))
#define qDIVIDER_RATE_IL2                                               (_IQ(-4.38096907036))
#define qDIVIDER_RATE_IL3                                               (_IQ(-4.38096907036))
#define qDIVIDER_RATE_VL1                                               (_IQ(8.79002428545))
#define qDIVIDER_RATE_VL2                                               (_IQ(8.79002428545))
#define qDIVIDER_RATE_VN                                                (_IQ(8.79002428545))
#define qDIVIDER_RATE_VL3                                               (_IQ(8.79002428545))
*/
#define qOFFSET_IL1                                                     (_IQ(2047.0))
#define qOFFSET_IL2                                                     (_IQ(2047.0))
#define qOFFSET_IL3                                                     (_IQ(2047.0))
#define qOFFSET_VL1                                                     (_IQ(2047.0))
#define qOFFSET_VL2                                                     (_IQ(2047.0))
#define qOFFSET_VN                                                      (_IQ(2047.0))
#define qOFFSET_VL3                                                     (_IQ(2047.0))

typedef struct{
    float Vcp;
    float Vpp;
    float Vn;
    float VL[3];
    float IL[3];
    float VrelayOut[3];

} t_measurements;

typedef struct{
    _iq Vcp;
    _iq Vpp;
    _iq Vn;
    _iq VL[3];
    _iq IL[3];
    _iq VrelayOut[3];
} t_iq_measurements;

t_measurements emeter = {0};
t_iq_measurements qemeter;


void iq_GET_ADC_MEASUREMENT(_iq *qout, _iq qin, _iq qoffset, _iq qdr){
    _iq qtemp;

    qtemp = _IQmpy(qin, qVREF_ADC);
    qtemp = _IQmpy(qtemp, qADC_MAX_INV);
    qtemp = qtemp - qoffset;
    *qout = _IQmpy(qtemp, qdr);

}

void iq_GET_SD24_MEASUREMENT(_iq *qout, _iq qin, _iq qoffset, _iq qdr){
    _iq qtemp, qconstant;
    qtemp = qin - qoffset;
    qconstant = _IQmpy2(qVREF_SD24);
    qtemp = _IQmpy(qtemp, qconstant);
    qtemp = _IQmpy(qSD24_MAX_INV, qtemp);
    *qout = _IQmpy(qtemp, qdr);

}

static inline void mLib_getAdcVoltages(unsigned int adcVal[]){
    _iq qadc[6], qres[6];
    qadc[0] = _IQ((adcVal[0]));
    qadc[1] = _IQ((adcVal[1]));
    qadc[2] = _IQ((adcVal[2]));
    qadc[3] = _IQ((adcVal[3]));
    qadc[4] = _IQ((adcVal[4]));
    qadc[5] = _IQ((adcVal[5]));


    iq_GET_ADC_MEASUREMENT(&qres[0], qadc[5], qOFFSET_MCU_L1_Relay_Vout, qDIVIDER_MCU_L1_Relay_Vout);
    iq_GET_ADC_MEASUREMENT(&qres[1], qadc[4], qOFFSET_AIN_CP, qDIVIDER_RATE_AIN_CP);
    iq_GET_ADC_MEASUREMENT(&qres[2], qadc[3], qOFFSET_PP_ADC, qDIVIDER_RATE_PP_ADC);
    iq_GET_ADC_MEASUREMENT(&qres[3], qadc[2], qOFFSET_EMPTY, qDIVIDER_RATE_EMPTY);
    iq_GET_ADC_MEASUREMENT(&qres[4], qadc[1], qOFFSET_MCU_L2_Relay_Vout, qDIVIDER_MCU_L2_Relay_Vout);
    iq_GET_ADC_MEASUREMENT(&qres[5], qadc[0], qOFFSET_MCU_L3_Relay_Vout, qDIVIDER_MCU_L3_Relay_Vout);

    /*emeter.VrelayOut[0] = _IQtoF(qres[0]);
    emeter.Vcp =          _IQtoF(qres[1]);
    emeter.Vpp =          _IQtoF(qres[2]);
    emeter.VrelayOut[1] = _IQtoF(qres[4]);
    emeter.VrelayOut[2] = _IQtoF(qres[5]);*/

    qemeter.VrelayOut[0] = qres[0];
    qemeter.Vcp =          qres[1];
    qemeter.Vpp =          qres[2];
    qemeter.VrelayOut[1] = qres[4];
    qemeter.VrelayOut[2] = qres[5];


}


static inline void mlib_getSigmaDeltaValues(unsigned long* out){
    *(out+0) = SD24BMEMH0;
    *(out+0) = *(out+0)<<16 | SD24BMEML0;
    *(out+1) = SD24BMEMH1;
    *(out+1) = *(out+1)<<16 | SD24BMEML1;
    *(out+2) = SD24BMEMH2;
    *(out+2) = *(out+2)<<16 | SD24BMEML2;
    *(out+3) = SD24BMEMH3;
    *(out+3) = *(out+3)<<16 | SD24BMEML3;
    *(out+4) = SD24BMEMH4;
    *(out+4) = *(out+4)<<16 | SD24BMEML4;
    *(out+5) = SD24BMEMH5;
    *(out+5) = *(out+5)<<16 | SD24BMEML5;
    *(out+6) = SD24BMEMH6;
    *(out+6) = *(out+6)<<16 | SD24BMEML6;

}

static inline void mLib_getSigmaDeltaDVoltages(unsigned long* in){
    _iq qsd24[7], qres[7];
    unsigned int inShifted[7];

    inShifted[0] = *(in+0) >> OFFSET_VAL;
    inShifted[1] = *(in+1) >> OFFSET_VAL;
    inShifted[2] = *(in+2) >> OFFSET_VAL;
    inShifted[3] = *(in+3) >> OFFSET_VAL;
    inShifted[4] = *(in+4) >> OFFSET_VAL;
    inShifted[5] = *(in+5) >> OFFSET_VAL;
    inShifted[6] = *(in+6) >> OFFSET_VAL;

    qsd24[0] = _IQ(inShifted[0]);
    qsd24[1] = _IQ(inShifted[1]);
    qsd24[2] = _IQ(inShifted[2]);
    qsd24[3] = _IQ(inShifted[3]);
    qsd24[4] = _IQ(inShifted[4]);
    qsd24[5] = _IQ(inShifted[5]);
    qsd24[6] = _IQ(inShifted[6]);

    iq_GET_SD24_MEASUREMENT(&qres[0], qsd24[0], qOFFSET_IL1, qDIVIDER_RATE_IL1);
    iq_GET_SD24_MEASUREMENT(&qres[1], qsd24[1], qOFFSET_IL2, qDIVIDER_RATE_IL2);
    iq_GET_SD24_MEASUREMENT(&qres[2], qsd24[2], qOFFSET_IL3, qDIVIDER_RATE_IL3);
    iq_GET_SD24_MEASUREMENT(&qres[3], qsd24[3], qOFFSET_VL1, qDIVIDER_RATE_VL1);
    iq_GET_SD24_MEASUREMENT(&qres[4], qsd24[4], qOFFSET_VL2, qDIVIDER_RATE_VL2);
    iq_GET_SD24_MEASUREMENT(&qres[5], qsd24[5], qOFFSET_VN,  qDIVIDER_RATE_VN);
    iq_GET_SD24_MEASUREMENT(&qres[6], qsd24[6], qOFFSET_VL3, qDIVIDER_RATE_VL3);

    // calculate line to neutral voltages
    qres[3] = qres[3] - qres[5];
    qres[4] = qres[4] - qres[5];
    qres[6] = qres[6] - qres[5];
    //qres[5] = qres[5] - qres[5];

    /*emeter.IL[0] = _IQtoF(qres[0]);
    emeter.IL[1] = _IQtoF(qres[1]);
    emeter.IL[2] = _IQtoF(qres[2]);
    emeter.VL[0] = _IQtoF(qres[3]);
    emeter.VL[1] = _IQtoF(qres[4]);
    emeter.Vn =    _IQtoF(qres[5]);
    emeter.VL[2] = _IQtoF(qres[6]);*/

    qemeter.IL[0] = qres[0];
    qemeter.IL[1] = qres[1];
    qemeter.IL[2] = qres[2];
    qemeter.VL[0] = qres[3];
    qemeter.VL[1] = qres[4];
    qemeter.Vn =    qres[5];
    qemeter.VL[2] = qres[6];

}








#endif /* MEASUREMENTS_H_ */
