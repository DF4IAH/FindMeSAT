/*
 * LoRaWAN.c
 *
 *  Created on: 12.05.2018
 *      Author: DF4IAH
 */

#include "LoRaWAN.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
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


/* SPI communication buffers */
extern uint8_t            spi1TxBuffer[SPI1_BUFFERSIZE];
extern uint8_t            spi1RxBuffer[SPI1_BUFFERSIZE];
extern osMessageQId       loraInQueueHandle;
extern osMessageQId       loraOutQueueHandle;
extern osMessageQId       loraMacQueueHandle;
extern osSemaphoreId      trackMeApplUpDataBinarySemHandle;
extern osSemaphoreId      trackMeApplDownDataBinarySemHandle;
extern EventGroupHandle_t loRaWANEventGroupHandle;

const uint16_t            loRaWANWait_EGW_MaxWaitTicks  = 60000 / portTICK_PERIOD_MS;           // One minute
const uint16_t            LoRaWAN_MaxWaitMs             = 100;


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
LoRaWANctx_t          loRaWANctx                        = { 0 };

/* Message buffers */
LoRaWAN_RX_Message_t  loRaWanRxMsg                      = { 0 };
LoRaWAN_TX_Message_t  loRaWanTxMsg                      = { 0 };

/* Application data for track_me */
TrackMeApp_up_t       trackMeApp_up                     = { 0 };
TrackMeApp_down_t     trackMeApp_down                   = {   };

/* Application data for loralive */
LoraliveApp_up_t      loraliveApp_up                    = { 0 };
LoraliveApp_down_t    loraliveApp_down                  = {   };


inline
uint8_t GET_BYTE_OF_WORD(uint32_t word, uint8_t pos)
{
  return (uint8_t) ((word >> (8 * pos)) & 0x000000ffUL);
}


