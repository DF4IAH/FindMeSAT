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
	static uint8_t s_ts = 0;
	//time_t now;
	uint64_t now10;
	uint8_t ts;
	struct timeval tv;

	printf("%c\r\n\r\n\r\n", 0x0c);
        printf("===========================================\r\n");
        printf("* POCSAG TimeSlot Berechnung    by DF4IAH *\r\n");
	printf("*                                         *\r\n");
	printf("* Zum Abbrechen STRG-C drücken.  20180207 *\r\n");
	printf("===========================================\r\n\r\n");
 	printf("POCSAG Unix-Zeit = ");

	while (1) {
#if 0
		now = time(NULL);
		ts = (( 5ULL * now ) / 32) & 0xf;
       		printf("%ld,\tTS = %X ", now, ts);
#else
                (void)gettimeofday(&tv, NULL);
		now10 = tv.tv_sec * 10ULL + tv.tv_usec / 100000UL;
		ts = (now10 / 64) & 0xf;

		if (s_ts != ts) {
			s_ts = ts;
			putchar(0x07);
			fflush(NULL);
		}

		printf("%09lld.%01d,\tTS = %X ", now10 / 10ULL, (int) (now10 % 10ULL), ts);
#endif
		fflush(NULL);

		usleep(50000);

#if 1
		for (int count = 28; count; count--) {
			putchar(0x08);
		}
#else
		printf("\r\n");
#endif
	}
}

