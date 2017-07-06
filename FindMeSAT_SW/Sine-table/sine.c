
#include <stdio.h>
#include <math.h>


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
	return 0;
}

