#ifndef DISPLAY_H_
#define DISPLAY_H_

typedef enum {
    VCP=0, VPP,
    Vn, Vl1, Vl2, Vl3,
    IL1, IL2, IL3,
    voltage_RelayOut1, voltage_RelayOut2, voltage_RelayOut3

}DSP_SELECT;

DSP_SELECT voltageSelect = Vl1;
float voltageBuffer[400], voltageDisplay[400];



#endif /* DISPLAY_H_ */
