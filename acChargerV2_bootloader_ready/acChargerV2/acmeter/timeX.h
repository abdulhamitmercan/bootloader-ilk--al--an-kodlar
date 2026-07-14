#ifndef TIMEX_H_
#define TIMEX_H_



#include "stdint.h"

unsigned int timerVal = 0;
static inline void is_1s_Occured(unsigned int* res, unsigned int timerFreqHz){


    timerVal++;
    if(timerVal > timerFreqHz){
        *res = 1;
        timerVal = 0;
    }

}



static inline unsigned int timeoutProcess(unsigned int startCounter, unsigned int timeoutms, unsigned int timerPeriodms){
    static int counter = 0;
    int result = 0;

    counter = (startCounter == 1 ? counter+1 : 0);

    int val = counter * timerPeriodms;
    if(val > timeoutms){
        // error condition -- timeout occured
        counter = 0;
        result |= 1;
    }

    // if timeout is tooooo large, reset the error
    if(val > timeoutms*10){
        result &= 0;
    }

    return result;
}



#endif /* TIMEX_H_ */
