/*
 * LoRaWAN.c
 *
 *  Created on: 12.05.2018
 *      Author: DF4IAH
 */

#include "LoRaWAN.h"

#include <string.h>
#include "crypto.h"
#include "crc.h"

#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"

#include "spi.h"


/* SPI communication buffers */
extern uint8_t aSpi1TxBuffer[SPI1_BUFFERSIZE];
extern uint8_t aSpi1RxBuffer[SPI1_BUFFERSIZE];


/* Non-volatile counters in the RTC_Backup domain */
volatile LoRaWANctxBkpRam_t *const LoRaWANctxBkpRam     = (void*) 0x40002850UL;

/* Network context of LoRaWAN */
LoRaWANctx_t loRaWANctx                                 = { 0 };

/* Application data for loralive */
LoraliveApp_t loraliveApp = { 0 };


static void LoRaWAN_FOpts_Encrypt(LoRaWANctx_t* ctx,
    uint8_t* msg_FOpts_Encoded,
    const uint8_t* msg_FOpts_Buf, uint8_t msg_FOpts_Len)
{
  /* Create crypto xor matrix */
  uint8_t* key  = ctx->NwkSEncKey;
  FRMPayloadBlockA_Up_t a_i  = { 0x01U, 0x00000000UL, (uint8_t) ctx->Dir, ctx->DevAddr, ctx->bkpRAM->FCntUp, 0x00, 0x00 };
  uint8_t ecbPad[16] = { 0 };
  memcpy(ecbPad, ((char*) &a_i), msg_FOpts_Len);
  cryptoAesEcb(key, ecbPad);

  /* encode FOpts */
  for (uint8_t idx = 0; idx < msg_FOpts_Len; idx++) {
    msg_FOpts_Encoded[idx] = ecbPad[idx] ^ msg_FOpts_Buf[idx];
  }
}

static uint8_t LoRaWAN_App_loralive_data2FRMPayload(LoRaWANctx_t* ctx,
    uint8_t* payloadEncoded, uint8_t maxLen,
    uint8_t FPort,
    const LoraliveApp_t* app, uint8_t appSize)
{
  uint8_t pad48[48] = { 0 };
  uint8_t len = 0;

  if (appSize <= 48) {
    uint8_t ecbPad[16];

    memcpy((char*) pad48, app, appSize);

    /* Encode application data */
    uint8_t* key  = (FPort > 0) ?  ctx->appSKey : ctx->NwkSEncKey;
    FRMPayloadBlockA_Up_t a_i  = { 0x01U, 0x00000000UL, (uint8_t) ctx->Dir, ctx->DevAddr, ctx->bkpRAM->FCntUp, 0x00, 0x00 };

    uint8_t k = (appSize + 15U) >> 4;
    for (uint8_t i = 1; i <= k; ++i) {
      a_i.idx = i;

      memcpy(ecbPad, ((char*) &a_i) + len, 16);
      cryptoAesEcb(key, ecbPad);

      for (uint8_t idx = 0; idx < 16; ++idx) {
        if ((len + idx) >= appSize) {
          return len + idx;
        }
        payloadEncoded[len + idx] = ecbPad[idx] ^ *(((uint8_t*) &app) + (len + idx));
      }
      len += 16;
    }
  }
  return len;
}

