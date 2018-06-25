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
extern uint32_t           spiPreviousWakeTime;

/* SPI communication buffers */
extern uint8_t            spi1TxBuffer[SPI1_BUFFERSIZE];
extern uint8_t            spi1RxBuffer[SPI1_BUFFERSIZE];
extern osMessageQId       loraInQueueHandle;
extern osMessageQId       loraOutQueueHandle;
extern EventGroupHandle_t loRaWANEventGroupHandle;
extern osMessageQId       loraMacQueueHandle;


const uint16_t loRaWANWait_EGW_MaxWaitTicks = 60000 / portTICK_PERIOD_MS;                       // One minute


#ifdef USE_ABP

/* TheThingsNetwork - assigned codes to this device - sufficient for R1.0 [LW10, LW102] */
//const char *TTNdevAddr = "26011E42";
//const char *TTNnwkSKey = "4386E7FF679BF9810462B2192B2F5211";
//const char *TTNappSKey = "ADDA9AD9B4D746323E221E7E69447DF5";

const uint8_t  DevAddr_LE[4]		                        = { 0x3FU, 0x1AU, 0x01U, 0x26U };
const uint8_t  DevEUI_LE[8]                             = { 0x31U, 0x6FU, 0x72U, 0x65U, 0x70U, 0x73U, 0x65U, 0x00U };
const uint8_t  JoinEUI_LE[8]                            = { 0x00U, 0x86U, 0x00U, 0xD0U, 0x7EU, 0xD5U, 0xB3U, 0x70U };
const uint8_t  NwkSKey_BE[16]                           = { 0x43U, 0x86U, 0xE7U, 0xFFU, 0x67U, 0x9BU, 0xF9U, 0x81U, 0x04U, 0x62U, 0xB2U, 0x19U, 0x2BU, 0x2FU, 0x52U, 0x11U };
const uint8_t  AppSKey_BE[16]                           = { 0xADU, 0xDAU, 0x9AU, 0xD9U, 0xB4U, 0xD7U, 0x46U, 0x32U, 0x3EU, 0x22U, 0x1EU, 0x7EU, 0x69U, 0x44U, 0x7DU, 0xF5U };

#else

/* USE_OTAA */

/* TheThingsNetwork - assigned codes to this device - R1.1 */
// Device ID   findmesat2_002
//const uint8_t  DevEUI_LE[8]                           = { 0x31, 0x30, 0x30, 0x5F, 0x32, 0x53, 0x4D, 0x46 };  // "FMS2_001"
const uint8_t  DevEUI_LE[8]                             = { 0x32, 0x30, 0x30, 0x5F, 0x32, 0x53, 0x4D, 0x46 };  // "FMS2_002"
const uint8_t  AppEUI_LE[8]                             = { 0x08, 0xF6, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
//const uint8_t  JoinEUI_LE[8]                          = { 0 };                                // V1.1: former AppEUI
//const uint8_t  NwkKey_BE[16]                          = { 0 };                                // Since LoRaWAN V1.1
//const uint8_t  AppKey_BE[16]                          = { 0x01, 0xE3, 0x27, 0x88, 0xBA, 0x99, 0x2C, 0x45, 0x6D, 0x92, 0xBF, 0xE0, 0xEE, 0xAD, 0xBE, 0x45 };  // findmesat2_001
const uint8_t  AppKey_BE[16]                            = { 0xD9, 0x0E, 0x09, 0x0B, 0xDB, 0x61, 0xF1, 0xBB, 0x37, 0x4C, 0xE7, 0x9B, 0x23, 0x96, 0x07, 0x11 };  // findmesat2_002
#endif


/* Non-volatile counters in the RTC_Backup domain */
volatile LoRaWANctxBkpRam_t *const LoRaWANctxBkpRam     = (void*) 0x40002850UL;

/* Network context of LoRaWAN */
LoRaWANctx_t loRaWANctx                                 = { 0 };

/* Message buffers */
LoRaWAN_RX_Message_t loRaWanRxMsg                       = { 0 };
LoRaWAN_TX_Message_t loRaWanTxMsg                       = { 0 };

/* Application data for loralive */
LoraliveApp_t loraliveApp                               = { 0 };


inline
uint8_t GET_BYTE_OF_WORD(uint32_t word, uint8_t pos)
{
  return (uint8_t) (((word) >> (8 << (pos))) & 0x000000ffUL);
}


static void LoRaWANctx_readFLASH(void)
{
  /* TODO: read from FLASH NVM instead of default settings */

  /* Crystal drift PPM */
  loRaWANctx.CrystalPpm = +13.79f;                                                              // Real value of this device
  loRaWANctx.GatewayPpm =  -0.70f;                                                              // Gateway drift

  /* Apply keys of the track_me App */
  LoRaWANctx_applyKeys_trackMeApp();
}

void LoRaWANctx_applyKeys_trackMeApp(void)
{
  /* The root key(s) and EUIs */
  {
    memcpy((void*)loRaWANctx.DevEUI_LE,   (const void*)&DevEUI_LE,  sizeof(DevEUI_LE));
    memcpy((void*)loRaWANctx.AppEUI_LE,   (const void*)&AppEUI_LE,  sizeof(AppEUI_LE));
#ifdef LORAWAN_1V1
    memcpy((void*)loRaWANctx.JoinEUI_LE), (const void*)&JoinEUI_LE, sizeof(JoinEUI_LE));
#endif
    memcpy((void*)loRaWANctx.AppKey,      (const void*)&AppKey_BE,  sizeof(AppKey_BE));
  }

  /* Current transmission state */
  {
#ifdef LORAWAN_1V02
    loRaWANctx.LoRaWAN_ver      = LoRaWANVersion_10;
#elif defined LORAWAN_1V1
    loRaWANctx.LoRaWAN_ver      = LoRaWANVersion_11;
#endif
    loRaWANctx.Dir              = Up;
    loRaWANctx.FPort            = 1U;
  }
}


static void LoRaWAN_QueueIn_Process(void)
{
  static uint8_t  buf[32]   = { 0 };
  static uint8_t  bufCtr    = 0;
  static uint8_t  bufMsgLen = 0;
  BaseType_t      xStatus;
  uint8_t         inChr;

  do {
    /* Take next character from the queue, if any */
    inChr = 0;
    xStatus = xQueueReceive(loraInQueueHandle, &inChr, 100 / portTICK_PERIOD_MS);               // Wait max. 100 ms for completion
    if (pdPASS == xStatus) {
      if (!bufMsgLen) {
        bufMsgLen = inChr;

      } else {
        /* Process incoming message */
        buf[bufCtr++] = inChr;

        if (bufCtr == bufMsgLen) {
          /* Message complete */
          break;
        }
      }

    } else {
      /* Reset the state of the queue */
      goto clrInBuf;
    }
  } while (1);

  /* Process the message */
  switch (buf[0]) {
  case loraInQueueCmds__Init:
    {
      /* Set event mask bit for INIT */
      xEventGroupSetBits(loRaWANEventGroupHandle, LORAWAN_EGW__DO_INIT);
    }
    break;

  case loraInQueueCmds__NOP:
  default:
    /* Nothing to do */
    { }
  }  // switch (buf[0])


  /* Clear the buffer */
clrInBuf:
  bufCtr = bufMsgLen = 0;
  memset(buf, 0, sizeof(buf));
}


static float LoRaWAN_calc_CFListEntry_2_FrqMHz(uint8_t packed[3])
{
  uint32_t ui32 = (((uint32_t) (packed[2])) << 16) | (((uint32_t) (packed[1])) << 8) | packed[0];
  return 1e-4 * ui32;
}

static void LoRaWAN_calc_Decode_CFList(LoRaWANctx_t* ctx)
{
  for (uint8_t idx = 0; idx < 5; idx++) {
    ctx->Ch_Frequencies_MHz[idx + 3] = LoRaWAN_calc_CFListEntry_2_FrqMHz((uint8_t*)ctx->CFList +  3 * idx);
  }
}


uint8_t LoRaWAN_calc_randomChannel(LoRaWANctx_t* ctx)
{
  static uint8_t s_channel = 255;
  uint8_t channel;

  do {
    channel = (rand() % 16);
    if (!((1UL << channel) & ctx->LinkADR_ChannelMask)) {
      /* Channel disabled, try another channel */
      channel = s_channel;
    }
  } while (channel == s_channel);
  s_channel = channel;

  return channel + 1;
}

float LoRaWAN_calc_Channel_to_MHz(LoRaWANctx_t* ctx, uint8_t channel, uint8_t dflt)
{
  /* EU863-870*/
  float mhz = 0.f;

  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);                                  // Blue off for any channel of the RX1 timeslot

  switch (channel) {
  case 1:
    mhz = 868.1f;                                                                               // SF7BW125 to SF12BW125 - default value which never changes
    break;

  case 2:
    mhz = 868.3f;                                                                               // SF7BW125 to SF12BW125  and  SF7BW250 - default value which never changes
    break;

  case 3:
    mhz = 868.5f;                                                                               // SF7BW125 to SF12BW125 - default value which never changes
    break;

  case 4:
    mhz = 867.1f;                                                                               // SF7BW125 to SF12BW125
    break;

  case 5:
    mhz = 867.3f;                                                                               // SF7BW125 to SF12BW125
    break;

  case 6:
    mhz = 867.5f;                                                                               // SF7BW125 to SF12BW125
    break;

  case 7:
    mhz = 867.7f;                                                                               // SF7BW125 to SF12BW125
    break;

  case 8:
    mhz = 867.9f;                                                                               // SF7BW125 to SF12BW125
    break;

  case 9:
    mhz = 868.8f;                                                                               // FSK
    break;

  case  0:
  case 16:
    mhz = 869.525f;                                                                             // RX2 channel
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);                                  // Blue on when RX2 timeslot channel selected
    break;

  default:
    Error_Handler();
  }

  /* Current channel list */
  if (!dflt) {
    if ((1UL << (channel - 1)) & ctx->LinkADR_ChannelMask) {
      /* Memorized value returned */
      return ctx->Ch_Frequencies_MHz[channel - 1];
    }
  }

  /* Default value returned */
  return mhz;
}


