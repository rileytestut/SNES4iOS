
#include <unistd.h>
#include "giz_kgsdk.h"
#include <FrameworkAudio.h>

unsigned short *framebuffer16[4]={0,0,0,0};
unsigned char  *framebuffer8[4];
void *GizPrimaryFrameBuffer=NULL;
volatile unsigned short  gp2x_palette[512][2];
int Timer=0;
static int bppMode = 16;
static unsigned int padValues[2]={0,0};

//Temp framebuffer
unsigned short frameBufferMemory[320*250];

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

/* 
########################
Graphics functions
########################
 */

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

		for (l=0;l<len;l++) 
		{
			if (bppMode==8)
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


unsigned int gp_getButton(unsigned char enable_diagnals)
{
	unsigned int pad=0;
	char text[256];
	int x=0;

	Framework_ProcessMessages();

	if(Framework_IsButtonPressed(0))	pad|=(1<<INP_BUTTON_LEFT);
	if(Framework_IsButtonPressed(1)) pad|=(1<<INP_BUTTON_RIGHT);
	if(Framework_IsButtonPressed(2)) pad|=(1<<INP_BUTTON_UP);
	if(Framework_IsButtonPressed(3)) pad|=(1<<INP_BUTTON_DOWN);
	if(Framework_IsButtonPressed(4)) pad|=(1<<INP_BUTTON_STOP);
	if(Framework_IsButtonPressed(5)) pad|=(1<<INP_BUTTON_PLAY);
	if(Framework_IsButtonPressed(6)) pad|=(1<<INP_BUTTON_FORWARD);
	if(Framework_IsButtonPressed(7)) pad|=(1<<INP_BUTTON_REWIND);
	if(Framework_IsButtonPressed(8)) pad|=(1<<INP_BUTTON_L);
	if(Framework_IsButtonPressed(9)) pad|=(1<<INP_BUTTON_R);
	if(Framework_IsButtonPressed(10)) pad|=(1<<INP_BUTTON_HOME);
	if(Framework_IsButtonPressed(11)) pad|=(1<<INP_BUTTON_BRIGHT);
	if(Framework_IsButtonPressed(12)) pad|=(1<<INP_BUTTON_TRIANGLE);
	if(Framework_IsButtonPressed(13)) pad|=(1<<INP_BUTTON_POWER);

	return pad;
}

void gp_initGraphics(unsigned short bpp, int flip, int applyMmuHack)
{
	if(!Framework_Init(GetModuleHandleW(NULL),0))
	{
		return;
	}
	
	if(!Framework2D_Init())
	{
		return;
	}

	bppMode = bpp;
	// Get pointer to Giz framebuffer, this will be where graphics data is blitted to
	if (GizPrimaryFrameBuffer==NULL) GizPrimaryFrameBuffer=(void*)Framework2D_LockBuffer();
	
	framebuffer8[0]=(unsigned char*)&frameBufferMemory[320*5];
	framebuffer8[1]=framebuffer8[0];
	framebuffer8[2]=framebuffer8[0];
	framebuffer8[3]=framebuffer8[0];
	
	/*if (framebuffer16[0] == 0)
	{
		framebuffer16[0]=Framework2D_LockBuffer();
		framebuffer16[1]=framebuffer16[0];
		framebuffer16[2]=framebuffer16[0];
		framebuffer16[3]=framebuffer16[0];
	}*/
	if (bppMode==16)
	{
		framebuffer16[0]=&frameBufferMemory[320*5];
		framebuffer16[1]=framebuffer16[0];
		framebuffer16[2]=framebuffer16[0];
		framebuffer16[3]=framebuffer16[0];
	}
	else
	{
		framebuffer16[0]=(unsigned short*)GizPrimaryFrameBuffer;
		framebuffer16[1]=framebuffer16[0];
		framebuffer16[2]=framebuffer16[0];
		framebuffer16[3]=framebuffer16[0];
	}
}


void gp_setFramebuffer(int flip, int sync)
{
	if (bppMode==16)
	{
		// 16bit mode, so simply copy temp buffer into Giz framebuffer
		BlitBufferToScreen((void*)(unsigned short*)framebuffer16[0],(void*)GizPrimaryFrameBuffer);
#if 0
		unsigned short *bufTo=(unsigned short*)GizPrimaryFrameBuffer;
		unsigned short *bufFrom=(unsigned short*)framebuffer16[0];
		int x,y;
		for(y=0;y<240;y++)
		{
			for(x=0;x<320;x++)
			{
				*bufTo++=*bufFrom++;
			}
			bufTo++; // increase buffer again because Giz framebuffer is 321*240 for some reason
		}
#endif
	}
	else
	{
		// 8bit mode so translate 8bit buffer to Giz 16bit framebuffer
	}
	
	/*Framework2D_UnlockBuffer();
	Framework2D_End(0);
	framebuffer16[0]=Framework2D_LockBuffer();
	framebuffer16[1]=framebuffer16[0];
	framebuffer16[2]=framebuffer16[0];
	framebuffer16[3]=framebuffer16[0];
	framebuffer8[0]=&frameBufferMemory[320*5];
	framebuffer8[1]=framebuffer8[0];
	framebuffer8[2]=framebuffer8[0];
	framebuffer8[3]=framebuffer8[0];*/
	
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
	FrameworkAudio_SetVolume(r,l);
} 

unsigned long gp2x_timer_read(void)
{
	return Framework_GetTicks();
}

void gp_initSound(int rate, int bits, int stereo, int Hz, int frag)
{
	char text[256];
	int x=0;

	x=FrameworkAudio_Init(rate, stereo, Hz);

	if(x)
	{
		sprintf(text,"Audio Init Failed, error: %x",x);
		gp_drawString(0,60,strlen(text),text,0xFFFF,framebuffer16[0]);
		MenuFlip();
		MenuPause();
	}
	
}

void gp_stopSound(void)
{
	FrameworkAudio_Close();
}


/* 
########################
System functions
########################
 */
void gp_Reset(void)
{
	Framework2D_Close();
	Framework_Close();
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




