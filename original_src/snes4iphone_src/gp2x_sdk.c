
#define DEBUG

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <linux/fb.h>
#include <pthread.h>
#include "menu.h"
#include "gp2x_sdk.h"
#include "squidgehack.h"
#include <time.h>

#define SYS_CLK_FREQ 7372800

volatile unsigned short  gp2x_palette[512][2];

static int fb_size=(320*240*2)+(16*2);
static int mmuHackStatus=0;

//unsigned long gp2x_ticks_per_second=7372800/1000;
unsigned long   gp2x_dev[5]={0,0,0,0,0};
unsigned long gp2x_physvram[4]={0,0,0,0};

unsigned short *framebuffer16[4]={0,0,0,0};
static unsigned short *framebuffer_mmap[4]={0,0,0,0};
unsigned short *gp2x_logvram15[2], gp2x_sound_buffer[4+((44100*2)*8)]; //*2=stereo, *4=max buffers
volatile unsigned short *gp2x_memregs;
volatile unsigned long *gp2x_memregl;
volatile unsigned long *gp2x_blitter = NULL;
unsigned int *gp2x_intVectors;
unsigned char  *framebuffer8[4], *gp2x_screen8prev, *gp2x_logvram8[2];

volatile short *pOutput[8];
int InitFramebuffer=0;
int Timer=0;
volatile int SoundThreadFlag=0;
volatile int CurrentSoundBank=0;
int CurrentFrameBuffer=0;
int CurrentFrag=0;
unsigned int ExistingIntHandler;
unsigned int VolumeMultiplier = 0x50;

// 1024x8   8x8 font, i love it :)
const unsigned int font8x8[]= {0x0,0x0,0xc3663c18,0x3c2424e7,0xe724243c,0x183c66c3,0xc16f3818,0x18386fc1,0x83f61c18,0x181cf683,0xe7c3993c,0x3c99c3,0x3f7fffff,0xe7cf9f,0x3c99c3e7,0xe7c399,0x3160c080,0x40e1b,0xcbcbc37e,0x7ec3c3db,0x3c3c3c18,0x81c087e,0x8683818,0x60f0e08,0x81422418,0x18244281,0xbd5a2418,0x18245abd,0x818181ff,0xff8181,0xa1c181ff,0xff8995,0x63633e,0x3e6363,0x606060,0x606060,0x3e60603e,0x3e0303,0x3e60603e,0x3e6060,0x3e636363,0x606060,0x3e03033e,0x3e6060,0x3e03033e,0x3e6363,0x60603e,0x606060,0x3e63633e,0x3e6363,0x3e63633e,0x3e6060,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x18181818,0x180018,0x666666,0x0,0x367f3600,0x367f36,0x3c067c18,0x183e60,0x18366600,0x62660c,0xe1c361c,0x6e337b,0x181818,0x0,0x18183870,0x703818,0x18181c0e,0xe1c18,0xff3c6600,0x663c,0x7e181800,0x1818,0x0,0x60c0c00,0x7e000000,0x0,0x0,0x181800,0x18306040,0x2060c,0x6e76663c,0x3c6666,0x18181c18,0x7e1818,0x3060663c,0x7e0c18,0x3018307e,0x3c6660,0x363c3830,0x30307e,0x603e067e,0x3c6660,0x3e06063c,0x3c6666,0x1830607e,0xc0c0c,0x3c66663c,0x3c6666,0x7c66663c,0x1c3060,0x181800,0x1818,0x181800,0xc1818,0xc183060,0x603018,0x7e0000,0x7e00,0x30180c06,0x60c18,0x3060663c,0x180018,0x5676663c,0x7c0676,0x66663c18,0x66667e,0x3e66663e,0x3e6666,0x606663c,0x3c6606,0x6666361e,0x1e3666,0x3e06067e,0x7e0606,0x3e06067e,0x60606,0x7606067c,0x7c6666,0x7e666666,0x666666,0x1818183c,0x3c1818,0x60606060,0x3c6660,0xe1e3666,0x66361e,0x6060606,0x7e0606,0x6b7f7763,0x636363,0x7e7e6e66,0x666676,0x6666663c,0x3c6666,0x3e66663e,0x60606,0x6666663c,0x6c366e,0x3e66663e,0x666636,0x3c06663c,0x3c6660,0x1818187e,0x181818,0x66666666,0x7c6666,0x66666666,0x183c66,0x6b636363,0x63777f,0x183c6666,0x66663c,0x3c666666,0x181818,0x1830607e,0x7e060c,0x18181878,0x781818,0x180c0602,0x406030,0x1818181e,0x1e1818,0x63361c08,0x0,0x0,0x7f0000,0xc060300,0x0,0x603c0000,0x7c667c,0x663e0606,0x3e6666,0x63c0000,0x3c0606,0x667c6060,0x7c6666,0x663c0000,0x3c067e,0xc3e0c38,0xc0c0c,0x667c0000,0x3e607c66,0x663e0606,0x666666,0x181c0018,0x3c1818,0x18180018,0xe181818,0x36660606,0x66361e,0x1818181c,0x3c1818,0x7f370000,0x63636b,0x663e0000,0x666666,0x663c0000,0x3c6666,0x663e0000,0x63e6666,0x667c0000,0x607c6666,0x663e0000,0x60606,0x67c0000,0x3e603c,0x187e1800,0x701818,0x66660000,0x7c6666,0x66660000,0x183c66,0x63630000,0x363e6b,0x3c660000,0x663c18,0x66660000,0x3e607c66,0x307e0000,0x7e0c18,0xc181870,0x701818,0x18181818,0x18181818,0x3018180e,0xe1818,0x794f0600,0x30};

