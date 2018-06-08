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
  uint8_t                             DevEUI[8];
  volatile uint8_t                    DevAddr[4];           // JOIN-ACCEPT
  volatile uint8_t                    DevNonce[2];          // JOIN-REQUEST, REJOIN-REQUEST - V1.02: random value
  volatile uint8_t                    Home_NetID[3];        // JOIN-ACCEPT

  /* Network / MAC specific */
  LoRaWANVersion_t                    LoRaWAN_ver;
//uint8_t                             NwkKey_1V1[16];       // Network root key (for OTA devices)
  volatile uint8_t                    NwkSKey_1V02[16];     // Session key - JOIN-ACCEPT (derived from: AppKey)
//uint8_t                             FNwkSKey_1V1[16];     // JOIN-ACCEPT
//uint8_t                             FNwkSIntKey_1V1[16];  // JOIN-ACCEPT (derived from: NwkKey)
//uint8_t                             SNwkSIntKey_1V1[16];  // JOIN-ACCEPT (derived from: NwkKey)
//uint8_t                             NwkSEncKey_1V1[16];   // JOIN-ACCEPT (derived from: NwkKey)
//uint8_t                             JSIntKey_1V1[16];     // (for OTA devices)
//uint8_t                             JSEncKey_1V1[16];     // (for OTA devices)
  volatile uint8_t                    DLSettings;           // JOIN-ACCEPT
  volatile uint8_t                    RXDelay;              // JOIN-ACCEPT
  volatile uint8_t                    CFList[16];           // JOIN-ACCEPT - EU-868: Freq Ch3[2:0], Freq Ch4[2:0], Freq Ch5[2:0], Freq Ch6[2:0], Freq Ch7[2:0], CFListType[0]
  volatile float                      Ch_Frequencies_MHz[8];
  volatile uint8_t                    Ch_EnabledMsk;

  /* Join Server specific */
//uint8_t                             JoinEUI_1V1[8];       // JOIN-REQUEST, REJOIN-REQUEST
//volatile uint8_t                    JoinNonce_1V1[4];     // JOIN-ACCEPT
//volatile uint8_t                    ServerNonce_1V1[3];

  /* Application specific */
  uint8_t                             AppEUI[8];
  uint8_t                             AppKey[16];           // Application root key (for OTA devices)
  volatile uint8_t                    AppSKey_1V02[16];     // Session key - JOIN-ACCEPT (derived from: AppKey)

  /* Current transmission */
  volatile LoRaWANctxDir_t            Dir;
  volatile uint8_t                    FCtrl_ADR;
  volatile uint8_t                    FCtrl_ADRACKReq;
  volatile uint8_t                    FCtrl_ACK;
  volatile uint8_t                    FCtrl_ClassB;
  volatile uint8_t                    FCtrl_FPending;
  volatile uint8_t                    FPort;
  volatile uint16_t                   ConfFCnt;
  volatile uint8_t                    TxDr;
  volatile uint8_t                    TxCh;

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
  uint8_t                             _noUse[4];
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntUp[4];
  uint8_t                             _pad;
  uint8_t                             idx;
} FRMPayloadBlockA_Up_t;

typedef struct MICBlockB0_Up {
  uint8_t                             variant;
  uint8_t                             _noUse[4];
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
  uint8_t                             _noUse[2];
  uint8_t                             Dir;
  uint8_t                             DevAddr[4];
  uint8_t                             FCntDn[4];
  uint8_t                             _pad;
  uint8_t                             len;
} MICBlockB0_Dn_t;


typedef enum LoRaWAN_CalcMIC_JOINREQUEST {
  MIC_JOINREQUEST                     = 0x01,
  MIC_DATAMESSAGE                     = 0x11,
} LoRaWAN_CalcMIC_JOINREQUEST_t;


typedef struct LoRaWAN_Message {
volatile uint8_t  msg_Len;
volatile uint8_t  msg_Buf[LoRaWAN_MsgLenMax];
uint8_t           msg_MHDR;
uint8_t           msg_FCtrl;
uint16_t          msg_FCnt;
uint8_t           msg_FOpts_Len;
uint8_t           msg_FOpts_Buf[16];
uint8_t           msg_FOpts_Encoded[16];
uint8_t           msg_FRMPayload_Len;
uint8_t           msg_FRMPayload_Encoded[LoRaWAN_FRMPayloadMax];
} LoRaWAN_Message_t;


typedef struct LoraliveApp {

  uint8_t                             voltage_32_v;

  /* Big endian */
  uint8_t                             dust025_10_hi;
  uint8_t                             dust025_10_lo;

  /* Big endian */
  uint8_t                             dust100_10_hi;
  uint8_t                             dust100_10_lo;

  char  id;

  union {
    struct loraliveApp_len6 {
    } l6;

    struct loraliveApp_len8 {
      int8_t                          temperature;
      uint8_t                         humidity_100;
    } l8;

    struct loraliveApp_len14 {
      uint8_t                         latitude_1000_sl24;
      uint8_t                         latitude_1000_sl16;
      uint8_t                         latitude_1000_sl08;
      uint8_t                         latitude_1000_sl00;

      uint8_t                         longitude_1000_sl24;
      uint8_t                         longitude_1000_sl16;
      uint8_t                         longitude_1000_sl08;
      uint8_t                         longitude_1000_sl00;
    } l14;

    struct loraliveApp_len16 {
      int8_t                          temperature;
      uint8_t                         humidity_100;

      uint8_t                         latitude_1000_sl24;
      uint8_t                         latitude_1000_sl16;
      uint8_t                         latitude_1000_sl08;
      uint8_t                         latitude_1000_sl00;

      uint8_t                         longitude_1000_sl24;
      uint8_t                         longitude_1000_sl16;
      uint8_t                         longitude_1000_sl08;
      uint8_t                         longitude_1000_sl00;
    } l16;
  } u;

} LoraliveApp_t;



uint8_t LoRaWAN_calc_randomChannel(LoRaWANctx_t* ctx);
float LoRaWAN_calc_Channel_to_MHz(LoRaWANctx_t* ctx, uint8_t channel, uint8_t dflt);

void LoRaWAN_Init(void);

void LoRaWANctx_readFLASH(void);
void LoRaWANctx_applyKeys_trackMeApp(void);

void LoRaWAN_MAC_JOINREQUEST(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg);

void LoRaWAN_TX_msg(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg);
void LoRaWAN_RX_msg(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg, uint32_t timeout_ms);

void LoRaWAN_App_trackMeApp_pushUp(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg, LoraliveApp_t* app, uint8_t size);
void LoRaWAN_App_trackMeApp_receiveLoop(LoRaWANctx_t* ctx);

#endif /* LORAWAN_H_ */