static uint32_t LoRaWAN_calc_MIC(LoRaWANctx_t* ctx, const uint8_t* msg_Buf, uint8_t msg_Len)
{
  uint32_t  mic;
  uint8_t   cmacLen;
  uint8_t   cmacBuf[64];

  if (ctx->Dir == Up) {
    /* Uplink */
    uint8_t cmacF[16];

    MICBlockB0_Up_t b0  = { 0x49U, 0x00000000UL, (uint8_t) Up, ctx->DevAddr, ctx->bkpRAM->FCntUp, 0x00, msg_Len };
    cmacLen = sizeof(MICBlockB0_Up_t);
    memcpy(cmacBuf, (char*) &b0, cmacLen);
    memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
    cmacLen += msg_Len;
    cryptoAesCmac(ctx->FNwkSIntKey, cmacBuf, cmacLen, cmacF);

    if (ctx->LoRaWAN_ver == LoRaWANVersion_10) {
      mic  = ((uint32_t) (cmacF[0])) <<  0;
      mic |= ((uint32_t) (cmacF[1])) <<  8;
      mic |= ((uint32_t) (cmacF[2])) << 16;
      mic |= ((uint32_t) (cmacF[3])) << 24;
      return mic;

    } else if (ctx->LoRaWAN_ver == LoRaWANVersion_11) {
      uint8_t cmacS[16];

      MICBlockB1_Up_t b1 = { 0x49U, ctx->ConfFCnt, ctx->TxDr, ctx->TxCh, (uint8_t) Up, ctx->DevAddr, ctx->bkpRAM->FCntUp, 0x00, msg_Len };
      cmacLen = sizeof(MICBlockB1_Up_t);
      memcpy(cmacBuf, (char*) &b1, cmacLen);
      memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
      cmacLen += msg_Len;
      cryptoAesCmac(ctx->SNwkSIntKey, cmacBuf, cmacLen, cmacS);

      mic  = ((uint32_t) (cmacS[0])) <<  0;
      mic |= ((uint32_t) (cmacS[1])) <<  8;
      mic |= ((uint32_t) (cmacF[0])) <<  0;
      mic |= ((uint32_t) (cmacF[1])) <<  8;
      return mic;
    }

  } else {
    /* Downlink */
    uint8_t cmac[16];

    MICBlockB0_Dn_t b0  = { 0x49U, (uint16_t) ctx->ConfFCnt, (uint16_t) 0x0000U, (uint8_t) Dn, ctx->DevAddr, ctx->bkpRAM->AFCntDwn, 0x00, msg_Len };
    cmacLen = sizeof(MICBlockB0_Dn_t);
    memcpy(cmacBuf, (char*) &b0, cmacLen);
    memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
    cmacLen += msg_Len;
    cryptoAesCmac(ctx->SNwkSIntKey, cmacBuf, cmacLen, cmac);

    mic  = ((uint32_t) (cmac[0])) <<  0;
    mic |= ((uint32_t) (cmac[1])) <<  8;
    mic |= ((uint32_t) (cmac[2])) << 16;
    mic |= ((uint32_t) (cmac[3])) << 24;
    return mic;
  }

  /* Should not happen */
  Error_Handler();
  return 0UL;
}


void LoRaWAN_Init(void)
{
  const uint8_t bkpRAMLen = &LoRaWANctxBkpRam->_end - &LoRaWANctxBkpRam->LoRaWANcrc;
  loRaWANctx.bkpRAM = LoRaWANctxBkpRam;

  // Check CRC
  uint32_t crcC = crcCalc((const uint32_t*) ((&LoRaWANctxBkpRam->LoRaWANcrc) + 1), bkpRAMLen - 1);
  if (crcC != LoRaWANctxBkpRam->LoRaWANcrc) {
    /* Non valid content - reset all to zero */
    volatile uint32_t* ptr = &LoRaWANctxBkpRam->LoRaWANcrc;
    for (uint8_t idx = 1; idx < bkpRAMLen; idx++) {
      *++ptr = 0UL;
    }

    /* Calc new CRC */
    LoRaWANctxBkpRam->LoRaWANcrc = crcCalc((const uint32_t*) ((&LoRaWANctxBkpRam->LoRaWANcrc) + 1), bkpRAMLen - 1);
  }

  /* Setup data from FLASH NVM */
  LoRaWANctx_readFLASH();
}

void LoRaWANctx_readFLASH(void)
{
  /* TODO: read from FLASH NVM instead of default settings */

  /* Apply keys of the loralive App */
  LoRaWANctx_applyKeys_loralive();
}

