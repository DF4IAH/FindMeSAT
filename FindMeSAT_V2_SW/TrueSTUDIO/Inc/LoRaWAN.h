/*
 * LoRaWAN.h
 *
 *  Created on: 12.05.2018
 *      Author: DF4IAH
 */

#ifndef LORAWAN_H_
#define LORAWAN_H_

#include "stm32l4xx_hal.h"


#define LoRaWAN_MsgLenMax             51
#define LoRaWAN_FRMPayloadMax         48


typedef enum LoRaWANVersion {
  LoRaWANVersion_10                   = 0,
  LoRaWANVersion_11                   = 1,
} LoRaWANVersion_t;

typedef enum LoRaWANctxDir {
  Up                                  = 0,
  Dn                                  = 1
} LoRaWANctxDir_t;


typedef struct LoRaWANctxBkpRam {

  /* Skip first entries */
  uint32_t                            _skip[16];

  /* CRC */
  uint32_t                            LoRaWANcrc;

  /* Counters */
  uint32_t                            FCntUp;
  uint32_t                            NFCntDwn;         // LW 1.0: FCntDwn
  uint32_t                            AFCntDwn;

  uint32_t                            n8_n8_DLSettings8_RxDelay8;
  uint32_t                            CFList_03_02_01_00;
  uint32_t                            CFList_07_06_05_04;
  uint32_t                            CFList_11_10_09_08;
  uint32_t                            CFList_15_14_13_12;

  uint32_t                            _end;

} LoRaWANctxBkpRam_t;

typedef struct LoRaWANctx {

  /* Non-volatile counters */
  volatile LoRaWANctxBkpRam_t*        bkpRAM;


  /* Device specific */
  uint8_t                             DevEUI_LE[8];
  uint8_t                             DevAddr_LE[4];

  /* Network / MAC specific */
  LoRaWANVersion_t                    LoRaWAN_ver;
  uint8_t                             NwkKey[16];       // Network root key (for OTA devices)
  uint8_t                             FNwkSIntKey[16];
  uint8_t                             SNwkSIntKey[16];
  uint8_t                             NwkSEncKey[16];
  uint8_t                             JSIntKey[16];     // (for OTA devices)
  uint8_t                             JSEncKey[16];     // (for OTA devices)

  /* Join Server specific */
  uint8_t                             JoinEUI_LE[8];
  uint16_t                            DevNonce;
  uint32_t                            ServerNonce;      // actual 3 octets
  uint32_t                            Home_NetID;       // actual 3 octets
  //uint8_t                           DLSettings;
  //uint8_t                           RxDelay;
  //uint8_t                           CFList[16];

  /* Application specific */
  uint8_t                             AppEUI_LE[8];
  uint8_t                             AppSKey[16];

  /* Current transmission */
  LoRaWANctxDir_t                     Dir;
  uint8_t                             FCtrl_ADR;
  uint8_t                             FCtrl_ADRACKReq;
  uint8_t                             FCtrl_ACK;
  uint8_t                             FCtrl_ClassB;
  uint8_t                             FCtrl_FPending;
  uint8_t                             FPort;
  uint16_t                            ConfFCnt;
  uint8_t                             TxDr;
  uint8_t                             TxCh;

} LoRaWANctx_t;


typedef enum LoRaWAN_MTypeBF {
  JoinRequest                         = 0,
  JoinAccept                          = 1,
  UnconfDataUp                        = 2,
  UncondDataDn                        = 3,
  ConfDataUp                          = 4,
  ConfDataDn                          = 5,
  RejoinReq                           = 6,
  Proprietary                         = 7
} LoRaWAN_MTypeBF_t;

typedef enum LoRaWAN_MajorBF {
  LoRaWAN_R1                          = 0,
} LoRaWAN_MajorBF_t;


#define LoRaWAN_MHDR_MType_SL         5
#define LoRaWAN_MHDR_Major_SL         0

#define LoRaWAN_FCtl_ADR_SL           7
#define LoRaWAN_FCtl_ADRACKReq_SL     6
#define LoRaWAN_FCtl_ACK_SL           5
#define LoRaWAN_FCtl_ClassB_SL        4
#define LoRaWAN_FCtl_FPending_SL      4
#define LoRaWAN_FCtl_FOptsLen_SL      0


uint8_t GET_BYTE_OF_WORD(uint32_t word, uint8_t pos);