static void LoRaWANctx_readFLASH(void)
{
  /* TODO: read from FLASH NVM instead of default settings */

  /* Crystal drift PPM */
  loRaWANctx.CrystalPpm = +13.79f;                                                              // Real value of this device

  /* Drift of the Gateway PPM */
//loRaWANctx.GatewayPpm =  -0.70f;                                                              // Gateway drift
  loRaWANctx.GatewayPpm =   0.00f;                                                              // Gateway drift

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


static uint8_t LoRaWAN_marshalling_PayloadCompress_TrackMeAppUp(uint8_t outBuf[], const TrackMeApp_up_t* trackMeApp_UL)
{
  uint8_t outLen = 0;

  /* TTN Mapper entities */
  if ((  -90.0 <= trackMeApp_UL->latitude_deg    && trackMeApp_UL->latitude_deg    <=    90.0) &&
      ( -180.0 <= trackMeApp_UL->longitude_deg   && trackMeApp_UL->longitude_deg   <=   180.0) &&
      (-8192.0 <= trackMeApp_UL->altitude_m      && trackMeApp_UL->altitude_m      <= 57343.0) &&
      (    1.0 <= trackMeApp_UL->accuracy_10thM  && trackMeApp_UL->accuracy_10thM  <=  1262.0)) {

    uint32_t lat = (int32_t) ((  90.0 + trackMeApp_UL->latitude_deg ) * 10000.0);
    uint32_t lon = (int32_t) (( 180.0 + trackMeApp_UL->longitude_deg) * 10000.0);
    uint16_t alt = (int16_t) ( 8192.0 + trackMeApp_UL->altitude_m);
    uint8_t  acc = (uint8_t) (log10(trackMeApp_UL->accuracy_10thM) / log10(1.25));

    outBuf[0]  = (lat >> 13) & 0xff;                                                            // 8 bits
    outBuf[1]  = (lat >>  5) & 0xff;                                                            // 8 bits
    outBuf[2]  = (lat <<  3) & 0xf8;                                                            // 5 bits

    outBuf[2] |= (lon >> 19) & 0x07;                                                            // 3 bits
    outBuf[3]  = (lon >> 11) & 0xff;                                                            // 8 bits
    outBuf[4]  = (lon >>  3) & 0xff;                                                            // 8 bits
    outBuf[5]  = (lon <<  5) & 0xf8;                                                            // 3 bits

    outBuf[5] |= (alt >> 11) & 0x07;                                                            // 5 bits
    outBuf[6]  = (alt >>  3) & 0xff;                                                            // 8 bits
    outBuf[7]  = (alt <<  5) & 0xe0;                                                            // 3 bits

    outBuf[7] |=  acc        & 0x1f;                                                            // 5 bits
    outLen    += 8;


    /* Motion vector entities */
    if (trackMeApp_UL->course_deg || trackMeApp_UL->speed_m_s) {
      uint16_t  crs     = trackMeApp_UL->course_deg % 360;
      float     spd     = trackMeApp_UL->speed_m_s;
      uint32_t  spdMan  = 0UL;
      uint8_t   spdExp  = 32U;

      if (spd) {
        spdExp  = (32 - 4) + ceil(log10(spd));
        spdMan  = spd * pow(10, (32 - spdExp));
      }

      outBuf[8]    = (crs >>  1) & 0xff;                                                     // 8 bits
      outBuf[9]    = (crs <<  7) & 0x80;                                                     // 1 bit

      outBuf[9]   |= (spdMan >> 10) & 0x7f;                                                     // 7 bits (5 decimal digits)
      outBuf[10]   = (spdMan >>  2) & 0xff;                                                     // 8 bits
      outBuf[11]   = (spdMan <<  6) & 0xc0;                                                     // 2 bits

      outBuf[11]  |=  spdExp        & 0x3f;                                                     // 6 bits
      outLen      += 4;

      if (trackMeApp_UL->vertspeed_m_s    ||
          trackMeApp_UL->vbat_mV          ||
          trackMeApp_UL->ibat_uA          ||
          trackMeApp_UL->temp_100th_C     ||
          trackMeApp_UL->humitidy_1000th  ||
          trackMeApp_UL->baro_Pa) {
        uint8_t flags = 0x00;
        float     vSpd  = trackMeApp_UL->vertspeed_m_s;
        uint16_t  vbat  = trackMeApp_UL->vbat_mV;
        int32_t   ibat  = trackMeApp_UL->ibat_uA;
        int32_t   temp  = trackMeApp_UL->temp_100th_C;
        uint16_t  rh    = trackMeApp_UL->humitidy_1000th;
        uint32_t  baro  = trackMeApp_UL->baro_Pa;

        if (fabs(vSpd) >= 0.1f) {
          flags |= 0x01;
        }
        if (vbat) {
          flags |= 0x04;
        }
        if (ibat) {
          flags |= 0x08;
        }
        if (temp) {
          flags |= 0x10;
        }
        if (rh) {
          flags |= 0x20;
        }
        if (baro) {
          flags |= 0x40;
        }
        outBuf[outLen++] = flags;

        if (fabs(vSpd) >= 0.1f) {
          uint8_t   vSpdSgn =  0;
          uint32_t  vSpdMan =  0;
          uint8_t   vSpdExp = 32;

          if (vSpd < 0.0) {
            vSpdSgn = 1;
            vSpd = -vSpd;
          }
          vSpdExp  = (32 - 4) + ceil(log10(vSpd));
          vSpdMan  = vSpd * pow(10, (32 - vSpdExp));

          outBuf[outLen] = (vSpdSgn << 7);                                                      // 1 bit

          outBuf[outLen++]  |= (vSpdMan >> 10) & 0x7f;                                          // 7 bits (5 decimal digits)
          outBuf[outLen++]   = (vSpdMan >>  2) & 0xff;                                          // 8 bits
          outBuf[outLen]     = (vSpdMan <<  6) & 0xc0;                                          // 2 bits

          outBuf[outLen++]  |=  vSpdExp        & 0x3f;                                          // 6 bits
        }

        if (vbat) {
          vbat /= 10;

          outBuf[outLen++]   = (vbat  >> 8) & 0xff;
          outBuf[outLen++]   =  vbat        & 0xff;
        }

        if (ibat) {
          uint8_t   ibatSgn  = 0;
          uint16_t  ibatBA;

          if (ibat < 0) {
            ibatSgn   = 1;
            ibatBA    = (uint16_t) (-ibat / 100);
          } else {
            ibatBA    = (uint16_t) ( ibat / 100);
          }

          outBuf[outLen]     = (ibatSgn << 7) & 0x80;
          outBuf[outLen++]  |= (ibatBA  >> 8) & 0x7f;
          outBuf[outLen++]   =  ibatBA        & 0xff;
        }

        if (temp) {
          uint8_t   tempSgn  = 0;
          uint16_t  tempBA;

          if (temp < 0) {
            tempSgn   = 1;
            tempBA    = (uint16_t) (-temp);
          } else {
            tempBA    = (uint16_t) ( temp);
          }

          outBuf[outLen]     = (tempSgn << 7) & 0x80;
          outBuf[outLen++]  |= (tempBA  >> 8) & 0x7f;
          outBuf[outLen++]   =  tempBA        & 0xff;
        }

        if (rh) {
          uint8_t rhBA    = (int8_t) (rh / 5);

          outBuf[outLen++] = rhBA;                                                               // 8 bits
        }

        if (baro) {
          uint16_t  baroBA  = baro >> 1;

          outBuf[outLen++]  = (baroBA >> 8) & 0xff;                                             // 8 bits
          outBuf[outLen++]  =  baroBA       & 0xff;                                             // 8 bits
        }
      }
    }
  }
  return outLen;
}

static uint8_t LoRaWAN_marshalling_PayloadExpand_TrackMeAppDown(TrackMeApp_down_t* trackMeApp_DL, const uint8_t* compressedMsg, uint8_t compressedMsgLen)
{

  return 0;
/*
function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};

  decoded.latitude  = 0.0;
  decoded.longitude = 0.0;
  decoded.altitude  = 0;
  decoded.accuracy  = 1262.0;
  decoded.course    = 0;
  decoded.speed     = 0;
  decoded.flags     = 0x00;

  // if (port === 1) decoded.led = bytes[0];
  if (port === 1) {
    if (bytes.length >= 8) {
      decoded.latitude  =   -90.0 + ((((bytes[0] & 0xff) << 13) | ((bytes[1] & 0xff) <<  5) | ((bytes[2] & 0xf8) >> 3)) / 10000.0);
      decoded.longitude =  -180.0 + ((((bytes[2] & 0x07) << 19) | ((bytes[3] & 0xff) << 11) | ((bytes[4] & 0xff) << 3) | ((bytes[5] & 0xe0) >> 5)) / 10000.0);
      decoded.altitude  = -8192.0 +  (((bytes[5] & 0x1f) << 11) | ((bytes[6] & 0xff) <<  3) | ((bytes[7] & 0xe0) >> 5));
      decoded.accuracy  = Math.pow(1.25, (bytes[7] & 0x1f));

      if (bytes.length >= 12) {
        var spdMan = 0;
        var spdExp = 32;

        decoded.course  = ((bytes[8] & 0xff) << 1) | ((bytes[9] & 0x80) >> 7);

        spdMan = ((bytes[9]  & 0x7f) << 10) | (bytes[10] << 2) | ((bytes[11] & 0xc0) >> 6);
        spdExp = ( bytes[11] & 0x3f);
        decoded.speed = Number(spdMan) * Math.pow(10, (spdExp - 32));

        if (bytes.length >= 13) {
          var idx = 12;

          decoded.flags = bytes[idx++];

          if (decoded.flags & 0x01) {
            var vSpdSgn   = (bytes[idx]     & 0x80) >> 7;
            var vSpdMan   = (bytes[idx++]   & 0x7f) << 10;
                vSpdMan  |= (bytes[idx++]   & 0xff) << 2;
                vSpdMan  |= (bytes[idx]     & 0xc0) >> 6;
            var vSpdExp   = (bytes[idx++]   & 0x3f);

            vSpd = Number(vSpdMan) * Math.pow(10, (vSpdExp - 32));
            if (vSpdSgn) {
              vSpd = -vSpd;
            }
            decoded.vertspeed = vSpd;
          }

          if (decoded.flags & 0x04) {
            var vbat     = (bytes[idx++] & 0xff) << 8;
                vbat    |=  bytes[idx++];

            decoded.vbat = vbat/ 100.0;
          }

          if (decoded.flags & 0x08) {
            var ibatSgn  = (bytes[idx] & 0x80) >> 7;
            var ibat     = (bytes[idx++] & 0x7f) << 8;
                ibat    |=  bytes[idx++];

            if (ibatSgn === 1) {
              ibat= -ibat;
            }

            decoded.ibat = ibat/ 10000.0;
          }

          if (decoded.flags & 0x10) {
            var tempSgn = (bytes[idx] & 0x80) >> 7;
            var temp   = (bytes[idx++] & 0x7f) << 8;
                temp  |=  bytes[idx++];

            if (tempSgn === 1) {
              temp = -temp;
            }

            decoded.temp = temp / 100.0;
          }

          if (decoded.flags & 0x20) {
            decoded.hygro = bytes[idx++] / 2.0;
          }

          if (decoded.flags & 0x40) {
            var baro   = bytes[idx++] << 8;
                baro  |= bytes[idx++];
            decoded.baro = baro / 50.0;
          }
        }
      }
    }
  }

  // Thin security
  decoded.ts = Date.now();
  decoded.cm = ((0x0392ea84 + (decoded.ts & 0x7fffffff)) % 0x17ef23ab) ^ ((0x339dc627 + (decoded.ts & 0x07ffffff) * 7) % 0x2a29e2cc);
  return decoded;
}
 */
}


#ifdef NEW
static uint8_t LoRaWAN_marshalling_PayloadCompress_LoraliveAppUp(const LoraliveApp_up_t* loraliveApp_UL)
{

  return 0;

/*



 */
}

static uint8_t LoRaWAN_marshalling_PayloadExpand_LoraliveAppDown(LoraliveApp_up_t* loraliveApp_DL, const uint8_t* compressedMsg, uint8_t compressedMsgLen)
{

  return 0;

/*
function Decoder(bytes, port) {
  var voltage = bytes[0]/32;
  var dust025 = (256.0*bytes[1] + bytes[2])/10;
  var dust100 = (256.0*bytes[3] + bytes[4])/10;
  var id = String.fromCharCode(bytes[5]);
  if(bytes.length == 14) {
    var latitude = (256.0*256*256*bytes[6]+256*256*bytes[7]+256*bytes[8]+bytes[9])/1000;
    var longitude = (256.0*256*256*bytes[10]+256*256*bytes[11]+256*bytes[12]+bytes[13])/1000;
    return {
      state:[id,voltage],
      dust:[dust025,dust100],
      position:[latitude,longitude]
    };
  } if(bytes.length == 16) {
    var temperature = bytes[6];
    var humidity = bytes[7];
    var latitude = (256.0*256*256*bytes[8]+256*256*bytes[9]+256*bytes[10]+bytes[11])/1000;
    var longitude = (256.0*256*256*bytes[12]+256*256*bytes[13]+256*bytes[14]+bytes[15])/1000;
    return {
      state:[id,voltage],
      dust:[dust025,dust100, temperature, humidity],
      position:[latitude,longitude]
    };
  } if(bytes.length == 8) {
    var temperature = bytes[6];
    var humidity = bytes[7];
    return {
      state:[id,voltage],
      dust:[dust025,dust100, temperature, humidity],
    };
  } else {
    return {
      state:[id,voltage],
      dust:[dust025,dust100],
    };
  }
}
// sample payload (in hexadecimal)
// 90 00 30 00 60 4D 00 00 C0 8F 00 00 21 F8
// 90 = 4.5V
//    00 30 = 4.8 ppm of 2.5 μm fine dust
//          00 60 = 9.6 ppm 10 μm fine dust
//                4D = 'M' Marcus fine dust sensor
//                    00 00 C0 8F = 49.295° North (Wiesloch)
//                              00 00 21 F8 = 8.696° West (Wiesloch)
// 90 00 30 00 60 4D 10 10
//
 */
}
#endif


#ifdef DOES_THIS_MAKE_SENSE
static void LoRaWAN_marshalling_PayloadUpCompress(LoRaWAN_RX_Message_t* msg)
{
  if (msg->msg_parted_FRMPayload_Len) {
    uint8_t variant = 1;

    switch (variant) {
    case /* PayloadCompress_TrackMeAppUp */ 1:
      {
        TrackMeApp_up_t trackMeApp_DL = { 0 };

        /* Expand the message into TrackMeApp data structure */
        LoRaWAN_marshalling_PayloadCompress_TrackMeAppUp(&trackMeApp_DL, msg->msg_parted_FRMPayload_Buf, msg->msg_parted_FRMPayload_Len);

        /* Send message to the controller */
        // TODO
      }
      break;

    case /* PayloadCompress_LoraliveAppUp */ 2:
      {
        LoraliveApp_up_t loraliveApp_DL = { 0 };

        LoRaWAN_marshalling_PayloadCompress_LoraliveAppUp(&loraliveApp_DL, msg->msg_parted_FRMPayload_Buf, msg->msg_parted_FRMPayload_Len);

        /* Send message to the controller */
        // TODO
      }
      break;
    }
  }
}

static void LoRaWAN_marshalling_PayloadDownExpand(LoRaWAN_RX_Message_t* msg)
{
  if (msg->msg_parted_FRMPayload_Len) {
    uint8_t variant = 1;

    switch (variant) {
    case /* PayloadExpand_TrackMeApp */ 1:
      {
        TrackMeApp_up_t trackMeApp_DL = { 0 };

        /* Expand the message into TrackMeApp data structure */
        LoRaWAN_marshalling_PayloadExpand_TrackMeAppDown(&trackMeApp_DL, msg->msg_parted_FRMPayload_Buf, msg->msg_parted_FRMPayload_Len);

        /* Send message to the controller */
        // TODO
      }
      break;

    case /* PayloadExpand_LoraliveApp */ 2:
      {
        LoraliveApp_up_t loraliveApp_DL = { 0 };

        LoRaWAN_marshalling_PayloadExpand_LoraliveAppDown(&loraliveApp_DL, msg->msg_parted_FRMPayload_Buf, msg->msg_parted_FRMPayload_Len);

        /* Send message to the controller */
        // TODO
      }
      break;
    }
  }
}
#endif


static void LoRaWAN_QueueIn_Process__Fsm_TX(void)
{
  /* Prepare to transmit data buffer */
  if (loRaWANctx.FsmState == Fsm_NOP) {
    /* Adjust the context */
    loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
    loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
    loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

    /* Requesting for confirmed data up-transport */
    loRaWANctx.MHDR_MType = ConfDataUp;
    loRaWANctx.MHDR_Major = LoRaWAN_R1;

    /* Request to encode TX message (again) */
    loRaWanTxMsg.msg_encoded_EncDone = 0;

    /* Start preparations for TX */
    loRaWANctx.FsmState = Fsm_TX;

  } else {
    /* Drop data when (again?) Join-Request is just exchanged */
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
      goto error_clrInBuf;
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

  case loraInQueueCmds__TrackMeApplUp:
    {
      /* Prepare data to upload */
      {
        /* Wait for semaphore to access LoRaWAN TrackMeApp */
        osSemaphoreWait(trackMeApplUpDataBinarySemHandle, 0);

        /* Marshal data for upload */
        loRaWanTxMsg.msg_prep_FRMPayload_Len = LoRaWAN_marshalling_PayloadCompress_TrackMeAppUp(loRaWanTxMsg.msg_prep_FRMPayload_Buf, &trackMeApp_up);

        /* Free semaphore */
        osSemaphoreRelease(trackMeApplUpDataBinarySemHandle);
      }

      /* Prepare to transmit data buffer */
      LoRaWAN_QueueIn_Process__Fsm_TX();
    }
    break;

#if 0
  case loraInQueueCmds__LoraliveApplUp:
    {
      /* Prepare data to upload */
      {
        /* Wait for semaphore to access LoRaWAN LoraliveApp */
        osSemaphoreWait(loraliveApplUpDataBinarySemHandle, 0);

        /* Marshal data for upload */
        loRaWanTxMsg.msg_prep_FRMPayload_Len = LoRaWAN_marshalling_PayloadCompress_LoraliveAppUp(&(loRaWanTxMsg.msg_prep_FRMPayload_Buf), &loraliveApp_up);

        /* Free semaphore */
        osSemaphoreRelease(loraliveApplUpDataBinarySemHandle);
      }

      /* Prepare to transmit data buffer */
      LoRaWAN_QueueIn_Process__Fsm_TX();
    }
    break;
#endif

  case loraInQueueCmds__NOP:
  default:
    /* Nothing to do */
    { }
  }  // switch (buf[0])


error_clrInBuf:
  {
    /* Clear the buffer to sync */
    bufCtr = bufMsgLen = 0;
    memset(buf, 0, sizeof(buf));
  }
}

static void LoRaWAN_QueueOut_Process(uint8_t signal)
{
  switch (signal) {
    case LoRaSIG_Ready:
      {
        const uint8_t c_qM[2] = { 1, (uint8_t)'R' };

        for (uint8_t idx = 0; idx < sizeof(c_qM); idx++) {
          xQueueSendToBack(loraOutQueueHandle, c_qM + idx, LoRaWAN_MaxWaitMs / portTICK_PERIOD_MS);
        }

        /* Set QUEUE_OUT bit */
        xEventGroupSetBits(loRaWANEventGroupHandle, LORAWAN_EGW__QUEUE_OUT);
      }
      break;
  }
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
    mhz = 868.100f;                                                                             // SF7BW125 to SF12BW125 - default value which never changes
    break;

  case 2:
    mhz = 868.300f;                                                                             // SF7BW125 to SF12BW125  and  SF7BW250 - default value which never changes
    break;

  case 3:
    mhz = 868.500f;                                                                             // SF7BW125 to SF12BW125 - default value which never changes
    break;

  case 4:
    mhz = 867.100f;                                                                             // SF7BW125 to SF12BW125
    break;

  case 5:
    mhz = 867.300f;                                                                             // SF7BW125 to SF12BW125
    break;

  case 6:
    mhz = 867.500f;                                                                             // SF7BW125 to SF12BW125
    break;

  case 7:
    mhz = 867.700f;                                                                             // SF7BW125 to SF12BW125
    break;

  case 8:
    mhz = 867.900f;                                                                             // SF7BW125 to SF12BW125
    break;

  case 9:
    mhz = 868.800f;                                                                             // FSK
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
          MHDR_SIZE + sizeof(ctx->AppEUI_LE) + sizeof(ctx->DevEUI_LE) + sizeof(ctx->DevNonce_LE),
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
          0x49,
          { 0x00, 0x00, 0x00, 0x00 },
          (uint8_t) Up,
          { ctx->DevAddr_LE[0],
            ctx->DevAddr_LE[1],
            ctx->DevAddr_LE[2],
            ctx->DevAddr_LE[3]},
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

static void LoRaWAN_calc_TxMsg_Reset(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* Reset timestamp of last transmission */
  ctx->TsEndOfTx = 0UL;

  /* Clear TX message buffer */
  memset(msg, 0, sizeof(LoRaWAN_TX_Message_t));

  /* Pre-sets for a new TX message */
  ctx->FCtrl_ADR              = ctx->ADR_enabled;
  ctx->FCtrl_ADRACKReq        = 0;
  ctx->FCtrl_ACK              = 0;
  ctx->FCtrl_ClassB           = 0;
  ctx->FPort_absent           = 1;
  ctx->FPort                  = 0;
}

static void LoRaWAN_calc_TxMsg_MAC_JOINREQUEST(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* JoinRequest does not have encryption and thus no packet compilation follows, instead write directly to msg_encoded[] */

  /* Start new message and write directly to msg_encoded[] - no packet compilation used here */
  memset(msg, 0, sizeof(LoRaWAN_TX_Message_t));

  /* MHDR */
  uint8_t l_MHDR = (((uint8_t) JoinRequest) << LoRaWAN_MHDR_MType_SHIFT) |
                   (((uint8_t) LoRaWAN_R1)  << LoRaWAN_MHDR_Major_SHIFT);
  msg->msg_encoded_Buf[msg->msg_encoded_Len++] = l_MHDR;

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
  }

  /* MIC */
  LoRaWAN_calc_TX_MIC(ctx, msg, (uint8_t*)(msg->msg_encoded_Buf + msg->msg_encoded_Len), MIC_JOINREQUEST);
  msg->msg_encoded_Len += 4;

  /* Packet now ready for TX */
}

static void LoRaWAN_calc_TxMsg_Compiler(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  /* Start new encoded message */
  {
    msg->msg_encoded_Len = 0;
    memset((uint8_t*)msg->msg_encoded_Buf, 0, sizeof(msg->msg_encoded_Buf));
  }

  /* PHYPayload */
  {
    /* MHDR */
    {
      uint8_t l_MHDR = (((uint8_t) ctx->MHDR_MType) << LoRaWAN_MHDR_MType_SHIFT) |
                       (((uint8_t) ctx->MHDR_Major) << LoRaWAN_MHDR_Major_SHIFT);
      msg->msg_encoded_Buf[msg->msg_encoded_Len++] = l_MHDR;
    }

    /* MACPayload */
    {
      /* FHDR */
      {
        /* DevAddr */
        for (uint8_t i = 0; i < 4; i++) {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->DevAddr_LE[i];
        }

        /* FCtrl */
        {
          uint8_t l_FCtrl = (ctx->FCtrl_ADR       << LoRaWAN_FCtl_ADR_SHIFT)       |
                            (ctx->FCtrl_ADRACKReq << LoRaWAN_FCtl_ADRACKReq_SHIFT) |
                            (ctx->FCtrl_ACK       << LoRaWAN_FCtl_ACK_SHIFT)       |
                            (ctx->TX_MAC_Len      << LoRaWAN_FCtl_FOptsLen_SHIFT );

          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = l_FCtrl;

          /* Reset acknowledges */
          ctx->FCtrl_ACK        = 0;
          ctx->FCtrl_ADRACKReq  = 0;
        }

        /* FCnt */
        {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 0);
          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = GET_BYTE_OF_WORD(ctx->bkpRAM->FCntUp, 1);
        }

        /* FOpts (V1.1: encoded) - not emitted when msg_FOpts_Len == 0 */
        {
#ifdef LORAWAN_1V02
          for (uint8_t l_FOpts_Idx = 0; l_FOpts_Idx < ctx->TX_MAC_Len; l_FOpts_Idx++) {
            msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->TX_MAC_Buf[l_FOpts_Idx];
          }
#elif LORAWAN_1V1
          if (msg->msg_prep_FOpts_Len) {
            LoRaWAN_calc_FOpts_Encrypt(ctx, msg->msg_prep_FOpts_Encoded, msg->msg_prep_FOpts_Buf, msg->msg_prep_FOpts_Len);

            for (uint8_t FOpts_Idx = 0; FOpts_Idx < msg->msg_prep_FOpts_Len; FOpts_Idx++) {
              msg->msg_encoded_Buf[msg->msg_encoded_Len++]  = msg->msg_prep_FOpts_Encoded[FOpts_Idx];
            }
          }
#endif

          /* Reset for next entries to be filled in */
          ctx->TX_MAC_Len = 0;
          memset((uint8_t*) ctx->TX_MAC_Buf, 0, sizeof(ctx->TX_MAC_Buf));
        }  // FOpts
      }  // FHDR

      /* FPort - not emitted when FPort_absent is set */
      {
        if (msg->msg_prep_FRMPayload_Len) {
          ctx->FPort_absent = 0;
          ctx->FPort        = FPort_TrackMeAppl_Default;
        } else {
          ctx->FPort_absent = 1;
        }

        if (!(ctx->FPort_absent)) {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = ctx->FPort;
        }
      }

      /* FRMPayload - not emitted when msg_prep_FRMPayload_Len == 0 */
      if (msg->msg_prep_FRMPayload_Len) {
        /* Encode FRMPayload*/
        LoRaWAN_calc_FRMPayload_Encrypt(ctx, msg);

        /* Copy into target msg_encoded_Buf[] */
        for (uint8_t l_FRMPayload_Idx = 0; l_FRMPayload_Idx < msg->msg_prep_FRMPayload_Len; l_FRMPayload_Idx++) {
          msg->msg_encoded_Buf[msg->msg_encoded_Len++] = msg->msg_prep_FRMPayload_Encoded[l_FRMPayload_Idx];
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


static void LoRaWAN_calc_RxMsg_Reset(LoRaWAN_RX_Message_t* msg)
{
  /* Clear RX message buffer */
  memset(msg, 0, sizeof(LoRaWAN_RX_Message_t));
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
      uint8_t dlSettings    = decPad[decPadIdx++];
      ctx->LinkADR_DataRate_RXTX2       = (DataRates_t) (dlSettings & 0x0f);
      ctx->LinkADR_DataRate_RX1_DRofs   = (dlSettings & 0x70) >> 4;
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
      msg->msg_parted_MIC_Valid = micValid;
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

#ifdef LORAWAN_1V1
      if (ctx->MHDR_MType == JoinAccept)
      if (!(++(ctx->DevNonce_LE[0]))) {
        ++(ctx->DevNonce_LE[1]);
      }
#endif
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

void LoRaWAN_MAC_Queue_Push(const uint8_t* macBuf, uint8_t cnt)
{
  /* Send MAC commands with their options */
  for (uint8_t idx = 0; idx < cnt; ++idx) {
    BaseType_t xStatus = xQueueSendToBack(loraMacQueueHandle, macBuf + idx, LoRaWAN_MaxWaitMs / portTICK_PERIOD_MS);
    if (pdTRUE != xStatus) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

void LoRaWAN_MAC_Queue_Pull(uint8_t* macBuf, uint8_t cnt)
{
  /* Receive MAC commands with their options */
  for (uint8_t idx = 0; idx < cnt; ++idx) {
    BaseType_t xStatus = xQueueReceive(loraMacQueueHandle, macBuf + idx, LoRaWAN_MaxWaitMs / portTICK_PERIOD_MS);
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
    const LoraliveApp_up_t* app)
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


#define DEBUG_TX_TIMING

/* Push the complete message to the FIFO and go to transmission mode */
static void LoRaWAN_TX_msg(LoRaWANctx_t* ctx, LoRaWAN_TX_Message_t* msg)
{
  uint32_t now;

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
  {
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);                                  // Red on

    /* Start transmitter and wait until the message is being sent */
    {
      now = xTaskGetTickCount();
      spiSX127xMode(MODE_LoRa | ACCESS_SHARE_OFF | LOW_FREQ_MODE_OFF | TX);
    }

    /* Wait until TX has finished - the transceiver changes to STANDBY by itself */
    ctx->TsEndOfTx = spiSX127x_WaitUntil_TxDone(now + LORAWAN_EU868_MAX_TX_DURATION_MS);

    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);                                // Red off
  }

#ifdef DEBUG_TX_TIMING
  /* USB: info */
  {
    char usbDbgBuf[48];
    int  len;

    len = sprintf(usbDbgBuf, "LoRaWAN:\tb=%06lu,\te=%06lu\r\n", now, ctx->TsEndOfTx);
    usbLogLen(usbDbgBuf, len);
  }
#endif
}


#define DEBUG_RX_TIMING

static void LoRaWAN_RX_msg(LoRaWANctx_t* ctx, LoRaWAN_RX_Message_t* msg, uint32_t diffToTxStartMs, uint32_t diffToTxStopMs)
{
#ifdef DEBUG_RX_TIMING
  uint32_t rxBeginTs    = xTaskGetTickCount();
  uint32_t rxBalBeginTs = 0UL;
  uint32_t rxBalEndTs   = 0UL;
  uint32_t rxEndTs      = 0UL;
#endif

  /* Sanity checks - no receiver operation w/o transmission before */
  if (!ctx || !ctx->TsEndOfTx || !msg) {
    return;
  }

  TickType_t xLastWakeTime = ctx->TsEndOfTx;

  /* Clear receiving message buffer */
  LoRaWAN_calc_RxMsg_Reset(msg);

  uint32_t tsNow          = xTaskGetTickCount();
  uint32_t diffToTxNowMs  = tsNow - ctx->TsEndOfTx;

  /* Receive window */
  if (diffToTxNowMs < diffToTxStartMs) {
    /* Prepare RX */
    spiSX127x_TxRx_Preps(ctx, (ctx->Current_RXTX_Window == CurWin_RXTX2 ?  TxRx_Mode_RX2 : TxRx_Mode_RX), NULL);

    /* Prepare the FIFO */
    spiSX127xLoRa_Fifo_Init();
    spiSX127xLoRa_Fifo_SetFifoPtrFromRxBase();

    /* Sleep until JOIN_ACCEPT_DELAY1 window comes */
    vTaskDelayUntil(&xLastWakeTime, diffToTxStartMs / portTICK_PERIOD_MS);

    HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);                                  // Green on

    /* Receiver on */
    {
      /* Turn on receiver continuously and wait for the next message */
      spiSX127xMode(MODE_LoRa | ACCESS_SHARE_OFF | LOW_FREQ_MODE_OFF | RXCONTINUOUS);
      spiSX127x_WaitUntil_RxDone(ctx, msg, ctx->TsEndOfTx + diffToTxStopMs);

      /* After the RX time decide whether to be able for changing the frequency quickly or to turn the receiver off */
      spiSX127xMode(MODE_LoRa | ACCESS_SHARE_OFF | LOW_FREQ_MODE_OFF | ((ctx->Current_RXTX_Window == CurWin_RXTX2 ?  FSRX : STANDBY)));
    }
    rxEndTs = xTaskGetTickCount();

    HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);                                // Green off
  }

#ifdef DEBUG_RX_TIMING
  /* USB: info */
  {
    char usbDbgBuf[48];
    int  len;

    len = sprintf(usbDbgBuf, "LoRaWAN:\tb=%06lu,\tbb=%06lu,\tbe=%06lu,\te=%06lu\r\n", rxBeginTs, rxBalBeginTs, rxBalEndTs, rxEndTs);
    usbLogLen(usbDbgBuf, len);
  }
#endif
}


//#define INIT_BALANCING_ENABLED

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

  /* Set-up LoRaWAN context */
  {
    /* Copy default channel settings */
    for (uint8_t ch = 1; ch <= 8; ch++) {
      loRaWANctx.Ch_Frequencies_MHz[ch - 1] = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, ch, 1);  // Default values / default channels
    }

    loRaWANctx.Current_RXTX_Window          = CurWin_none;                                      // out of any window
    loRaWANctx.MHDR_MType                   = ConfDataUp;                                       // Confirmed data transport in use
    loRaWANctx.MHDR_Major                   = LoRaWAN_R1;                                       // Major release in use
    loRaWANctx.FPort_absent                 = 1;                                                // Without FRMPayload this field is disabled
    loRaWANctx.FPort                        = 1;                                                // Default application port
    loRaWANctx.ADR_enabled                  = 0 /* 1 */;        // TODO: remove specials        // Global setting for ADR
    loRaWANctx.LinkADR_TxPowerReduction_dB  = 0;                                                // No power reduction
    loRaWANctx.LinkADR_DataRate_TX1         = DR3_SF9_125kHz_LoRa  /* DR0_SF12_125kHz_LoRa */ ; // Start slowly
    loRaWANctx.LinkADR_DataRate_RX1_DRofs   = 0;                                                // Default
    loRaWANctx.LinkADR_DataRate_RXTX2       = DR3_SF9_125kHz_LoRa  /* DR0_SF12_125kHz_LoRa */ ; // Default for RXTX2
    loRaWANctx.LinkADR_ChannelMask          = 0x0007U;                                          // Enable default channels (1-3) only
    loRaWANctx.LinkADR_NbTrans              = 1;                                                // Default
    loRaWANctx.LinkADR_ChMaskCntl           = ChMaskCntl__appliesTo_1to16;                      // Mask settings are valid
  }

  /* Seed randomizer */
  {
    /* Prepare and start the receiver */
    loRaWANctx.FrequencyMHz                 = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        16,
        1);                                                                                     // Most traffic on the RX2 channel
    loRaWANctx.SpreadingFactor              = SF7_DR5_VAL;                                      // Use that SF for more noise
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

    /* Reset to POR/Reset defaults */
    spiSX127xReset();
  }

  /* Delay until USB DCD is ready */
  {
    uint32_t PreviousWakeTime = 0UL;

    osDelayUntil(&PreviousWakeTime, 3600);
  }

#ifdef INIT_BALANCING_ENABLED
  /* I/Q balancing - no SX127x reset or band-change without re-balancing */
  {
    /* Set center frequency of EU-868 */
    loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        1,
        1);                                                                                     // First default channel is about in the middle of the band

    /* Do I/Q balancing with that center frequency */
    spiSX127x_TxRx_Preps(&loRaWANctx, TxRx_Mode_IQ_Balancing, NULL);
  }
#endif

  /* Start with JOIN-REQUEST */
  loRaWANctx.FsmState = Fsm_MAC_JoinRequest;
}


static void loRaWANLoRaWANTaskLoop__Fsm_RX1(void)
{
  if (loRaWANctx.TsEndOfTx) {
    /* TX response after DELAY1 at RX1 - switch on receiver */

    /* USB: info */
    usbLog("LoRaWAN: RX1 (1 s).\r\n");

    /* Reset to POR/Reset defaults */
    spiSX127xReset();

    /* Balance I/Q  */
    spiSX127x_TxRx_Preps(&loRaWANctx, TxRx_Mode_IQ_Balancing, NULL);

    /* Accelerated RX1 on request by GW */
    loRaWANctx.SpreadingFactor = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

    /* Gateway response after DELAY1 at RX1 - switch on receiver */
    LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
        LORAWAN_EU868_DELAY1_MS - LORAWAN_RX_PREPARE_MS,
        LORAWAN_EU868_DELAY2_MS - LORAWAN_FRQ_HOPPING_MS);                                      // Same frequency and SF as during transmission

    if (loRaWanRxMsg.msg_encoded_Len == 0) {
      /* Listen to next window */
      loRaWANctx.Current_RXTX_Window  = CurWin_RXTX2;
      loRaWANctx.FsmState             = Fsm_RX2;

    } else {
      /* USB: info */
      usbLog("LoRaWAN: received packet within RX1 window.\r\n");
      usbLog("LoRaWAN: (TRX off)\r\n");

      /* Process message */
      loRaWANctx.FsmState = Fsm_MAC_Decoder;
    }
  }
}