pthread_t       gp2x_sound_thread=0, gp2x_sound_thread_exit=0;
struct fb_fix_screeninfo fb0_fixed_info;
struct fb_fix_screeninfo fb1_fixed_info;

/* 
########################
Graphics functions
########################
 */

static void debug(char *text, int pause)
{
	unsigned short bppmode;
	bppmode=gp2x_memregs[0x28DA>>1];
	bppmode>>=9;
	bppmode<<=3;
	
	if(bppmode==8)
	{
		gp_clearFramebuffer8(framebuffer8[currFB],0);
		gp_drawString(0,0,strlen(text),text,0x51,framebuffer16[currFB]);
	}
	else
	{
		gp_clearFramebuffer8(framebuffer16[currFB],0);
		gp_drawString(0,0,strlen(text),text,(unsigned short)RGB(31,31,31),framebuffer16[currFB]);
	}
	MenuFlip();
	if(pause)	MenuPause();

}
static __inline__
void gp_drawPixel8 ( int x, int y, unsigned char c, unsigned char *framebuffer ) 
{
	*(framebuffer +(320*y)+x ) = c;
}

static __inline__
void gp_drawPixel16 ( int x, int y, unsigned short c, unsigned short *framebuffer ) 
{
	*(framebuffer +(320*y)+x ) = c;
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
	unsigned short bppmode;
	bppmode=gp2x_memregs[0x28DA>>1];
	bppmode>>=9;
	bppmode<<=3;

		for (l=0;l<len;l++) 
		{
			if (bppmode==8)
			{
				set_char8x8_8bpp (x+base,y,buffer[l],color,framebuffer);
			}
			else 
			{
				set_char8x8_16bpp (x+base,y,buffer[l],color,framebuffer);
			}
			base+=8;
		}
}

void gp_clearFramebuffer16(unsigned short *framebuffer, unsigned short pal)
{
	int x,y;
	for (y=0;y<240;y++)
	{
		for (x=0;x<320;x++)
		{
			*framebuffer++ = pal;
		}
	}
}