#ifdef LORAWAN_1V1
static void LoRaWAN_calc_FOpts_Encrypt(LoRaWANctx_t* ctx,
    uint8_t* msg_FOpts_Encoded,
    const uint8_t* msg_FOpts_Buf, uint8_t msg_FOpts_Len)
{
  /* Short way out when nothing to do */
  if (!msg_prep_FOpts_Len) {
    return;
  }

  /* Create crypto xor matrix */
  uint8_t* key  = ctx->NwkSEncKey;

  FRMPayloadBlockA_Up_t a_i  = {
    0x01U,
    { 0x00U, 0x00U, 0x00U, 0x00U },
    (uint8_t) ctx->Dir,
    { ctx->DevAddr_LE[0],
      ctx->DevAddr_LE[1],
      ctx->DevAddr_LE[2],
      ctx->DevAddr_LE[3] },
    { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3) },
    0x00,
    0x00
  };

  /* Create crypto modulator */
  uint8_t ecbPad[16] = { 0 };
  memcpy((void*)ecbPad, ((const void*) &a_i), msg_prep_FOpts_Len);
  cryptoAesEcb(key, ecbPad);

  /* Encode FOpts */
  for (uint8_t idx = 0; idx < msg_prep_FOpts_Len; idx++) {
    msg_prep_FOpts_Encoded[idx] = ecbPad[idx] ^ msg_prep_FOpts_Buf[idx];
  }
}
#endif

static uint8_t LoRaWAN_calc_FRMPayload_Encrypt(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* Calculate the number of blocks needed */
  const uint8_t maxLen  = sizeof(msg->msg_prep_FRMPayload_Buf);
  const uint8_t len     = msg->msg_prep_FRMPayload_Len;
  const uint8_t blocks  = (len + 15U) >> 4;

  /* Leave if no FRMPayload exists */
  if(!len) {
    return 0;
  }

  /* Leave when target buffer is too small */
  if ((blocks << 4) > maxLen) {
    return 0;
  }

  /* Encrypt application data - "Table 3: FPort list" */
#ifdef LORAWAN_1V02
  const uint8_t* key = (ctx->FPort == 0) ?  (const uint8_t*) ctx->NwkSKey : (const uint8_t*) ctx->AppSKey;
#elif LORAWAN_1V1
  const uint8_t* key = (ctx->FPort == 0) ?  (const uint8_t*) ctx->NwkSEncKey : (const uint8_t*) ctx->AppSKey;
#endif

  FRMPayloadBlockA_Up_t a_i  = {
    0x01U,
    { 0x00U, 0x00U, 0x00U, 0x00U },
    (uint8_t) ctx->Dir,                                                                         // "The direction field (Dir) is 0 for uplink frames and 1 for downlink frames"
    { ctx->DevAddr_LE[0],
      ctx->DevAddr_LE[1],
      ctx->DevAddr_LE[2],
      ctx->DevAddr_LE[3] },
    { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3)},
    0x00,
    0x00                                                                                        // i: value of block index
  };

  /* Process all blocks */
  {
    for (uint8_t i = 1, blockPos = 0U; i <= blocks; i++, blockPos += 16U) {
      uint8_t ecbPad[16]  = { 0U };

      /* Update index to generate another crypto modulator for the next block */
      a_i.idx = i;

      /* Create crypto modulator
       * Si = aes128_encrypt(K, Ai) for i = 1..k
       * S  = S1 | S2 | .. | Sk                   */
      memcpy((void*)ecbPad, ((const void*)&a_i), 16);
      cryptoAesEcb_Encrypt(key, ecbPad);

      /* Encrypt data by XOR mask - "(pld | pad16) xor S" */
      for (uint8_t idx = 0; idx < 16; idx++) {
        msg->msg_prep_FRMPayload_Encoded[blockPos + idx] = msg->msg_prep_FRMPayload_Buf[blockPos + idx] ^ ecbPad[idx];

        /* Cut block on message size - "to the first len(pld) octets" */
        if ((blockPos + idx) >= (len - 1)) {
          return len;
        }
      }
    }
  }

  /* Should not happen */
  return 0;
}

static uint8_t LoRaWAN_calc_FRMPayload_Decrypt(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg)
{
  /* Calculate the number of blocks needed */
  const uint8_t maxLen  = sizeof(msg->msg_parted_FRMPayload_Buf);
  const uint8_t len     = msg->msg_parted_FRMPayload_Len;
  const uint8_t blocks  = (len + 15U) >> 4;

  /* Leave if no FRMPayload exists */
  if(!len) {
    return len;
  }

  /* Leave when target buffer is too small */
  if ((blocks << 4) > maxLen) {
    return 0;
  }

  /* Decrypt application data */
#ifdef LORAWAN_1V02
  const uint8_t* key = (ctx->FPort == 0) ?  (const uint8_t*) ctx->NwkSKey : (const uint8_t*) ctx->AppSKey;
#elif LORAWAN_1V1
  const uint8_t* key = (ctx->FPort == 0) ?  (const uint8_t*) ctx->NwkSEncKey : (const uint8_t*) ctx->AppSKey;
#endif

  FRMPayloadBlockA_Dn_t a_i  = {
    0x01U,
    { 0x00U, 0x00U, 0x00U, 0x00U },
    (uint8_t) ctx->Dir,                                                                         // "The direction field (Dir) is 0 for uplink frames and 1 for downlink frames"
    { ctx->DevAddr_LE[0],
      ctx->DevAddr_LE[1],
      ctx->DevAddr_LE[2],
      ctx->DevAddr_LE[3] },
    { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntDwn, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntDwn, 1),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntDwn, 2),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntDwn, 3)},
    0x00,
    0x00                                                                                        // i: value of block index
  };

  /* Process all blocks */
  {
    for (uint8_t i = 1, blockPos = 0U; i <= blocks; i++, blockPos += 16U) {
      uint8_t ecbPad[16]  = { 0U };

      /* Update index to generate another crypto modulator for the next block */
      a_i.idx = i;

      /* Create crypto modulator
       * Si = aes128_encrypt(K, Ai) for i = 1..k
       * S  = S1 | S2 | .. | Sk                   */
      memcpy((void*)ecbPad, ((const void*)&a_i), 16);
      cryptoAesEcb_Encrypt(key, ecbPad);

      /* Encrypt data by XOR mask - "(pld | pad16) xor S" */
      for (uint8_t idx = 0; idx < 16; idx++) {
        msg->msg_parted_FRMPayload_Buf[blockPos + idx] = msg->msg_parted_FRMPayload_Encoded[blockPos + idx] ^ ecbPad[idx];

        /* Cut block on message size - "to the first len(pld) octets" */
        if ((blockPos + idx) >= (len - 1)) {
          return len;
        }
      }
    }
  }

  /* Should not happen */
  return 0;
}

