/*
 * LoRaWAN.c
 *
 *  Created on: 12.05.2018
 *      Author: DF4IAH
 */

#include "LoRaWAN.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "crypto.h"
#include "crc.h"

#include "FreeRTOS.h"
#include "stm32l496xx.h"
#include "cmsis_os.h"
#include "stm32l4xx_nucleo_144.h"
#include "stm32l4xx_hal.h"

#include "spi.h"
#include "usb.h"
#include "adc.h"


/* Holds RTOS timing info */
extern uint32_t spiPreviousWakeTime;

/* SPI communication buffers */
extern uint8_t            spi1TxBuffer[SPI1_BUFFERSIZE];
extern uint8_t            spi1RxBuffer[SPI1_BUFFERSIZE];

extern osSemaphoreId      usbToHostBinarySemHandle;


#ifdef USE_ABP

/* TheThingsNetwork - assigned codes to this device - sufficient for R1.0 [LW10, LW102] */
//const char *TTNdevAddr = "26011E42";
//const char *TTNnwkSKey = "4386E7FF679BF9810462B2192B2F5211";
//const char *TTNappSKey = "ADDA9AD9B4D746323E221E7E69447DF5";

const uint8_t  DevAddr_LE[4]		                        = { 0x3FU, 0x1AU, 0x01U, 0x26U };
const uint8_t  DevEUI_LE[8]                             = { 0x31U, 0x6FU, 0x72U, 0x65U, 0x70U, 0x73U, 0x65U, 0x00U };
const uint8_t  JoinEUI_LE[8]                             = { 0x00U, 0x86U, 0x00U, 0xD0U, 0x7EU, 0xD5U, 0xB3U, 0x70U };
const uint8_t  NwkSKey_BE[16]                           = { 0x43U, 0x86U, 0xE7U, 0xFFU, 0x67U, 0x9BU, 0xF9U, 0x81U, 0x04U, 0x62U, 0xB2U, 0x19U, 0x2BU, 0x2FU, 0x52U, 0x11U };
const uint8_t  AppSKey_BE[16]                           = { 0xADU, 0xDAU, 0x9AU, 0xD9U, 0xB4U, 0xD7U, 0x46U, 0x32U, 0x3EU, 0x22U, 0x1EU, 0x7EU, 0x69U, 0x44U, 0x7DU, 0xF5U };

#else

/* USE_OTA */

/* TheThingsNetwork - assigned codes to this device - R1.1 */
// Device ID   findmesat2_001
const uint8_t  DevEUI_LE[8]                             = { 0x31, 0x30, 0x30, 0x5F, 0x32, 0x53, 0x4D, 0x46 };  // "FMS2_001"
const uint8_t  AppEUI_LE[8]                             = { 0x08, 0xF6, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
//const uint8_t  JoinEUI_LE[8]                          = { 0x08, 0xF6, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };  // V1.1: former AppEUI
const uint8_t  NwkKey_BE[8]                             = { 0 };   // Since LoRaWAN V1.1
const uint8_t  AppKey_BE[16]                            = { 0x01, 0xE3, 0x27, 0x88, 0xBA, 0x99, 0x2C, 0x45, 0x6D, 0x92, 0xBF, 0xE0, 0xEE, 0xAD, 0xBE, 0x45 };

#endif


/* Non-volatile counters in the RTC_Backup domain */
volatile LoRaWANctxBkpRam_t *const LoRaWANctxBkpRam     = (void*) 0x40002850UL;

/* Network context of LoRaWAN */
LoRaWANctx_t loRaWANctx                                 = { 0 };

/* Message buffers */
LoRaWAN_Message_t loRaWanTxMsg                          = { 0 };
LoRaWAN_Message_t loRaWanRxMsg                          = { 0 };

/* Application data for loralive */
LoraliveApp_t loraliveApp                               = { 0 };


inline
uint8_t GET_BYTE_OF_WORD(uint32_t word, uint8_t pos)
{
  return (uint8_t) ((word >> (8 << pos)) & 0x000000ffUL);
}


static void LoRaWAN_FOpts_Encrypt(LoRaWANctx_t* ctx,
    uint8_t* msg_FOpts_Encoded,
    const uint8_t* msg_FOpts_Buf, uint8_t msg_FOpts_Len)
{
#if 0
  /* Short way out when nothing to do */
  if (!msg_FOpts_Len) {
    return;
  }

  /* Create crypto xor matrix */
  uint8_t* key  = ctx->NwkSEncKey;

  FRMPayloadBlockA_Up_t a_i  = {
    0x01U,
    { 0x00U, 0x00U, 0x00U, 0x00U },
    (uint8_t) ctx->Dir,
    { ctx->DevAddr[0],
      ctx->DevAddr[1],
      ctx->DevAddr[2],
      ctx->DevAddr[3] },
    { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3) },
    0x00,
    0x00
  };

  /* Create crypto modulator */
  uint8_t ecbPad[16] = { 0 };
  memcpy(ecbPad, ((char*) &a_i), msg_FOpts_Len);
  cryptoAesEcb(key, ecbPad);

  /* Encode FOpts */
  for (uint8_t idx = 0; idx < msg_FOpts_Len; idx++) {
    msg_FOpts_Encoded[idx] = ecbPad[idx] ^ msg_FOpts_Buf[idx];
  }
#endif
}

