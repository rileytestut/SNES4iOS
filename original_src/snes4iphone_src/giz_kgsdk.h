#ifndef _GIZ_KGSDK_H_
#define _GIZ_KGSDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SOUND_THREAD_SOUND_ON			1
#define SOUND_THREAD_SOUND_OFF		2
#define SOUND_THREAD_PAUSE				3

#define INP_BUTTON_PLAY				(0)
#define INP_BUTTON_STOP				(1)
#define INP_BUTTON_REWIND			(2)
#define INP_BUTTON_FORWARD			(3)
#define INP_BUTTON_L					(4)
#define INP_BUTTON_R					(5)
#define INP_BUTTON_HOME				(6)
#define INP_BUTTON_VOL					(7)
#define INP_BUTTON_BRIGHT			(8)
#define INP_BUTTON_TRIANGLE			(9)  // Not received
#define INP_BUTTON_POWER				(10) // Not received
#define INP_BUTTON_UP					(11)
#define INP_BUTTON_LEFT				(12)
#define INP_BUTTON_DOWN				(13)
#define INP_BUTTON_RIGHT				(14)

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
unsigned int gp_getButton(unsigned char enable_diagnals);
void gp2x_video_RGB_setscaling(int W, int H);
void gp2x_sound_play_bank(int bank);
void gp2x_sound_sync(void);
void BlitBufferToScreen(void *bufferFrom, void *bufferTo);
void set_gamma(int g100);

extern unsigned short *framebuffer16[];
extern unsigned char *framebuffer8[];
extern void *GizPrimaryFrameBuffer;
extern volatile unsigned short  gp2x_palette[512][2];

#ifdef __cplusplus
}
#endif

#endif