static void LoRaWAN_calc_TX_MIC(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg, uint8_t micOutPad[4], LoRaWAN_CalcMIC_VARIANT_t variant)
{
  switch (variant) {
  case MIC_JOINREQUEST:
    {
      uint8_t cmac[16];

      /*  V1.02: cmac = aes128_cmac(AppKey, MHDR | AppEUI  | DevEUI | DevNonce)
      /   V1.1:  cmac = aes128_cmac(NwkKey, MHDR | JoinEUI | DevEUI | DevNonce)
      /   MIC = cmac[0..3]                                                       */

      /* "msg" contains data for CMAC hashing already */
#ifdef LORAWAN_1V02
      cryptoAesCmac(AppKey_BE, msg->msg_encoded_Buf,
          sizeof(msg->msg_prep_MHDR) + sizeof(ctx->AppEUI_LE) + sizeof(ctx->DevEUI_LE) + sizeof(ctx->DevNonce_LE),
          cmac);
#elif LORAWAN_1V1
      cryptoAesCmac(NwkKey_BE, msg->msg_encoded_Buf,
          sizeof(msg->msg_prep_MHDR) + sizeof(ctx->JoinEUI_LE) + sizeof(ctx->DevEUI_LE) + sizeof(ctx->DevNonce_LE),
          cmac);
#endif

      /* MIC to buffer */
      for (uint8_t i = 0; i < 4; i++) {
        micOutPad[i] = cmac[i];
      }

      return;
    }
    break;

  case MIC_DATAMESSAGE:
    {
      uint8_t   cmacLen;
      uint8_t   cmacBuf[64] = { 0 };

      if (ctx->Dir == Up) {
        /* Uplink */

        MICBlockB0_Up_t b0 = {
          0x49U,
          { 0x00U, 0x00, 0x00U, 00U },
          (uint8_t) Up,
          { ctx->DevAddr_LE[0],
            ctx->DevAddr_LE[1],
            ctx->DevAddr_LE[2],
            ctx->DevAddr_LE[3] },
          { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
            GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
            GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
            GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3)},
          0x00,
          msg->msg_encoded_Len
        };

        /* Concatenate b0 and msg */
        cmacLen = sizeof(MICBlockB0_Up_t);
        memcpy((void*)cmacBuf,            (const void*)&b0,                     cmacLen);
        memcpy((void*)cmacBuf + cmacLen,  (const void*)(msg->msg_encoded_Buf),  msg->msg_encoded_Len);  // "msg = MHDR | FHDR | FPort | FRMPayload"
        cmacLen += msg->msg_encoded_Len;

        uint8_t cmacF[16] = { 0 };
#ifdef LORAWAN_1V02
        /* "cmac = aes128_cmac(NwkSKey, B0 | msg)" */
        cryptoAesCmac((const uint8_t*)ctx->NwkSKey, cmacBuf, cmacLen, cmacF);
#elif LORAWAN_1V1
        if ((!(msg->msg_prep_FPort)) || (msg->msg_prep_FPort_absent)) {
          cryptoAesCmac((const uint8_t*)ctx->NwkSKey, cmacBuf, cmacLen, cmacF);
        } else {
          cryptoAesCmac((const uint8_t*)ctx->AppSKey, cmacBuf, cmacLen, cmacF);
        }
        cryptoAesCmac(ctx->FNwkSIntKey, cmacBuf, cmacLen, cmacF);
#endif

        if (ctx->LoRaWAN_ver == LoRaWANVersion_10) {
          /* MIC to buffer */
          for (uint8_t i = 0; i < 4; i++) {
            micOutPad[i] = cmacF[i];
          }

          return;

#ifdef LORAWAN_1V1
        } else if (ctx->LoRaWAN_ver == LoRaWANVersion_11) {
          MICBlockB1_Up_t b1 = {
            0x49U,
            { GET_BYTE_OF_WORD(ctx->FCntUp, 0),
              GET_BYTE_OF_WORD(ctx->FCntUp, 1) },
            ctx->TxDr,
            ctx->TxCh,
            (uint8_t) Up,
            { ctx->DevAddr_LE[0],
              ctx->DevAddr_LE[1],
              ctx->DevAddr_LE[2],
              ctx->DevAddr_LE[3] },
            { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
              GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1),
              GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 2),
              GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 3)},
            0x00,
            msg->msg_encoded_Len
          };

          /* Concatenate b1 and message */
          cmacLen = sizeof(MICBlockB1_Up_t);
          memcpy((void*)cmacBuf,            (const void*)&b1,                     cmacLen);
          memcpy((void*)cmacBuf + cmacLen,  (const void*)(msg->msg_encoded_Buf),  msg->msg_encoded_Len);
          cmacLen += msg->msg_encoded_Len;

          uint8_t cmacS[16];
          cryptoAesCmac(ctx->SNwkSIntKey, cmacBuf, cmacLen, cmacS);

          /* MIC to buffer */
          micOutPad[0] = cmacS[0];
          micOutPad[1] = cmacS[1];
          micOutPad[2] = cmacF[0];
          micOutPad[3] = cmacF[1];

          return;
#endif
        }
      }

      /* Should not happen */
      Error_Handler();
    }
    break;

    default:
    {
      /* Nothing to do */
    }
  }  // switch ()
}

static void LoRaWAN_calc_RX_MIC(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg, uint8_t micOutPad[4])
{
  const uint8_t micLen      = sizeof(msg->msg_parted_MIC_Buf);
  uint8_t       cmacLen;
  uint8_t       cmacBuf[64] = { 0 };

  /* Downlink */

  uint32_t FCntDwn = ctx->bkpRAM->FCntDwn;
  if ((!(msg->msg_parted_FCntDwn[0]) && (!(msg->msg_parted_FCntDwn[1]))) && ((FCntDwn & 0x0000ffffUL) == 0x0000ffffUL)) {
    FCntDwn += 0x00010000UL;
  }
  FCntDwn &= 0xffff0000UL;
  FCntDwn |= (int32_t)(msg->msg_parted_FCntDwn[0]);
  FCntDwn |= (int32_t)(msg->msg_parted_FCntDwn[1]) << 8;
  ctx->bkpRAM->FCntDwn = FCntDwn;

#ifdef LORAWAN_1V02
  MICBlockB0_Dn_t b0  = {
    0x49U,
    { 0x00U, 0x00U, 0x00U, 0x00U },
    (uint8_t) Dn,
    { msg->msg_parted_DevAddr[0],
      msg->msg_parted_DevAddr[1],
      msg->msg_parted_DevAddr[2],
      msg->msg_parted_DevAddr[3]},
    { GET_BYTE_OF_WORD(FCntDwn, 0),
      GET_BYTE_OF_WORD(FCntDwn, 1),
      GET_BYTE_OF_WORD(FCntDwn, 2),
      GET_BYTE_OF_WORD(FCntDwn, 3)},
    0x00,
    (msg->msg_encoded_Len - micLen)
  };
#elif LORAWAN_1V1
  MICBlockB0_Dn_t b0  = {
    0x49U,
    { GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1)},
    { 0x00U, 0x00U },
    (uint8_t) Dn,
    { ctx->DevAddr_LE[0],
      ctx->DevAddr_LE[1],
      ctx->DevAddr_LE[2],
      ctx->DevAddr_LE[3] },
    { GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 0),
      GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 1),
      GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 2),
      GET_BYTE_OF_WORD(ctx->bkpRAM->AFCntDwn, 3)},
    0x00,
    msg->msg_encoded_Len
  };
#endif

  /* Concatenate b0 and message */
  cmacLen = sizeof(MICBlockB0_Dn_t);
  memcpy((void*)cmacBuf,            (const void*)&b0,                     cmacLen);
  memcpy((void*)cmacBuf + cmacLen,  (const void*)(msg->msg_encoded_Buf),  msg->msg_encoded_Len);
  cmacLen += (msg->msg_encoded_Len - micLen);

  uint8_t cmac[16] = { 0 };
#ifdef LORAWAN_1V02
  cryptoAesCmac((const uint8_t*)ctx->NwkSKey, cmacBuf, cmacLen, cmac);
#elif LORAWAN_1V1
  if ((!(msg->msg_parted_FPort)) || (msg->msg_parted_FPort_absent)) {
    cryptoAesCmac((const uint8_t*)ctx->SNwkSIntKey, cmacBuf, cmacLen, cmac);
  } else {
    cryptoAesCmac((const uint8_t*)ctx->AppSKey,   cmacBuf, cmacLen, cmac);
#endif

  /* MIC to buffer */
  for (uint8_t i = 0; i < 4; i++) {
    micOutPad[i] = cmac[i];
  }
}

static void LoRaWAN_calc_TxMsg_MAC_JOINREQUEST(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* JoinRequest does not have encryption and thus no packet compilation follows, instead write directly to msg_encoded[] */

  /* Start new message and write directly to msg_encoded[] - no packet compilation used here */
  memset(msg, 0, sizeof(LoRaWAN_TX_Message_t));

  /* MHDR */
  msg->msg_prep_MHDR = (((uint8_t) JoinRequest) << LoRaWAN_MHDR_MType_SHIFT) |
                       (((uint8_t) LoRaWAN_R1)  << LoRaWAN_MHDR_Major_SHIFT);
  msg->msg_encoded_Buf[msg->msg_encoded_Len++] = msg->msg_prep_MHDR;

  for (uint8_t i = 0; i < 8; i++) {
#ifdef LORAWAN_1V02
    msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->AppEUI_LE[i];
#elif LORAWAN_1V1
    msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->JoinEUI_LE[i];
#endif
  }

  /* DevEUI[0:7] */
  for (uint8_t i = 0; i < 8; i++) {
    msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->DevEUI_LE[i];
  }

  /* DevNonce */
  {
#ifdef LORAWAN_1V02
    ctx->DevNonce_LE[0] = rand() & 0xffU;
    ctx->DevNonce_LE[1] = rand() & 0xffU;
#elif LORAWAN_1V1
    ctx->DevNonce_LE[0] = 0;
    ctx->DevNonce_LE[1] = 0;
#endif

    msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->DevNonce_LE[0];
    msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->DevNonce_LE[1];

#ifdef LORAWAN_1V1
    // TODO: move me to dn:'ACK'
    if (!(++ctx->DevNonce_LE[0])) {
      ++ctx->DevNonce_LE[1];
    }
#endif
  }

  /* MIC */
  LoRaWAN_calc_TX_MIC(ctx, msg, (uint8_t*)(msg->msg_encoded_Buf + msg->msg_encoded_Len), MIC_JOINREQUEST);
  msg->msg_encoded_Len += 4;

  /* Packet now ready for TX */
}