static uint8_t LoRaWAN_App_loralive_data2FRMPayload(LoRaWANctx_t* ctx,
    uint8_t* payloadEncoded, uint8_t maxLen,
    const LoraliveApp_t* app)
{
#if 0
  uint8_t payload[48] = { 0 };
  uint8_t len;

  {
    /* Forge a byte sequence out of the App */
    {
      /* Application: loralive */
      const uint32_t latitude_1000  = 49473;  // 49473182
      const uint32_t longitude_1000 =  8615;  // 8614814
      const int8_t   temperature    = 20;
      const uint8_t  humidity       = 50;
      const uint16_t dust025        = 0U;
      const uint16_t dust100        = 0U;

      len             = 0U;
      payload[len++]  = (uint8_t) (3.3f * 32 + 0.5);  // Voltage
      payload[len++]  = (uint8_t) ((dust025 >> 8) & 0x00ffU);   // Dust025 HI
      payload[len++]  = (uint8_t) ((dust025 >> 0) & 0x00ffU);   // Dust025 LO
      payload[len++]  = (uint8_t) ((dust100 >> 8) & 0x00ffU);   // Dust100 HI
      payload[len++]  = (uint8_t) ((dust100 >> 0) & 0x00ffU);   // Dust100 LO
      payload[len++]  = (uint8_t) 'E';  // ID
      payload[len++]  = (uint8_t) temperature;
      payload[len++]  = (uint8_t) humidity;
      payload[len++]  = (uint8_t) ((latitude_1000  >> 24) & 0xffUL);
      payload[len++]  = (uint8_t) ((latitude_1000  >> 16) & 0xffUL);
      payload[len++]  = (uint8_t) ((latitude_1000  >>  8) & 0xffUL);
      payload[len++]  = (uint8_t) ((latitude_1000  >>  0) & 0xffUL);
      payload[len++]  = (uint8_t) ((longitude_1000 >> 24) & 0xffUL);
      payload[len++]  = (uint8_t) ((longitude_1000 >> 16) & 0xffUL);
      payload[len++]  = (uint8_t) ((longitude_1000 >>  8) & 0xffUL);
      payload[len++]  = (uint8_t) ((longitude_1000 >>  0) & 0xffUL);
    }

    /* Calculate the number of blocks needed */
    const uint8_t blocks = (len + 15U) >> 4;

    /* Leave when target buffer is too small */
    if ((blocks << 4) > maxLen) {
      return 0;
    }

    /* Encode application data */
    if (blocks) {
      const uint8_t* key = (ctx->FPort > 0) ?  ctx->AppSKey : ctx->NwkSEncKey;

      FRMPayloadBlockA_Up_t a_i  = {
        0x01U,
        { 0x00U, 0x00U, 0x00U, 0x00U },
        (uint8_t) ctx->Dir,
        { ctx->DevAddr[0],
          ctx->DevAddr[1],
          ctx->DevAddr[2],
          ctx->DevAddr[3] },
        { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3) },
        0x00,
        0x00    // This value is to be overwritten
        };

      /* Process all blocks */
      {
        uint8_t blockPos = 0U;

        for (uint8_t i = 1; i <= blocks; i++) {
          uint8_t ecbPad[16]  = { 0U };

          a_i.idx = i;

          /* Create crypto modulator */
          memcpy(ecbPad, ((char*) &a_i), 16);
          cryptoAesEcb(key, ecbPad);

          /* Forge encoded data */
          for (uint8_t idx = 0; idx < 16; idx++) {
            payloadEncoded[blockPos + idx] = ecbPad[idx] ^ payload[blockPos + idx];

            /* Cut block on message size */
            if ((blockPos + idx) >= (len - 1)) {
              return len;
            }
          }

          blockPos += 16U;
        }
      }
    }
  }
