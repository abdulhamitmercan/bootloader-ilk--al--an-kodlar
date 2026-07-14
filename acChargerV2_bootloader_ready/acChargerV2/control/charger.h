#ifndef CHARGER_H_
#define CHARGER_H_

#include "controlPilot.h"
#include "chargeParams.h"

//-- IEC 61851_1 Table A.6 - List of sequences
#define CP_1_1              (0x11) // Sequence 1.1 Plug-in (With S2)
#define CP_1_2              (0x12) // Sequence 1.2 Plug-in  (w/o S2 or S2 always in close position)
#define CP_2_1              (0x21) // Sequence 2.1 Unplug at state Bx
#define CP_2_2              (0x22) // Sequence 2.2 Unplug at state Cx, Dx
#define CP_3_1              (0x31) // Sequence 3.1 EV supply equipment power available (state B)
#define CP_3_2              (0x32) // Sequence 3.2 EV supply equipment power available (state C)
#define CP_4                (0x40) // Sequence 4 EV ready to charge
#define CP_5                (0x50) // Sequence 5 EV starts charging
#define CP_6                (0x60) // Sequence 6 Current change
#define CP_7                (0x70) // Sequence 7 EV stops charging
#define CP_8_1              (0x81) // Sequence 8.1 EV supply equipment responds to EV opens S2 (with PWM)
#define CP_8_2              (0x82) // Sequence 8.2 EV supply equipment responds to EV opens S2 (w/o PWM)
#define CP_9_1              (0x91) // Sequence 9.1 EV supply equipment requests to stop charging
#define CP_9_2              (0x92) // Sequence 9.2 EV supply equipment stops PWM at state B
#define CP_9_3              (0x93) // Sequence 9.3 EV supply equipment stops PWM at state A
#define CP_10_1             (0xA1) // Sequence 10.1  EV responds to stop charging request
#define CP_10_2             (0xA2) // Sequence 10.2 EV does not respond to a stop charging request
#define CP_11               (0xB1) // Sequence 11 EV signal to the EV supply equipment
#define CP_12               (0xB2) // Sequence 12 State caused by error or fault condition



//-- typical control pilot states
#define A1              0
#define A2              1
#define B1              2
#define B2              3
#define C1              4
#define C2              5


#define PWM_STATE_ON                                    (0x8000)
#define PWM_STATE_OFF                                   (0x7FFF)

#define CP_LEVEL_A1                                     (1<<0 & PWM_STATE_OFF)
#define CP_LEVEL_A2                                     (1<<0 | PWM_STATE_ON)

#define CP_LEVEL_B1                                     (1<<1 & PWM_STATE_OFF)
#define CP_LEVEL_B2                                     (1<<1 | PWM_STATE_ON)

#define CP_LEVEL_C1                                     (1<<2 & PWM_STATE_OFF)
#define CP_LEVEL_C2                                     (1<<2 | PWM_STATE_ON)

#define CP_LEVEL_D1                                     (1<<3 & PWM_STATE_OFF)
#define CP_LEVEL_D2                                     (1<<3 | PWM_STATE_ON)

#define CP_LEVEL_E                                      (1<<4)


//-- State machine variables
//-- Out is:
//-- | bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 |
//-- bit 0 : [1 -> charging ready, 0 -> charging not ready]
//-- bit 1 : [1 -> PWM on, 0 -> PWM off]
typedef struct {
    unsigned int Out; // 6-bit output
    unsigned int current_state;
    unsigned int next_state;
    unsigned int voltageLevel;
    unsigned int transition;
} State;


typedef struct {
    unsigned int voltageLevel;
    unsigned int transition;
} t_chargerParams;

State FSM;
t_chargerParams acevse;

volatile int unlockManual = 0;


void resetFSM(){
    FSM.Out &= 0x0000;
    FSM.current_state = A1;
    FSM.next_state &= 0x0000;
    FSM.voltageLevel &= 0x0000;
    FSM.transition &= 0x0000;
}

