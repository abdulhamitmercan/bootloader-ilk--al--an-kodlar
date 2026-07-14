#ifndef CONTROLPILOT_H_
#define CONTROLPILOT_H_

#include "dspLib.h"
#include "ux.h"


#define PWM_12V(i)                                      (i>=_IQ(11.0) && i<_IQ(13.0))
#define PWM_9V(i)                                       (i>=_IQ(8.0) && i<_IQ(11.0))
#define PWM_6V(i)                                       (i>_IQ(4.0) && i<_IQ(8.0))
#define PWM_3V(i)                                       (i>_IQ(2.0) && i<_IQ(4.0))
#define PWM_0V(i)                                       (i>_IQ(0.5) && i<_IQ(-0.5))


typedef struct {
    t_mvngAvg avg;
    unsigned int count12;
    unsigned int count9;
    unsigned int count6;
    unsigned int count3;
    _iq qout;
    float out;
    unsigned int dutyReg;
} t_cpVoltage;

t_cpVoltage cpLevel;




static inline void cp_VoltageLevel(_iq voltage){
#if HW_VERSION < 30
    if(voltage > _IQ(1.0)){
        cpLevel.avg.qoutVal = dsp_MovingAvg(cpLevel.avg.buffer, &cpLevel.avg.sum, cpLevel.avg.pos, LENGTH_OF_MOVIG_AVG, voltage);


            if(PWM_12V(cpLevel.avg.qoutVal)){
                  if(cpLevel.count12 == 32){
                      cpLevel.qout = _IQ(12.0);
                      cpLevel.count12 &= 0x0000;
                  }
                  cpLevel.count12++;
                  cpLevel.count9 &= 0x0000;
                  cpLevel.count6 &= 0x0000;
                  cpLevel.count3 &= 0x0000;
              }
              else if(PWM_9V(cpLevel.avg.qoutVal)){
                  if(cpLevel.count9 == 32){
                      cpLevel.qout = _IQ(9.0);
                      cpLevel.count9 &= 0x0000;
                  }
                  cpLevel.count9++;
                  cpLevel.count12 &= 0x0000;
                  cpLevel.count6 &= 0x0000;
                  cpLevel.count3 &= 0x0000;
              }
              else if(PWM_6V(cpLevel.avg.qoutVal)){
                if(cpLevel.count6 == 32){
                    cpLevel.qout = _IQ(6.0);
                    cpLevel.count6 &= 0x0000;
                }
                cpLevel.count6++;
                cpLevel.count12 &= 0x0000;
                cpLevel.count9 &= 0x0000;
                cpLevel.count3 &= 0x0000;
            }

            //cpLevel.out = _IQtoF(cpLevel.qout);
              cpLevel.avg.pos++;
              if (cpLevel.avg.pos >= LENGTH_OF_MOVIG_AVG){
                  cpLevel.avg.pos = 0;
              }
    }

#else

    if(voltage > _IQ(3.0)){
        cpLevel.avg.qoutVal = dsp_MovingAvg(cpLevel.avg.buffer, &cpLevel.avg.sum, cpLevel.avg.pos, LENGTH_OF_MOVIG_AVG, voltage);


            if(PWM_12V(cpLevel.avg.qoutVal)){
                  if(cpLevel.count12 == 32){
                      cpLevel.qout = _IQ(12.0);
                      cpLevel.count12 &= 0x0000;
                  }
                  cpLevel.count12++;
                  cpLevel.count9 &= 0x0000;
                  cpLevel.count6 &= 0x0000;
                  cpLevel.count3 &= 0x0000;
              }
              else if(PWM_9V(cpLevel.avg.qoutVal)){
                  if(cpLevel.count9 == 32){
                      cpLevel.qout = _IQ(9.0);
                      cpLevel.count9 &= 0x0000;
                  }
                  cpLevel.count9++;
                  cpLevel.count12 &= 0x0000;
                  cpLevel.count6 &= 0x0000;
                  cpLevel.count3 &= 0x0000;
              }
              else if(PWM_6V(cpLevel.avg.qoutVal)){
                if(cpLevel.count6 == 32){
                    cpLevel.qout = _IQ(6.0);
                    cpLevel.count6 &= 0x0000;
                }
                cpLevel.count6++;
                cpLevel.count12 &= 0x0000;
                cpLevel.count9 &= 0x0000;
                cpLevel.count3 &= 0x0000;
            }

            //cpLevel.out = _IQtoF(cpLevel.qout);
              cpLevel.avg.pos++;
              if (cpLevel.avg.pos >= LENGTH_OF_MOVIG_AVG){
                  cpLevel.avg.pos = 0;
              }
    }
#endif
}


#endif /* CONTROLPILOT_H_ */