#endif
  return 0;
}

float LoRaWAN_calc_Channel_to_MHz(LoRaWANctx_t* ctx, uint8_t channel, uint8_t dflt)
{
  /* EU863-870*/
  float mhz = 0.f;

  switch (channel) {
  case 1:
    mhz = 868.1f;   // SF7BW125 to SF12BW125
    break;

  case 2:
    mhz = 868.3f;   // SF7BW125 to SF12BW125  and  SF7BW250
    break;

  case 3:
    mhz = 868.5f;   // SF7BW125 to SF12BW125
    break;

  case 4:
    mhz = 867.1f;   // SF7BW125 to SF12BW125
    break;

  case 5:
    mhz = 867.3f;   // SF7BW125 to SF12BW125
    break;

  case 6:
    mhz = 867.5f;   // SF7BW125 to SF12BW125
    break;

  case 7:
    mhz = 867.7f;   // SF7BW125 to SF12BW125
    break;

  case 8:
    mhz = 867.9f;   // SF7BW125 to SF12BW125
    break;

  case 9:
    mhz = 868.8f;   // FSK
    break;

  case  0:
  case 16:
    mhz = 869.525f; // RX2 channel
    break;

  default:
    Error_Handler();
  }

  /* Current channel list */
  if (!dflt) {
    if ((1UL << channel) & ctx->Ch_EnabledMsk) {
      return ctx->Ch_Frequencies_MHz[channel - 1];
    }
  }

  return mhz;
}



