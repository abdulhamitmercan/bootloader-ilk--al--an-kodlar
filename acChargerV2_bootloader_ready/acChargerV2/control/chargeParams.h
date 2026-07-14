#ifndef CHARGEPARAMS_H_
#define CHARGEPARAMS_H_



typedef enum {
    AC_EVSEChargeStatus_NO_CHARGING = 0,
    AC_EVSEChargeStatus_CHARGING = 1,
} AC_EVSE_ChargeStatus;

typedef enum {
    AC_EVSEConnectorStatus_NOT_PLUGGED = 0,
    AC_EVSEConnectorStatus_PLUGGED = 1,
} AC_EVSE_ConnectorStatus;

typedef enum {
    AC_EVSEChargeRequest_NO_REQ = 0,
    AC_EVSEChargeRequest_START_CHARGE = 1,
    AC_EVSEChargeRequest_STOP_CHARGE = 2
} AC_EVSE_ChargeRequest;

typedef enum {
    AC_EVSEChargeStopFromEV_NOT_STOP = 0,
    AC_EVSEChargeStopFromEV_STOP = 1,
} AC_EVSE_ChargeStopFromEV;

typedef enum {
    AC_EVSEConnectorLockedStatus_NOT_LOCKED = 0,
    AC_EVSEConnectorLockedStatus_LOCKED = 1,
} AC_EVSE_ConnectorLockedStatus;


typedef struct{
    uint16_t charging;
    uint16_t connector;
    uint16_t chargeReq;
    uint16_t stopReqEv;
    uint16_t connLocked;
} ChargerStatus;


typedef struct {
    unsigned int outRelay1;
    unsigned int outRelay2;
    unsigned int relayN_L1;
    unsigned int relayL2_L3;
} t_relays;



ChargerStatus vngEvse;


#endif /* CHARGEPARAMS_H_ */