void gp_clearFramebuffer8(unsigned char *framebuffer, unsigned char pal)
{
	int x,y;
	for (y=0;y<240;y++)
	{
		for (x=0;x<320;x++)
		{
			*framebuffer++ = pal;
		}
	}
}

void gp_clearFramebuffer(void *framebuffer, unsigned int pal)
{
	unsigned short bppmode;
	bppmode=gp2x_memregs[0x28DA>>1];
	bppmode>>=9;
	bppmode<<=3;
	if(bppmode==8) 	gp_clearFramebuffer8((unsigned char*)framebuffer,(unsigned char)pal);
	else 					gp_clearFramebuffer16((unsigned short*)framebuffer,(unsigned short)pal);
}




unsigned int gp_getButton(unsigned char enable_diagnals)
{
	unsigned int value=(gp2x_memregs[0x1198>>1] & 0x00FF);
   //gp2x_memregs[0x1198>>1]
  /*
  0x1FE = UP		1 1111 1110	
  0x17E			1 0111 1110
  0x17F			1 0111 1111
  0X13F			1 0011 1111
  0x1BF = RIGHT	1 1011 1111
  0X19F			1 1001 1111
  0X1DF			1 1101 1111
  0X1CF			1 1100 1111
  0x1EF = DOWN	1 1110 1111
  0x1E7			1 1110 0111
  0x1F7			1 1111 0111
  0x1F3			1 1111 0011
  0x1FB = LEFT	1 1111 1011
  0x1F9			1 1111 1001
  0x1FD			1 1111 1101
  0x1FC			1 1111 1100
 
  */
	switch(value)
	{
		//UP
		case 0x7E:
		case 0xFC:
			value = 0xFE;
			break;
			
		//RIGHT	
		case 0x3F:
		case 0x9F:
			value = 0xBF;
			break;
			
		//DOWN
		case 0xCF:
		case 0xE7:
			value = 0xEF;
			break;
			
		//LEFT
		case 0xF3:
		case 0xF9:
			value = 0xFB;
			break;	
	}
	
	if (enable_diagnals)
	{
	
		if(value==0xFD) value=0xFA;
		if(value==0xF7) value=0xEB;
		if(value==0xDF) value=0xAF;
		if(value==0x7F) value=0xBE;
	}

  
  return ~((gp2x_memregs[0x1184>>1] & 0xFF00) | value | (gp2x_memregs[0x1186>>1] << 16));
}