static void LoRaWAN_calc_MIC_msgAppend(LoRaWAN_Message_t* msg, LoRaWANctx_t* ctx, LoRaWAN_CalcMIC_JOINREQUEST_t variant)
{
  uint8_t* l_NwkKey;

#ifdef LORAWAN_1V02
    l_NwkKey = (uint8_t*) AppKey_BE;
#elif LORAWAN_1V1
    l_NwkKey = (uint8_t*) NwkKey_BE;
#endif

  switch (variant) {
  case MIC_JOINREQUEST:
  {
    uint8_t cmac[16];
    uint8_t i;

    // V1.01: cmac = aes128_cmac(AppKey, MHDR | AppEUI  | DevEUI | DevNonce)
    // V1.1:  cmac = aes128_cmac(NwkKey, MHDR | JoinEUI | DevEUI | DevNonce)
    // MIC = cmac[0..3]

    /* msg contains data for CMAC hashing already */
    cryptoAesCmac(l_NwkKey, msg->msg_Buf,
        sizeof(msg->msg_MHDR) + sizeof(ctx->AppEUI) + sizeof(ctx->DevEUI) + sizeof(ctx->DevNonce),
        cmac);

    /* MIC to buffer */
    for (i = 0; i < 4; i++) {
      msg->msg_Buf[msg->msg_Len++] = cmac[i];
    }
  }
  break;

  case MIC_DATAMESSAGE:
  {
#if 0
  uint8_t   cmacLen;
  uint8_t   cmacBuf[64] = { 0 };

  if (ctx->Dir == Up) {
    /* Uplink */

    MICBlockB0_Up_t b0 = {
      0x49U,
      { 0x00U, 0x00, 0x00U, 00U },
      (uint8_t) Up,
      { ctx->DevAddr[0],
        ctx->DevAddr[1],
        ctx->DevAddr[2],
        ctx->DevAddr[3] },
      { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
        GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
        GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
        GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3) },
      0x00,
      msg_Len
    };

    /* Concatenate b0 and message */
    cmacLen = sizeof(MICBlockB0_Up_t);
    memcpy(cmacBuf, (char*) &b0, cmacLen);
    memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
    cmacLen += msg_Len;

    uint8_t cmacF[16];
    cryptoAesCmac(ctx->FNwkSIntKey, cmacBuf, cmacLen, cmacF);

    if (ctx->LoRaWAN_ver == LoRaWANVersion_10) {
      uint8_t idx = msg_Len;
      msg_Buf[idx++] = cmacF[0];
      msg_Buf[idx++] = cmacF[1];
      msg_Buf[idx++] = cmacF[2];
      msg_Buf[idx++] = cmacF[3];
      return;

    } else if (ctx->LoRaWAN_ver == LoRaWANVersion_11) {
      MICBlockB1_Up_t b1 = {
        0x49U,
        { GET_BYTE_OF_WORD(ctx->ConfFCnt, 0),
          GET_BYTE_OF_WORD(ctx->ConfFCnt, 1) },
        ctx->TxDr,
        ctx->TxCh,
        (uint8_t) Up,
        { ctx->DevAddr[0],
          ctx->DevAddr[1],
          ctx->DevAddr[2],
          ctx->DevAddr[3] },
        { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
          GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3) },
        0x00,
        msg_Len
      };

      /* Concatenate b1 and message */
      cmacLen = sizeof(MICBlockB1_Up_t);
      memcpy(cmacBuf, (char*) &b1, cmacLen);
      memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
      cmacLen += msg_Len;

      uint8_t cmacS[16];
      cryptoAesCmac(ctx->SNwkSIntKey, cmacBuf, cmacLen, cmacS);

      uint8_t idx = msg_Len;
      msg_Buf[idx++] = cmacS[0];
      msg_Buf[idx++] = cmacS[1];
      msg_Buf[idx++] = cmacF[0];
      msg_Buf[idx++] = cmacF[1];
      return;
    }

  } else {
    /* Downlink */

    MICBlockB0_Dn_t b0  = {
      0x49U,
      { GET_BYTE_OF_WORD(ctx->ConfFCnt, 0),
        GET_BYTE_OF_WORD(ctx->ConfFCnt, 1) },
      { 0x00U, 0x00U },
      (uint8_t) Dn,
      { ctx->DevAddr[0],
        ctx->DevAddr[1],
        ctx->DevAddr[2],
        ctx->DevAddr[3] },
      { GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 0),
        GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 1),
        GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 2),
        GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 3) },
      0x00,
      msg_Len
    };

    /* Concatenate b0 and message */
    cmacLen = sizeof(MICBlockB0_Dn_t);
    memcpy(cmacBuf, (char*) &b0, cmacLen);
    memcpy(cmacBuf + cmacLen, msg_Buf, msg_Len);
    cmacLen += msg_Len;

    uint8_t cmac[16];
    cryptoAesCmac(ctx->SNwkSIntKey, cmacBuf, cmacLen, cmac);

    uint8_t idx = msg_Len;
    msg_Buf[idx++] = cmac[0];
    msg_Buf[idx++] = cmac[1];
    msg_Buf[idx++] = cmac[2];
    msg_Buf[idx++] = cmac[3];
    return;
  }

  /* Should not happen */
  Error_Handler();
#endif
  }
  break;

  default:
  {
    // Nothing to do
  }

  }  // switch ()
}