void LoRaWANctx_applyKeys_loralive(void)
{
  const uint8_t  DevEUI_LE[8]         = { 0x31U, 0x6FU, 0x72U, 0x65U, 0x70U, 0x73U, 0x65U, 0x00U };
  memcpy(&(loRaWANctx.DevEUI), &DevEUI_LE, sizeof(DevEUI_LE));

  const uint32_t DevAddr              = 0x26011E42;
  loRaWANctx.DevAddr = DevAddr;

  const uint8_t  appEUI_LE[8]         = { 0x00U, 0x86U, 0x00U, 0xD0U, 0x7EU, 0xD5U, 0xB3U, 0x70U };
  memcpy(&(loRaWANctx.appEUI), &appEUI_LE, sizeof(appEUI_LE));

  const uint8_t  FNwkSIntKey_LE[16]   = { 0x11U, 0x52U, 0x2FU, 0x2BU, 0x19U, 0xB2U, 0x62U, 0x04U, 0x81U, 0xF9U, 0x9BU, 0x67U, 0xFFU, 0xE7U, 0x86U, 0x43U };
  //const uint8_t  nwkSKey_BE[16]     = { 0x43U, 0x86U, 0xE7U, 0xFFU, 0x67U, 0x9BU, 0xF9U, 0x81U, 0x04U, 0x62U, 0xB2U, 0x19U, 0x2BU, 0x2FU, 0x52U, 0x11U };
  memcpy(&(loRaWANctx.FNwkSIntKey), &FNwkSIntKey_LE, sizeof(FNwkSIntKey_LE));
  memcpy(&(loRaWANctx.SNwkSIntKey), &FNwkSIntKey_LE, sizeof(FNwkSIntKey_LE));
  memcpy(&(loRaWANctx.NwkSEncKey),  &FNwkSIntKey_LE, sizeof(FNwkSIntKey_LE));

  const uint8_t  appSKey_LE[16]       = { 0xF5U, 0x7DU, 0x44U, 0x69U, 0x7EU, 0x1EU, 0x22U, 0x3EU, 0x32U, 0x46U, 0xD7U, 0xB4U, 0xD9U, 0x9AU, 0xDAU, 0xADU };
  //const uint8_t  appSKey_BE[16]     = { 0xADU, 0xDAU, 0x9AU, 0xD9U, 0xB4U, 0xD7U, 0x46U, 0x32U, 0x3EU, 0x22U, 0x1EU, 0x7EU, 0x69U, 0x44U, 0x7DU, 0xF5U };
  memcpy(&(loRaWANctx.appSKey), &appSKey_LE, sizeof(appSKey_LE));

  /* Current transmission state */
  loRaWANctx.LoRaWAN_ver              = LoRaWANVersion_10;    // TheThingsNetwork - assigned codes to this device - sufficient for R1.0 [LW10, LW102]
  loRaWANctx.Dir                      = Up;
  loRaWANctx.FCtrl_ADR                = 0U;
  loRaWANctx.FCtrl_ADRACKReq          = 0U;
  loRaWANctx.FCtrl_ACK                = 0U;
  loRaWANctx.FCtrl_ClassB             = 0U;
  loRaWANctx.FCtrl_FPending           = 0U;
  loRaWANctx.FPort                    = 1U;   // Default App port
  loRaWANctx.ConfFCnt                 = 0U;
  loRaWANctx.TxDr                     = 0U;
  loRaWANctx.TxCh                     = 0U;
}

