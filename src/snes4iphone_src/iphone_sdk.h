#ifndef _IPHONE_SDK_H_
#define _IPHONE_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#define INP_BUTTON_UP				(0)
#define INP_BUTTON_LEFT				(2)
#define INP_BUTTON_DOWN				(4)
#define INP_BUTTON_RIGHT			(6)
#define INP_BUTTON_START			(8)
#define INP_BUTTON_SELECT			(9)
#define INP_BUTTON_L				(10)
#define INP_BUTTON_R				(11)
#define INP_BUTTON_HARDLEFT			(12)
#define INP_BUTTON_HARDRIGHT		(13)
#define INP_BUTTON_HARDDOWN			(14)
#define INP_BUTTON_HARDUP			(15)
#define INP_BUTTON_L2				(22)
#define INP_BUTTON_R2				(23)
#define INP_BUTTON_STICK_PUSH		(27)
#define INP_BUTTON_MENU				(31)

#define SOUND_THREAD_SOUND_ON			1
#define SOUND_THREAD_SOUND_OFF		2
#define SOUND_THREAD_PAUSE				3
#define gp2x_flipscreen()

void gp_drawString (int x,int y,int len,char *buffer,unsigned short color,void *framebuffer);
void gp_clearFramebuffer16(unsigned short *framebuffer, unsigned short pal);
void gp_setCpuspeed(unsigned int cpuspeed);
void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack);
void gp_setFramebuffer(int flip, int sync);
void gp2x_video_setpalette(void);
void gp_initSound(int rate, int bits, int stereo, int Hz, int frag);
void gp_stopSound(void);
void gp_Reset(void);
void gp2x_enableIRQ(void);
void gp2x_disableIRQ(void);
void gp2x_sound_volume(int l, int r);
unsigned long gp2x_timer_read(void);
unsigned int gp_getButton(unsigned char which1);
void gp2x_video_RGB_setscaling(int W, int H);
void gp2x_sound_play_bank(int bank);
void gp2x_sound_sync(void);
void BlitBufferToScreen(void *bufferFrom, void *bufferTo);
void set_gamma(int g100);
void updateScreen();
void gp_deinitGraphics(void);

extern unsigned short *framebuffer16[];
extern unsigned char *framebuffer8[];
extern void *GizPrimaryFrameBuffer;
extern volatile unsigned short  gp2x_palette[512][2];

extern unsigned short* screenbuffer;
extern unsigned short BaseAddress[320*240];
extern int __emulation_run;
extern int __emulation_saving;

#ifdef __cplusplus
}
#endif

#endif