void LoRaWAN_Init(void)
{
  const uint8_t bkpRAMLen = &LoRaWANctxBkpRam->_end - &LoRaWANctxBkpRam->LoRaWANcrc;

  /* Seed randomizer */
  {
    uint32_t r = 0x12345678U;
    r ^= getRunTimeCounterValue();
    r ^= adcGetTemp_100();
    r ^= adcGetVdda_mV();
    srand(r);
  }

  // Prepare LoRaWAN context
  memset(&loRaWANctx, 0, sizeof(loRaWANctx));
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

  /* Copy default channel settings */
  for (uint8_t ch = 1; ch <= 8; ch++) {
    loRaWANctx.Ch_Frequencies_MHz[ch - 1]  = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, ch, 1);
  }
  loRaWANctx.Ch_EnabledMsk = 0xff;  // All channels valid

  /* JOIN-REQUEST request */
  LoRaWAN_MAC_JOINREQUEST(&loRaWANctx, &loRaWanTxMsg);
  LoRaWAN_TX_msg(&loRaWANctx, &loRaWanTxMsg);

  /* JOIN-ACCEPT response*/
  LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg, 5750);
  spiSX1272Frequency_MHz(LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, 16, 1));  // Switch to RX2
  LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg, 1000);

  __asm volatile( "nop" );
}

void LoRaWANctx_readFLASH(void)
{
  /* TODO: read from FLASH NVM instead of default settings */

  /* Apply keys of the track_me App */
  LoRaWANctx_applyKeys_trackMeApp();
}

void LoRaWANctx_applyKeys_trackMeApp(void)
{
  memcpy(loRaWANctx.DevEUI,     &DevEUI_LE, sizeof(DevEUI_LE));
  memcpy(loRaWANctx.AppEUI,     &AppEUI_LE, sizeof(AppEUI_LE));
//memcpy(&(loRaWANctx.JoinEUI), &JoinEUI_LE, sizeof(JoinEUI_LE));
  memcpy(loRaWANctx.AppKey,     &AppKey_BE, sizeof(AppKey_BE));

  /* Current transmission state */
  loRaWANctx.LoRaWAN_ver              = LoRaWANVersion_10;    // TheThingsNetwork - assigned codes to this device
  loRaWANctx.Dir                      = Up;
  loRaWANctx.FPort                    = 1U;
}

void LoRaWAN_MAC_JOINREQUEST(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg)
{
  uint8_t i;

  /* Start new message */
  memset(msg, 0, sizeof(LoRaWAN_Message_t));

  /* MHDR */
  msg->msg_MHDR = (((uint8_t) JoinRequest) << LoRaWAN_MHDR_MType_SL) | (((uint8_t) LoRaWAN_R1) << LoRaWAN_MHDR_Major_SL);
  msg->msg_Buf[msg->msg_Len++] = msg->msg_MHDR;

  /* AppEUI[0:7]  (JoinEUI[0:7]) */
  for (i = 0; i < 8; i++) {
    msg->msg_Buf[msg->msg_Len++] = ctx->AppEUI[i];
  }

  /* DevEUI[0:7] */
  for (i = 0; i < 8; i++) {
    msg->msg_Buf[msg->msg_Len++] = ctx->DevEUI[i];
  }

  /* DevNonce */
#ifdef LORAWAN_1V02
  ctx->DevNonce[0] = rand() & 0xffU;
  ctx->DevNonce[1] = rand() & 0xffU;
#endif

  msg->msg_Buf[msg->msg_Len++] = ctx->DevNonce[0];
  msg->msg_Buf[msg->msg_Len++] = ctx->DevNonce[1];

  /* MIC */
  LoRaWAN_calc_MIC_msgAppend(msg, ctx, MIC_JOINREQUEST);

#ifdef LORAWAN_1V1
  if (!(++ctx->DevNonce[0])) {
    ++ctx->DevNonce[1];
  }
#endif
}

void LoRaWAN_TX_msg(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg)
{
  /* Push the complete message to the FIFO and go to transmission mode */
  static uint8_t s_channel = 255;
  uint8_t channel;

  do {
    channel = 1 + (rand() % 8);
    if (!((1UL << channel) & ctx->Ch_EnabledMsk)) {
      /* Channel disabled, try another channel */
      channel = s_channel;
    }
  } while (channel == s_channel);
  s_channel = channel;

  /* Get the current frequency for the channel number [1..8] */
  float frequency = LoRaWAN_calc_Channel_to_MHz(ctx, channel, 0);

  /* Prepare TX for a randomly selected channel (TX1/RX1) */
  spiSX1272Mode_LoRa_TX_Preps(frequency, msg->msg_Len);

  /* Prepare the FIFO */
  spiSX1272LoRa_Fifo_Init();
  spiSX1272LoRa_Fifo_SetTxBaseToFifoPtr();

  /* Push the message to the FIFO */
  uint8_t fifoCmd = SPI_WR_FLAG | 0x00;
  memcpy(spi1TxBuffer, &fifoCmd, 1);
  memcpy(spi1TxBuffer + 1, (char*) msg->msg_Buf, msg->msg_Len);
  spiProcessSpiMsg(1 + msg->msg_Len);

  /* Transmit FIFO content */
  {
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);    // Red on

    /* Start transmission */
    spiSX1272Mode_LoRa_TX_Run();

    /* Wait until the message is being sent */
    spiSX1272_WaitUntil_TxDone(1);
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);  // Red off
  }
}

