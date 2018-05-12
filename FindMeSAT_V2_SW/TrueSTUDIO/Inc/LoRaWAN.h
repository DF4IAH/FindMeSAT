/*
 * LoRaWAN.h
 *
 *  Created on: 12.05.2018
 *      Author: DF4IAH
 */

#ifndef LORAWAN_H_
#define LORAWAN_H_

#include "stm32l4xx_hal.h"


#define LoRaWAN_App_loralive_pushUp__FRMPayloadMax    48


typedef enum LoRaWANMAC_CID {

  ResetInd_UP             = 0x01,   // ABP devices only, LW 1.1
  ResetConf_DN            = 0x01,   // ABP devices only, LW 1.1

  LinkCheckReq_UP         = 0x02,
  LinkCheckAns_DN         = 0x02,

  LinkADRReq_DN           = 0x03,
  LinkADRAns_UP           = 0x03,

  DutyCycleReq_DN         = 0x04,
  DutyCycleAns_UP         = 0x04,

  RXParamSetupReq_DN      = 0x05,
  RXParamSetupAns_UP      = 0x05,

  DevStatusReq_DN         = 0x06,
  DevStatusAns_UP         = 0x06,

  NewChannelReq_DN        = 0x07,
  NewChannelAns_UP        = 0x07,

  RXTimingSetupReq_DN     = 0x08,
  RXTimingSetupAns_UP     = 0x08,

  TxParamSetupReq_DN      = 0x09,
  TxParamSetupAns_UP      = 0x09,

  DlChannelReq_DN         = 0x0A,
  DlChannelAns_UP         = 0x0A,

  RekeyInd_UP             = 0x0B,
  RekeyConf_DN            = 0x0B,

  ADRParamSetupReq_DN     = 0x0C,
  ADRParamSetupAns_UP     = 0x0C,

  DeviceTimeReq_UP        = 0x0D,
  DeviceTimeAns_DN        = 0x0D,

  ForceRejoinReq_DN       = 0x0E,

  RejoinParamSetupReq_DN  = 0x0F,
  RejoinParamSetupAns_UP  = 0x0F,

} LoRaWANMAC_CID_t;

typedef enum LoRaWANVersion {
  LoRaWANVersion_10 = 0,
  LoRaWANVersion_11 = 1,
} LoRaWANVersion_t;


typedef struct LoRaWANctx {

  /* Device specific */
  uint8_t           DevEUI[8];
  uint8_t           DevAddr[4];

  /* Network / MAC specific */
  LoRaWANVersion_t  LoRaWAN_ver;
  uint8_t           NwkKey[16];       // Network root key (for OTA devices)
  uint8_t           FNwkSIntKey[16];
  uint8_t           SNwkSIntKey[16];
  uint8_t           NwkSEncKey[16];
  uint8_t           JSIntKey[16];     // (for OTA devices)
  uint8_t           JSEncKey[16];     // (for OTA devices)

  /* Join Server specific */
  uint8_t           JoinEUI[8];
  uint16_t          DevNonce;
  uint32_t          ServerNonce;      // actual 3 octets
  uint32_t          Home_NetID;       // actual 3 octets
  uint8_t           DLSettings;
  uint8_t           RxDelay;
  uint8_t           CFList[16];

  /* Application specific */
  uint8_t           appEUI[8];
  uint8_t           appSKey[16];

  /* Counters */
  uint32_t          FCntUp;
  uint32_t          NFCntDwn;         // LW 1.0: FCntDwn
  uint32_t          AFCntDwn;

} LoRaWANctx_t;


typedef struct LoraliveApp {

  uint8_t voltage_32_v;

  /* Big endian */
  uint8_t dust025_10_hi;
  uint8_t dust025_10_lo;

  /* Big endian */
  uint8_t dust100_10_hi;
  uint8_t dust100_10_lo;

  char  id;

  union {
    struct loraliveApp_len6 {
    } l6;

    struct loraliveApp_len8 {
      int8_t  temperature;
      uint8_t humidity_100;
    } l8;

    struct loraliveApp_len14 {
      uint8_t latitude_1000_sl24;
      uint8_t latitude_1000_sl16;
      uint8_t latitude_1000_sl08;
      uint8_t latitude_1000_sl00;

      uint8_t longitude_1000_sl24;
      uint8_t longitude_1000_sl16;
      uint8_t longitude_1000_sl08;
      uint8_t longitude_1000_sl00;
    } l14;

    struct loraliveApp_len16 {
      int8_t  temperature;
      uint8_t humidity_100;

      uint8_t latitude_1000_sl24;
      uint8_t latitude_1000_sl16;
      uint8_t latitude_1000_sl08;
      uint8_t latitude_1000_sl00;

      uint8_t longitude_1000_sl24;
      uint8_t longitude_1000_sl16;
      uint8_t longitude_1000_sl08;
      uint8_t longitude_1000_sl00;
    } l16;
  } u;

} LoraliveApp_t;


typedef struct FRMPayloadBlockA {

  uint8_t   variant;
  uint32_t  _noUse;
  uint8_t   Dir;
  uint8_t   DevAddr[4];
  uint32_t  FCntUp_FCntDwn;
  uint8_t   _pad;
  uint8_t   i;

} FRMPayloadBlockA_t;


void LoRaWANctx_readNVM(void);
void LoRaWANctx_applyKeys_loralive(void);
void LoRaWAN_App_loralive_pushUp(LoRaWANctx_t* ctx, uint8_t FPort, LoraliveApp_t* app, uint8_t size);

#endif /* LORAWAN_H_ */