void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack)
{
	
	int x = 0;
	unsigned int key = 0;
	unsigned int offset = 0;
	char buf[256];
	

#ifdef DEBUG
    printf("Entering gp_initGraphics....\r\n");
#endif    
	/*
	First check that frame buffer memory has not already been setup
	*/
	if (!InitFramebuffer)
	{
#ifdef DEBUG
		sprintf(buf, "Initing buffer\r\n");
		printf(buf);
#endif 
		gp2x_dev[0] = open("/dev/fb0", O_RDWR);
		gp2x_dev[1] = open("/dev/fb1", O_RDWR);
		gp2x_dev[2] = open("/dev/mem", O_RDWR);
		
#ifdef DEBUG
		sprintf(buf, "Devices opened\r\n");
		printf(buf);
		sprintf(buf, "/dev/fb0: %x \r\n", gp2x_dev[0]);
		printf(buf);
		sprintf(buf, "/dev/fb1: %x \r\n", gp2x_dev[1]);
		printf(buf);
		sprintf(buf, "/dev/mem: %x \r\n", gp2x_dev[2]);
		printf(buf);
#endif 		

		gp2x_memregs=(unsigned short *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], 0xc0000000);
		gp2x_memregl=(unsigned long *)gp2x_memregs;
		if (!gp2x_blitter) gp2x_blitter=(unsigned long  *)mmap(0, 0x100, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], 0xe0020000);
  
		if (!framebuffer_mmap[0]) framebuffer_mmap[0]=(void *)mmap(0, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], (gp2x_physvram[0]=0x04000000-(0x26000*4) )); 
		if (!framebuffer_mmap[1]) framebuffer_mmap[1]=(void *)mmap(0, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], (gp2x_physvram[1]=0x04000000-(0x26000*3) )); 
		if (!framebuffer_mmap[2]) framebuffer_mmap[2]=(void *)mmap(0, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], (gp2x_physvram[2]=0x04000000-(0x26000*2) )); 
		if (!framebuffer_mmap[3]) framebuffer_mmap[3]=(void *)mmap(0, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_dev[2], (gp2x_physvram[3]=0x04000000-(0x26000*1) )); 

		if (applyMmuHack) 
		{
			printf("Applying MMUHACK..."); fflush(stdout);
			mmuHackStatus = mmuhack();
			sprintf(buf, "Done\r\n. MMUHACK returned: %x\r\n", mmuHackStatus); 
			printf(buf); fflush(stdout); 
		}
		
		// offset externally visible buffers by 8
		// this allows DrMD to not worry about clipping
		framebuffer16[0]=framebuffer_mmap[0]+8;
		framebuffer16[1]=framebuffer_mmap[1]+8;
		framebuffer16[2]=framebuffer_mmap[2]+8;
		framebuffer16[3]=framebuffer_mmap[3]+8;
		//ofset physical buffer as well
		gp2x_physvram[0]+=16;
		gp2x_physvram[1]+=16;
		gp2x_physvram[2]+=16;
		gp2x_physvram[3]+=16;
		// clear all the framebuffers to black
		// otherwise you get crap displayed on the screen the first time
		// you start 
		framebuffer8[0]=(unsigned char*)framebuffer16[0];
		framebuffer8[1]=(unsigned char*)framebuffer16[1];
		framebuffer8[2]=(unsigned char*)framebuffer16[2];
		framebuffer8[3]=(unsigned char*)framebuffer16[3];

		// Clear the frame buffers
		memset(framebuffer16[0],0,320*240*2);
		memset(framebuffer16[1],0,320*240*2);
		memset(framebuffer16[2],0,320*240*2);
		memset(framebuffer16[3],0,320*240*2);
	
		InitFramebuffer=1;
		
		//gp2x_memregs[0x0F16>>1] = 0x830a; 
		//usleep(1000000); 
		//gp2x_memregs[0x0F58>>1] = 0x100c; 
		//usleep(1000000); 
		
		
	}
	
	
	// Set graphics mode
	gp2x_memregs[0x28DA>>1]=(((bpp+1)/8)<<9)|0xAB; /*8/15/16/24bpp...*/
	gp2x_memregs[0x290C>>1]=320*((bpp+1)/8);
	
	//TV out fix
	gp2x_video_RGB_setscaling(320,240);
	
	// 2d accel
  	//gp2x_memregs[0x904 >> 1] |= 1<<10;  //SYSCLKENREG (System Clock Enable Register) maybe bit 10 is 2d accer
	//gp2x_memregs[0x90a >> 1] = 0xffff;  // Reset clock timings for all devices
	
	gp_setFramebuffer(flip,1);
	
#ifdef DEBUG
    printf("Leaving gp_initGraphics....\r\n");
#endif 
}

void gp_setFramebuffer(int flip, int sync)
{
	CurrentFrameBuffer=flip;
	unsigned int address=(unsigned int)gp2x_physvram[flip];
	unsigned short x=0;
	/*switch(sync)
	{
		case 0:
			// No sync
			break;
		
		case 1:
			// VSync
			while(1)
			{
				x=gp2x_memregs[0x1182>>1];
				if((x&(1<<4)) == 0)
				{
					break;
				}
			}
			break;
		case 2:
			// HSync
			while(1)
			{
				x=gp2x_memregs[0x1182>>1];
				if((x&(1<<5)) == 0)
				{
					break;
				}
			}
			break;
	}*/
	
	gp2x_memregs[0x290E>>1]=(unsigned short)(address & 0xffff);
	gp2x_memregs[0x2910>>1]=(unsigned short)(address >> 16);
	gp2x_memregs[0x2912>>1]=(unsigned short)(address & 0xffff);
	gp2x_memregs[0x2914>>1]=(unsigned short)(address >> 16);
}