void LoRaWAN_RX_msg(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg, uint32_t timeout_ms)
{
  uint32_t now        = osKernelSysTick();
  uint32_t startTime  = now;
  uint32_t stopTime   = startTime + timeout_ms;

  while (stopTime > now) {
    //LoRaWAN_App_trackMeApp_receiveLoop(&loRaWANctx);
    spiSX1272_WaitUntil_RxDone(startTime + timeout_ms);

    now = osKernelSysTick();
  }
}

void LoRaWAN_App_trackMeApp_pushUp(LoRaWANctx_t* ctx, LoRaWAN_Message_t* msg, LoraliveApp_t* app, uint8_t size)
{
#if 0
  volatile uint8_t  msg_Len;
  volatile uint8_t  msg_Buf[LoRaWAN_MsgLenMax]                      = { 0 };
  uint8_t           msg_MHDR;
  uint8_t           msg_FCtrl;
  uint16_t          msg_FCnt;
  uint8_t           msg_FOpts_Len;
  uint8_t           msg_FOpts_Buf[16]                               = { 0 };
  uint8_t           msg_FOpts_Encoded[16]                           = { 0 };
  uint8_t           msg_FRMPayload_Len                              =   0U;
  uint8_t           msg_FRMPayload_Encoded[LoRaWAN_FRMPayloadMax]   = { 0 };
#endif

  /* PHY Payload: MHDR | MACPayload | MIC */
  /* MHDR */
  msg->msg_MHDR = (((uint8_t) UnconfDataUp) << LoRaWAN_MHDR_MType_SL) | (((uint8_t) LoRaWAN_R1) << LoRaWAN_MHDR_Major_SL);

  /* MACPayload: FHDR | FPort | FRMPayload */
  {
    /* FHDR: DevAddr | FCtrl | FCnt | FOpts */
    {
      /* FOpts */
      // LoRaWAN V1.1 spec @p64: send ResetInd MAC for the first time in the FOpt field (after reset)
      // each packet has to set that, until a response with ResetInd MAC comes
      msg->msg_FOpts_Len = 0U;
      //msg->msg_FOpts_Buf[msg->msg_FOpts_Len++] = (uint8_t) ResetInd_UP;

      /* Encode FOpts */
      LoRaWAN_FOpts_Encrypt(ctx,  // TODO: msg alone
          msg->msg_FOpts_Encoded,
          msg->msg_FOpts_Buf, msg->msg_FOpts_Len);

      /* FCtrl */
      if (ctx->Dir == Up) {
        msg->msg_FCtrl = (ctx->FCtrl_ADR         << LoRaWAN_FCtl_ADR_SL      ) |
                         (ctx->FCtrl_ADRACKReq   << LoRaWAN_FCtl_ADRACKReq_SL) |
                         (ctx->FCtrl_ACK         << LoRaWAN_FCtl_ACK_SL      ) |
                         (ctx->FCtrl_ClassB      << LoRaWAN_FCtl_ClassB_SL   ) |
                         (msg->msg_FOpts_Len     << LoRaWAN_FCtl_FOptsLen_SL );

      } else /* if (ctx->Dir == Dn) */ {
        msg->msg_FCtrl = (ctx->FCtrl_ADR         << LoRaWAN_FCtl_ADR_SL      ) |
                         (ctx->FCtrl_ACK         << LoRaWAN_FCtl_ACK_SL      ) |
                         (ctx->FCtrl_FPending    << LoRaWAN_FCtl_FPending_SL ) |
                         (msg->msg_FOpts_Len     << LoRaWAN_FCtl_FOptsLen_SL );
      }

      /* FCnt */
      msg->msg_FCnt = (uint16_t) ctx->bkpRAM->FCntUp;
    }

#if 1
    /* FRMPayload */
    msg->msg_FRMPayload_Len = LoRaWAN_App_loralive_data2FRMPayload(ctx,
        msg->msg_FRMPayload_Encoded, LoRaWAN_FRMPayloadMax,
        app);
#endif
  }

  /* Message sequencer */
  {
    /* MHDR */
    msg->msg_Len                    = 0U;
    msg->msg_Buf[msg->msg_Len++]    = msg->msg_MHDR;

    /* DevAddr */
    for (uint8_t i = 0; i < 4; i++) {
      msg->msg_Buf[msg->msg_Len++] = ctx->DevAddr[i];
    }

    /* FCtrl */
    msg->msg_Buf[msg->msg_Len++]    = msg->msg_FCtrl;

    /* FCnt */
    msg->msg_Buf[msg->msg_Len++]    = (uint8_t) ((msg->msg_FCnt >>  0) & 0xffU);
    msg->msg_Buf[msg->msg_Len++]    = (uint8_t) ((msg->msg_FCnt >>  8) & 0xffU);

    /* FOpts (encoded) - not emitted when msg_FOpts_Len == 0 */
    for (uint8_t FOpts_Idx = 0; FOpts_Idx < msg->msg_FOpts_Len; FOpts_Idx++) {
      msg->msg_Buf[msg->msg_Len++]  = msg->msg_FOpts_Encoded[FOpts_Idx];
    }

    /* Drop this part when no FRMPayload exists */
    if (msg->msg_FRMPayload_Len) {
      /* FPort */
      msg->msg_Buf[msg->msg_Len++]  = ctx->FPort;

      /* FRMPayload */
      for (uint8_t FRMPayload_Idx = 0; FRMPayload_Idx < msg->msg_FRMPayload_Len; FRMPayload_Idx++) {
        msg->msg_Buf[msg->msg_Len++]  = msg->msg_FRMPayload_Encoded[FRMPayload_Idx];
      }
    }

    /* MIC */
    LoRaWAN_calc_MIC_msgAppend(msg, ctx, MIC_DATAMESSAGE);
  }

  /* Push the complete message to the FIFO and go to transmission mode */
  //LoRaWAN_tx_msg(ctx, msg);
}

