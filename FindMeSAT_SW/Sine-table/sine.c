
#include <stdio.h>
#include <math.h>

#define MAX_SIN_STEP 50


int main(int argc, char* argv[])
{
	for (int idx = 0; idx < 4096; idx += 8) {
		unsigned short x0, x1, x2, x3, x4, x5, x6, x7;
		x0 = (unsigned short)(32768 + 32767 * __sinpi((idx + 0) * 2 / 4096.));
                x1 = (unsigned short)(32768 + 32767 * __sinpi((idx + 1) * 2 / 4096.));
                x2 = (unsigned short)(32768 + 32767 * __sinpi((idx + 2) * 2 / 4096.));
                x3 = (unsigned short)(32768 + 32767 * __sinpi((idx + 3) * 2 / 4096.));
                x4 = (unsigned short)(32768 + 32767 * __sinpi((idx + 4) * 2 / 4096.));
                x5 = (unsigned short)(32768 + 32767 * __sinpi((idx + 5) * 2 / 4096.));
                x6 = (unsigned short)(32768 + 32767 * __sinpi((idx + 6) * 2 / 4096.));
                x7 = (unsigned short)(32768 + 32767 * __sinpi((idx + 7) * 2 / 4096.));

		printf("\t0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x, 0x%04x,\n", x0, x1, x2, x3, x4, x5, x6, x7);
	}
	printf("\n");

	for (int var = 0; var < MAX_SIN_STEP * 2; ++var) {
		unsigned char x[16] = { 0 };

		for (int prop = 0; prop < 16; ++prop) {
			x[prop] = (unsigned char) ((var >= MAX_SIN_STEP ?  0.5 : -0.5) + ((var - MAX_SIN_STEP) * prop / 16.));
		}
		printf("\t0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n", x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9], x[10], x[11], x[12], x[13], x[14], x[15]);
	}

	return 0;
}