static void LoRaWAN_calc_TxMsg_Compiler(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* PHYPayload */
  {
    /* MHDR */
    {
      msg->msg_encoded_Len                                  = 0U;
      msg->msg_encoded_Buf[msg->msg_encoded_Len++]          = msg->msg_prep_MHDR;
    }

    /* MACPayload */
    {
      /* FHDR */
      {
        /* DevAddr */
        for (uint8_t i = 0; i < 4; i++) {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++]      = ctx->DevAddr_LE[i];
        }

        /* FCtrl */
        msg->msg_encoded_Buf[msg->msg_encoded_Len++]        = msg->msg_prep_FCtrl;

        /* FCnt */
        msg->msg_encoded_Buf[msg->msg_encoded_Len++]        = msg->msg_prep_FCnt[0];
        msg->msg_encoded_Buf[msg->msg_encoded_Len++]        = msg->msg_prep_FCnt[1];

        /* FOpts (V1.1: encoded) - not emitted when msg_FOpts_Len == 0 */
        {
#ifdef LORAWAN_1V02
          for (uint8_t FOpts_Idx = 0; FOpts_Idx < msg->msg_prep_FOpts_Len; FOpts_Idx++) {
            msg->msg_encoded_Buf[msg->msg_encoded_Len++]    = msg->msg_prep_FOpts_Buf[FOpts_Idx];
          }
#elif LORAWAN_1V1
          if (msg->msg_prep_FOpts_Len) {
            LoRaWAN_calc_FOpts_Encrypt(ctx, msg->msg_prep_FOpts_Encoded, msg->msg_prep_FOpts_Buf, msg->msg_prep_FOpts_Len);

            for (uint8_t FOpts_Idx = 0; FOpts_Idx < msg->msg_prep_FOpts_Len; FOpts_Idx++) {
              msg->msg_encoded_Buf[msg->msg_encoded_Len++]  = msg->msg_prep_FOpts_Encoded[FOpts_Idx];
            }
          }
#endif
        }  // FOpts
      }  // FHDR

      /* FPort - not emitted when msg_prep_FPort_absent is set */
      if (!(msg->msg_prep_FPort_absent)) {
        msg->msg_encoded_Buf[msg->msg_encoded_Len++]        = msg->msg_prep_FPort;
      }

      /* FRMPayload - not emitted when msg_prep_FRMPayload_Len == 0 */
      if (msg->msg_prep_FRMPayload_Len) {
        /* Encode FRMPayload*/
        LoRaWAN_calc_FRMPayload_Encrypt(ctx, msg);

        /* Copy into target msg_encoded_Buf[] */
        for (uint8_t FRMPayload_Idx = 0; FRMPayload_Idx < msg->msg_prep_FRMPayload_Len; FRMPayload_Idx++) {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++]      = msg->msg_prep_FRMPayload_Encoded[FRMPayload_Idx];
        }
      }
    }

    /* MIC */
    {
      LoRaWAN_calc_TX_MIC(ctx, msg, (uint8_t*)(msg->msg_encoded_Buf + msg->msg_encoded_Len), MIC_DATAMESSAGE);
      msg->msg_encoded_Len += 4;
    }
  }

  /* Set flag for Encoding Done */
  msg->msg_encoded_EncDone = 1;

  /* Packet now ready for TX */
}

static void LoRaWAN_calc_TxMsg_Reset(uint32_t* ts, LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* Reset timestamp of last transmission */
  *ts = 0UL;

  /* Clear TX message buffer */
  memset(msg, 0, sizeof(LoRaWAN_TX_Message_t));

  /* Pre-sets for a new TX message */
  ctx->FCtrl_ADR = 0; // TODO: set to one when ADR is implemented
  ctx->FCtrl_ADRACKReq = 0;
  ctx->FCtrl_ACK = 0;
}

static uint8_t LoRaWAN_calc_RxMsg_MAC_JOINACCEPT(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg)
{
  /* Process the message */
  if (msg->msg_encoded_Len) {
    uint8_t decPad[64] = { 0 };
    uint8_t micPad[16] = { 0 };

    memcpy((void*)decPad, (const void*)msg->msg_encoded_Buf, msg->msg_encoded_Len);

    uint8_t ecbCnt = (msg->msg_encoded_Len - 1 + 15) / 16;
    for (uint8_t i = 0, idx = 1; i < ecbCnt; i++, idx += 16) {
      cryptoAesEcb_Encrypt(ctx->AppKey, (uint8_t*)&decPad + idx);                               // Reversed operation as explained in 6.2.5
    }
    memset(decPad + msg->msg_encoded_Len, 0, sizeof(decPad) - msg->msg_encoded_Len);

    /* Check MIC */
    cryptoAesCmac(ctx->AppKey, decPad, msg->msg_encoded_Len - 4, micPad);
    uint8_t micMatch = 1;
    for (uint8_t idx = 0; idx < 4; idx++) {
      if (micPad[idx] != decPad[idx + 29]) {
        micMatch = 0;
        break;
      }
    }

    /* Process the data */
    if (micMatch) {
      uint8_t encPad[16] = { 0 };
      uint8_t keyPad[16] = { 0 };

      /* Copy values to own context */
      uint8_t decPadIdx = 1;
      ctx->AppNonce_LE[0]   = decPad[decPadIdx++];
      ctx->AppNonce_LE[1]   = decPad[decPadIdx++];
      ctx->AppNonce_LE[2]   = decPad[decPadIdx++];
      //
      ctx->NetID_LE[0]      = decPad[decPadIdx++];
      ctx->NetID_LE[1]      = decPad[decPadIdx++];
      ctx->NetID_LE[2]      = decPad[decPadIdx++];
      //
      ctx->DevAddr_LE[0]    = decPad[decPadIdx++];
      ctx->DevAddr_LE[1]    = decPad[decPadIdx++];
      ctx->DevAddr_LE[2]    = decPad[decPadIdx++];
      ctx->DevAddr_LE[3]    = decPad[decPadIdx++];
      //
      ctx->DLSettings       = decPad[decPadIdx++];
      //
      ctx->RXDelay          = decPad[decPadIdx++];
      //
      for (uint8_t cfIdx = 0; cfIdx < sizeof(ctx->CFList); cfIdx++) {
        ctx->CFList[cfIdx]  = decPad[decPadIdx++];
      }
      LoRaWAN_calc_Decode_CFList(ctx);

      /* NwkSKey = aes128_encrypt(AppKey, 0x01 | AppNonce | NetID | DevNonce | pad16) */
      encPad[0] = 0x01;
      memcpy(&(encPad[1]), (uint8_t*)ctx->AppNonce_LE,  3);
      memcpy(&(encPad[4]), (uint8_t*)ctx->NetID_LE,     3);
      memcpy(&(encPad[7]), (uint8_t*)ctx->DevNonce_LE,  2);
      //
      memcpy(keyPad, encPad, sizeof(keyPad));
      cryptoAesEcb_Encrypt(ctx->AppKey, keyPad);
      memcpy((uint8_t*)ctx->NwkSKey, (uint8_t*)keyPad, sizeof(ctx->NwkSKey));

      /* AppSKey = aes128_encrypt(AppKey, 0x02 | AppNonce | NetID | DevNonce | pad16) */
      encPad[0] = 0x02;
      //
      memcpy(keyPad, encPad, sizeof(keyPad));
      cryptoAesEcb_Encrypt(ctx->AppKey, keyPad);
      memcpy((uint8_t*)ctx->AppSKey, (uint8_t*)keyPad, sizeof(ctx->AppSKey));

      return HAL_OK;
    }
  }

  return HAL_ERROR;
}