#if 0
void LoRaWAN_App_trackMeApp_receiveLoop(LoRaWANctx_t* ctx)
{
  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) "\r\n  RX1: ", 9);
  osSemaphoreRelease(usbToHostBinarySemHandle);

  /* Switch on the receiver RX1 channel 2 */
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);    // Blue on
  spiSX1272Mode_LoRa_RX(2);
  spiSX1272_WaitUntil_RxDone(spiPreviousWakeTime + 1950);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) "\r\n  RX2: ", 9);
  osSemaphoreRelease(usbToHostBinarySemHandle);

  /* Switch to RX2 channel */
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);    // Blue off
  spiSX1272Mode_LoRa_RX(0);
  spiSX1272_WaitUntil_RxDone(spiPreviousWakeTime + 4950);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) "\r\n  RX1: ", 9);
  osSemaphoreRelease(usbToHostBinarySemHandle);
  /* Switch back to RX1: Ch2 channel, again */
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);    // Blue on
  spiSX1272Mode_LoRa_RX(2);
  spiSX1272_WaitUntil_RxDone(spiPreviousWakeTime + 5950);

  osSemaphoreWait(usbToHostBinarySemHandle, 0);
  usbToHostWait((uint8_t*) "\r\n  RX2: ", 9);
  osSemaphoreRelease(usbToHostBinarySemHandle);

  /* Switch to RX2 channel, again */
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);    // Blue off
  spiSX1272Mode_LoRa_RX(0);
  spiSX1272_WaitUntil_RxDone(spiPreviousWakeTime + 7000);
}
#endif
