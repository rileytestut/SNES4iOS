#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "frame_skip.h"
#include "memmap.h"

#ifndef uclock_t
#define uclock_t unsigned int
#endif

#define TICKS_PER_SEC 1000000UL
//#define CPU_FPS 60
static int CPU_FPS=60;
static uclock_t F;

#define MAX_FRAMESKIP 10


static char init_frame_skip = 1;
char skip_next_frame = 0;
static struct timeval init_tv = { 0, 0 };


void reset_frame_skip(void)
{
	//static Uint8 init=0;

	init_tv.tv_usec = 0;
	init_tv.tv_sec = 0;
	skip_next_frame = 0;
	init_frame_skip = 1;
	CPU_FPS=Memory.ROMFramesPerSecond;

	F = (uclock_t) ((double) TICKS_PER_SEC / CPU_FPS);
}

uclock_t get_ticks(void)
{
	struct timeval tv;

	gettimeofday(&tv, 0);
	if (init_tv.tv_sec == 0)
		init_tv = tv;
	return (tv.tv_sec - init_tv.tv_sec) * TICKS_PER_SEC + tv.tv_usec -
		init_tv.tv_usec;


}

int frame_skip(void)
{
	static int f2skip;
	static uclock_t sec = 0;
	static uclock_t rfd;
	static uclock_t target;
	static int nbFrame = 0;
	static int skpFrm = 0;

	if (init_frame_skip) {
		init_frame_skip = 0;
		target = get_ticks();
		nbFrame = 0;
		//f2skip=0;
		//skpFrm=0;
		sec = 0;
		return 0;
	}

	target += F;
	if (f2skip > 0) {
		f2skip--;
		skpFrm++;
		return 1;
	} else
		skpFrm = 0;


	rfd = get_ticks();

	if (rfd < target && f2skip == 0) {
		while (get_ticks() < target);
	} else {
		f2skip = (rfd - target) / (double) F;
		if (f2skip > MAX_FRAMESKIP) {
			f2skip = MAX_FRAMESKIP;
			reset_frame_skip();
		}
		// printf("Skip %d frame(s) %lu %lu\n",f2skip,target,rfd);
	}
	

	nbFrame++;
	if (get_ticks() - sec >= TICKS_PER_SEC) {
		nbFrame = 0;
		sec = get_ticks();
	}
	return 0;
}
