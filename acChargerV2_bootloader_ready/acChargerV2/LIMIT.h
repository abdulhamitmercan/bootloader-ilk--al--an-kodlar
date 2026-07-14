#ifndef LIMIT_H_
#define LIMIT_H_

#include <msp430.h>
#include "stdint.h"
#include "IQmathLib.h"

#define MEDIAN_SIZE                     (uint16_t)(128)

#define MAX_LINE1_VOLTAGE               (uint32_t)(2490)
#define MAX_LINE2_VOLTAGE               (uint32_t)(2490)
#define MAX_LINE3_VOLTAGE               (uint32_t)(2490)

#define MIN_LINE1_VOLTAGE               (uint32_t)(0)//(uint32_t)(2000)
#define MIN_LINE2_VOLTAGE               (uint32_t)(0)//(uint32_t)(2000)
#define MIN_LINE3_VOLTAGE               (uint32_t)(0)//(uint32_t)(2000)

static uint32_t maxCurrentValue = 1500;

#define MAX_LINE1_CURRENT (uint32_t)(maxCurrentValue)
#define MAX_LINE2_CURRENT (uint32_t)(maxCurrentValue)
#define MAX_LINE3_CURRENT (uint32_t)(maxCurrentValue)


void setMaxCurrentValue(uint32_t newValue) {
    maxCurrentValue = newValue;
}

typedef struct {
    unsigned int status;
    unsigned int countFailed;
    unsigned int countSucceed;
    int buf[128];
    int idx;
    int median;
} t_limitParams;

typedef struct {
    t_limitParams vlMax[3];
    t_limitParams vlMin[3];
    t_limitParams ilMax[3];
    t_limitParams temperature;
} t_limit;
t_limit myLimits;



#define ALPHA _IQ(0.6)
_iq lowPassFilter(_iq input, _iq previousOutput) {
    return _IQmpy(ALPHA, input) + _IQmpy((_IQ(1) - ALPHA), input);
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int calculate_median(int *data, size_t size) {
    qsort(data, size, sizeof(int), compare);

    if (size % 2 == 0) {
        return (data[size / 2 - 1] + data[size / 2]) / 2;
    } else {
        return data[size / 2];
    }
}

static inline void handleLimitMax(t_limitParams *limitObj, _iq in, uint32_t LIMIT){
    if(limitObj->idx < MEDIAN_SIZE){
        limitObj->buf[limitObj->idx] = (uint32_t)(_IQtoF(_IQmpy(in, _IQ(1000))));
        limitObj->idx = limitObj->idx + 1;
    }
    else{
        limitObj->median = calculate_median(limitObj->buf, MEDIAN_SIZE);
        if(limitObj->median > LIMIT){
            limitObj->countFailed = limitObj->countFailed + 1;
            limitObj->status = 1;
        }
        else{
            limitObj->countSucceed = limitObj->countSucceed + 1;
            limitObj->status = 0;
        }
        limitObj->idx = 0;
    }
}


static inline void handleLimitMin(t_limitParams *limitObj, _iq in, uint32_t LIMIT){
    if(limitObj->idx < MEDIAN_SIZE){
        limitObj->buf[limitObj->idx] = (uint32_t)(_IQtoF(_IQmpy(in, _IQ(1000))));
        limitObj->idx = limitObj->idx + 1;
    }
    else{
        limitObj->median = calculate_median(limitObj->buf, MEDIAN_SIZE);
        if(limitObj->median < LIMIT){
            limitObj->countFailed = limitObj->countFailed + 1;
            limitObj->status = 1;
        }
        else{
            limitObj->countSucceed = limitObj->countSucceed + 1;
            limitObj->status = 0;
        }
        limitObj->idx = 0;
    }
}

#endif /* LIMIT_H_ */
