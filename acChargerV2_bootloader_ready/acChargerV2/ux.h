#ifndef UX_H_
#define UX_H_



#define SW_VERSION  11
#define HW_VERSION  31



/*----------------------------------------- ST1 REGISTER --------------------------------------------------------*/
#define UX_ACTUATOR_BASE      (1u << 0)
#define UX_ACTUATOR(x)        ((x) * UX_ACTUATOR_BASE)
#define UX_ACTUATOR_MASK      (1u << 0)

#define UX_TEMP_BASE          (1u << 1)
#define UX_TEMP(x)            ((x) * UX_TEMP_BASE)
#define UX_TEMP_MASK          (0x1FFu << 1)      // 9 bit → 0x1FF

#define UX_LEAKAGE_BASE       (1u << 10)
#define UX_LEAKAGE(x)         ((x) * UX_LEAKAGE_BASE)
#define UX_LEAKAGE_MASK       (1u << 10)

#define UX_MSPSTATE_BASE      (1u << 11)
#define UX_MSPSTATE(x)        ((x) * UX_MSPSTATE_BASE)
#define UX_MSPSTATE_MASK      (0x0Fu << 11)      // 4 bit → 0x0F

#define UX_RGB_BASE           (1u << 15)
#define UX_RGB(x)             ((x) * UX_RGB_BASE) // x = 0 or 1
#define UX_RGB_MASK           (1u << 15)

/*----------------------------------------- ST2 REGISTER --------------------------------------------------------*/
#define UX_CONTROLPILOT_BASE       (1u << 0)
#define UX_CONTROLPILOT(x)         ((x) * UX_CONTROLPILOT_BASE)
#define UX_CONTROLPILOT_MASK       (0xFFu << 0)

#define UX_RELAYSTATUS_BASE        (1u << 8)
#define UX_RELAYSTATUS(x)          ((x) * UX_RELAYSTATUS_BASE)
#define UX_RELAYSTATUS_MASK        (1u << 8)

#define UX_PWMSTATUS_BASE          (1u << 12)
#define UX_PWMSTATUS(x)            ((x) * UX_PWMSTATUS_BASE)
#define UX_PWMSTATUS_MASK          (1u << 12)

#define UX_RSVST2_BASE             (1u << 13)
#define UX_RSVST2(x)               ((x) * UX_RSVST2_BASE)
#define UX_RSVST2_MASK             (1u << 13)

/*----------------------------------------- L1 REGISTER --------------------------------------------------------*/
#define UX_L1VOLTAGE_BASE          (1u << 0)
#define UX_L1VOLTAGE(x)            ((x) * UX_L1VOLTAGE_BASE)
#define UX_L1VOLTAGE_MASK          (0x1FFu << 0)   /* 9 bit */

#define UX_L1CURRENT_BASE          (1u << 9)
#define UX_L1CURRENT(x)            ((x) * UX_L1CURRENT_BASE)
#define UX_L1CURRENT_MASK          (0x7Fu << 9)    /* 7 bit */

/*----------------------------------------- L2 REGISTER --------------------------------------------------------*/
#define UX_L2VOLTAGE_BASE          (1u << 0)
#define UX_L2VOLTAGE(x)            ((x) * UX_L2VOLTAGE_BASE)
#define UX_L2VOLTAGE_MASK          (0x1FFu << 0)

#define UX_L2CURRENT_BASE          (1u << 9)
#define UX_L2CURRENT(x)            ((x) * UX_L2CURRENT_BASE)
#define UX_L2CURRENT_MASK          (0x7Fu << 9)

/*----------------------------------------- L3 REGISTER --------------------------------------------------------*/
#define UX_L3VOLTAGE_BASE          (1u << 0)
#define UX_L3VOLTAGE(x)            ((x) * UX_L3VOLTAGE_BASE)
#define UX_L3VOLTAGE_MASK          (0x1FFu << 0)

#define UX_L3CURRENT_BASE          (1u << 9)
#define UX_L3CURRENT(x)            ((x) * UX_L3CURRENT_BASE)
#define UX_L3CURRENT_MASK          (0x7Fu << 9)

/*----------------------------------------- VERSION REGISTER --------------------------------------------------------*/
#define UX_VERSIONHARDWARE_BASE       (1u << 0)
#define UX_VERSIONHARDWARE(x)         ((x) * UX_VERSIONHARDWARE_BASE)
#define UX_VERSIONHARDWARE_MASK       (0x1FFu << 0)

