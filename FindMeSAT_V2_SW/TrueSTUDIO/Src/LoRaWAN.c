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



/* Non-volatile counters in the RTC_Backup domain */
LoRaWANctxBkpRam_t *const LoRaWANctxBkpRam      = (void*) 0x40002850UL;

/* Network context of LoRaWAN */
LoRaWANctx_t loRaWANctx                         = { 0 };

/* Application data for loralive */
LoraliveApp_t loraliveApp = { 0 };


static uint8_t LoRaWAN_App_loralive_data2FRMPayload(LoRaWANctx_t* ctx,
    uint8_t* payloadEncoded, uint8_t maxLen,
    uint8_t FPort,
    LoraliveApp_t* app, uint8_t appSize)
{
  uint8_t pad48[48] = { 0 };
  uint8_t len = 0;

  if (appSize <= 48) {
    uint8_t ecbPad[16];

    memcpy((char*) pad48, app, appSize);

    /* Encode application data */
    uint8_t* key            = (FPort > 0) ?  ctx->appSKey : ctx->NwkSEncKey;
    FRMPayloadBlockA_t a_i  = { 0x01U, 0x00000000UL, 0x00, { ctx->DevAddr[0], ctx->DevAddr[1], ctx->DevAddr[2], ctx->DevAddr[3] }, ctx->bkpRAM->FCntUp, 0x00, 0x00 };

    uint8_t k = (appSize + 15U) >> 4;
    for (uint8_t i = 1; i <= k; ++i) {
      a_i.i = i;

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


void LoRaWAN_Init(void)
{
  const uint8_t bkpRAMLen = &LoRaWANctxBkpRam->_end - &LoRaWANctxBkpRam->LoRaWANcrc;
  loRaWANctx.bkpRAM = LoRaWANctxBkpRam;

  // Check CRC
  uint32_t crcC = crcCalc((&LoRaWANctxBkpRam->LoRaWANcrc) + 1, bkpRAMLen - 1);
  if (crcC != LoRaWANctxBkpRam->LoRaWANcrc) {
    /* Non valid content - reset all to zero */
    uint32_t* ptr = &LoRaWANctxBkpRam->LoRaWANcrc;
    for (uint8_t idx = 1; idx < bkpRAMLen; idx++) {
      *++ptr = 0UL;
    }

    /* Calc new CRC */
    LoRaWANctxBkpRam->LoRaWANcrc = crcCalc((&LoRaWANctxBkpRam->LoRaWANcrc) + 1, bkpRAMLen - 1);
  }

  /* Setup data from FLASH NVM */
  LoRaWANctx_readFLASH();
}

void LoRaWANctx_readFLASH(void)
{
  /* TODO: read from NVM instead of clearing all counters and keys */
  memset(&loRaWANctx, 0, sizeof(LoRaWANctx_t));

  /* Apply keys of the loralive App */
  LoRaWANctx_applyKeys_loralive();
}

void LoRaWANctx_applyKeys_loralive(void)
{
  /* TheThingsNetwork - assigned codes to this device - sufficient for R1.0 [LW10, LW102] */
  loRaWANctx.LoRaWAN_ver = LoRaWANVersion_10;

  const uint8_t  devEUI_LE[8]         = { 0x31U, 0x6FU, 0x72U, 0x65U, 0x70U, 0x73U, 0x65U, 0x00U };
  memcpy(&(loRaWANctx.DevEUI), &devEUI_LE, sizeof(devEUI_LE));

  const uint8_t  devAddr_LE[4]        = { 0x42U, 0x1EU, 0x01U, 0x26U };
  memcpy(&(loRaWANctx.DevAddr), &devAddr_LE, sizeof(devAddr_LE));

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
}

void LoRaWAN_App_loralive_pushUp(LoRaWANctx_t* ctx, uint8_t FPort, LoraliveApp_t* app, uint8_t size)
{
  // LoRaWAN V1.1 spec @p64: send ResetInd MAC for the first time in the FOpt field (after reset)
  // each packet has to set that, until a response with ResetInd MAC comes

  uint8_t payloadEncoded[LoRaWAN_App_loralive_pushUp__FRMPayloadMax] = { 0 };

  uint8_t FRMPayloadLen = LoRaWAN_App_loralive_data2FRMPayload(ctx,
      payloadEncoded, LoRaWAN_App_loralive_pushUp__FRMPayloadMax,
      FPort,
      app, size);

  // TODO: continue here

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