void LoRaWAN_App_loralive_pushUp(LoRaWANctx_t* ctx, uint8_t FPort, LoraliveApp_t* app, uint8_t size)
{
  volatile uint8_t   msg_Len;
  volatile uint8_t   msg_Buf[LoRaWAN_MsgLenMax]                      = { 0 };
  uint8_t   msg_MHDR;
  uint8_t   msg_FPort;
  volatile uint32_t  msg_MIC;
  uint32_t  msg_DevAddr;
  uint8_t   msg_FCtrl;
  uint16_t  msg_FCnt;
  uint8_t   msg_FOpts_Len;
  uint8_t   msg_FOpts_Buf[16]                               = { 0 };
  uint8_t   msg_FOpts_Encoded[16]                           = { 0 };
  uint8_t   msg_FRMPayload_Len;
  uint8_t   msg_FRMPayload_Encoded[LoRaWAN_FRMPayloadMax]   = { 0 };

  /* PHY Payload: MHDR | MACPayload | MIC */
  /* MHDR */
  msg_MHDR = (((uint8_t) UnconfDataUp) << LoRaWAN_MHDR_MType_SL) | (((uint8_t) LoRaWAN_R1) << LoRaWAN_MHDR_Major_SL);

  /* MACPayload: FHDR | FPort | FRMPayload */
  {
    /* FHDR: DevAddr | FCtrl | FCnt | FOpts */
    {
      /* DevAddr */
      msg_DevAddr = ctx->DevAddr;

      /* FOpts */
      // LoRaWAN V1.1 spec @p64: send ResetInd MAC for the first time in the FOpt field (after reset)
      // each packet has to set that, until a response with ResetInd MAC comes
      msg_FOpts_Len = 0U;
      msg_FOpts_Buf[msg_FOpts_Len++] = (uint8_t) ResetInd_UP;
      LoRaWAN_FOpts_Encrypt(ctx,
          msg_FOpts_Encoded,
          msg_FOpts_Buf, msg_FOpts_Len);

      /* FCtrl */
      if (ctx->Dir == Up) {
        msg_FCtrl = (ctx->FCtrl_ADR         << LoRaWAN_FCtl_ADR_SL      ) |
                    (ctx->FCtrl_ADRACKReq   << LoRaWAN_FCtl_ADRACKReq_SL) |
                    (ctx->FCtrl_ACK         << LoRaWAN_FCtl_ACK_SL      ) |
                    (ctx->FCtrl_ClassB      << LoRaWAN_FCtl_ClassB_SL   ) |
                    (msg_FOpts_Len          << LoRaWAN_FCtl_FOptsLen_SL );

      } else /* if (ctx->Dir == Dn) */ {
        msg_FCtrl = (ctx->FCtrl_ADR         << LoRaWAN_FCtl_ADR_SL      ) |
                    (ctx->FCtrl_ACK         << LoRaWAN_FCtl_ACK_SL      ) |
                    (ctx->FCtrl_FPending    << LoRaWAN_FCtl_FPending_SL ) |
                    (msg_FOpts_Len          << LoRaWAN_FCtl_FOptsLen_SL );
      }

      /* FCnt */
      msg_FCnt = (uint16_t) ctx->bkpRAM->FCntUp;
    }

    /* FPort */
    msg_FPort = ctx->FPort;

    /* FRMPayload */
    msg_FRMPayload_Len = LoRaWAN_App_loralive_data2FRMPayload(ctx,
        msg_FRMPayload_Encoded, LoRaWAN_FRMPayloadMax,
        msg_FPort,
        app, size);
  }

  /* Message sequencer part 1 */
  {
    /* MHDR */
    msg_Len               = 0U;
    msg_Buf[msg_Len++]    = msg_MHDR;

    /* DevAddr */
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_DevAddr >>  0) & 0xffU);
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_DevAddr >>  8) & 0xffU);
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_DevAddr >> 16) & 0xffU);
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_DevAddr >> 24) & 0xffU);

    /* FCtrl */
    msg_Buf[msg_Len++]    = msg_FCtrl;

    /* FCnt */
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_FCnt >>  0) & 0xffU);
    msg_Buf[msg_Len++]    = (uint8_t) ((msg_FCnt >>  8) & 0xffU);

    /* FOpts (encoded) */
    for (uint8_t FOpts_Idx = 0; FOpts_Idx < msg_FOpts_Len; FOpts_Idx++) {
      msg_Buf[msg_Len++]  = msg_FOpts_Encoded[FOpts_Idx];
    }

    /* Drop this part when no FRMPayload exists */
    if (msg_FRMPayload_Len) {
      /* FPort */
      msg_Buf[msg_Len++]    = msg_FPort;

      /* FRMPayload */
      for (uint8_t FRMPayload_Idx = 0; FRMPayload_Idx < msg_FRMPayload_Len; FRMPayload_Idx++) {
        msg_Buf[msg_Len++]  = msg_FRMPayload_Encoded[FRMPayload_Idx];
      }
    }
  }

  /* MIC */
  msg_MIC = LoRaWAN_calc_MIC(ctx, (uint8_t *) msg_Buf, msg_Len);

  /* Message composition part 2 */
  {
    msg_Buf[msg_Len++]  = (uint8_t) ((msg_MIC >>  0) & 0xffU);
    msg_Buf[msg_Len++]  = (uint8_t) ((msg_MIC >>  8) & 0xffU);
    msg_Buf[msg_Len++]  = (uint8_t) ((msg_MIC >> 16) & 0xffU);
    msg_Buf[msg_Len++]  = (uint8_t) ((msg_MIC >> 24) & 0xffU);
  }

  /* Push the complete message to the FIFO */
  {
    /* Change to STANDBY mode */
    spiSX1272Mode(STANDBY);

    /* Prepare the FIFO */
    spiSX1272LoRa_Fifo_Init();
    spiSX1272LoRa_Fifo_TxSetToBasePtr();

    /* Push the message to the FIFO */
    uint8_t fifoCmd = SPI_WR_FLAG | 0x00;
    memcpy(aSpi1TxBuffer, &fifoCmd, 1);
    memcpy(aSpi1TxBuffer + 1, (char*) msg_Buf, msg_Len);
    spiProcessSpiMsg(1 + msg_Len);

    /* Set message length */
    spiSX1272LoRa_setTxMsgLen(msg_Len);
  }

  /* Enable the transmitter and wait for TX_Done */
  {
    spiSX1272Mode(TX);

    do {
      aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x12;    // RegIrqFlags
      spiProcessSpiMsg(2);
      uint8_t irq = aSpi1RxBuffer[1];

      if (!irq) {
        osDelay(1);

      } else {
        aSpi1TxBuffer[0] = SPI_WR_FLAG | 0x12;    // RegIrqFlags
        aSpi1TxBuffer[1] = irq;
        spiProcessSpiMsg(2);

        aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x18;    // RegModemStat
        spiProcessSpiMsg(2);
        volatile uint8_t modemStat = aSpi1RxBuffer[1];

        aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x2c;    // RegRssiWideband
        spiProcessSpiMsg(2);
        volatile uint8_t rssiWideband = aSpi1RxBuffer[1];

        aSpi1TxBuffer[0] = SPI_RD_FLAG | 0x14;    // RegIrqFlags
        spiProcessSpiMsg(5);
        volatile uint8_t rxHeaderCnt   = ((uint16_t)aSpi1RxBuffer[1] << 8) | aSpi1RxBuffer[2];
        volatile uint8_t rxValidPktCnt = ((uint16_t)aSpi1RxBuffer[3] << 8) | aSpi1RxBuffer[4];

        (void) modemStat;
        (void) rssiWideband;
        (void) rxHeaderCnt;
        (void) rxValidPktCnt;

        /* Turn off TRX */
        spiSX1272Mode(SLEEP);

        break;
      }
    } while (1);
  }
}