static void loRaWANLoRaWANTaskLoop__Fsm_RX2(void)
{
  if (loRaWANctx.TsEndOfTx) {
    /* TX response after DELAY2 at RX2 */

    /* USB: info */
    usbLog("LoRaWAN: RX2 (2 s).\r\n");

    /* Jump to RX2 frequency (default frequency) */
    loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        0,
        1);

    /* Common SF for RXTX2 */
    loRaWANctx.SpreadingFactor = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_RXTX2);

    /* Gateway response after DELAY2 at RX2 */
    LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
        LORAWAN_EU868_DELAY2_MS - LORAWAN_RX_PREPARE_MS,
        LORAWAN_EU868_DELAY2_MS + LORAWAN_EU868_MAX_TX_DURATION_MS - LORAWAN_FRQ_HOPPING_MS);

    if (loRaWanRxMsg.msg_encoded_Len == 0) {
      /* No message received - try again if packet is of confirmed type */
      if (ConfDataUp == loRaWANctx.MHDR_MType) {
        /* USB: info */
        usbLog("LoRaWAN: Failed to RX.\r\n\r\n");

#ifdef DO_RESEND
        loRaWANctx.FsmState = Fsm_TX;

        /* Delay 2..10 secs */
        osDelay(2000 + rand() % 8000);
#else
        loRaWANctx.FsmState = Fsm_NOP;
#endif

      } else {
        loRaWANctx.FsmState = Fsm_NOP;
      }

    } else {
      /* USB: info */
      usbLog("LoRaWAN: received packet within RX2 window.\r\n");

      /* Process message */
      loRaWANctx.FsmState = Fsm_MAC_Decoder;
    }
  }
}