typedef enum LoRaWANMAC_CID {

  ResetInd_UP                         = 0x01,   // ABP devices only, LW 1.1
  ResetConf_DN                        = 0x01,   // ABP devices only, LW 1.1

  LinkCheckReq_UP                     = 0x02,
  LinkCheckAns_DN                     = 0x02,

  LinkADRReq_DN                       = 0x03,
  LinkADRAns_UP                       = 0x03,

  DutyCycleReq_DN                     = 0x04,
  DutyCycleAns_UP                     = 0x04,

  RXParamSetupReq_DN                  = 0x05,
  RXParamSetupAns_UP                  = 0x05,

  DevStatusReq_DN                     = 0x06,
  DevStatusAns_UP                     = 0x06,

  NewChannelReq_DN                    = 0x07,
  NewChannelAns_UP                    = 0x07,

  RXTimingSetupReq_DN                 = 0x08,
  RXTimingSetupAns_UP                 = 0x08,

  TxParamSetupReq_DN                  = 0x09,
  TxParamSetupAns_UP                  = 0x09,

  DlChannelReq_DN                     = 0x0A,
  DlChannelAns_UP                     = 0x0A,

  RekeyInd_UP                         = 0x0B,
  RekeyConf_DN                        = 0x0B,

  ADRParamSetupReq_DN                 = 0x0C,
  ADRParamSetupAns_UP                 = 0x0C,

  DeviceTimeReq_UP                    = 0x0D,
  DeviceTimeAns_DN                    = 0x0D,

  ForceRejoinReq_DN                   = 0x0E,

  RejoinParamSetupReq_DN              = 0x0F,
  RejoinParamSetupAns_UP              = 0x0F,

} LoRaWANMAC_CID_t;


typedef struct FRMPayloadBlockA_Up {
  uint8_t                             variant;
  uint32_t                            _noUse;
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntUp[4];
  uint8_t                             _pad;
  uint8_t                             idx;
} FRMPayloadBlockA_Up_t;

typedef struct MICBlockB0_Up {
  uint8_t                             variant;
  uint32_t                            _noUse;
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntUp[4];
  uint8_t                             _pad;
  uint8_t                             len;
} MICBlockB0_Up_t;

typedef struct MICBlockB1_Up {
  uint8_t                             variant;
  uint8_t                             ConfFCnt[2];
  uint8_t                             TxDr;
  uint8_t                             TxCh;
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntUp[4];
  uint8_t                             _pad;
  uint8_t                             len;
} MICBlockB1_Up_t;


typedef struct MICBlockB0_Dn {
  uint8_t                             variant;
  uint8_t                             ConfFCnt[2];
  uint16_t                            _noUse;
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntDn[4];
  uint8_t                             _pad;
  uint8_t                             len;
} MICBlockB0_Dn_t;


typedef struct LoraliveApp {

  uint8_t                           voltage_32_v;

  /* Big endian */
  uint8_t                           dust025_10_hi;
  uint8_t                           dust025_10_lo;

  /* Big endian */
  uint8_t                           dust100_10_hi;
  uint8_t                           dust100_10_lo;

  char  id;

  union {
    struct loraliveApp_len6 {
    } l6;

    struct loraliveApp_len8 {
      int8_t                        temperature;
      uint8_t                       humidity_100;
    } l8;

    struct loraliveApp_len14 {
      uint8_t                       latitude_1000_sl24;
      uint8_t                       latitude_1000_sl16;
      uint8_t                       latitude_1000_sl08;
      uint8_t                       latitude_1000_sl00;

      uint8_t                       longitude_1000_sl24;
      uint8_t                       longitude_1000_sl16;
      uint8_t                       longitude_1000_sl08;
      uint8_t                       longitude_1000_sl00;
    } l14;

    struct loraliveApp_len16 {
      int8_t                        temperature;
      uint8_t                       humidity_100;

      uint8_t                       latitude_1000_sl24;
      uint8_t                       latitude_1000_sl16;
      uint8_t                       latitude_1000_sl08;
      uint8_t                       latitude_1000_sl00;

      uint8_t                       longitude_1000_sl24;
      uint8_t                       longitude_1000_sl16;
      uint8_t                       longitude_1000_sl08;
      uint8_t                       longitude_1000_sl00;
    } l16;
  } u;

} LoraliveApp_t;



void LoRaWAN_Init(void);

void LoRaWANctx_readFLASH(void);
void LoRaWANctx_applyKeys_loralive(void);
void LoRaWAN_App_loralive_pushUp(LoRaWANctx_t* ctx, uint8_t FPort, LoraliveApp_t* app, uint8_t size);

#endif /* LORAWAN_H_ */