#if 0
static void template(void)
{
  /* Application: loralive */
  uint8_t ttnMsg[52] = { 0 };
  uint8_t payload[16] = { 0 };
  uint32_t latitude_1000  = 49473;  // 49473182
  uint32_t longitude_1000 =  8615;  // 8614814

  payload[ 0] = (uint8_t) (3.3f * 32 + 0.5);  // Voltage
  payload[ 5] = (uint8_t) 'E';  // ID
  payload[ 6] = (uint8_t) 22;  // Temperature
  payload[ 7] = (uint8_t) 50;  // Humidity
  payload[ 8] = (uint8_t) ((latitude_1000  >> 24) & 0xffUL);
  payload[ 9] = (uint8_t) ((latitude_1000  >> 16) & 0xffUL);
  payload[10] = (uint8_t) ((latitude_1000  >>  8) & 0xffUL);
  payload[11] = (uint8_t) ((latitude_1000  >>  0) & 0xffUL);
  payload[12] = (uint8_t) ((longitude_1000 >> 24) & 0xffUL);
  payload[13] = (uint8_t) ((longitude_1000 >> 16) & 0xffUL);
  payload[14] = (uint8_t) ((longitude_1000 >>  8) & 0xffUL);
  payload[15] = (uint8_t) ((longitude_1000 >>  0) & 0xffUL);

  int pos = sprintf((char*) ttnMsg, "%s", sendPayload);
  for (uint8_t idx = 0; idx < 16; ++idx) {
    pos += sprintf((char*) ttnMsg + pos, "%02X", payload[idx]);
  }

  __asm volatile( "nop" );
}
#endif