static void loRaWANLoRaWANTaskLoop__JoinRequest_NextTry(void)
{
  if (JoinRequest == loRaWANctx.MHDR_MType) {
    /* Try again with new JOINREQUEST - clears loRaWanTxMsg by itself */
    loRaWANctx.FsmState = Fsm_MAC_JoinRequest;

  } else if (ConfDataUp == loRaWANctx.MHDR_MType) {
    switch (loRaWANctx.TX_MAC_Buf[0]) {
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
  LoRaWAN_calc_TxMsg_Reset(&loRaWANctx, &loRaWanTxMsg);

  /* Remove RX message */
  LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

  /* Delay 2s before retransmitting */
  EventBits_t eb = xEventGroupWaitBits(loRaWANEventGroupHandle, LORAWAN_EGW__QUEUE_IN, LORAWAN_EGW__QUEUE_IN, 0, 1000 / portTICK_PERIOD_MS);
  if (eb) {
    /* New message came in - Rest of sleep time dropped */
    LoRaWAN_QueueIn_Process();
  }
}


static void loRaWANLoRaWANTaskLoop__JoinRequestRX1(void)
{
  if (loRaWANctx.TsEndOfTx) {
    /* JOIN-ACCEPT response after JOIN_ACCEPT_DELAY1 at RX1 - switch on receiver */

    /* USB: info */
    usbLog("LoRaWAN: JOIN-ACCEPT RX1 (5 s).\r\n");

    /* Reset to POR/Reset defaults */
    spiSX127xReset();

    /* Balance I/Q  */
    spiSX127x_TxRx_Preps(&loRaWANctx, TxRx_Mode_IQ_Balancing, NULL);

    /* Accelerated RX1 on request by GW */
    loRaWANctx.SpreadingFactor = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

    /* Receive on RX1 frequency */
    LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
        LORAWAN_EU868_JOIN_ACCEPT_DELAY1_MS - LORAWAN_RX_PREPARE_MS,
        LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS - LORAWAN_FRQ_HOPPING_MS);                          // Same frequency and SF as during transmission

    if (loRaWanRxMsg.msg_encoded_Len == 0) {
      /* Receive response at JOINREQUEST_RX2 */
      loRaWANctx.Current_RXTX_Window  = CurWin_RXTX2;
      loRaWANctx.FsmState             = Fsm_JoinRequestRX2;

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

static void loRaWANLoRaWANTaskLoop__JoinRequestRX2(void)
{
  if (loRaWANctx.TsEndOfTx) {
    /* JOIN-ACCEPT response after JOIN_ACCEPT_DELAY2 at RX2 */

    /* USB: info */
    usbLog("LoRaWAN: JOIN-ACCEPT RX2 (6 s).\r\n");

    /* Switch to RX2 frequency */
    loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
        &loRaWANctx,
        0,
        1);                                                                                     // Jump to RX2 frequency (default frequency)

    /* Common SF for RXTX2 */
    loRaWANctx.SpreadingFactor = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_RXTX2);

    /* Listen on RX2 frequency */
    LoRaWAN_RX_msg(&loRaWANctx, &loRaWanRxMsg,
        LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS - LORAWAN_RX_PREPARE_MS,
        LORAWAN_EU868_JOIN_ACCEPT_DELAY2_MS + LORAWAN_EU868_MAX_TX_DURATION_MS - LORAWAN_FRQ_HOPPING_MS);

    if (loRaWanRxMsg.msg_encoded_Len == 0) {
      usbLog("LoRaWAN: Failed to RX.\r\n");

#if 0
      if (loRaWANctx.SpreadingFactor < SF12_DR0_VAL) {
        loRaWANctx.SpreadingFactor++;
      }
#endif

      loRaWANLoRaWANTaskLoop__JoinRequest_NextTry();

    } else {  // if (loRaWanRxMsg.msg_encoded_Len == 0) {} else
      /* USB: info */
      usbLog("LoRaWAN: JOIN-RESPONSE received within window JR-Delay_RX2.\r\n");

      if (JoinRequest == loRaWANctx.MHDR_MType) {
        /* Continue with the JoinAccept handling */
        loRaWANctx.FsmState = Fsm_MAC_JoinAccept;

      } else if (ConfDataUp == loRaWANctx.MHDR_MType) {
        /* Process the message */
        loRaWANctx.FsmState = Fsm_MAC_Decoder;
      }
    }

  } else {  // if (tsEndOfTx)
    /* Sequence has ended */
    LoRaWAN_calc_TxMsg_Reset(&loRaWANctx, &loRaWanTxMsg);

    /* Remove RX message */
    LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

    /* Reset FSM */
    LoRaWAN_MAC_Queue_Reset();
    loRaWANctx.FsmState = Fsm_NOP;
  }
}

/* Compile message and TX */
static void loRaWANLoRaWANTaskLoop__Fsm_TX(void)
{
  /* Clear RX msg buffer */
  {
    LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);
    LoRaWAN_MAC_Queue_Reset();
  }

  /* Compile packet if not done, already */
  if (!(loRaWanTxMsg.msg_encoded_EncDone)) {
    LoRaWAN_calc_TxMsg_Compiler(&loRaWANctx, &loRaWanTxMsg);
  }

  /* USB: info */
  usbLog("LoRaWAN: TX packet.\r\n");

  /* Randomized TX1 frequency */
  loRaWANctx.FrequencyMHz = LoRaWAN_calc_Channel_to_MHz(
      &loRaWANctx,
      LoRaWAN_calc_randomChannel(&loRaWANctx),
      0);
  loRaWANctx.SpreadingFactor = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare transmitter and go on-air */
  LoRaWAN_TX_msg(&loRaWANctx, &loRaWanTxMsg);

  /* Receive response at RX1 */
  loRaWANctx.FsmState = Fsm_RX1;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_Decoder(void)
{
  /* Decode received message */

  /* Non-valid short message */
  if (loRaWanRxMsg.msg_encoded_Len < 5) {
    /* USB: info */
    usbLog("LoRaWAN: Decoder claims: message too short.\r\n\r\n");

    /* Sequence has ended */
    LoRaWAN_calc_TxMsg_Reset(&loRaWANctx, &loRaWanTxMsg);

    /* Remove RX message */
    LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

    if (ConfDataUp == loRaWANctx.MHDR_MType) {
      switch (loRaWANctx.TX_MAC_Buf[0]) {
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
    return;
  }

  /* Decode the RX msg */
  {
    LoRaWAN_calc_RxMsg_Decoder(&loRaWANctx, &loRaWanRxMsg);

    if (loRaWanRxMsg.msg_parted_MIC_Valid) {
      /* USB: info */
      usbLog("LoRaWAN: Decoder informs: successfully decoded packet!\r\n\r\n");

      /* Default setting */
      loRaWANctx.FsmState = Fsm_NOP;

      /* Single MAC command can be handled within the FSM directly */
      if ((loRaWanRxMsg.msg_parted_FCtrl_FOptsLen          > 0) &&                          // FOpts MAC
         ( loRaWanRxMsg.msg_parted_FPort_absent                 ||                          // no FPort0 MAC
          (loRaWanRxMsg.msg_parted_FPort                  != 0)))
      {
        /* Push all MAC data to the MAC queue from FOpts_Buf */
        LoRaWAN_MAC_Queue_Push(loRaWanRxMsg.msg_parted_FOpts_Buf, loRaWanRxMsg.msg_parted_FCtrl_FOptsLen);

        /* MAC processing */
        loRaWANctx.FsmState = Fsm_MAC_Proc;

      } else if ((loRaWanRxMsg.msg_parted_FCtrl_FOptsLen  == 0) &&                          // no FOpts MAC
                (!loRaWanRxMsg.msg_parted_FPort_absent)         &&                          // Payload MAC
                 (loRaWanRxMsg.msg_parted_FPort           == 0) &&
                 (loRaWanRxMsg.msg_parted_FRMPayload_Len   > 0))
      {
        /* Push all MAC data to the MAC queue from FRMPayload_Buf[] */
        LoRaWAN_MAC_Queue_Push(loRaWanRxMsg.msg_parted_FRMPayload_Buf, loRaWanRxMsg.msg_parted_FRMPayload_Len);

        /* MAC processing */
        loRaWANctx.FsmState = Fsm_MAC_Proc;
      }

      /* Downlink data processing */
      if ((loRaWanRxMsg.msg_parted_FPort > 0) && (loRaWanRxMsg.msg_parted_FRMPayload_Len > 0)) {
        /* Downlink TrackMeApp payload data */
        LoRaWAN_marshalling_PayloadExpand_TrackMeAppDown(&trackMeApp_down, loRaWanRxMsg.msg_parted_FRMPayload_Buf, loRaWanRxMsg.msg_parted_FRMPayload_Len);
      }

    } else {
      /* USB: info */
      usbLog("LoRaWAN: Decoder claims: message BAD format.\r\n\r\n");

      /* Sequence has ended */
      LoRaWAN_calc_TxMsg_Reset(&loRaWANctx, &loRaWanTxMsg);

      /* Remove RX message */
      LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

      /* Nothing to do with that */
      loRaWANctx.FsmState = Fsm_NOP;
    }
  }
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_Proc(void)
{
  /* MAC queue processing */

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
    return;
  }  // if (LoRaWAN_MAC_Queue_isAvail())

  /* Fall back to NOP */
  loRaWANctx.FsmState = Fsm_NOP;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_JoinRequest(void)
{
  /* MAC CID 0x01 - up:JoinRequest --> dn:JoinAccept */

  /* JoinRequest does reset frame counters */
  loRaWANctx.bkpRAM->FCntUp   = 0UL;
  loRaWANctx.bkpRAM->FCntDwn  = 0UL;

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
#ifdef PPM_CALIBRATION
  loRaWANctx.FrequencyMHz         = 870.0;
  loRaWANctx.SpreadingFactor      = SF12_DR0_VAL;
#else
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);
#endif

  /* USB: info */
  usbLog("\r\nLoRaWAN: JOIN-REQUEST going to be sent TX.\r\n");

  /* Compile packet */
  LoRaWAN_calc_TxMsg_MAC_JOINREQUEST(&loRaWANctx, &loRaWanTxMsg);

  /* Prepare transmitter and go on-air */
  LoRaWAN_TX_msg(&loRaWANctx, &loRaWanTxMsg);

  /* Expect response at JOINREQUEST_RX1 first */
  loRaWANctx.FsmState = Fsm_JoinRequestRX1;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_JoinAccept(void)
{
  /* JOIN-ACCEPT process the message */
  if (HAL_OK == LoRaWAN_calc_RxMsg_MAC_JOINACCEPT(&loRaWANctx, &loRaWanRxMsg)) {
    /* USB: info */
    usbLog("LoRaWAN: JOIN-RESPONSE message successfully decoded.\r\n\r\n");

    /* Sequence has ended */
    LoRaWAN_calc_TxMsg_Reset(&loRaWANctx, &loRaWanTxMsg);

    /* Remove RX message */
    LoRaWAN_calc_RxMsg_Reset(&loRaWanRxMsg);

    /* Next: do LinkCheckReq */
    loRaWANctx.FsmState = Fsm_MAC_LinkCheckReq;

  } else {
    /* USB: info */
    usbLog("LoRaWAN: JOIN-RESPONSE message failed to decode.\r\n\r\n");

    /* Try again with new JOINREQUEST */
    loRaWANLoRaWANTaskLoop__JoinRequest_NextTry();
  }
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkCheckReq(void)
{
  /* MAC CID 0x02 - up:LinkCheckReq --> dn:LinkCheckAns */

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Requesting for confirmed data up-transport */
  loRaWANctx.MHDR_MType = ConfDataUp;
  loRaWANctx.MHDR_Major = LoRaWAN_R1;

  loRaWANctx.TX_MAC_Buf[loRaWANctx.TX_MAC_Len++] = LinkCheckReq_UP;

  /* Request to encode TX message (again) */
  loRaWanTxMsg.msg_encoded_EncDone = 0;

  /* USB: info */
  usbLog("LoRaWAN: Going to TX LinkCheckReq.\r\n");

  /* Prepare for transmission */
 loRaWANctx.FsmState = Fsm_TX;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkCheckAns(void)
{
  uint8_t macBuf[2] = { 0 };

  /* Inform controller that link is established */
  LoRaWAN_QueueOut_Process(LoRaSIG_Ready);

  /* USB: info */
  usbLog("LoRaWAN: Got RX LinkCheckAns.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
  loRaWANctx.LinkCheck_Ppm_SNR  = macBuf[0];
  loRaWANctx.LinkCheck_GW_cnt   = macBuf[1];
  loRaWANctx.FsmState           = Fsm_MAC_Proc;

}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkADRReq(void)
{
  /* MAC CID 0x03 - dn:LinkADRReq --> up:LinkADRAns */

  uint8_t macBuf[4] = { 0 };
  uint8_t nextMac;

  /* USB: info */
  usbLog("LoRaWAN: Got RX LinkADRReq.\r\n");

  /* Iterate over all LinkADRReq MACs */
  do {
    LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
    loRaWANctx.LinkADR_TxPowerReduction_dB  = (macBuf[0] & 0x0f) << 1;
    loRaWANctx.LinkADR_DataRate_TX1         = (DataRates_t) ((macBuf[0] & 0xf0) >> 4);

    /* Prove each channel mask bit before acceptance */
    uint16_t newChMask                      =  macBuf[1] | ((uint16_t)macBuf[2] << 8);
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
      loRaWANctx.LinkADR_ChannelMask_OK     = 1;
    } else {
      loRaWANctx.LinkADR_ChannelMask_OK     = 0;
    }

    loRaWANctx.LinkADR_NbTrans              =  macBuf[3] & 0x0f;
    loRaWANctx.LinkADR_ChMaskCntl           = (ChMaskCntl_t) ((macBuf[3] & 0x70) >> 4);

    /* Acknowledge this packet */
    loRaWANctx.FCtrl_ACK = 1;

    if (!LoRaWAN_MAC_Queue_isAvail(&nextMac)) {
      break;
    }
    if (LinkADRReq_DN != nextMac) {
      break;
    }
  } while (1);

  loRaWANctx.FsmState = Fsm_MAC_LinkADRAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkADRAns(void)
{
  /* MAC to be added */
  loRaWANctx.TX_MAC_Buf[loRaWANctx.TX_MAC_Len++] = LinkADRAns_UP;
  loRaWANctx.TX_MAC_Buf[loRaWANctx.TX_MAC_Len++] = 0b010 | (0x1 & loRaWANctx.LinkADR_ChannelMask_OK);  // Power-change not OK, DataRate-change OK, ChannelMask-change is context
  loRaWanTxMsg.msg_encoded_EncDone = 0;

  /* USB: info */
  usbLog("LoRaWAN: Going to TX LinkADRAns.\r\n");

  /* Prepare for transmission */
  loRaWANctx.FsmState = Fsm_TX;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DutyCycleReq(void)
{
  /* MAC CID 0x04 - dn:DutyCycleReq --> up:DutyCycleAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX DutyCycleReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_DutyCycleAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DutyCycleAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX DutyCycleAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RXParamSetupReq(void)
{
  /* MAC CID 0x05 - dn:RXParamSetupReq --> up:RXParamSetupAns */

  uint8_t macBuf[4] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX RXParamSetupReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
//      loRaWANctx.xxx = macBuf[1];
//      loRaWANctx.xxx = macBuf[2];
//      loRaWANctx.xxx = macBuf[3];
  loRaWANctx.FsmState = Fsm_MAC_RXParamSetupAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RXParamSetupAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX RXParamSetupAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DevStatusReq(void)
{
  /* MAC CID 0x06 - dn:DevStatusReq --> up:DevStatusAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX DevStatusReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_DevStatusAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DevStatusAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX DevStatusAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_NewChannelReq(void)
{
  /* MAC CID 0x07 - dn:NewChannelReq --> up:NewChannelAns */

  uint8_t macBuf[5] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX NewChannelReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
//      loRaWANctx.xxx = macBuf[1];
//      loRaWANctx.xxx = macBuf[2];
//      loRaWANctx.xxx = macBuf[3];
//      loRaWANctx.xxx = macBuf[4];
  loRaWANctx.FsmState = Fsm_MAC_NewChannelAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_NewChannelAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX NewChannelAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RXTimingSetupReq(void)
{
  /* MAC CID 0x08 - dn:RXTimingSetupReq --> up:RXTimingSetupAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX RXTimingSetupReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_RXTimingSetupAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RXTimingSetupAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX RXTimingSetupAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_TxParamSetupReq(void)
{
  /* MAC CID 0x09 - dn:TxParamSetupReq --> up:TxParamSetupAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX TxParamSetupReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_TxParamSetupAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_TxParamSetupAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX TxParamSetupAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DlChannelReq(void)
{
  /* MAC CID 0x0A - dn:DlChannelReq --> up:DlChannelAns */

  uint8_t macBuf[4] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX DlChannelReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
//      loRaWANctx.xxx = macBuf[1];
//      loRaWANctx.xxx = macBuf[2];
//      loRaWANctx.xxx = macBuf[3];
  loRaWANctx.FsmState = Fsm_MAC_DlChannelAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DlChannelAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX DlChannelAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

#ifdef LORANET_1V1
static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RekeyConf(void)
{
  /* MAC CID 0x0? - dn:RekeyConf --> up:? */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX RekeyConf.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_RekeyConf2;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RekeyConf2(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX RekeyConf2.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_ADRParamSetupReq(void)
{
  /* MAC CID 0x0? - dn:ADRParamSetupReq --> up:ADRParamSetupAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX ADRParamSetupReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_ADRParamSetupAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_ADRParamSetupAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX ADRParamSetupAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DeviceTimeReq(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX DeviceTimeReq.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_DeviceTimeAns(void)
{
  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX DeviceTimeAns.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_Proc;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_ForceRejoinReq(void)
{
  /* MAC CID 0x0? - dn:ForceRejoinReq --> up:ForceRejoinAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX ForceRejoinReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//      loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_ForceRejoinAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_ForceRejoinAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX ForceRejoinAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RejoinParamSetupReq(void)
{
  /* MAC CID 0x0? - dn:RejoinParamSetupReq --> up:RejoinParamSetupAns */

  uint8_t macBuf[1] = { 0 };

  /* USB: info */
  usbLog("LoRaWAN: Got RX RejoinParamSetupReq.\r\n");

  LoRaWAN_MAC_Queue_Pull(macBuf, sizeof(macBuf));
//    loRaWANctx.xxx = macBuf[0];
  loRaWANctx.FsmState = Fsm_MAC_RejoinParamSetupAns;
}

static void loRaWANLoRaWANTaskLoop__Fsm_MAC_RejoinParamSetupAns(void)
{
  /* USB: info */
  usbLog("LoRaWAN: Going to TX RejoinParamSetupAns.\r\n");

  /* Adjust the context */
  loRaWANctx.Current_RXTX_Window  = CurWin_RXTX1;
  loRaWANctx.FrequencyMHz         = LoRaWAN_calc_Channel_to_MHz(&loRaWANctx, LoRaWAN_calc_randomChannel(&loRaWANctx), 0);                                                                                   // Randomized RX1 frequency
  loRaWANctx.SpreadingFactor      = spiSX127xDR_to_SF(loRaWANctx.LinkADR_DataRate_TX1 + loRaWANctx.LinkADR_DataRate_RX1_DRofs);

  /* Prepare for transmission */
  loRaWANctx.FsmState             = Fsm_NOP;    // TODO
}
#endif


void loRaWANLoRaWANTaskLoop(void)
{
  EventBits_t eb;

  switch (loRaWANctx.FsmState) {
  case Fsm_RX1:
    loRaWANLoRaWANTaskLoop__Fsm_RX1();
    break;

  case Fsm_RX2:
    loRaWANLoRaWANTaskLoop__Fsm_RX2();
    break;

  case Fsm_JoinRequestRX1:
    loRaWANLoRaWANTaskLoop__JoinRequestRX1();
    break;

  case Fsm_JoinRequestRX2:
    loRaWANLoRaWANTaskLoop__JoinRequestRX2();
    break;


  /* Compile message and TX */
  case Fsm_TX:
    loRaWANLoRaWANTaskLoop__Fsm_TX();
    break;


  /* Decode received message */
  case Fsm_MAC_Decoder:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_Decoder();
    break;


  /* MAC queue processing */
  case Fsm_MAC_Proc:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_Proc();
    break;


  /* MAC CID 0x01 - up:JoinRequest --> dn:JoinAccept */
  case Fsm_MAC_JoinRequest:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_JoinRequest();
    break;

  case Fsm_MAC_JoinAccept:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_JoinAccept();
    break;


  /* MAC CID 0x02 - up:LinkCheckReq --> dn:LinkCheckAns */
  case Fsm_MAC_LinkCheckReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkCheckReq();
    break;

  case Fsm_MAC_LinkCheckAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkCheckAns();
    break;


  /* MAC CID 0x03 - dn:LinkADRReq --> up:LinkADRAns */
  case Fsm_MAC_LinkADRReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkADRReq();
    break;

  case Fsm_MAC_LinkADRAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_LinkADRAns();
    break;


  /* MAC CID 0x04 - dn:DutyCycleReq --> up:DutyCycleAns */
  case Fsm_MAC_DutyCycleReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DutyCycleReq();
    break;

  case Fsm_MAC_DutyCycleAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DutyCycleAns();
    break;


  /* MAC CID 0x05 - dn:RXParamSetupReq --> up:RXParamSetupAns */
  case Fsm_MAC_RXParamSetupReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RXParamSetupReq();
    break;

  case Fsm_MAC_RXParamSetupAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RXParamSetupAns();
    break;


  /* MAC CID 0x06 - dn:DevStatusReq --> up:DevStatusAns */
  case Fsm_MAC_DevStatusReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DevStatusReq();
    break;

  case Fsm_MAC_DevStatusAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DevStatusAns();
    break;


  /* MAC CID 0x07 - dn:NewChannelReq --> up:NewChannelAns */
  case Fsm_MAC_NewChannelReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_NewChannelReq();
    break;

  case Fsm_MAC_NewChannelAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_NewChannelAns();
    break;


  /* MAC CID 0x08 - dn:RXTimingSetupReq --> up:RXTimingSetupAns */
  case Fsm_MAC_RXTimingSetupReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RXTimingSetupReq();
    break;

  case Fsm_MAC_RXTimingSetupAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RXTimingSetupAns();
    break;


  /* MAC CID 0x09 - dn:TxParamSetupReq --> up:TxParamSetupAns */
  case Fsm_MAC_TxParamSetupReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_TxParamSetupReq();
    break;

  case Fsm_MAC_TxParamSetupAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_TxParamSetupAns();
    break;


  /* MAC CID 0x0A - dn:DlChannelReq --> up:DlChannelAns */
  case Fsm_MAC_DlChannelReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DlChannelReq();
    break;

  case Fsm_MAC_DlChannelAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DlChannelAns();
    break;


#ifdef LORANET_1V1
  /* MAC CID 0x0? - dn:RekeyConf --> up:? */
  case Fsm_MAC_RekeyConf:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RekeyConf();
    break;

  case Fsm_MAC_RekeyConf2:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RekeyConf2();
    break;


  /* MAC CID 0x0? - dn:ADRParamSetupReq --> up:ADRParamSetupAns */
  case Fsm_MAC_ADRParamSetupReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_ADRParamSetupReq();
    break;

  case Fsm_MAC_ADRParamSetupAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_ADRParamSetupAns();
    break;


  case Fsm_MAC_DeviceTimeReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DeviceTimeReq();
    break;

  case Fsm_MAC_DeviceTimeAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_DeviceTimeAns();
    break;


  /* MAC CID 0x0? - dn:ForceRejoinReq --> up:ForceRejoinAns */
  case Fsm_MAC_ForceRejoinReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_ForceRejoinReq();
    break;

  case Fsm_MAC_ForceRejoinAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_ForceRejoinAns();
    break;


  /* MAC CID 0x0? - dn:RejoinParamSetupReq --> up:RejoinParamSetupAns */
  case Fsm_MAC_RejoinParamSetupReq:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RejoinParamSetupReq();
    break;

  case Fsm_MAC_RejoinParamSetupAns:
    loRaWANLoRaWANTaskLoop__Fsm_MAC_RejoinParamSetupAns();
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