static uint8_t LoRaWAN_calc_RxMsg_Decoder(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg)
{
  const uint8_t micLen = sizeof(msg->msg_parted_MIC_Buf);

  /* PHYPayload access */
  uint8_t idx       = 0;
  uint8_t micValid  = 1;

  /* Initial FRMPayload length calculation */
  msg->msg_parted_FRMPayload_Len = msg->msg_encoded_Len;

  /* PHYPayload */
  {
    /* MHDR */
    {
      msg->msg_parted_MHDR = msg->msg_encoded_Buf[idx++];
      msg->msg_parted_FRMPayload_Len--;

      /* MType */
      msg->msg_parted_MType = msg->msg_parted_MHDR >> LoRaWAN_MHDR_MType_SHIFT;

      /* Major */
      msg->msg_parted_Major = msg->msg_parted_MHDR & 0b11;

      if ((msg->msg_parted_MType == Proprietary) ||
          (msg->msg_parted_Major != LoRaWAN_R1)
      ) {
        return 0;
      }
    }

    /* MACPayload */
    {
      /* FHDR */
      {
        /* DevAddr */
        {
          memcpy(msg->msg_parted_DevAddr, (uint8_t*)(msg->msg_encoded_Buf + idx), sizeof(msg->msg_parted_DevAddr));

          const uint8_t DevAddrSize        = sizeof(msg->msg_parted_DevAddr);
          idx                             += DevAddrSize;
          msg->msg_parted_FRMPayload_Len  -= DevAddrSize;
        }

        /* FCtrl */
        {
          msg->msg_parted_FCtrl = msg->msg_encoded_Buf[idx++];
          msg->msg_parted_FRMPayload_Len--;

          msg->msg_parted_FCtrl_ADR      = 0x01 & (msg->msg_parted_FCtrl >> LoRaWAN_FCtl_ADR_SHIFT);
          msg->msg_parted_FCtrl_ACK      = 0x01 & (msg->msg_parted_FCtrl >> LoRaWAN_FCtl_ACK_SHIFT);
          msg->msg_parted_FCtrl_FPending = 0x01 & (msg->msg_parted_FCtrl >> LoRaWAN_FCtl_FPending_SHIFT);
          msg->msg_parted_FCtrl_FOptsLen = 0x0f & (msg->msg_parted_FCtrl >> LoRaWAN_FCtl_FOptsLen_SHIFT);
        }

        /* FCnt */
        {
          msg->msg_parted_FCntDwn[0]       = msg->msg_encoded_Buf[idx++];
          msg->msg_parted_FCntDwn[1]       = msg->msg_encoded_Buf[idx++];
          msg->msg_parted_FRMPayload_Len  -= sizeof(msg->msg_parted_FCntDwn);
        }

        /* FOpts */
        {
#ifdef LORAWAN_1V02
          for (uint8_t FOptsIdx = 0; FOptsIdx < msg->msg_parted_FCtrl_FOptsLen; FOptsIdx++) {
            msg->msg_parted_FOpts_Buf[FOptsIdx] = msg->msg_encoded_Buf[idx++];
          }
          msg->msg_parted_FRMPayload_Len -= msg->msg_parted_FCtrl_FOptsLen;
#elif LORAWAN_1V1
          /* Decrypt FOpts */

          msg->msg_parted_FRMPayload_Len -= msg->msg_parted_FCtrl_FOptsLen;
#endif
        }
      }

      /* Subtract MIC length to finally get the FRMPayload length */
      msg->msg_parted_FRMPayload_Len -= micLen;

      /* Are optional parts absent? */
      if (2 > msg->msg_parted_FRMPayload_Len) {
        msg->msg_parted_FPort_absent    = 1;
        msg->msg_parted_FPort           = 0;
        msg->msg_parted_FRMPayload_Len  = 0;

      } else {
        /* optional FPort */
        {
          msg->msg_parted_FPort_absent  = 0;
          msg->msg_parted_FPort         = msg->msg_encoded_Buf[idx++];
          msg->msg_parted_FRMPayload_Len--;
        }

        /* Copy optional encoded FRMPayload to its section */
        {
          memcpy(msg->msg_parted_FRMPayload_Encoded, (uint8_t*)(msg->msg_encoded_Buf + idx), msg->msg_parted_FRMPayload_Len);
          idx += msg->msg_parted_FRMPayload_Len;
        }
      }
    }

    /* MIC */
    {
      uint8_t micComparePad[4] = { 0 };

      memcpy(msg->msg_parted_MIC_Buf, (uint8_t*)(msg->msg_encoded_Buf + idx), micLen);
      LoRaWAN_calc_RX_MIC(ctx, msg, micComparePad);

      for (uint8_t micIdx = 0; micIdx < micLen; micIdx++) {
        if (micComparePad[micIdx] != msg->msg_parted_MIC_Buf[micIdx]) {
          micValid = 0;
          break;
        }
      }
    }
  }

  /* Data packet valid */
  if (!micValid) {
    return 0;
  }

  /* Confirmed frame does update frame counters */
  {
    /* FCntUp */
    if (loRaWanRxMsg.msg_parted_FCtrl_ACK) {
      ctx->bkpRAM->FCntUp++;
    }

    /* FCntDwn */
    {
      const uint32_t FCntDwnLast  = ctx->bkpRAM->FCntDwn;
      uint32_t FCntDwnNew         = FCntDwnLast;
      uint32_t FCntDwn16          = (loRaWanRxMsg.msg_parted_FCntDwn[0]) | ((uint32_t)loRaWanRxMsg.msg_parted_FCntDwn[1] << 8);

      /* 16-bit roll-over handling */
      if (((FCntDwnLast & 0x0000ffffUL) == 0x0000ffffUL) && !FCntDwn16) {
        FCntDwnNew += 0x00010000UL;
      }

      FCntDwnNew &= 0xffff0000UL;
      FCntDwnNew |= 0x0000ffffUL & FCntDwn16;

#if LORAWAN_1V1
      if (FCntDwnNew != (1 + FCntDwnLast)) {
        /* Unexpected FCntDwn - alert */
        return 0;
      }
#endif

      /* Write to backup register */
      ctx->bkpRAM->FCntDwn = FCntDwnNew;
    }
  }

  /* Decrypt the FRMPayload */
  return LoRaWAN_calc_FRMPayload_Decrypt(ctx, msg);
}

static void LoRaWAN_calc_RxMsg_Reset(LoRaWAN_RX_Message_t* msg)
{
  /* Clear RX message buffer */
  memset(msg, 0, sizeof(LoRaWAN_RX_Message_t));
}


const uint32_t maxWaitMs  = 100;
void LoRaWAN_MAC_Queue_Push(uint8_t* macAry, uint8_t cnt)
{
  /* Send MAC commands with their options */
  for (uint8_t idx = 0; idx < cnt; ++idx) {
    BaseType_t xStatus = xQueueSendToBack(loraMacQueueHandle, macAry + idx, maxWaitMs / portTICK_PERIOD_MS);
    if (pdTRUE != xStatus) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

void LoRaWAN_MAC_Queue_Pull(uint8_t* macAry, uint8_t cnt)
{
  /* Receive MAC commands with their options */
  for (uint8_t idx = 0; idx < cnt; ++idx) {
    BaseType_t xStatus = xQueueReceive(loraMacQueueHandle, macAry + idx, maxWaitMs / portTICK_PERIOD_MS);
    if (pdTRUE != xStatus) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

void LoRaWAN_MAC_Queue_Reset(void)
{
  xQueueReset(loraMacQueueHandle);
}

uint8_t LoRaWAN_MAC_Queue_isAvail(uint8_t* snoop)
{
  uint8_t* fill = snoop;
  uint8_t pad;

  if (!snoop) {
    fill = &pad;
  }

  return pdTRUE == xQueuePeek(loraMacQueueHandle, fill, 1);
}

#if 0
static uint8_t LoRaWAN_App_loralive_data2FRMPayload(LoRaWANctx_t* ctx,
    uint8_t* payloadEncoded, uint8_t maxLen,
    const LoraliveApp_t* app)
{
  uint8_t payload[48] = { 0 };
  uint8_t len;

  {
    /* Forge a byte sequence out of the App */
    {
      /* Application: loralive */
      const uint32_t latitude_1000  = 49473;                                                    // 49473182
      const uint32_t longitude_1000 =  8615;                                                    // 8614814
      const int8_t   temperature    = 20;
      const uint8_t  humidity       = 50;
      const uint16_t dust025        = 0U;
      const uint16_t dust100        = 0U;

      len             = 0U;
      payload[len++]  = (uint8_t) (3.3f * 32 + 0.5);                                            // Voltage
      payload[len++]  = (uint8_t) ((dust025 >> 8) & 0x00ffU);                                   // Dust025 HI
      payload[len++]  = (uint8_t) ((dust025 >> 0) & 0x00ffU);                                   // Dust025 LO
      payload[len++]  = (uint8_t) ((dust100 >> 8) & 0x00ffU);                                   // Dust100 HI
      payload[len++]  = (uint8_t) ((dust100 >> 0) & 0x00ffU);                                   // Dust100 LO
      payload[len++]  = (uint8_t) 'E';                                                          // ID
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
  }
  return len;
}
#endif


static uint32_t LoRaWAN_TX_msg(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* Push the complete message to the FIFO and go to transmission mode */

  /* Prepare TX */
  spiSX127x_TxRx_Preps(ctx, TxRx_Mode_TX, msg);

  /* Prepare the FIFO */
  spiSX127xLoRa_Fifo_Init();
  spiSX127xLoRa_Fifo_SetFifoPtrFromTxBase();

  /* Push the message to the FIFO */
  {
    /* FIFO data register */
    spi1TxBuffer[0] = SPI_WR_FLAG | 0x00;
    memcpy((void*)spi1TxBuffer + 1, (const void*)msg->msg_encoded_Buf, msg->msg_encoded_Len);
    spiProcessSpiMsg(1 + msg->msg_encoded_Len);
  }

  /* Transmission */
  uint32_t ts;
  {
    uint32_t now;

    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);                                  // Red on

    /* Start transmitter and wait until the message is being sent */
    spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | TX);
    now = xTaskGetTickCount();
    ts  = spiSX127x_WaitUntil_TxDone(1, now + 1990UL);

    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);                                // Red off
  }
  return ts;
}

static void LoRaWAN_RX_msg(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg, uint32_t tsEndOfTx, uint32_t startAfterTxMs, uint32_t stopAfterTxMs)
{
  TickType_t xLastWakeTime = tsEndOfTx;

  /* Clear receiving message buffer */
  memset(msg, 0, sizeof(LoRaWAN_RX_Message_t));

  /* Prepare RX */
  spiSX127x_TxRx_Preps(ctx, TxRx_Mode_RX, NULL);

  /* Prepare the FIFO */
  spiSX127xLoRa_Fifo_Init();
  spiSX127xLoRa_Fifo_SetFifoPtrFromRxBase();

  /* Sleep until JOIN_ACCEPT_DELAY1 window comes */
  vTaskDelayUntil(&xLastWakeTime, (startAfterTxMs) / portTICK_PERIOD_MS);

  HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);                                    // Green on

  /* Turn on receiver continuously and wait for the next message */
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | RXCONTINUOUS);
  spiSX127x_WaitUntil_RxDone(ctx, msg, tsEndOfTx + stopAfterTxMs);
  spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | STANDBY);

  HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);                                  // Green off
}


