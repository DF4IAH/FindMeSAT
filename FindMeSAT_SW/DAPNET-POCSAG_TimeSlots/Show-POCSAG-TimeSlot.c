/**
  * Show current POCSAG TimeSlot (TS)
  *
  * by DF4IAH
  */


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


int main(int argc, char* argv[])
{
	static uint8_t	s_ts = 0;
	uint64_t	now10;
	uint8_t		ts;
	struct timeval	tv;

	printf("%c\r\n\r\n\r\n", 0x0c);
        printf("*==============================================*==========================*\r\n");
        printf("* DAPNET / POCSAG TimeSlot Rechner von DF4IAH  *  Liste ist um            *\r\n");
	printf("* Voraussetzung:  Systemzeit ist NTP gefuehrt  *  DB0CPU / Mannheim       *\r\n");
	printf("* zum Abbrechen  STRG-C  druecken.   20180211  *  zentriert               *\r\n");
	printf("*==============================================*==========================*\r\n\r\n");

        printf("*==========*===================================*==========================*\r\n");
        printf("*  CALL    *  Slots                            *  QTH                     *\r\n");
        printf("*          *  0 1 2 3 4 5 6 7 8 9 A B C D E F  *                          *\r\n");
        printf("*==========*===================================*==========================*\r\n");
        printf("*  DB0CPU  *  x x x x . . . . x x x x . . . .  *  Mannheim (noch alte SW) *\r\n");
        printf("*  DB0FTC  *  . . . . x x x . . . . . x . . .  *  Quirnheim               *\r\n");
        printf("*  DB0IUK  *  . . . . . . . . . . . . . . . .  *  Heppenheim              *\r\n");
        printf("*  DB0HRF  *  x x x x . . . . . x . . . . . .  *  Feldberg / Ts.          *\r\n");
        printf("*  DB0ZAV  *  . . . . . . . x . . . . . x x x  *  Karben / Ts.            *\r\n");
        printf("*  DB0PRT  *  x x . . . . . . x x . . . . . .  *  Muehlacker (PF)         *\r\n");
        printf("*  DB0LDH  *  . . . . . x x . . . . . x . . .  *  Florstadt               *\r\n");
        printf("*  DB0LDK  *  x x . . . . . . x x . . . . . .  *  Wetzlar                 *\r\n");
        printf("*  DB0GH   *  x . . . x . . . . . x . . . . .  *  Weinstadt-Grossheppach  *\r\n");
        printf("*  DB0OFI  *  . . . . . x x . . . . . . x x .  *  Ostfildern              *\r\n");
        printf("*  DB0XHI  *  . . x x . . . . . . x x . . . .  *  Reutlingen-Nord         *\r\n");
        printf("*  DM0AI   *  x x . . . . . . x x . . . . . .  *  Reutlingen              *\r\n");
        printf("*==========*===================================*==========================*\r\n\r\n");

	printf("Unix-Zeit = ");


	while (1) {
                gettimeofday(&tv, NULL);
		now10 = tv.tv_sec * 10ULL + tv.tv_usec / 100000UL;
		ts = (now10 / 64) & 0xf;

		if (s_ts != ts) {
			s_ts = ts;
			putchar(0x07);
			fflush(NULL);
		}

		printf("%09lld.%01d secs    TS = %X ", now10 / 10ULL, (int) (now10 % 10ULL), ts);
		fflush(NULL);

		usleep(50000);

		for (int count = 28; count; count--) {
			putchar(0x08);
		}
	}
}

