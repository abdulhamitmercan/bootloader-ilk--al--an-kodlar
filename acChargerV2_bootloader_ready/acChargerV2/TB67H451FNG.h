#ifndef TB67H451FNG_H_
#define TB67H451FNG_H_

#include "HAL.h"


enum{
    STOP = 0,
    FORWARD,
    REVERSE,
    BRAKE,
};

typedef struct {
    uint16_t status;
} t_actuator;

t_actuator actuator;


void stopMotor(){
    HAL_RESET_GPIO_ACT_IN1();
    HAL_RESET_GPIO_ACT_IN2();
}

void forwardMotor(){
    HAL_SET_GPIO_ACT_IN1();
    HAL_RESET_GPIO_ACT_IN2();
}

void reverseMotor(){
    HAL_RESET_GPIO_ACT_IN1();
    HAL_SET_GPIO_ACT_IN2();
}

void brakeMotor(){
    HAL_SET_GPIO_ACT_IN1();
    HAL_SET_GPIO_ACT_IN2();
}



void processActuator(uint16_t actuatorMode){
    switch (actuatorMode){
        case STOP:
            stopMotor();
            break;
        case FORWARD:
            forwardMotor();
            break;
        case REVERSE:
            reverseMotor();
            break;
        case BRAKE:
            brakeMotor();
            break;
    }
}

uint16_t readMotorInput(){
    uint16_t motorIn = 0;
    if(P6IN & BIT3)
    {
        //P6.3 is HIGH
        motorIn = 1;
    }
    else
    {
        //P6.3 is LOW
        motorIn = 0;
    }
    return motorIn;
}

void updateMotorInput(uint16_t direction) {
    static uint16_t motorIn = 0;
    static uint16_t motorInOld = 0;
    static uint16_t flg = 1;

    motorIn = readMotorInput();

    if (!(motorIn ^ motorInOld)) {
        processActuator(STOP);
    }


    if(direction == 0 && flg == 0){
        processActuator(REVERSE);
        flg = 1;
    }
    else if(direction == 1 && flg == 1){
        processActuator(FORWARD);
        flg = 0;
    }

    motorInOld = motorIn;
}




#endif /* TB67H451FNG_H_ */