#if 0
static void cypherChecks(void)
{
#ifdef TEST1
  const uint8_t key_ecb[16]   = { 0x2bU, 0x7eU, 0x15U, 0x16U, 0x28U, 0xaeU, 0xd2U, 0xa6U, 0xabU, 0xf7U, 0x15U, 0x88U, 0x09U, 0xcfU, 0x4fU, 0x3cU };
  uint8_t pad128_ecb[16]      = { 0x6bU, 0xc1U, 0xbeU, 0xe2U, 0x2eU, 0x40U, 0x9fU, 0x96U, 0xe9U, 0x3dU, 0x7eU, 0x11U, 0x73U, 0x93U, 0x17U, 0x2aU };

  cryptoAesEcb(key_ecb, pad128_ecb);
#endif

  const uint8_t key_cmac[16]  = { 0x2bU, 0x7eU, 0x15U, 0x16U, 0x28U, 0xaeU, 0xd2U, 0xa6U, 0xabU, 0xf7U, 0x15U, 0x88U, 0x09U, 0xcfU, 0x4fU, 0x3cU };
  uint8_t out128_cmac[16]     = { 0 };
#ifdef TEST2
  uint8_t in128_cmac1[16]     = { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
  cryptoAesCmac(key_cmac, in128_cmac1, 0, out128_cmac);
#endif
#ifdef TEST3
  uint8_t in128_cmac2[16]     = { 0x6bU, 0xc1U, 0xbeU, 0xe2U, 0x2eU, 0x40U, 0x9fU, 0x96U, 0xe9U, 0x3dU, 0x7eU, 0x11U, 0x73U, 0x93U, 0x17U, 0x2aU };
  cryptoAesCmac(key_cmac, in128_cmac2, 16, out128_cmac);
#endif
#ifdef TEST4
  uint8_t in128_cmac3[40]     = { 0x6bU, 0xc1U, 0xbeU, 0xe2U, 0x2eU, 0x40U, 0x9fU, 0x96U, 0xe9U, 0x3dU, 0x7eU, 0x11U, 0x73U, 0x93U, 0x17U, 0x2aU,
                                  0xaeU, 0x2dU, 0x8aU, 0x57U, 0x1eU, 0x03U, 0xacU, 0x9cU, 0x9eU, 0xb7U, 0x6fU, 0xacU, 0x45U, 0xafU, 0x8eU, 0x51U,
                                  0x30U, 0xc8U, 0x1cU, 0x46U, 0xa3U, 0x5cU, 0xe4U, 0x11U };
  cryptoAesCmac(key_cmac, in128_cmac3, 40, out128_cmac);
#endif
}
#endif
