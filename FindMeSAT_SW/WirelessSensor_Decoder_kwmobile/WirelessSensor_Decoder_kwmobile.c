/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Distribution: The file is distributed "as is", without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Atollic AB permit registered Atollic TrueSTUDIO(R) users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the Atollic TrueSTUDIO(R) toolchain.
**
**
*****************************************************************************
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>


const char fileNamePfx[]  = "Z:\\Downloads\\WirelessSensor_Analytic\\";
const char fileNameDflt[] = "WirelessSensor_Analytic_SAMPLE.raw";


int main(int argc, const char* argv[]) {
#if 0
  printf("Hello World!\n");
  fflush(stdout);
#endif

  char codeOut[4096]    = { 0 };
  int  codeOutIdx       =   0;
  char fileNameIn[256]  = { 0 };
  char fileNameOut[256] = { 0 };
  struct stat statInfo  = { 0 };

	if (argc == 2 && argv[1]) {
	  strcpy(fileNameIn, argv[1]);

    if (-1 == stat(fileNameIn, &statInfo)) {
      printf("\r\nFILE NOT FOUND! %s\r\n\r\n", fileNameIn);
      return -1;
    }

	} else {
    strcpy(fileNameIn, fileNameDflt);
    if (-1 == stat(fileNameIn, &statInfo)) {
      strcpy(fileNameIn, fileNamePfx);
      strcat(fileNameIn, fileNameDflt);

      if (-1 == stat(fileNameIn, &statInfo)) {
        printf("\r\nDEFAULT FILE NOT FOUND!\r\n\r\n");
        return -1;
      }
    }
	}

	FILE* fpIn = fopen(fileNameIn, "rb");
	if (fpIn) {
    uint8_t  isOn     = 0;
    uint32_t pauseLen = 0UL;

    //fseek(fpIn, 0x44L, SEEK_SET);

    strcpy(fileNameOut, fileNameIn);
    strcat(fileNameOut, ".out");
	  FILE* fpOut = fopen(fileNameOut, "wb");
	  int32_t pos = 0L;

	  /* Signal detection */
	  {
      while (!feof(fpIn)) {
        uint8_t pauseCode = 0x00;

        uint8_t bl = fgetc(fpIn);
        if (feof(fpIn)) {
          break;
        }

        uint8_t bh = fgetc(fpIn);

        int16_t val = (int16_t) (((uint16_t)bh << 8) | bl);
        if (val > 5000 || val < -5000) {

          if (39 <= pauseLen && pauseLen <= 45) {
            pauseCode = 0x01;
          } else if (80 <= pauseLen && pauseLen <= 86) {
            pauseCode = 0x02;
          } else if (166 <= pauseLen && pauseLen <= 176) {
            pauseCode = 0x10;
          }

          if (!isOn && pauseCode) {
            codeOut[codeOutIdx++] = pauseCode;
          }

          isOn = 1;

        } else {
          if (isOn) {
            pauseLen = 0;
          } else {
            ++pauseLen;
          }

          isOn = 0;
        }

#if 0
        fprintf(fpOut, "Pos=%05d, val: %+06d, isOn: %x, pauseLen: %04d, pauseCode: %x\r\n", pos++, val, isOn, pauseLen, pauseCode);
#endif
        (void) pos;
      }
	  }

	  /* Decoder */
	  {
	    uint64_t codeAckd    = 0ULL;
	    uint8_t  codeAckdCtr = 0;
	    uint64_t codeThis   = 0ULL;
	    uint8_t  bitIdx     = 0;

      fprintf(fpOut, "\r\nCode out:\r\n");
      for (int idx = 0; idx < codeOutIdx; idx++) {
        char code = codeOut[idx];

        if (code == 0x01) {
          fputc('0', fpOut);
          codeThis <<= 1;
          bitIdx++;

        } else if (code == 0x02) {
          fputc('1', fpOut);
          codeThis <<= 1;
          codeThis |= 0x01;
          bitIdx++;

        } else if (code == 0x10) {
          fprintf(fpOut, "\r\n*\r\n");

          if (bitIdx == 36) {
            if (!codeAckd) {
              /* First time candidate code seen */
              codeAckd = codeThis;
              codeAckdCtr = 1;

            } else if (codeAckd == codeThis) {
              /* Code acknowledged */
              codeAckdCtr++;

            } else {
              /* Not the same */
              if (--codeAckdCtr == 0) {
                /* Previous code seems to be unreliable */
                codeAckd = 0ULL;
              }
            }
          }
          bitIdx = 0;
        }
      }

      int16_t temp  = (int16_t) ((codeAckd >> 12) & 0x7ff);
      uint8_t rh    = (uint8_t) ( codeAckd        & 0xff );

      fprintf(fpOut, "\r\n\r\nTemp = %+4.1fC   RH = %3d%%\r\n<EOF>", temp / 10.f, rh);

      fclose(fpOut);
      fclose(fpIn);
    }
	}

	return 0;
}

