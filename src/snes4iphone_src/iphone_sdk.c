
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#import	<CoreSurface/CoreSurface.h>
#import <AudioToolbox/AudioQueue.h>
#include "iphone_sdk.h"

enum  { GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
	      GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
	      GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<23, GP2X_VOL_DOWN=1<<22, GP2X_PUSH=1<<27 };

const int isStereo = 0;
#define AUDIO_BUFFERS 6
#define FRAME_SIZE 2048
#define AUDIO_BUFFER_SIZE soundBufferSize /*(FRAME_SIZE*(isStereo ? 4 : 2))*/

extern void app_MuteSound(void);

typedef struct AQCallbackStruct {
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

AQCallbackStruct in;

extern void updateScreen();
extern void S9xMixSamplesO (signed short *buffer, int sample_count, int sample_offset);

unsigned short BaseAddress[320*240];
int soundBufferSize = 0;
int soundInit = 0;
float __audioVolume = 1.0;
unsigned long gp2x_pad_status = 0;
unsigned long player2_pad_status = 0;

int Timer=0;
extern unsigned char *vrambuffer;
extern volatile int __emulation_paused;

// 1024x8   8x8 font, i love it :)
const unsigned int font8x8[]= {0x0,0x0,0xc3663c18,0x3c2424e7,0xe724243c,0x183c66c3,0xc16f3818,0x18386fc1,0x83f61c18,0x181cf683,0xe7c3993c,0x3c99c3,0x3f7fffff,0xe7cf9f,0x3c99c3e7,0xe7c399,0x3160c080,0x40e1b,0xcbcbc37e,
0x7ec3c3db,0x3c3c3c18,0x81c087e,0x8683818,0x60f0e08,0x81422418,0x18244281,0xbd5a2418,0x18245abd,0x818181ff,0xff8181,0xa1c181ff,0xff8995,0x63633e,0x3e6363,0x606060,0x606060,0x3e60603e,0x3e0303,0x3e60603e,0x3e6060,0x3e636363,
0x606060,0x3e03033e,0x3e6060,0x3e03033e,0x3e6363,0x60603e,0x606060,0x3e63633e,0x3e6363,0x3e63633e,0x3e6060,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x18181818,0x180018,0x666666,0x0,0x367f3600,0x367f36,0x3c067c18,
0x183e60,0x18366600,0x62660c,0xe1c361c,0x6e337b,0x181818,0x0,0x18183870,0x703818,0x18181c0e,0xe1c18,0xff3c6600,0x663c,0x7e181800,0x1818,0x0,0x60c0c00,0x7e000000,0x0,0x0,0x181800,0x18306040,0x2060c,0x6e76663c,0x3c6666,0x18181c18,
0x7e1818,0x3060663c,0x7e0c18,0x3018307e,0x3c6660,0x363c3830,0x30307e,0x603e067e,0x3c6660,0x3e06063c,0x3c6666,0x1830607e,0xc0c0c,0x3c66663c,0x3c6666,0x7c66663c,0x1c3060,0x181800,0x1818,0x181800,0xc1818,0xc183060,0x603018,0x7e0000,
0x7e00,0x30180c06,0x60c18,0x3060663c,0x180018,0x5676663c,0x7c0676,0x66663c18,0x66667e,0x3e66663e,0x3e6666,0x606663c,0x3c6606,0x6666361e,0x1e3666,0x3e06067e,0x7e0606,0x3e06067e,0x60606,0x7606067c,0x7c6666,0x7e666666,0x666666,0x1818183c,
0x3c1818,0x60606060,0x3c6660,0xe1e3666,0x66361e,0x6060606,0x7e0606,0x6b7f7763,0x636363,0x7e7e6e66,0x666676,0x6666663c,0x3c6666,0x3e66663e,0x60606,0x6666663c,0x6c366e,0x3e66663e,0x666636,0x3c06663c,0x3c6660,0x1818187e,0x181818,0x66666666,
0x7c6666,0x66666666,0x183c66,0x6b636363,0x63777f,0x183c6666,0x66663c,0x3c666666,0x181818,0x1830607e,0x7e060c,0x18181878,0x781818,0x180c0602,0x406030,0x1818181e,0x1e1818,0x63361c08,0x0,0x0,0x7f0000,0xc060300,0x0,0x603c0000,0x7c667c,0x663e0606,
0x3e6666,0x63c0000,0x3c0606,0x667c6060,0x7c6666,0x663c0000,0x3c067e,0xc3e0c38,0xc0c0c,0x667c0000,0x3e607c66,0x663e0606,0x666666,0x181c0018,0x3c1818,0x18180018,0xe181818,0x36660606,0x66361e,0x1818181c,0x3c1818,0x7f370000,0x63636b,0x663e0000,
0x666666,0x663c0000,0x3c6666,0x663e0000,0x63e6666,0x667c0000,0x607c6666,0x663e0000,0x60606,0x67c0000,0x3e603c,0x187e1800,0x701818,0x66660000,0x7c6666,0x66660000,0x183c66,0x63630000,0x363e6b,0x3c660000,0x663c18,0x66660000,0x3e607c66,0x307e0000,
0x7e0c18,0xc181870,0x701818,0x18181818,0x18181818,0x3018180e,0xe1818,0x794f0600,0x30};

typedef struct GPRECT
{
	int x;
	int y;
	int w;
	int h;	
} GPRECT;

GPRECT	gpbuttons[23];

#define GPRectMake(x,y,w,h) {x,y,w,h}

/* 
########################
Graphics functions
########################
 */

static __inline__
void gp_drawPixel8 ( int x, int y, unsigned char c, unsigned char *framebuffer ) 
{
	*(framebuffer +(256*y)+x ) = c;
}

static __inline__
void gp_drawPixel16 ( int x, int y, unsigned short c, unsigned short *framebuffer ) 
{
	*(framebuffer +(256*y)+x ) = c;
}

static
void set_char8x8_16bpp (int xx,int yy,int offset,unsigned short mode,unsigned short *framebuffer) 
{
	unsigned int y, pixel;
	offset *= 2;
	pixel = font8x8[0 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel16(xx+0, yy+y, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel16(xx+1, yy+y, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel16(xx+2, yy+y, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel16(xx+3, yy+y, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel16(xx+4, yy+y, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel16(xx+5, yy+y, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel16(xx+6, yy+y, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel16(xx+7, yy+y, mode, framebuffer);
	}
	pixel = font8x8[1 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel16(xx+0, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel16(xx+1, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel16(xx+2, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel16(xx+3, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel16(xx+4, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel16(xx+5, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel16(xx+6, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel16(xx+7, yy+y+4, mode, framebuffer);

	}
}

static
void set_char8x8_8bpp (int xx,int yy,int offset,unsigned char mode,unsigned char *framebuffer) 
{
	unsigned int y, pixel;
	offset *= 2;
	pixel = font8x8[0 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel8(xx+0, yy+y, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel8(xx+1, yy+y, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel8(xx+2, yy+y, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel8(xx+3, yy+y, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel8(xx+4, yy+y, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel8(xx+5, yy+y, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel8(xx+6, yy+y, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel8(xx+7, yy+y, mode, framebuffer);
	}
	pixel = font8x8[1 + offset];
	for (y = 0; y < 4; y++) 
	{
		if (pixel&(1<<(0+(y<<3)))) gp_drawPixel8(xx+0, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(1+(y<<3)))) gp_drawPixel8(xx+1, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(2+(y<<3)))) gp_drawPixel8(xx+2, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(3+(y<<3)))) gp_drawPixel8(xx+3, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(4+(y<<3)))) gp_drawPixel8(xx+4, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(5+(y<<3)))) gp_drawPixel8(xx+5, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(6+(y<<3)))) gp_drawPixel8(xx+6, yy+y+4, mode, framebuffer);
		if (pixel&(1<<(7+(y<<3)))) gp_drawPixel8(xx+7, yy+y+4, mode, framebuffer);

	}
}

void gp_drawString (int x,int y,int len,char *buffer,unsigned short color,void *framebuffer) 
{
	int l,base=0;

		for (l=0;l<len;l++) 
		{
#if 0
			if (bppMode==8)
			{
				set_char8x8_8bpp (x+base,y,buffer[l],color,framebuffer);
			}
			else 
#endif
			{
				set_char8x8_16bpp (x+base,y,buffer[l],color,framebuffer);
			}
			base+=8;
		}
}

void gp_clearFramebuffer16(unsigned short *framebuffer, unsigned short pal)
{
	int x,y;
	for (y=0;y<224;y++)
	{
		for (x=0;x<256;x++)
		{
			*framebuffer++ = pal;
		}
	}
}

extern unsigned long padStatusForPadNumber(int which);

unsigned int gp_getButton(unsigned char which1)
{
    if (which1 <= 4)
        return padStatusForPadNumber(which1);
    
    return 0;
}

extern unsigned int *screenPixels;  // defined in ScreenLayer.m


void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack)
{
}

void gp_deinitGraphics(void)
{

}



extern void refreshScreenSurface(void);

void gp_setFramebuffer(int flip, int sync)
{
    memcpy(screenPixels, vrambuffer, 256*224*2);
    refreshScreenSurface();
}



void gp2x_video_setpalette(void) 
{ 

} 

/* 
########################
Sound functions
########################
 */
static
void *gp2x_sound_play(void)
{
	 
	return NULL;
}

void gp2x_sound_play_bank(int bank)
{
	
}

void gp2x_sound_sync(void)
{
	
}

void gp2x_sound_volume(int l, int r) 
{
	__audioVolume = (float)l / 100.0;
} 

unsigned long gp2x_timer_read(void)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);

	return (unsigned long)(((unsigned long long)current_time.tv_sec * 1000LL) + (current_time.tv_usec / 1000LL));
}

void gp_initSound(int rate, int bits, int stereo, int Hz, int frag)
{
	
}

void gp_stopSound(void)
{
}


/* 
########################
System functions
########################
 */
void gp_Reset(void)
{
}

void gp2x_video_RGB_setscaling(int W, int H)
{

}

void gp_setCpuspeed(unsigned int MHZ)
{


}

void set_gamma(int g100)
{

}

static void AQBufferCallback(
							 void *userdata,
							 AudioQueueRef outQ,
							 AudioQueueBufferRef outQB)
{
	unsigned char *coreAudioBuffer;
	coreAudioBuffer = (unsigned char*) outQB->mAudioData;
	
	outQB->mAudioDataByteSize = AUDIO_BUFFER_SIZE;
	AudioQueueSetParameter(outQ, kAudioQueueParam_Volume, __audioVolume);
	//fprintf(stderr, "sound_lastlen %d\n", sound_lastlen);
	if(__emulation_paused)
	{
    memset(coreAudioBuffer, 0, AUDIO_BUFFER_SIZE);
	}
	else
	{
	  S9xMixSamplesO((short*)coreAudioBuffer, (AUDIO_BUFFER_SIZE) / 2, 0);
  }
	AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
}

int app_OpenSound(int buffersize) {
    Float64 sampleRate = 22050.0;
    int i;
    UInt32 bufferBytes;
	
	soundBufferSize = buffersize;
	
    app_MuteSound();
	
    soundInit = 0;
	
    in.mDataFormat.mSampleRate = sampleRate;
    in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
    in.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger
	| kAudioFormatFlagIsPacked;
    in.mDataFormat.mBytesPerPacket    =   4;
    in.mDataFormat.mFramesPerPacket   =   isStereo ? 1 : 2;
    in.mDataFormat.mBytesPerFrame     =   isStereo ? 4 : 2;
    in.mDataFormat.mChannelsPerFrame  =   isStereo ? 2 : 1;
    in.mDataFormat.mBitsPerChannel    =   16;
	
	
    /* Pre-buffer before we turn on audio */
    UInt32 err;
    err = AudioQueueNewOutput(&in.mDataFormat,
							  AQBufferCallback,
							  NULL,
							  NULL,
							  kCFRunLoopCommonModes,
							  0,
							  &in.queue);
	
	bufferBytes = AUDIO_BUFFER_SIZE;
	
	for (i=0; i<AUDIO_BUFFERS; i++) 
	{
		err = AudioQueueAllocateBuffer(in.queue, bufferBytes, &in.mBuffers[i]);
		/* "Prime" by calling the callback once per buffer */
		//AQBufferCallback (&in, in.queue, in.mBuffers[i]);
		in.mBuffers[i]->mAudioDataByteSize = AUDIO_BUFFER_SIZE; //samples_per_frame * 2; //inData->mDataFormat.mBytesPerFrame; //(inData->frameCount * 4 < (sndOutLen) ? inData->frameCount * 4 : (sndOutLen));
		AudioQueueEnqueueBuffer(in.queue, in.mBuffers[i], 0, NULL);
	}
	
	soundInit = 1;
	err = AudioQueueStart(in.queue, NULL);
	
	return 0;
}

void app_CloseSound(void) {
	if( soundInit == 1 )
	{
		AudioQueueDispose(in.queue, true);
		soundInit = 0;
	}
}


void app_MuteSound(void) {
	if( soundInit == 1 )
	{
		app_CloseSound();
	}
}

void app_DemuteSound(int buffersize) {
	if( soundInit == 0 )
	{
		app_OpenSound(buffersize);
	}
}