void gp2x_video_setpalette(void) 
{ 
  unsigned short *g=(unsigned short *)gp2x_palette; int i=512; 
  gp2x_memregs[0x2958>>1]=0;                                                      
  while(i--) gp2x_memregs[0x295A>>1]=*g++; 
} 

/* 
########################
Sound functions
########################
 */
static
void *gp2x_sound_play(void)
{
	//struct timespec ts; ts.tv_sec=0, ts.tv_nsec=1000;
	while(! gp2x_sound_thread_exit)
	{
		Timer++;
		CurrentSoundBank++;

		if (CurrentSoundBank >= 8) CurrentSoundBank = 0;
		
		if (SoundThreadFlag==SOUND_THREAD_SOUND_ON)
		{
			write(gp2x_dev[3], (void *)pOutput[CurrentSoundBank], gp2x_sound_buffer[1]);
			ioctl(gp2x_dev[3], SOUND_PCM_SYNC, 0); 
			//ts.tv_sec=0, ts.tv_nsec=(gp2x_sound_buffer[3]<<16)|gp2x_sound_buffer[2];
			//nanosleep(&ts, NULL);
		}
		else
		{
			write(gp2x_dev[3], (void *)&gp2x_sound_buffer[4], gp2x_sound_buffer[1]);
			ioctl(gp2x_dev[3], SOUND_PCM_SYNC, 0); 
			//ts.tv_sec=0, ts.tv_nsec=(gp2x_sound_buffer[3]<<16)|gp2x_sound_buffer[2];
			//nanosleep(&ts, NULL);
		}
	}
 
	return NULL;
}

void gp2x_sound_play_bank(int bank)
{
	write(gp2x_dev[3], (void *)(&gp2x_sound_buffer[4+(bank*gp2x_sound_buffer[1])]), gp2x_sound_buffer[1]);
}

void gp2x_sound_sync(void)
{
	ioctl(gp2x_dev[3], SOUND_PCM_SYNC, 0); 
}

void gp2x_sound_volume(int l, int r) 
{ 
	if(!gp2x_dev[4])
	{
		gp2x_dev[4] = open("/dev/mixer", O_WRONLY); 
	}
	l=(((l*VolumeMultiplier)/100)<<8)|((r*VolumeMultiplier)/100);          //0x5A, 0x60 
	ioctl(gp2x_dev[4], SOUND_MIXER_WRITE_PCM, &l); //SOUND_MIXER_WRITE_VOLUME 
} 

unsigned long gp2x_timer_read(void)
{
  // Once again another peice of direct hardware access bites the dust
  // the code below is broken in firmware 2.1.1 so I've replaced it with a
  // to a linux function which seems to work
  //return gp2x_memregl[0x0A00>>2]/gp2x_ticks_per_second;
  struct timeval tval; // timing
  
  gettimeofday(&tval, 0);
  //tval.tv_usec
  //tval.tv_sec
  return (tval.tv_sec*1000000)+tval.tv_usec;
}