void loRaWANLoRaWANTaskInit(void)
{
  const uint8_t bkpRAMLen = &LoRaWANctxBkpRam->_end - &LoRaWANctxBkpRam->LoRaWANcrc;

  /* Clear queue */
  uint8_t inChr = 0;
  while (xQueueReceive(loraInQueueHandle, &inChr, 0) == pdPASS) {
  }

  /* Wait until controller signals to init */
  do {
    EventBits_t eb = xEventGroupWaitBits(loRaWANEventGroupHandle,
        LORAWAN_EGW__QUEUE_IN | LORAWAN_EGW__DO_INIT,
        LORAWAN_EGW__QUEUE_IN | LORAWAN_EGW__DO_INIT,
        0, loRaWANWait_EGW_MaxWaitTicks);
    if (eb & LORAWAN_EGW__QUEUE_IN) {
      LoRaWAN_QueueIn_Process();
    }
    if (eb & LORAWAN_EGW__DO_INIT) {
      /* Now init the LoRaWAN module */
      break;
    }
  } while (1);

  /* Prepare LoRaWAN context */
  {
    memset(&loRaWANctx, 0, sizeof(LoRaWANctx_t));
    loRaWANctx.bkpRAM = LoRaWANctxBkpRam;

    /* Check CRC */
    uint32_t crcC = crcCalc((const uint32_t*) ((&LoRaWANctxBkpRam->LoRaWANcrc) + 1), bkpRAMLen - 1);
    crcC = 0;  // TODO: remove me!
    if (crcC != LoRaWANctxBkpRam->LoRaWANcrc) {
      /* Non valid content - reset all to zero */
      volatile uint32_t* ptr = &LoRaWANctxBkpRam->LoRaWANcrc;
      for (uint8_t idx = 1; idx < bkpRAMLen; idx++) {
        *++ptr = 0UL;
      }

      /* Calc new CRC */
      LoRaWANctxBkpRam->LoRaWANcrc = crcCalc((const uint32_t*) ((&LoRaWANctxBkpRam->LoRaWANcrc) + 1), bkpRAMLen - 1);
    }
  }

  /* Setup data from FLASH NVM */
  LoRaWANctx_readFLASH();

  /* Copy default channel settings */
  {
    for (uint8_t ch = 1; ch <= 8; ch++) {
      loRaWANctx.Ch_Frequencies_MHz[ch - 1] = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, ch, 1);  // Default values / default channels
    }

    loRaWANctx.LinkADR_TxPowerReduction_dB  = 0;                                                // No power reduction
    loRaWANctx.LinkADR_DataRate             = (DataRates_t) DR0_SF12_125kHz_LoRa;               // Start slowly
    loRaWANctx.LinkADR_ChannelMask          =  0x0007U;                                         // Enable default channels (1-3) only
    loRaWANctx.LinkADR_NbTrans              =  1;                                               // Default
    loRaWANctx.LinkADR_ChMaskCntl           = ChMaskCntl__appliesTo_1to16;                      // Mask settings are valid
  }

  /* Seed randomizer */
  {
    /* Prepare and start the receiver */
    loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        16,
        1);                                                                                     // Most traffic on the RX2 channel
    loRaWANctx.SpreadingFactor = SF7_DR5_VAL;     // Use that SF for more noise
    spiSX127x_TxRx_Preps(&loRaWANctx, TxRx_Mode_RX_Randomizer, NULL);

    /* Forging the random number */
    {
      uint32_t r = 0x12345678U;
      for (uint8_t cnt = 32; cnt; cnt--) {
        uint8_t rssi;
        uint32_t rotBit;

        /* Left rotating */
        rotBit    = r >> 31;
        r       <<= 1;
        r        |= rotBit;

        /* Read the current broadband RSSI value */
        osDelay(1);
        rssi = spiSX127xMode_LoRa_GetBroadbandRSSI();
        r ^= rssi;
      }
      srand(r);
    }

    /* Return transceiver to STANDBY mode */
    spiSX127xMode(MODE_LoRa | ACCES_SHARE_OFF | LOW_FREQ_MODE_OFF | STANDBY);
  }

  /* I/Q balancing */
  {
    /* Set center frequency of EU-868 */
    loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        1,
        1);                                                                                     // First default channel is about in the middle of the band

    /* Reset to POR/Reset defaults */
    spiSX127xReset();

    /* Do I/Q balancing with that center frequency */
    spiSX127x_TxRx_Preps(&loRaWANctx, TxRx_Mode_IQ_Balancing, NULL);
  }

  /* Start with JOIN-REQUEST */
  loRaWANctx.FsmState = Fsm_MAC_JoinRequest;
}

void loRaWANLoRaWANTaskLoop(void)
{
  static uint32_t   tsEndOfTx                 = 0UL;
  static uint8_t    sLinkADR_ChannelMask_OK   = 0;
  EventBits_t       eb;

  switch (loRaWANctx.FsmState) {
  case Fsm_RX1:
    {
      if (tsEndOfTx) {
        /* Gateway response after DELAY1 at RX1 - switch on receiver */
        LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
            tsEndOfTx,
            LORAWAN_EU868_DELAY1_MS - LORAWAN_RX_PREPARE_MS,
            LORAWAN_EU868_DELAY2_MS - LORAWAN_RX_PREPARE_MS);                                   // Same frequency and SF as during transmission

        if (loRaWanRxMsg.msg_encoded_Len == 0) {
          /* Listen to next window */
          loRaWANctx.FsmState = Fsm_RX2;

        } else {
          /* USB: info */
          usbLog("LoRaWAN: received packet within RX1 window.\r\n");

          /* Process message */
          loRaWANctx.FsmState = Fsm_MAC_Decode;
        }
      }
    }
    break;

  case Fsm_RX2:
    {
      if (tsEndOfTx) {
        /* Gateway response after DELAY2 at RX2 */
        loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
            &loRaWANctx,
            0,
            1);                                                                                 // Jump to RX2 frequency (default frequency)
        loRaWANctx.SpreadingFactor = SF12_DR0_VAL;                                              // TODO: use right one

        /* Gateway response after DELAY2 at RX2 */
        LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
            tsEndOfTx,
            LORAWAN_EU868_DELAY2_MS - LORAWAN_RX_PREPARE_MS,
            LORAWAN_EU868_DELAY2_MS + LORAWAN_EU868_MAX_TX_DURATION_MS - LORAWAN_RX_PREPARE_MS);

        if (loRaWanRxMsg.msg_encoded_Len == 0) {
          /* No message received - try again if packet is of confirmed type */
          if (ConfDataUp == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
            loRaWANctx.FsmState = Fsm_TX;

            /* Delay 2..10 secs */
            osDelay(2000 + rand() % 8000);

          } else {
            loRaWANctx.FsmState = Fsm_NOP;
          }

        } else {
          /* USB: info */
          usbLog("LoRaWAN: received packet within RX2 window.\r\n");

          /* Process message */
          loRaWANctx.FsmState = Fsm_MAC_Decode;
        }
      }
    }
    break;

  case Fsm_JoinRequestRX1:
    {
      if (tsEndOfTx) {
        /* JOIN-ACCEPT response after JOIN_ACCEPT_DELAY1 at RX1 - switch on receiver */
        LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
            tsEndOfTx,
            LORAWAN_EU868_JOIN_ACCEPT_DELAY1_MS - LORAWAN_RX_PREPARE_MS,
            LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS - LORAWAN_RX_PREPARE_MS);                       // Same frequency and SF as during transmission

        if (loRaWanRxMsg.msg_encoded_Len == 0) {
          /* Receive response at JOINREQUEST_RX2 */
          loRaWANctx.FsmState = Fsm_JoinRequestRX2;

        } else {
          /* USB: info */
          usbLog("LoRaWAN: JOIN-RESPONSE received within window JR-Delay_RX1.\r\n");

          /* Process message */
          loRaWANctx.FsmState = Fsm_MAC_JoinAccept;
        }

      } else {  // if (tsEndOfTx)
        /* Reset FSM */
        loRaWANctx.FsmState = Fsm_NOP;
      }
    }
    break;

  case Fsm_JoinRequestRX2:
    {
      if (tsEndOfTx) {
        /* JOIN-ACCEPT response after JOIN_ACCEPT_DELAY2 at RX2 */
        loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
            &loRaWANctx,
            0,
            1);                                                                                 // Jump to RX2 frequency (default frequency)
        loRaWANctx.SpreadingFactor = SF12_DR0_VAL;                                              // Use that SF

        /* Prepare receiver and listen to the ether */
        LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
            tsEndOfTx,
            LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS - LORAWAN_RX_PREPARE_MS,
            LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS + LORAWAN_EU868_MAX_TX_DURATION_MS - LORAWAN_RX_PREPARE_MS);

        if (loRaWanRxMsg.msg_encoded_Len == 0) {
#if 0
          if (loRaWANctx.SpreadingFactor < SF12_DR0_VAL) {
            loRaWANctx.SpreadingFactor++;
          }
#endif

JR_next_try:
          if (JoinRequest == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
            /* Try again with new JOINREQUEST - clears loRaWanTxMsg by itself */
            loRaWANctx.FsmState = Fsm_MAC_JoinRequest;

          } else if (ConfDataUp == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
            switch (loRaWanTxMsg.msg_prep_FOpts_Buf[0]) {
            case LinkCheckReq_UP:
              {
                loRaWANctx.FsmState = Fsm_MAC_JoinRequest;
              }
              break;

            default:
              loRaWANctx.FsmState = Fsm_NOP;
            }
          }

          /* Sequence has ended */
          LoRaWAN_calc_TxMsg_Reset(&tsEndOfTx, &loRaWANctx, &loRaWanTxMsg);

          /* Delay 2s before retransmitting */
          eb = xEventGroupWaitBits(loRaWANEventGroupHandle, LORAWAN_EGW__QUEUE_IN, LORAWAN_EGW__QUEUE_IN, 0, 1000 / portTICK_PERIOD_MS);
          if (eb) {
            /* New message came in - Rest of sleep time dropped */
            LoRaWAN_QueueIn_Process();
          }

        } else {  // if (loRaWanRxMsg.msg_encoded_Len == 0) {} else
          /* USB: info */
          usbLog("LoRaWAN: JOIN-RESPONSE received within window JR-Delay_RX2.\r\n");

          if (JoinRequest == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
            /* Continue with the JoinAccept handling */
            loRaWANctx.FsmState = Fsm_MAC_JoinAccept;

          } else if (ConfDataUp == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
            /* Process the message */
            loRaWANctx.FsmState = Fsm_MAC_Decode;
          }
        }

      } else {  // if (tsEndOfTx)
        /* Clear TX and RX message buffers */
        LoRaWAN_calc_TxMsg_Reset(&tsEndOfTx, &loRaWANctx, &loRaWanTxMsg);
        LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

        /* Reset FSM */
        LoRaWAN_MAC_Queue_Reset();
        loRaWANctx.FsmState = Fsm_NOP;
      }
    }
    break;


  /* Compile message and TX */
  case Fsm_TX:
    {
      /* Randomized TX1 frequency */
      loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
          &loRaWANctx,
          LoRaWAN_calc_randomChannel(&loRaWANctx),
          0);
      loRaWANctx.SpreadingFactor = SF7_DR5_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF8_DR4_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF9_DR3_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF10_DR2_VAL;                                                // Use that SF
