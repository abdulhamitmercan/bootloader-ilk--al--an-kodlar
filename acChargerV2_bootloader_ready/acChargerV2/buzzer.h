#ifndef BUZZER_H_
#define BUZZER_H_

#include "HAL.h"

#define BUZZER_ON()     HAL_SET_GPIO_BUZZER()
#define BUZZER_OFF()    HAL_RESET_GPIO_BUZZER()

typedef enum {
    BUZZER_STATE_OFF,
    BUZZER_STATE_WAIT_ON,
    BUZZER_STATE_ON,
    BUZZER_STATE_WAIT_OFF
} BuzzerState;


void updateBuzzer(int buzzerVal, int *buzzerRsp) {
    static BuzzerState buzzerState = BUZZER_STATE_OFF;
    static int buzzerTimeout = 0;
    switch (buzzerState) {
        case BUZZER_STATE_OFF:
            if (buzzerVal == 1) {
                buzzerState = BUZZER_STATE_WAIT_ON;
                buzzerTimeout = 0;
                BUZZER_OFF();
            }
            break;

        case BUZZER_STATE_WAIT_ON:
            if (buzzerTimeout < 1200) {
                buzzerTimeout++;
                BUZZER_ON();
                *buzzerRsp = 2;
            } else {
                buzzerState = BUZZER_STATE_ON;
                buzzerTimeout = 0;
                BUZZER_OFF();
                *buzzerRsp = 2;
            }
            break;

        case BUZZER_STATE_ON:
            if (buzzerTimeout < 15000) {
                buzzerTimeout++;
                BUZZER_OFF();
                *buzzerRsp = 2;
            } else {
                buzzerState = BUZZER_STATE_WAIT_OFF;
                buzzerTimeout = 0;
                BUZZER_OFF();
                *buzzerRsp = 3;
            }
            break;

        case BUZZER_STATE_WAIT_OFF:
            if (buzzerVal == 0) {
                buzzerState = BUZZER_STATE_OFF;
                buzzerTimeout = 0;
                BUZZER_OFF();
                *buzzerRsp = 0;
            }

            break;
    }
}

#endif /* BUZZER_H_ */