int gp_initSound(int rate, int bits, int stereo, int Hz, int frag)
{
	int status;
	int i=0;
	int nonblocking=1;
	unsigned int bufferStart=0;
	int result;
	char text[256];
	
	//Check GP2x model
	int fd;
	fd=open("/dev/touchscreen/wm97xx", O_RDONLY | O_NOCTTY);
	if (fd!=-1)
	{
		//F200 model
		VolumeMultiplier = 0x28;
		close(fd);
	}
	
	//int frag=0x00020010;   // double buffer - frag size = 1<<0xf = 32768

	//8 = 256				= 2 fps loss			= good sound
	//9 = 512				= 1 fps loss			= good sound
	//A = 1024				= 
	//f = 32768				= 0 fps loss			= bad sound
	if ((frag!= CurrentFrag)&&(gp2x_dev[3]!=0))
	{
		// Different frag config required
		// close device in order to re-adjust
		close(gp2x_dev[3]);
		gp2x_dev[3]=0;
	}

	if (gp2x_dev[3]==0)
	{
		gp2x_dev[3] = open("/dev/dsp", O_WRONLY);
		printf("Opening sound device: %x\r\n",gp2x_dev[3]);
		ioctl(gp2x_dev[3], SNDCTL_DSP_SETFRAGMENT, &frag);
		CurrentFrag=frag; // save frag config
	}

	//ioctl(gp2x_dev[3], SNDCTL_DSP_RESET, 0);
	result=ioctl(gp2x_dev[3], SNDCTL_DSP_SPEED,  &rate);
	if(result==-1)
	{
		debug("Error setting DSP Speed",1);
		return(-1);
	}
	
	result=ioctl(gp2x_dev[3], SNDCTL_DSP_SETFMT,  &bits);
	if(result==-1)
	{
		debug("Error setting DSP format",1);
		return(-1);
	}

	result=ioctl(gp2x_dev[3], SNDCTL_DSP_STEREO,  &stereo);
	if(result==-1)
	{
		debug("Error setting DSP format",1);
		return(-1);
	}
	//printf("Disable Blocking: %x\r\n",ioctl(gp2x_dev[3], 0x5421, &nonblocking));
	
	gp2x_sound_buffer[1]=(gp2x_sound_buffer[0]=(rate/Hz)) << (stereo + (bits==16));
	gp2x_sound_buffer[2]=(1000000000/Hz)&0xFFFF;
	gp2x_sound_buffer[3]=(1000000000/Hz)>>16;
 
	bufferStart= &gp2x_sound_buffer[4];
	pOutput[0] = (short*)bufferStart+(0*gp2x_sound_buffer[1]);
	pOutput[1] = (short*)bufferStart+(1*gp2x_sound_buffer[1]);
	pOutput[2] = (short*)bufferStart+(2*gp2x_sound_buffer[1]);
	pOutput[3] = (short*)bufferStart+(3*gp2x_sound_buffer[1]);
	pOutput[4] = (short*)bufferStart+(4*gp2x_sound_buffer[1]);
	pOutput[5] = (short*)bufferStart+(5*gp2x_sound_buffer[1]);
	pOutput[6] = (short*)bufferStart+(6*gp2x_sound_buffer[1]);
	pOutput[7] = (short*)bufferStart+(7*gp2x_sound_buffer[1]);
	
	if(!gp2x_sound_thread) 
	{ 
		pthread_create( &gp2x_sound_thread, NULL, gp2x_sound_play, NULL);
		//atexit(gp_Reset); 
	}
	
	for(i=0;i<(gp2x_sound_buffer[1]*8);i++)
	{
		gp2x_sound_buffer[4+i] = 0;
	}
	
	return(0);
}

void gp_stopSound(void)
{
	unsigned int i=0;
	gp2x_sound_thread_exit=1;
	printf("Killing Thread\r\n");
	for(i=0;i<(gp2x_sound_buffer[1]*8);i++)
	{
		gp2x_sound_buffer[4+i] = 0;
	}
	usleep(100000); 
	printf("Thread is dead\r\n");
	gp2x_sound_thread=0;
	gp2x_sound_thread_exit=0;
	CurrentSoundBank=0;
}


/* 
########################
System functions
########################
 */