//    loRaWANctx.SpreadingFactor = SF11_DR1_VAL;                                                // Use that SF
//    loRaWANctx.SpreadingFactor = SF12_DR0_VAL;                                                // Use that SF

      /* USB: info */
      usbLog("LoRaWAN: TX packet.\r\n");

      /* Clear RX msg buffer */
      {
        memset(&loRaWanRxMsg, 0, sizeof(LoRaWAN_RX_Message_t));
        LoRaWAN_MAC_Queue_Reset();
      }

      /* Compile packet if not done, already */
      if (!(loRaWanTxMsg.msg_encoded_EncDone)) {
        LoRaWAN_calc_TxMsg_Compiler(&loRaWANctx, &loRaWanTxMsg);
      }

      /* Prepare transmitter and go on-air */
      tsEndOfTx = LoRaWAN_TX_msg(&loRaWANctx, &loRaWanTxMsg);

      /* Receive response at RX1 */
      loRaWANctx.FsmState = Fsm_RX1;
    }
    break;


  /* Decode received message */
  case Fsm_MAC_Decode:
    {
      /* Non-valid short message */
      if (loRaWanRxMsg.msg_encoded_Len < 5) {
        /* Remove RX message */
        memset(&loRaWanRxMsg, 0, sizeof(LoRaWAN_RX_Message_t));

        if (ConfDataUp == (loRaWanTxMsg.msg_prep_MHDR >> LoRaWAN_MHDR_MType_SHIFT)) {
          switch (loRaWanTxMsg.msg_prep_FOpts_Buf[0]) {
          case LinkCheckReq_UP:
            {
              /* Try again */
              loRaWANctx.FsmState = Fsm_MAC_LinkCheckReq;
            }
            break;

          default:
            {
              /* Nothing to do with that */
              loRaWANctx.FsmState = Fsm_NOP;
            }
          }  // switch (loRaWanTxMsg.msg_prep_FOpts_Buf[0])
        }
        break;
      }

      /* Decode the RX msg */
      {
        LoRaWAN_calc_RxMsg_Decoder(&loRaWANctx, &loRaWanRxMsg);

        /* Single MAC command can be handled within the FSM directly */
        if ((loRaWanRxMsg.msg_parted_FOpts_Len              >= 1) &&
           (!loRaWanRxMsg.msg_parted_FPort_absent)                &&
            (loRaWanRxMsg.msg_parted_FPort                  != 0))
        {
          /* Push all MAC data to the MAC queue from FOpts_Buf */
          LoRaWAN_MAC_Queue_Push(loRaWanRxMsg.msg_parted_FOpts_Buf, loRaWanRxMsg.msg_parted_FOpts_Len);

        } else if ((loRaWanRxMsg.msg_parted_FOpts_Len       == 0) &&
                  (!loRaWanRxMsg.msg_parted_FPort_absent)         &&
                   (loRaWanRxMsg.msg_parted_FPort           == 0) &&
                   (loRaWanRxMsg.msg_parted_FRMPayload_Len  >= 1))
        {
          /* Push all MAC data to the MAC queue from FRMPayload_Buf[] */
          LoRaWAN_MAC_Queue_Push(loRaWanRxMsg.msg_parted_FRMPayload_Buf, loRaWanRxMsg.msg_parted_FRMPayload_Len);
        }

        /* MAC processing */
        loRaWANctx.FsmState = Fsm_MAC_Proc;
      }
    }
    break;


  /* MAC queue processing */
  case Fsm_MAC_Proc:
    {
      if (LoRaWAN_MAC_Queue_isAvail(NULL)) {
        uint8_t mac = 0;
        LoRaWAN_MAC_Queue_Pull(&mac, 1);

        switch (mac) {
        case LinkCheckAns_DN:
          loRaWANctx.FsmState = Fsm_MAC_LinkCheckAns;
          break;

        case LinkADRReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_LinkADRReq;
          break;

        case DutyCycleReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_DutyCycleReq;
          break;

        case RXParamSetupReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_RXParamSetupReq;
          break;

        case DevStatusReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_DevStatusReq;
          break;

        case NewChannelReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_NewChannelReq;
          break;

        case RXTimingSetupReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_RXTimingSetupReq;
          break;

        case TxParamSetupReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_TxParamSetupReq;
          break;

        case DlChannelReq_DN:
          loRaWANctx.FsmState = Fsm_MAC_DlChannelReq;
          break;

        default:
          LoRaWAN_MAC_Queue_Reset();
          loRaWANctx.FsmState = Fsm_NOP;
        }  // switch (mac)
        break;
      }  // if (LoRaWAN_MAC_Queue_isAvail())

      /* Fall back to NOP */
      loRaWANctx.FsmState = Fsm_NOP;
    }
    break;


  /* MAC CID 0x01 - up:JoinRequest --> dn:JoinAccept */
  case Fsm_MAC_JoinRequest:
    {
      /* JoinRequest does reset frame counters */
      loRaWANctx.bkpRAM->FCntUp   = 0UL;
      loRaWANctx.bkpRAM->FCntDwn  = 0UL;

      /* Adjust the context */
#ifdef PPM_CALIBRATION
      loRaWANctx.FrequencyMHz = 870.0;
      loRaWANctx.SpreadingFactor = SF12_DR0_VAL;
#else
      loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
          &loRaWANctx,
          LoRaWAN_calc_randomChannel(&loRaWANctx),
          0);                                                                                   // Randomized RX1 frequency
      loRaWANctx.SpreadingFactor = SF7_DR5_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF8_DR4_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF9_DR3_VAL;                                                 // Use that SF
