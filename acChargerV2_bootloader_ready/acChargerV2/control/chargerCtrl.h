#ifndef CHARGERCTRL_H_
#define CHARGERCTRL_H_

#include "charger.h"
#include "chargeParams.h"


void initEVSE_Status(ChargerStatus *evse){
    evse->charging = AC_EVSEChargeStatus_NO_CHARGING;
    evse->connector = AC_EVSEConnectorStatus_NOT_PLUGGED;
    evse->chargeReq = AC_EVSEChargeRequest_NO_REQ;
    evse->stopReqEv = AC_EVSEChargeStopFromEV_NOT_STOP;
    evse->connLocked = AC_EVSEConnectorLockedStatus_NOT_LOCKED;
}


void checkCableStatus(ChargerStatus *out, State *in){
    out->charging = (in->current_state == C2 ? 1 :0);
    out->connector = (in->current_state == B1 || in->current_state == B2 ||
                      in->current_state == C1 || in->current_state == C2? 1 : 0);

}

uint16_t relayCtrl(uint16_t charging, uint16_t socketLocked, uint16_t error){
    return charging && socketLocked && !error;
}

uint16_t isSockedLocked(uint16_t motorIn){
    return !motorIn;
}

#endif /* CHARGERCTRL_H_ */