void gp_Reset(void)
{
	unsigned int i=0;
	
	
	if( gp2x_sound_thread) 
	{ 
		gp2x_sound_thread_exit=1; 
		usleep(500); 
	}
 
	gp2x_memregs[0x28DA>>1]=0x4AB; 
	gp2x_memregs[0x290C>>1]=640;   
	munmap((void *)gp2x_memregs,      0x10000);
	
	munmap(framebuffer_mmap[0],    fb_size);
	munmap(framebuffer_mmap[1],    fb_size);
	munmap(framebuffer_mmap[2],    fb_size);
	munmap(framebuffer_mmap[3],    fb_size);
  
	if (gp2x_dev[0]) close(gp2x_dev[0]);
	if (gp2x_dev[1]) close(gp2x_dev[1]);
	if (gp2x_dev[2]) close(gp2x_dev[2]);
	if (gp2x_dev[3]) close(gp2x_dev[3]);
	if (gp2x_dev[4]) close(gp2x_dev[4]);
	
	fcloseall();

	// If MMUHACK was applied succesfully then remove it 
	if (mmuHackStatus) mmuunhack();
 
	chdir("/usr/gp2x");
	execl("gp2xmenu",NULL);
}

void gp2x_video_RGB_setscaling(int W, int H)
{
 float escalaw,escalah;
 int bpp=(gp2x_memregs[0x28DA>>1]>>9)&0x3;

 if(gp2x_memregs[0x2800>>1]&0x100) //TV-Out
 {
   escalaw=489.0; //RGB Horiz TV (PAL, NTSC)
   if (gp2x_memregs[0x2818>>1]  == 287) //PAL
     escalah=274.0; //RGB Vert TV PAL
   else if (gp2x_memregs[0x2818>>1]  == 239) //NTSC
     escalah=331.0; //RGB Vert TV NTSC
 }
 else //LCD
 {
   escalaw=1024.0; //RGB Horiz LCD
   escalah=320.0; //RGB Vert LCD
 }

 // scale horizontal
 gp2x_memregs[0x2906>>1]=(unsigned short)((float)escalaw *(W/320.0));
 // scale vertical
 gp2x_memregl[0x2908>>2]=(unsigned long)((float)escalah *bpp *(H/240.0));
}

void gp_setCpuspeed(unsigned int MHZ)
{
	unsigned v;
	unsigned mdiv,pdiv=3,scale=0;

	MHZ*=1000000;
	mdiv=(MHZ*pdiv)/SYS_CLK_FREQ;
	mdiv=((mdiv-8)<<8) & 0xff00;
	pdiv=((pdiv-2)<<2) & 0xfc;
	scale&=3;
	v=mdiv | pdiv | scale;
	gp2x_memregs[0x910>>1]=v;

}

// craigix: --trc 6 --tras 4 --twr 1 --tmrd 1 --trfc 1 --trp 2 --trcd 2
// set_RAM_Timings(6, 4, 1, 1, 1, 2, 2);
void set_RAM_Timings(int tRC, int tRAS, int tWR, int tMRD, int tRFC, int tRP, int tRCD)
{
	tRC -= 1; tRAS -= 1; tWR -= 1; tMRD -= 1; tRFC -= 1; tRP -= 1; tRCD -= 1; // ???
	gp2x_memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
	gp2x_memregs[0x3804>>1] = /*0x9000 |*/ ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
}

void set_gamma(int g100)
{
	float gamma = (float) g100 / 100;
	int i;
	gamma = 1/gamma;

    //enable gamma
    gp2x_memregs[0x2880>>1]&=~(1<<12);

    gp2x_memregs[0x295C>>1]=0;
    for(i=0; i<256; i++)
    {
		unsigned char g;
        unsigned short s;
        g =(unsigned char)(255.0*pow(i/255.0,gamma));
        s = (g<<8) | g;
		gp2x_memregs[0x295E>>1]= s;
        gp2x_memregs[0x295E>>1]= g;
    }
}