//    loRaWANctx.SpreadingFactor = SF10_DR2_VAL;                                                // Use that SF
//    loRaWANctx.SpreadingFactor = SF11_DR1_VAL;                                                // Use that SF
//    loRaWANctx.SpreadingFactor = SF12_DR0_VAL;                                                // Use that SF
#endif

      /* USB: info */
      usbLog("LoRaWAN: JOIN-REQUEST going to be sent.\r\n");

      /* Compile packet */
      LoRaWAN_calc_TxMsg_MAC_JOINREQUEST(&loRaWANctx, &loRaWanTxMsg);

      /* Prepare transmitter and go on-air */
      tsEndOfTx = LoRaWAN_TX_msg(&loRaWANctx, &loRaWanTxMsg);

      /* Receive response at JOINREQUEST_RX1 */
      loRaWANctx.FsmState = Fsm_JoinRequestRX1;
    }
    break;

  case Fsm_MAC_JoinAccept:
    {
      /* JOIN-ACCEPT process the message */
      if (HAL_OK == LoRaWAN_calc_RxMsg_MAC_JOINACCEPT(&loRaWANctx, &loRaWanRxMsg)) {
        /* USB: info */
        usbLog("LoRaWAN: JOIN-RESPONSE message successfully decoded.\r\n\r\n");

        /* Sequence has ended */
        LoRaWAN_calc_TxMsg_Reset(&tsEndOfTx, &loRaWANctx, &loRaWanTxMsg);

        /* Next: do LinkCheckReq */
        loRaWANctx.FsmState = Fsm_MAC_LinkCheckReq;

      } else {
        /* USB: info */
        usbLog("LoRaWAN: JOIN-RESPONSE message failed to decode.\r\n\r\n");

        /* Try again with new JOINREQUEST */
        goto JR_next_try;
      }
    }
    break;


  /* MAC CID 0x02 - up:LinkCheckReq --> dn:LinkCheckAns */
  case Fsm_MAC_LinkCheckReq:
    {
      /* Requesting for confirmed data up-transport */
      loRaWanTxMsg.msg_prep_MHDR = (ConfDataUp << LoRaWAN_MHDR_MType_SHIFT) | (LoRaWAN_R1 << LoRaWAN_MHDR_Major_SHIFT);

      /* FHDR */
      {
        /* DevAddr included by TX message compiler */

        /* FOpts list */
        {
          loRaWanTxMsg.msg_prep_FOpts_Len = 0;

          /* MAC - up: LinkCheckReq (piggybacked)*/
          loRaWanTxMsg.msg_prep_FOpts_Buf[loRaWanTxMsg.msg_prep_FOpts_Len++] = LinkCheckReq_UP;
        }

        /* One FOpts entry for LinkCheckReq */
        loRaWanTxMsg.msg_prep_FCtrl  =  (loRaWANctx.FCtrl_ADR             << LoRaWAN_FCtl_ADR_SHIFT)       |
                                        (loRaWANctx.FCtrl_ADRACKReq       << LoRaWAN_FCtl_ADRACKReq_SHIFT) |
                                        (loRaWANctx.FCtrl_ACK             << LoRaWAN_FCtl_ACK_SHIFT)       |
                                        (loRaWanTxMsg.msg_prep_FOpts_Len  << LoRaWAN_FCtl_FOptsLen_SHIFT );

        /* Current up:frame counter */
        loRaWanTxMsg.msg_prep_FCnt[0]   =  loRaWANctx.bkpRAM->FCntUp       & 0x000000ffUL;
        loRaWanTxMsg.msg_prep_FCnt[1]   = (loRaWANctx.bkpRAM->FCntUp >> 8) & 0x000000ffUL;
      }

      /* FPort absent */
      loRaWanTxMsg.msg_prep_FPort_absent = 1;

      /*  FRMPayload */
      {
        /* FRMPayload absent */
        loRaWanTxMsg.msg_prep_FRMPayload_Len = 0;
        memset(loRaWanTxMsg.msg_prep_FRMPayload_Buf,      0, sizeof(loRaWanTxMsg.msg_prep_FRMPayload_Buf));
        memset(loRaWanTxMsg.msg_prep_FRMPayload_Encoded,  0, sizeof(loRaWanTxMsg.msg_prep_FRMPayload_Encoded));
      }

      /* USB: info */
      usbLog("LoRaWAN: Going to TX LinkCheckReq.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_TX;
    }
    break;

  case Fsm_MAC_LinkCheckAns:
    {
      uint8_t macAry[2] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX LinkCheckAns.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
      loRaWANctx.LinkCheck_Ppm_SNR  = macAry[0];
      loRaWANctx.LinkCheck_GW_cnt   = macAry[1];
      loRaWANctx.FsmState           = Fsm_MAC_Proc;
    }
    break;


  /* MAC CID 0x03 - dn:LinkADRReq --> up:LinkADRAns */
  case Fsm_MAC_LinkADRReq:
    {
      uint8_t macAry[4] = { 0 };
      uint8_t nextMac;

      /* USB: info */
      usbLog("LoRaWAN: Got RX LinkADRReq.\r\n");

      /* Iterate over all LinkADRReq MACs */
      do {
        LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
        loRaWANctx.LinkADR_TxPowerReduction_dB  = (macAry[0] & 0x0f) << 1;
        loRaWANctx.LinkADR_DataRate             = (DataRates_t) ((macAry[0] & 0xf0) >> 4);

        /* Prove each channel mask bit before acceptance */
        uint16_t newChMask                      =  macAry[1] | ((uint16_t)macAry[2] << 8);
        uint8_t  newChMaskValid = (newChMask != 0) ?  1 : 0;
        for (uint8_t chIdx = 0; chIdx < 16; chIdx++) {
          if ((1UL << chIdx) & newChMask) {
            if (!loRaWANctx.Ch_Frequencies_MHz[chIdx]) {
              newChMaskValid = 0;
              break;
            }
          }
        }
        if (newChMaskValid) {
          loRaWANctx.LinkADR_ChannelMask        =  newChMask;
          sLinkADR_ChannelMask_OK = 1;
        } else {
          sLinkADR_ChannelMask_OK = 0;
        }

        loRaWANctx.LinkADR_NbTrans              =  macAry[3] & 0x0f;
        loRaWANctx.LinkADR_ChMaskCntl           = (ChMaskCntl_t) ((macAry[3] & 0x70) >> 4);

        if (!LoRaWAN_MAC_Queue_isAvail(&nextMac)) {
          break;
        }
        if (LinkADRReq_DN != nextMac) {
          break;
        }
      } while (1);

      loRaWANctx.FsmState = Fsm_MAC_LinkADRAns;
    }
    break;

  case Fsm_MAC_LinkADRAns:
    {
      loRaWANctx.TX_MAC_Buf[loRaWANctx.TX_MAC_Cnt++] = 0b11 | (0x1 & sLinkADR_ChannelMask_OK);  // Power OK, DataRate OK, ChannelMask

      /* USB: info */
      usbLog("LoRaWAN: Going to TX LinkADRAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_TX;
    }
    break;


  /* MAC CID 0x04 - dn:DutyCycleReq --> up:DutyCycleAns */
  case Fsm_MAC_DutyCycleReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX DutyCycleReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_DutyCycleAns;
    }
    break;

  case Fsm_MAC_DutyCycleAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX DutyCycleAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x05 - dn:RXParamSetupReq --> up:RXParamSetupAns */
  case Fsm_MAC_RXParamSetupReq:
    {
      uint8_t macAry[4] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX RXParamSetupReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
//      loRaWANctx.xxx = macAry[1];
//      loRaWANctx.xxx = macAry[2];
//      loRaWANctx.xxx = macAry[3];
      loRaWANctx.FsmState = Fsm_MAC_RXParamSetupAns;
    }
    break;

  case Fsm_MAC_RXParamSetupAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX RXParamSetupAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x06 - dn:DevStatusReq --> up:DevStatusAns */
  case Fsm_MAC_DevStatusReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX DevStatusReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_DevStatusAns;
    }
    break;

  case Fsm_MAC_DevStatusAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX DevStatusAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x07 - dn:NewChannelReq --> up:NewChannelAns */
  case Fsm_MAC_NewChannelReq:
    {
      uint8_t macAry[5] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX NewChannelReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
//      loRaWANctx.xxx = macAry[1];
//      loRaWANctx.xxx = macAry[2];
//      loRaWANctx.xxx = macAry[3];
//      loRaWANctx.xxx = macAry[4];
      loRaWANctx.FsmState = Fsm_MAC_NewChannelAns;
    }
    break;

  case Fsm_MAC_NewChannelAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX NewChannelAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x08 - dn:RXTimingSetupReq --> up:RXTimingSetupAns */
  case Fsm_MAC_RXTimingSetupReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX RXTimingSetupReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_RXTimingSetupAns;
    }
    break;

  case Fsm_MAC_RXTimingSetupAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX RXTimingSetupAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x09 - dn:TxParamSetupReq --> up:TxParamSetupAns */
  case Fsm_MAC_TxParamSetupReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX TxParamSetupReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_TxParamSetupAns;
    }
    break;

  case Fsm_MAC_TxParamSetupAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX TxParamSetupAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x0A - dn:DlChannelReq --> up:DlChannelAns */
  case Fsm_MAC_DlChannelReq:
    {
      uint8_t macAry[4] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX DlChannelReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
//      loRaWANctx.xxx = macAry[1];
//      loRaWANctx.xxx = macAry[2];
//      loRaWANctx.xxx = macAry[3];
      loRaWANctx.FsmState = Fsm_MAC_DlChannelAns;
    }
    break;

  case Fsm_MAC_DlChannelAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX DlChannelAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


#ifdef LORANET_1V1
  /* MAC CID 0x0? - dn:RekeyConf --> up:? */
  case Fsm_MAC_RekeyConf:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX RekeyConf.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_RekeyConf2;
    }
    break;

  case Fsm_MAC_RekeyConf2:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX RekeyConf2.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x0? - dn:ADRParamSetupReq --> up:ADRParamSetupAns */
  case Fsm_MAC_ADRParamSetupReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX ADRParamSetupReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_ADRParamSetupAns;
    }
    break;

  case Fsm_MAC_ADRParamSetupAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX ADRParamSetupAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  case Fsm_MAC_DeviceTimeReq:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX DeviceTimeReq.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_TX;
    }
    break;

  case Fsm_MAC_DeviceTimeAns:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX DeviceTimeAns.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_Proc;
    }
    break;


  /* MAC CID 0x0? - dn:ForceRejoinReq --> up:ForceRejoinAns */
  case Fsm_MAC_ForceRejoinReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX ForceRejoinReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//      loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_ForceRejoinAns;
    }
    break;

  case Fsm_MAC_ForceRejoinAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX ForceRejoinAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;


  /* MAC CID 0x0? - dn:RejoinParamSetupReq --> up:RejoinParamSetupAns */
  case Fsm_MAC_RejoinParamSetupReq:
    {
      uint8_t macAry[1] = { 0 };

      /* USB: info */
      usbLog("LoRaWAN: Got RX RejoinParamSetupReq.\r\n");

      LoRaWAN_MAC_Queue_Pull(macAry, sizeof(macAry));
//    loRaWANctx.xxx = macAry[0];
      loRaWANctx.FsmState = Fsm_MAC_RejoinParamSetupAns;
    }
    break;

  case Fsm_MAC_RejoinParamSetupAns:
    {

      /* USB: info */
      usbLog("LoRaWAN: Going to TX RejoinParamSetupAns.\r\n");

      /* Prepare for transmission */
      loRaWANctx.FsmState = Fsm_NOP;    // TODO
    }
    break;
#endif

  default:
    loRaWANctx.FsmState = Fsm_NOP;
    // Fall-through.
  case Fsm_NOP:
    eb = xEventGroupWaitBits(loRaWANEventGroupHandle, LORAWAN_EGW__QUEUE_IN, LORAWAN_EGW__QUEUE_IN, 0, loRaWANWait_EGW_MaxWaitTicks);
    if (eb) {
      LoRaWAN_QueueIn_Process();
    }
  }  // switch ()
}