#define UX_VERSIONSOFTWARE_BASE       (1u << 9)
#define UX_VERSIONSOFTWARE(x)         ((x) * UX_VERSIONSOFTWARE_BASE)
#define UX_VERSIONSOFTWARE_MASK       (0x7Fu << 9)


typedef struct {
    uint16_t ST1;
    uint16_t ST2;
    uint16_t ST3;
    uint16_t L1;
    uint16_t L2;
    uint16_t L3;
    uint16_t VERSION;

} t_uxReg;

t_uxReg uxReg;

void ux_init(t_uxReg *uxObj){
    uxObj->ST1 = 0x0000;
    uxObj->ST2 = 0x0000;
    uxObj->ST3 = 0x0000;
    uxObj->L1 = 0x0000;
    uxObj->L2 = 0x0000;
    uxObj->L3 = 0x0000;
    uxObj->VERSION = 0x0000;
}


void ux_update_ST1(t_uxReg *uxObj, uint16_t actuator, int16_t temparature, uint16_t leakageCurr, uint16_t mspState, uint16_t rgbState){
    uxObj->ST1 &= ~UX_ACTUATOR_MASK;
    uxObj->ST1 |= UX_ACTUATOR(actuator);

    uxObj->ST1 &= ~UX_TEMP_MASK;
    uxObj->ST1 |= UX_TEMP(temparature);

    uxObj->ST1 &= ~UX_LEAKAGE_MASK;
    uxObj->ST1 |= UX_LEAKAGE(leakageCurr);

    uxObj->ST1 &= ~UX_MSPSTATE_MASK;
    uxObj->ST1 |= UX_MSPSTATE(mspState);

    uxObj->ST1 &= ~UX_RGB_MASK;
    uxObj->ST1 |= UX_RGB(rgbState);
}

void ux_update_ST2(t_uxReg *uxObj, uint16_t controlPilot, int16_t rlyStatus, uint16_t pwmStatus){
    uxObj->ST2 &= ~UX_CONTROLPILOT_MASK;
    uxObj->ST2 |= UX_CONTROLPILOT(controlPilot);

    uxObj->ST2 &= ~UX_RELAYSTATUS_MASK;
    uxObj->ST2 |= UX_RELAYSTATUS(rlyStatus);

    uxObj->ST2 &= ~UX_PWMSTATUS_MASK;
    uxObj->ST2 |= UX_PWMSTATUS(pwmStatus);

    uxObj->ST2 &= ~UX_RSVST2_MASK;
    uxObj->ST2 |= UX_RSVST2(0);
}


void ux_update_L1(t_uxReg *uxObj, uint16_t voltage, uint16_t current){
    uxObj->L1 &= ~UX_L1VOLTAGE_MASK;
    uxObj->L1 |= UX_L1VOLTAGE(voltage);

    uxObj->L1 &= ~UX_L1CURRENT_MASK;
    uxObj->L1 |= UX_L1CURRENT(current);
}

void ux_update_L2(t_uxReg *uxObj, uint16_t voltage, uint16_t current){
    uxObj->L2 &= ~UX_L2VOLTAGE_MASK;
    uxObj->L2 |= UX_L2VOLTAGE(voltage);

    uxObj->L2 &= ~UX_L2CURRENT_MASK;
    uxObj->L2 |= UX_L2CURRENT(current);
}

void ux_update_L3(t_uxReg *uxObj, uint16_t voltage, uint16_t current){
    uxObj->L3 &= ~UX_L3VOLTAGE_MASK;
    uxObj->L3 |= UX_L3VOLTAGE(voltage);

    uxObj->L3 &= ~UX_L3CURRENT_MASK;
    uxObj->L3 |= UX_L3CURRENT(current);
}


void ux_update_Version(t_uxReg *uxObj, uint16_t swVer, uint16_t hwVer){
    uxObj->VERSION &= ~UX_VERSIONHARDWARE_MASK;
    uxObj->VERSION |= UX_VERSIONHARDWARE(hwVer);

    uxObj->VERSION &= ~UX_VERSIONSOFTWARE_MASK;
    uxObj->VERSION |= UX_VERSIONSOFTWARE(swVer);
}


#endif /* UX_H_ */