uint16_t stopReq = 0;
void runCharger(unsigned int input, State* cs){
    switch (cs->current_state){
        //-- STATE A1 ----> 12V, NO PWM
        case A1:
            switch (input){
                    case CP_1_1:
                        cs->next_state = B1;
                        break;
            }
            cs->Out = 0x0000;
            break;

        //-- STATE A2 ----> 12V, PWM
        case A2:
            switch (input){
                    case CP_1_1:
                        cs->next_state = B2;
                        break;
                    case CP_9_3:
                        cs->next_state = A1;
                        break;
                }
            cs->Out = 0x0000;
            break;

        //-- STATE B1 ----> 9V, NO PWM
        case B1:
            switch (input){
                    case CP_2_1:
                        cs->next_state = A1;
                        break;
                    case CP_3_1:
                        cs->next_state = B2;
                        break;
                }
            cs->Out = 0x0000;
            break;

        //-- STATE B2 ----> 9V, PWM
        case B2:
            switch (input){
                    case CP_4:
                        cs->next_state = C2;
                        break;
                    case CP_2_1:
                        cs->next_state = A2;
                        break;
                    case CP_9_2:
                        cs->next_state = B1;
                        break;
                }
            cs->Out = 0x0000;
            break;

        //-- STATE C1 ----> 6V, NO PWM
        case C1:
            switch (input){
                    case CP_3_2:
                        cs->next_state = C2;
                        break;
                    case CP_8_2:
                        cs->next_state = B1;
                        break;
                }
            // relays are off
            cs->Out = 0x0000;
            break;

        //-- STATE C2 ----> 6V, PWM
        case C2:

            switch (input){
                    case CP_6:
                        cs->next_state = C2;
                        break;
                    case CP_9_1:
                        vngEvse.stopReqEv = AC_EVSEChargeStopFromEV_STOP;
                        cs->next_state = C1;
                        break;
                    case CP_8_1:
                        vngEvse.stopReqEv = AC_EVSEChargeStopFromEV_STOP;
                        cs->next_state = B2;
                        break;
                    case CP_2_2:
                        vngEvse.stopReqEv = AC_EVSEChargeStopFromEV_STOP;
                        cs->next_state = A2;
                        break;
                }
            cs->Out = 0x0000;
            break;
    }

    cs->current_state = cs->next_state;
    cs->transition = input;
}


void cpVoltageLevel(unsigned int* level, _iq cpVoltage, unsigned int pwmState){
    if(pwmState == 1){
        if(PWM_12V(cpVoltage) == 1){
            *level = CP_LEVEL_A2;
        }
        else if(PWM_9V(cpVoltage) == 1){
            *level = CP_LEVEL_B2;
        }
        else if(PWM_6V(cpVoltage) == 1){
            *level = CP_LEVEL_C2;
        }
        else if(PWM_3V(cpVoltage) == 1){
            *level = CP_LEVEL_D2;
        }
        else{
            *level = 0;
        }
    }
    else{
        if(PWM_12V(cpVoltage) == 1){
            *level = CP_LEVEL_A1;
        }
        else if(PWM_9V(cpVoltage) == 1){
            *level = CP_LEVEL_B1;
        }
        else if(PWM_6V(cpVoltage) == 1){
            *level = CP_LEVEL_C1;
        }
        else if(PWM_3V(cpVoltage) == 1){
            *level = CP_LEVEL_D1;
        }
        else{
            *level = 0;
        }
    }


}

void cpTransition_A1(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_B1){
        *transition = CP_2_1;
        unlockManual = 1;
    }
    else if(levelOld == CP_LEVEL_A2){
        *transition = CP_9_3;
        unlockManual = 1;
    }
    else if(levelOld == CP_LEVEL_C2){
        unlockManual = 1;
    }
    else if(levelOld == CP_LEVEL_C1){
        unlockManual = 1;
    }
}

void cpTransition_A2(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_B2){
        *transition = CP_2_1;
    }
    else if(levelOld == CP_LEVEL_C2){
        *transition = CP_2_2;
    }
}

void cpTransition_B1(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_A1){
        *transition = CP_1_1;
    }
    else if(levelOld == CP_LEVEL_B2){
        *transition = CP_9_2;
    }
    else if(levelOld == CP_LEVEL_C1){
        *transition = CP_8_2;
    }
}

void cpTransition_B2(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_A2){
        *transition = CP_1_1;
    }
    else if(levelOld == CP_LEVEL_B1){
        *transition = CP_3_1;
    }
    else if(levelOld == CP_LEVEL_C2){
        *transition = CP_8_1;
    }
}

void cpTransition_C1(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_C2){
        *transition = CP_9_1;
    }
}

void cpTransition_C2(unsigned int *transition, unsigned int levelOld){
    if(levelOld == CP_LEVEL_B2){
        *transition = CP_4;
    }
    else if(levelOld == CP_LEVEL_C1){
        *transition = CP_3_2;
    }

}

void cpTransition(unsigned int *transition, unsigned int level){
    static unsigned int levelOld = 0;
    switch (level){
        case CP_LEVEL_A1:
            cpTransition_A1(transition, levelOld);
            break;
        case CP_LEVEL_A2:
            cpTransition_A2(transition, levelOld);
            break;
        case CP_LEVEL_B1:
            cpTransition_B1(transition, levelOld);
            break;
        case CP_LEVEL_B2:
            cpTransition_B2(transition, levelOld);
            break;
        case CP_LEVEL_C1:
            cpTransition_C1(transition, levelOld);
            break;
        case CP_LEVEL_C2:
            cpTransition_C2(transition, levelOld);
            break;
    }


    levelOld = level;
}



void runFSM(_iq Vcp, unsigned int pwmState){
    cpVoltageLevel(&acevse.voltageLevel, Vcp, pwmState);
    cpTransition(&acevse.transition, acevse.voltageLevel);
    runCharger(acevse.transition, &FSM);
}





#endif /* CHARGER_H_ */
