#ifndef DSPLIB_H_
#define DSPLIB_H_

#include "IQmathLib.h"



#define SAMPLING_FREQ_HZ                                             ((unsigned int)(2500))
#define ONE_OVER_SQRT_TWO                                            (_IQ(0.70710678118)) // 1/(sqrt(2))

#define LENGTH_OF_MOVIG_AVG                                          ((unsigned int)(64))
#define DELTA_T_MOVING_AVG                                           ((float)(1.0/LENGTH_OF_MOVIG_AVG))
#define qDELTA_T_MOVING_AVG                                          (_IQ(DELTA_T_MOVING_AVG))

typedef struct {
    _iq inVal;
    _iq outVal;
    _iq sum;

    _iq maxVrms;
} t_rmsParams;

typedef struct {
    _iq buffer[LENGTH_OF_MOVIG_AVG];
    _iq sum;
    _iq qoutVal;
    unsigned int pos;
} t_mvngAvg;


void dsp_resetMvnAvgParams(t_mvngAvg *p){
    int i = 0;
    for(i=0; i<LENGTH_OF_MOVIG_AVG; i++){
        p->buffer[i] = _IQ(0.0);
    }
    p->sum = _IQ(0.0);
    p->qoutVal = _IQ(0.0);
    p->pos = 0;
}

_iq dsp_MovingAvg(_iq *ptrArrNumbers, _iq *ptrSum, unsigned int pos, unsigned int len, _iq nextNum)
{
  //Subtract the oldest number from the prev sum, add the new number
  *ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;
  //Assign the nextNum to the position in the array
  ptrArrNumbers[pos] = nextNum;
  //return the average
  return _IQmpy(*ptrSum, qDELTA_T_MOVING_AVG);
}


static inline _iq dsp_CalculateActiveRms(t_rmsParams *p, _iq deltaT, unsigned int counter){
    _iq product = _IQ(0.0);

    product = _IQmpy(p->inVal, p->inVal);
    product = _IQmpy(product, _IQ(0.02));
    if(product > _IQ(0.0)){
        p->sum = p->sum + product;
    }
    if(counter == 0){
        //p->outVal = _IQsqrt(_IQmpy(p->sum, deltaT));
        //p->outVal = _IQsqrt(_IQmpy(p->sum, _IQ(0.01)));
        //p->outVal = p->sum;
        p->outVal = _IQsqrt(p->sum);
        p->sum = _IQ(0.0);
    }

    return p->outVal;
}


#endif /* DSPLIB_H_ */
