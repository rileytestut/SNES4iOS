#include "port.h"
#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"

#include "resource.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <aygshell.h>
#include <sipapi.h>
#include <dbgapi.h>
#include <mmsystem.h>
#ifdef _WIN32_WCE_EMULATION
//#include "gapi.h"
#else
#include "gx.h"
#endif

#include "pocketpc.h"

extern HWND	g_hWnd;

#ifdef ARM
extern "C" ScreenCopyPort38(void *pBuffer, void *pDisplay, int xPitch, int yPitch);
extern "C" ScreenCopyLand38(void *pBuffer, void *pDisplay, int xPitch, int yPitch);
extern "C" ScreenCopyLand38r(void *pBuffer, void *pDisplay, int xPitch, int yPitch);
#endif

// Local functions:
__inline void Blit16pPortrait(); //Casio E-125, EM-500, Jornada 54x/56x
__inline void Blit16uPortrait();
#ifdef ARM
__inline void Blit1638portrait();  // iPAQ 38xx
#endif
__inline void Blit16mLandscapeLeft(); //@migo and iPAQ 36xx/37xx
__inline void Blit16uLandscapeLeft();
__inline void Blit16uLandscapeRight();
#ifdef ARM
__inline void Blit1638landscape(); //iPAQ 38xx
__inline void Blit1638right();	   //iPAQ 38xx
#endif

__inline void Blit16uStretchedLeft();
__inline void Blit16uStretchedRight();
__inline void Blit4uportrait();
__inline void Blit4ulandscape();

bool LoadKeypad();

//****************************************************************
// Globals
//****************************************************************
GXDisplayProperties g_gxdp;
bool                g_bGXBuffered;
GXKeyList			g_gxkl;
HWND				g_hWndCB;
TCHAR				g_szTitle[64];
EmulationMode		g_emMode         = emStopped;
bool                g_bLandscape     = false;	//Landscape mode?
bool				g_bLandLeft		 = true;	//Rotate landscape left?
bool				g_bAutoSkip		 = false;
bool				g_bCompat		 = false;	//Compatibility mode flag
bool                g_bSmoothStretch = false;
bool                g_bH3800         = false;	//Is this PDA an iPAQ 38xx?
uint32              g_iSoundQuality;
uint32				g_iCycles		 = 100;
char *				g_sFreezeGameDir;
bool				g_bUseGameFolders	 = false;
void                (*S9xBlit) ();
LARGE_INTEGER		g_liFreq;	//processor performance counter frequency

//Thread globals
//HANDLE				g_hDrawThread;
//CRITICAL_SECTION	g_cWaitForNextFrame;
HANDLE				g_hS9xMainLoopThread;

long WINAPI S9xMainLoopThread();

#define THREADCPUQUANTUM 50

bool				g_bResumeAfterLoadState = false;
bool				g_bResumeAfterSaveState = false;

typedef struct BMIWrapper
{
	BITMAPINFOHEADER	bmiHeader;
	DWORD				bmiColors[3];
} BMIWrapper;

extern "C" void S9xSyncSpeed()
{
	static LARGE_INTEGER lastframe, thisframe;

	if(g_bAutoSkip)
	{	//If the time to render the last frame is less than 13ms (77fps),
		//then blit this frame to the screen (with the ~7ms blit time,
		//this makes for a total frame period of 20ms, which is ~50fps)
		QueryPerformanceCounter(&thisframe);
		if( ((double)thisframe.QuadPart-(double)lastframe.QuadPart)/
			 ((double)g_liFreq.QuadPart) < .013
			|| IPPU.SkippedFrames >= g_iFrameSkip )
		{
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
		else
		{
			IPPU.RenderThisFrame = FALSE;
			IPPU.SkippedFrames++;
		}
		lastframe = thisframe;
	}
	else
	{
		if (IPPU.SkippedFrames < g_iFrameSkip)
		{
			IPPU.SkippedFrames++;
			IPPU.RenderThisFrame = FALSE;
		}
		else
		{
			IPPU.RenderThisFrame = TRUE;
			IPPU.SkippedFrames = 0;
		}
	}
}

#define MONO_RED_MASK			(0x1f << 11)
#define MONO_GREEN_MASK			(0x3f << 5)
#define MONO_BLUE_MASK			(0x1f)

#define RGB565_TO_MONO(a)		((a & MONO_RED_MASK) >> 14)  + \
								((a & MONO_GREEN_MASK) >> 8) + \
								((a & MONO_BLUE_MASK) >> 3)

#define	WAV_BUFFERS				64
#define	WAV_MASK				0x3F
#define	WAV_BUFFER_SIZE			0x0400

static int	sample16;
static int	snd_sent, snd_completed;
HANDLE		hData;
char 		*lpData;
HGLOBAL		hWaveHdr;
LPWAVEHDR	lpWaveHdr;
HWAVEOUT    hWaveOut; 
DWORD	    g_dwSoundBufferSize;

void CloseSoundDevice()
{ 
	if (hWaveOut)
	{
		waveOutReset(hWaveOut);

		if (lpWaveHdr)
		{
			for (int i = 0; i < WAV_BUFFERS; i++)
				waveOutUnprepareHeader(hWaveOut, lpWaveHdr + i, sizeof(WAVEHDR));
		}

		waveOutClose(hWaveOut);

		if (hWaveHdr)
		{
			GlobalUnlock(hWaveHdr); 
			GlobalFree(hWaveHdr);
		}

		if (hData)
		{
			GlobalUnlock(hData);
			GlobalFree(hData);
		}

	}

    hWaveOut  = 0;
	hData     = 0;
	hWaveHdr  = 0;
	lpWaveHdr = NULL;
}

static int iRates[8] =
{
	0, 8000, 11025, 16000, 22050, 32000, 44100, 48000
};

/*
static int iRates[8] =
{
    0, 11025, 22050, 44100, 0, 0, 0, 0
};
*/

bool8_32 S9xOpenSoundDevice(int _iMode, bool8_32 _bStereo, int _iBufferSize)
{
	WAVEFORMATEX    format; 
	int				i;
	HRESULT			hr;
	
    so.buffer_size   = WAV_BUFFER_SIZE;
    so.mute_sound    = FALSE;
    so.stereo        = FALSE;
    so.playback_rate = iRates[_iMode & 7];
    so.encoded       = 0;

    snd_sent         = 0;
	snd_completed    = 0;

	memset(&format, 0, sizeof(format));
	format.wFormatTag      = WAVE_FORMAT_PCM;
	format.nChannels       = _bStereo?(2):(1);
	format.nSamplesPerSec  = iRates[_iMode & 7];
    format.wBitsPerSample  = Settings.SixteenBitSound?(16):(8);
	format.nBlockAlign     = format.nChannels * format.wBitsPerSample / 8;
	format.cbSize          = 0;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign; 
	
	/* Open a waveform device for output using window callback. */ 
	while ((hr = waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER, &format, 
					         0, 0L, CALLBACK_NULL)) != MMSYSERR_NOERROR)
	{
		if (hr != MMSYSERR_ALLOCATED)
		{
			return false;
		}
	} 

	// Allocate and lock memory for the waveform data. The memory 
	// for waveform data must be globally allocated with 
	// GMEM_MOVEABLE and GMEM_SHARE flags. 
	g_dwSoundBufferSize = WAV_BUFFERS*WAV_BUFFER_SIZE;
	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, g_dwSoundBufferSize); 
	if (!hData) 
	{ 
		CloseSoundDevice();
		return false; 
	}

	lpData = (char *) GlobalLock(hData);
	if (!lpData)
	{ 
		CloseSoundDevice();
		return false; 
	} 
	memset (lpData, 0, g_dwSoundBufferSize);

	// Allocate and lock memory for the header. This memory must 
	// also be globally allocated with GMEM_MOVEABLE and 
	// GMEM_SHARE flags. 
	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR) * WAV_BUFFERS); 

	if (hWaveHdr == NULL)
	{ 
		CloseSoundDevice();
		return false; 
	} 

	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr); 

	if (lpWaveHdr == NULL)
	{ 
		CloseSoundDevice();
		return false; 
	}

	memset (lpWaveHdr, 0, sizeof(WAVEHDR) * WAV_BUFFERS);

	/* After allocation, set up and prepare headers. */ 
	for (i=0 ; i<WAV_BUFFERS ; i++)
	{
		lpWaveHdr[i].dwBufferLength = WAV_BUFFER_SIZE; 
		lpWaveHdr[i].lpData = lpData + i*WAV_BUFFER_SIZE;

		if (waveOutPrepareHeader(hWaveOut, lpWaveHdr+i, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			CloseSoundDevice();
			return false;
		}
	}

    sample16 = (Settings.SixteenBitSound)?(1):(0);

	return true;
}


extern "C" void S9xGenerateSound()
{
	LPWAVEHDR	h;
	int			wResult;

    if (!hWaveOut)
        return;

	//
	// find which sound blocks have completed
	//
	while (1)
	{
		if ( snd_completed == snd_sent )
		{
			break;
		}

		if ( ! (lpWaveHdr[ snd_completed & WAV_MASK].dwFlags & WHDR_DONE) )
		{
			break;
		}

		snd_completed++;	// this buffer has been played
	}

	//
	// submit two new sound blocks
	//
	while (((snd_sent - snd_completed) >> sample16) < 4)
	{
        h = lpWaveHdr + ( snd_sent&WAV_MASK );
		S9xMixSamplesO((unsigned char *) h->lpData,
                       (Settings.SixteenBitSound == TRUE)?(WAV_BUFFER_SIZE / 2):(WAV_BUFFER_SIZE), 0);

		snd_sent++;

        // Now the data block can be sent to the output device. The 
		// waveOutWrite function returns immediately and waveform 
		// data is sent to the output device in the background. 
		wResult = waveOutWrite(hWaveOut, h, sizeof(WAVEHDR)); 

		if (wResult != MMSYSERR_NOERROR)
		{ 
			CloseSoundDevice();
			return; 
		} 
	}
}

unsigned short *g_pBuffer;

extern "C" bool8_32 S9xInitUpdate()
{
	return (TRUE);
}

// Blitting functions
//
// Blitn[p|m|u|38][portrait|landscape|stretched|right]
//
//   n = bits per pixel (16 or 4)
//   p = optimized for +2 pitch (x for portrait, y for landscape)
//   m = optimized for -2 pitch (x for portrait, y for landscape)
//   u = not optimized (other pitch)
//	 38 = optimized for iPAQ 38xx
// e.g.  Blit16pportrait is 16 bpp with
//       x pitch = +2

__inline void Blit16pPortrait() //Casio E-125, EM-500, Jornada 54x/56x
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	unsigned short *pBuffer;
	unsigned short *pGFX = (unsigned short *) GFX.Screen + (SNES_WIDTH-g_gxdp.cxWidth)/2;
	int iCbyPitch2 = g_gxdp.cbyPitch>>1;
	unsigned int y;

	//while(!g_bLandscape)
	//{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		pBuffer = g_pBuffer;
		y = SNES_HEIGHT;
		do
		{
			memcpy(pBuffer, pGFX, 480);
			pGFX += SNES_WIDTH;
			pBuffer += iCbyPitch2;
		} while (--y);
		pGFX -= SNES_WIDTH * SNES_HEIGHT;
	//}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit16uPortrait()
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	unsigned short *pBuffer;
	unsigned short *pGFX;
	int iCbxPitch2 = g_gxdp.cbxPitch >> 1;
	int incBUF = (g_gxdp.cbyPitch>>1) - (240 * iCbxPitch2);
	unsigned int x, y;

	//while(!g_bLandscape)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		pBuffer = g_pBuffer;
		pGFX = (unsigned short *) GFX.Screen + (SNES_WIDTH-g_gxdp.cxWidth)/2;
		y = SNES_HEIGHT;
		do
		{
			x=240;
			do
			{
				*pBuffer = *pGFX++;
				pBuffer += iCbxPitch2;
			} while (--x);

			pGFX += SNES_WIDTH - 240;
			pBuffer += incBUF;
		} while (--y);
	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

#ifdef ARM
__inline void Blit1638portrait()  // iPAQ 38xx
{
	unsigned short *pGFX = (unsigned short *) GFX.Screen + 8; // + (SNES_WIDTH-g_gxdp.cxWidth)/2;
	unsigned short *pBuffer = (unsigned short *) g_pBuffer;
	//while(!g_bLandscape)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		ScreenCopyPort38(pGFX, g_pBuffer, g_gxdp.cbxPitch, g_gxdp.cbyPitch);
	}
}
#endif

__inline void Blit16mLandscapeLeft() //@migo and iPAQ 36xx/37xx
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	unsigned int y;
	unsigned int copyGFX = SNES_WIDTH<<1;
	int incBUF = g_gxdp.cbxPitch>>1;
	unsigned short *pBuffer = (unsigned short *) g_pBuffer + 
		(g_gxdp.cbyPitch>>1)*((g_gxdp.cyHeight+SNES_WIDTH)/2-1) + 
		(g_gxdp.cbxPitch>>1)*((g_gxdp.cxWidth-SNES_HEIGHT)/2-1);
	unsigned short *pGFX;

	//while(g_bLandscape && !g_bSmoothStretch)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);
		
		pGFX = (unsigned short *) GFX.Screen;
		y = SNES_HEIGHT;
		do
		{
			memcpy(pBuffer, pGFX, copyGFX);
			pBuffer += incBUF;
			pGFX += SNES_WIDTH;
		} while (--y);
		pBuffer -= incBUF * SNES_HEIGHT;
	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit16uLandscapeLeft()
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	int iCbyPitch2 = g_gxdp.cbyPitch >> 1;
	unsigned int x, y;
	unsigned short *pStart = g_pBuffer + 
		((g_gxdp.cyHeight+SNES_WIDTH)/2-1)*(iCbyPitch2) +
		((g_gxdp.cxWidth-SNES_HEIGHT)/2-1)*(g_gxdp.cbxPitch>>1);
	unsigned short *pBuffer;
	unsigned short *pGFX;

	//while(g_bLandscape && !g_bSmoothStretch)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		pGFX = (unsigned short *) GFX.Screen;
		y = SNES_HEIGHT;
		do
		{
			pBuffer = pStart + (SNES_HEIGHT-y)*(g_gxdp.cbxPitch>>1);
			
			x=SNES_WIDTH;
			do
			{
				*pBuffer = *pGFX++;
				pBuffer -= iCbyPitch2;
			} while (--x);

		} while (--y);
	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit16uLandscapeRight()
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	int iCbyPitch2 = g_gxdp.cbyPitch >> 1;
	unsigned int x, y;
	unsigned short *pStart = g_pBuffer + 
		((g_gxdp.cxWidth-SNES_HEIGHT)/2-1)*(g_gxdp.cbxPitch>>1) -
		((g_gxdp.cyHeight+SNES_WIDTH)/2-1)*(iCbyPitch2);

	unsigned short *pBuffer;
	unsigned short *pGFX;
		pGFX = (unsigned short *) GFX.Screen;
		y = 1;
		do
		{
			pBuffer = pStart + (SNES_HEIGHT-y)*(g_gxdp.cbxPitch>>1);
			
			x=0;
			do
			{
				*pBuffer = *pGFX++;
				pBuffer += iCbyPitch2;
			} while (++x < SNES_WIDTH);

		} while (++y < SNES_HEIGHT);
    if (g_bGXBuffered)
	    GXEndDraw();
}

#ifdef ARM
__inline void Blit1638landscape() //iPAQ 38xx
{
	unsigned short* pBuffer = (unsigned short *) g_pBuffer +
		(g_gxdp.cbyPitch>>1)*((g_gxdp.cyHeight+SNES_WIDTH)/2-1) +
		(g_gxdp.cbxPitch>>1)*((g_gxdp.cxWidth-SNES_HEIGHT)/2-1);

	//while(g_bLandscape && !g_bSmoothStretch && g_bLandLeft)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		ScreenCopyLand38(GFX.Screen, pBuffer, g_gxdp.cbxPitch, g_gxdp.cbyPitch);
	}
}

__inline void Blit1638right() //iPAQ 38xx, landscape rotated right
{
	unsigned short* pBuffer = (unsigned short *) g_pBuffer +
		(g_gxdp.cbyPitch>>1)*((g_gxdp.cyHeight-SNES_WIDTH)/2-1) +
		(g_gxdp.cbxPitch>>1)*((g_gxdp.cxWidth+SNES_HEIGHT)/2-1);

	//while(g_bLandscape && !g_bSmoothStretch && !g_bLandLeft)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		ScreenCopyLand38r(GFX.Screen, pBuffer, g_gxdp.cbxPitch, g_gxdp.cbyPitch);
	}
}
#endif

__inline void Blit16uStretchedLeft()
{
    if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	unsigned short* pStart = (unsigned short *) g_pBuffer + ((g_gxdp.cyHeight-1) * (g_gxdp.cbyPitch >> 1));
	unsigned short* pGFX;
	unsigned short* pBuffer;
	unsigned int  iOriginal1, iOriginal2, iOriginal3;
	unsigned int x, y;

	//while(g_bSmoothStretch && g_bLandscape)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);
		
		pGFX = (unsigned short *) GFX.Screen + 8;
		y = SNES_HEIGHT;
		do
		{
			pBuffer = pStart + ((g_gxdp.cxWidth>>1) + (SNES_HEIGHT>>1) - y) * (g_gxdp.cbxPitch >> 1);
			x = 240;
			do
			{
				iOriginal1 = *pGFX;
				iOriginal2 = *(++pGFX);
				iOriginal3 = *(++pGFX);
				++pGFX;

 				*pBuffer   = iOriginal1;
				pBuffer   -= g_gxdp.cbyPitch >> 1;
				*pBuffer   = (iOriginal1 & 0xf800)|(iOriginal2 &0x07ff);
				pBuffer   -= g_gxdp.cbyPitch >> 1;
				*pBuffer   = (iOriginal2 & 0xffe0)|(iOriginal3 &0x001f);
				pBuffer   -= g_gxdp.cbyPitch >> 1;
				*pBuffer   = iOriginal3;
				pBuffer   -= g_gxdp.cbyPitch >> 1;
				
			} while (x-=3);

			pGFX += 16;
		} while (--y);
	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit16uStretchedRight()
{
    if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	unsigned short* pStart = (unsigned short *) g_pBuffer + ((g_gxdp.cyHeight-1) * (g_gxdp.cbyPitch >> 1));
	unsigned short* pGFX;
	unsigned short* pBuffer;
	unsigned int  iOriginal1, iOriginal2, iOriginal3;
	unsigned int x, y;

//	while(g_bSmoothStretch && g_bLandscape)
//	{
		////EnterCriticalSection(&g_cWaitForNextFrame);
		////LeaveCriticalSection(&g_cWaitForNextFrame);
		
		pGFX = (unsigned short *) GFX.Screen + 8;
		y = 1;
		do
		{
			pBuffer = pStart + ((g_gxdp.cxWidth>>1) + (SNES_HEIGHT>>1) - y) * (g_gxdp.cbxPitch >> 1);
			x = 240;
			do
			{
				iOriginal1 = *pGFX;
				iOriginal2 = *(++pGFX);
				iOriginal3 = *(++pGFX);
				++pGFX;

 				*pBuffer   = iOriginal1;
				pBuffer   += g_gxdp.cbyPitch >> 1;
				*pBuffer   = (iOriginal1 & 0xf800)|(iOriginal2 &0x07ff);
				pBuffer   += g_gxdp.cbyPitch >> 1;
				*pBuffer   = (iOriginal2 & 0xffe0)|(iOriginal3 &0x001f);
				pBuffer   += g_gxdp.cbyPitch >> 1;
				*pBuffer   = iOriginal3;
				pBuffer   += g_gxdp.cbyPitch >> 1;
				
			} while (x-=3);

			pGFX += 16;
		} while (++y < SNES_HEIGHT);
//	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit4uportrait()
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	//while(!g_bLandscape)
	{
		//EnterCriticalSection(&g_cWaitForNextFrame);
		//LeaveCriticalSection(&g_cWaitForNextFrame);

		static int      iXOffset   = (SNES_WIDTH - g_gxdp.cxWidth) >> 1;
		static int      iYOffset   = (g_gxdp.cxWidth - SNES_HEIGHT) >> 1;
		static int		iGFXPitch2 = GFX.Pitch >> 1;
		static int      iCbxPitch2 = (g_gxdp.cbxPitch) >> 1;
		static int      iCbyPitch2 = (g_gxdp.cbyPitch) / 2;
		int				iYGFXPitch = 0;

		unsigned char  *pBuffer;
		unsigned short *pGFX;
		unsigned short  iColor1, iColor2;

		if (g_pBuffer)
		{
			for (int y = 0; y < (SNES_HEIGHT >> 1); y++)
			{
				pBuffer = (unsigned char *) g_pBuffer - y - 1;
				pGFX    = (unsigned short *) GFX.Screen + iXOffset + iYGFXPitch;

				for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
				{
					iColor1 = *pGFX;
					iColor2 = *(pGFX + iGFXPitch2);

					pGFX++;

					iColor1 = RGB565_TO_MONO(iColor1);
					iColor2 = RGB565_TO_MONO(iColor2);

					*pBuffer = 255 - ((iColor1)|(iColor2 << 4));

					pBuffer += 160;
				}

				iYGFXPitch += (iGFXPitch2 * 2);
			}
		}
	}

    if (g_bGXBuffered)
	    GXEndDraw();
}

__inline void Blit4ulandscape()
{
	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

	//while(g_bLandscape)
	{
	//	EnterCriticalSection(&g_cWaitForNextFrame);
	//	LeaveCriticalSection(&g_cWaitForNextFrame);

		static int      iXOffset   = (SNES_WIDTH - g_gxdp.cxWidth) >> 1;
		static int      iYOffset   = (g_gxdp.cxWidth - SNES_HEIGHT) >> 1;
		static int		iGFXPitch2 = GFX.Pitch >> 1;
		static int      iCbxPitch2 = (g_gxdp.cbxPitch) >> 1;
		static int      iCbyPitch2 = (g_gxdp.cbyPitch) / 2;
		int				iYGFXPitch = 0;

		unsigned char  *pBuffer;
		unsigned short *pGFX;
		unsigned short  iColor1, iColor2;

		if (g_pBuffer)
		{
			for (int y = iYOffset; y < (SNES_HEIGHT + iYOffset); y++)
			{
				pBuffer = (unsigned char *) g_pBuffer + (y * 160) + ((g_gxdp.cyHeight - SNES_WIDTH) >> 2);
				pGFX    = (unsigned short *) GFX.Screen + iYGFXPitch;

				for (int x = 0; x < (SNES_WIDTH / 2); x++)
				{
					iColor1 = *pGFX++;
					iColor2 = *pGFX++;

					iColor1 = RGB565_TO_MONO(iColor1);
					iColor2 = RGB565_TO_MONO(iColor2);

					*pBuffer++ = 255 - ((iColor2)|(iColor1 << 4));
				}

				iYGFXPitch += iGFXPitch2;
			}
		}
	}
    if (g_bGXBuffered)
	    GXEndDraw();
}

void Initialize16()
{
    SetKeypad();

	if (g_bGXBuffered && !g_bH3800)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

    // Clear screen
    unsigned short *pBuffer;

	for (int y = 0; y < (int) g_gxdp.cyHeight; y++)
	{
		pBuffer = (unsigned short *) g_pBuffer + ((y * g_gxdp.cbyPitch) >> 1);

		for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
		{
            *pBuffer = 0;

			pBuffer += (g_gxdp.cbxPitch >> 1);
        }
    }

    // Draw keypad
    if (!g_bLandscape)
    {
		if (LoadKeypad())
		{
			static int      iXOffset   = (256 - g_gxdp.cxWidth) >> 1;
			int             iCbxPitch2 = g_gxdp.cbxPitch >> 1;
			unsigned short *pvBits     = g_pKeypad + (240 * 96);

			for (int y = 319; y >= 224; y--)
			{
				pBuffer = g_pBuffer + (y * g_gxdp.cbyPitch / 2);

				for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
				{
					*pBuffer = *pvBits;

					pBuffer += iCbxPitch2;
					pvBits++;
				}
			}
		}
    }

	if (g_bGXBuffered && !g_bH3800)
	    GXEndDraw();
}

void Initialize4()
{
    SetKeypad();

	if (g_bGXBuffered)
	    g_pBuffer = (unsigned short *) GXBeginDraw();

    // Clear screen
	unsigned char *pBuffer;

	for (int y = 0; y < (int) g_gxdp.cyHeight; y++)
	{
		pBuffer = (unsigned char *) g_pBuffer - (y >> 1) - 1;

		for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
		{
            *pBuffer = 0xff;

			pBuffer += 160;
        }
    }

    // Draw keypad
    if (!g_bLandscape)
    {
		if (LoadKeypad())
		{
			static int      iXOffset   = (256 - g_gxdp.cxWidth) / 2;
			int             iCbxPitch2 = g_gxdp.cbxPitch / 2;
			unsigned short *pvBits     = g_pKeypad + (240 * 96);
			unsigned short  iColor1, iColor2;

			for (int y = 319; y >= 224; y--)
			{
				pBuffer = (unsigned char *) g_pBuffer - (y >> 1) - 1;

				for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
				{
					iColor1 = *pvBits;
					iColor2 = *(pvBits + 240);

					iColor1 = RGB565_TO_MONO(iColor1);
					iColor2 = RGB565_TO_MONO(iColor2);

					*pBuffer = 255 - ((iColor2)|((iColor1) << 4));

					pBuffer += 160;
					pvBits++;
				}
			}
		}
    }

    if (g_bGXBuffered)
	    GXEndDraw();
}

/*
long WINAPI DrawThread()
{
	EnterCriticalSection(&g_cWaitForNextFrame);
	LeaveCriticalSection(&g_cWaitForNextFrame);

	for(;;)
	{
		if (g_gxdp.cBPP == 16)
		{
			if (g_bLandscape)
			{
				if (g_bSmoothStretch)
				{
					Blit16stretched();
				}
				else if (g_gxdp.cbyPitch == -2)
				{
					// Landscape optimized
					Blit16mlandscape();
				}
#ifdef ARM
				else if (g_bH3800)
				{
					// Landscape optimized (iPAQ 38xx)
					if(g_bLandLeft)
						Blit1638landscape();
					else
						Blit1638right();
				}
#endif
				else
				{
    				// Landscape unoptimized
					Blit16ulandscape();
				}
			}
			else
			{
				if (g_gxdp.cbxPitch == 2)
				{
					// Portrait optimized
					Blit16pportrait();
				}
#ifdef ARM
				else if(g_bH3800)
				{
					// Portrait optimized (iPAQ 38xx)
					Blit1638portrait();
				}
#endif
				else
				{
					// Portrait unoptimized
					Blit16uportrait();
				}
			}
		}
		else if (g_gxdp.cBPP == 4)
		{
			if (g_bLandscape)
			{
				Blit4ulandscape();
			}
			else
			{
				Blit4uportrait();
			}
		}
	}

	return 1;
}
*/

extern "C" bool8_32 S9xDeinitUpdate(int _iWidth, int _iHeight, bool8_32 _bSixteenBit)
{
	//LeaveCriticalSection(&g_cWaitForNextFrame);
	//EnterCriticalSection(&g_cWaitForNextFrame);
	
					if (g_gxdp.cBPP == 16)
		{
			if (g_bLandscape)
			{
				if (g_bSmoothStretch)
				{
					if (g_bLandLeft)
						Blit16uStretchedLeft();
					else
						Blit16uStretchedRight();
				}
				else if (g_gxdp.cbyPitch == -2)
				{
					// Landscape optimized
					if (g_bLandLeft)
						Blit16mLandscapeLeft();
					else
						Blit16uLandscapeRight();
				}
#ifdef ARM
				else if (g_bH3800)
				{
					// Landscape optimized (iPAQ 38xx)
					if(g_bLandLeft)
						Blit1638landscape();
					else
						Blit1638right();
				}
#endif
				else
				{
    				// Landscape unoptimized
					if (g_bLandLeft)
					{
						Blit16uLandscapeLeft();
					}
					else
					{
						Blit16uLandscapeRight();
					}
				}
			}
			else
			{
				if (g_gxdp.cbxPitch == 2)
				{
					// Portrait optimized
					Blit16pPortrait();
				}
#ifdef ARM
				else if(g_bH3800)
				{
					// Portrait optimized (iPAQ 38xx)
					Blit1638portrait();
				}
#endif
				else
				{
					// Portrait unoptimized
					Blit16uPortrait();
				}
			}
		}
		else if (g_gxdp.cBPP == 4)
		{
			if (g_bLandscape)
			{
				Blit4ulandscape();
			}
			else
			{
				Blit4uportrait();
			}
		}
	return (TRUE);
}

extern "C" void S9xTextMode()
{
}

extern "C" void S9xGraphicsMode()
{
}

extern "C" void S9xExit()
{
}

extern "C" void S9xSetPalette()
{
}

extern "C" void S9xMessage(int, int, const char *_pszMessage)
{
	DEBUGMSG(TRUE, (L"%S\n",_pszMessage));
}

extern "C" bool8_32 S9xReadMousePosition(int _iWhich, int &_iX, int &_iY, uint32 &_dwButtons)
{
	return 0;
}

extern "C" bool8_32 S9xReadSuperScopePosition(int &_iX, int &_iY, uint32 &_dwButtons)
{
	return 0;
}

uint32  g_iJoypadState;

extern "C" uint32 S9xReadJoypad(int _iWhich)
{
    if (_iWhich == 0)
    {
        return g_iJoypadState | 0x80000000;
    }

	return 0;
}

extern "C" bool8_32 S9xOpenSnapshotFile(const char *_pszFilename, bool8_32 _bReadOnly, STREAM *_File)
{
    if (_bReadOnly)
    {
	    if ((*_File = OPEN_STREAM(_pszFilename, "rb")))
	        return (TRUE);
    }
    else
    {
	    if ((*_File = OPEN_STREAM(_pszFilename, "wb")))
	        return (TRUE);
    }

	return 0;
}

extern "C" void S9xCloseSnapshotFile(STREAM file)
{
    CLOSE_STREAM(file);
}

void S9xAutoSaveSRAM()
{
    Memory.SaveSRAM(S9xGetSRAMFilename());
}

extern "C" const char *S9xBasename(const char *_pszF)
{
	return "";
}

extern "C" void S9xExtraUsage()
{
}

extern "C" void S9xParseArg(char **argv, int &i, int argc)
{
}

const char *S9xGetSRAMFilename()
{
    static char filename [PATH_MAX];
    char drive [_MAX_DRIVE];
    char dir [_MAX_DIR];
    char fname [_MAX_FNAME];
    char ext [_MAX_EXT];

    _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
    strcpy (filename, fname);
    strcat (filename, ".srm");

    return (filename);
}

BYTE ScreenBuffer [256*239*2];
BYTE SubScreenBuffer [256*239*2];
BYTE ZBuffer [256*239];
BYTE SubZBuffer [256*239];

void S9xInitDisplay()
{
    GFX.SubScreen = SubScreenBuffer;
    GFX.ZBuffer = ZBuffer;
    GFX.SubZBuffer = SubZBuffer;

    GFX.RealPitch = GFX.Pitch = 256 * 2;
    GFX.Screen = ScreenBuffer;
    GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
    
	if (Settings.SixteenBit)
    {
        GFX.PPL = GFX.Pitch >> 1;
        GFX.PPLx2 = GFX.Pitch;
    }
    else
    {
        GFX.PPL = GFX.Pitch;
        GFX.PPLx2 = GFX.Pitch * 2;
    }
}

void S9xSetTitle(const char *_szTitle)
{
	wsprintf(g_szTitle, TEXT("%S"), _szTitle);
}


#define MAX_LOADSTRING 100

unsigned short *g_pKeypad = NULL;
HBITMAP         hbmTemp   = NULL;
HDC             hMemoryDC;

bool LoadKeypad()
{
	unsigned short 	*pvBits = NULL;

    if (!hbmTemp)
    {
	    HDC hDC = GetDC(g_hWnd);
        
        hMemoryDC = CreateCompatibleDC(hDC);

	    BMIWrapper	     bmInfo;

	    memset(&bmInfo, 0, sizeof(BMIWrapper));
	    bmInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	    bmInfo.bmiHeader.biWidth       = 240;
	    bmInfo.bmiHeader.biHeight      = 192;
	    bmInfo.bmiHeader.biPlanes      = 1;
	    bmInfo.bmiHeader.biBitCount    = 16;
	    bmInfo.bmiHeader.biCompression = BI_BITFIELDS;
	    bmInfo.bmiColors[0]			   = 0xf800;
	    bmInfo.bmiColors[1]            = 0x07e0;
	    bmInfo.bmiColors[2]            = 0x001f;

	    hbmTemp = CreateDIBSection(hMemoryDC, (BITMAPINFO *) &bmInfo, DIB_RGB_COLORS, (void **) &pvBits, NULL, 0);

        ReleaseDC(g_hWnd, hDC);
    }

	if (hbmTemp)
	{
        SelectObject(hMemoryDC, hbmTemp);

        BitBlt(hMemoryDC, 0, 0, 240, 192, g_hSkinDC, 0, 0, SRCCOPY);

        if (!g_pKeypad)
            g_pKeypad = pvBits;
    }

    return (g_pKeypad != NULL);
}

void DrawKeypad()
{
	if (g_gxdp.cBPP == 16)
	{
        Initialize16();
	}
	else if (g_gxdp.cBPP == 4)
	{
        Initialize4();
	}
}

bool InitializeEmulation(int _iTransparency, int _iSoundEnabled, int _iSixteenBitSound)
{
    ZeroMemory(&Settings, sizeof(Settings));

	LoadOptions();

    Settings.JoystickEnabled = FALSE;
    //Settings.Stereo = FALSE;
    Settings.SoundBufferSize = 0;
    Settings.CyclesPercentage = g_iCycles;
    //Settings.DisableSoundEcho = TRUE;
    Settings.APUEnabled = _iSoundEnabled;
    Settings.NextAPUEnabled = Settings.APUEnabled;
    Settings.SixteenBitSound = _iSixteenBitSound;
    Settings.H_Max = (SNES_CYCLES_PER_SCANLINE * Settings.CyclesPercentage) / 100;
    Settings.SkipFrames = AUTO_FRAMERATE;
    Settings.Shutdown = Settings.ShutdownMaster = TRUE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.FrameTime = Settings.FrameTimeNTSC;
    Settings.DisableSampleCaching = !(_iSoundEnabled);
    Settings.DisableMasterVolume = !(_iSoundEnabled);
    Settings.Mouse = FALSE;
    Settings.SuperScope = FALSE;
    Settings.MultiPlayer5 = FALSE;
    Settings.ControllerOption = SNES_MULTIPLAYER5;
    Settings.Transparency = _iTransparency;
    Settings.SixteenBit = TRUE;
    Settings.SupportHiRes = FALSE;
	Settings.SoundSync = _iSoundEnabled;
    Settings.ThreadSound = FALSE;
	Settings.Mute = !_iSoundEnabled;
    //Settings.DisplayFrameRate = TRUE;

	Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

	if (!Memory.Init() || !S9xInitAPU())
	{
		return false;
	}

	S9xInitDisplay();
	// S9xInitInputDevices();

#ifdef GFX_MULTI_FORMAT
    S9xSetRenderPixelFormat(RGB565);
#endif

	return true;
}

bool LoadROM()
{
    // Load ROM
	LRESULT			lResult = TRUE;
	TCHAR			szFile[MAX_PATH] = TEXT("\0");
	OPENFILENAME	ofn;
	char			szRom[MAX_PATH];
   
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;   
	ofn.lpstrFilter = TEXT("All (*.smc,*.fig,*.zip)\0*.smc,*.fig,*.zip\0");   
    ofn.lpstrTitle = TEXT("Open ROM");
	ofn.Flags = OFN_EXPLORER;
    
	if (GetOpenFileName(&ofn))
    {
		sprintf(szRom, "%S", ofn.lpstrFile);
   		if (Memory.LoadROM(szRom))
		{
			Memory.LoadSRAM(S9xGetSRAMFilename());
		}
		else
		{
			S9xReset();
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool8_32 LoadSlot(int slot)
{
	LRESULT			lResult = TRUE;
	static char filename [_MAX_PATH + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
	
	if(g_emMode != emStopped)
	{
		//if (freezefiledir exists)
		//{
		//	_splitpath(Memory::ROMFilename, drive, dir, filename, ext);
		//	sprintf (ext, ".%03d", savenumber);
		//	_makepath (szState, dir, freezefiledir, filename, ext);
		//}
		//else
		{
			_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
			sprintf (ext, "%03d", slot);
			_makepath (filename, drive, dir, fname, ext);
		}

   		if (!S9xUnfreezeGame(filename))
		{
			return false;
		}

		if(g_bResumeAfterLoadState)
			StartEmulation(g_hWnd, g_hWndCB);

		return true;
	}
	else
	{
		return false;
	}
}


bool LoadState()
{
	// load state
	LRESULT			lResult = TRUE;
	TCHAR			szFile[MAX_PATH] = TEXT("\0");
	OPENFILENAME	ofn;
	char			szState[MAX_PATH];
   
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;   
	ofn.lpstrFilter = TEXT("All (*.pss)\0*.pss\0");   
    ofn.lpstrTitle = TEXT("Load State");
	ofn.Flags = OFN_EXPLORER;
    
	if (GetOpenFileName(&ofn))
    {
		sprintf(szState, "%S", ofn.lpstrFile);
   		if (!S9xUnfreezeGame(szState))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool SaveState()
{
	// Save state
	LRESULT			lResult = TRUE;
	TCHAR			szFile[MAX_PATH] = TEXT("\0");
	OPENFILENAME	ofn;
	char			szState[MAX_PATH];
   
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;   
	ofn.lpstrFilter = TEXT("All (*.pss)\0*.pss\0");   
    ofn.lpstrTitle = TEXT("Save State");
	ofn.Flags = OFN_EXPLORER;
    
	if (GetSaveFileName(&ofn))
    {
		sprintf(szState, "%S", ofn.lpstrFile);
   		if (!S9xFreezeGame(szState))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool8_32 SaveSlot(int slot)
{
	LRESULT			lResult = TRUE;

	static char filename [_MAX_PATH + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];
	
	if(g_emMode != emStopped)
	{
		//if (freezefiledir exists)
		//{
		//	_splitpath(Memory::ROMFilename, drive, dir, filename, ext);
		//	sprintf (ext, ".%03d", savenumber);
		//	_makepath (szState, dir, freezefiledir, filename, ext);
		//}
		//else
		{
			_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
			sprintf (ext, "%03d", slot);
			_makepath (filename, drive, dir, fname, ext);
		}

   			if (!S9xFreezeGame(filename))
			{
				return false;
			};

			if(g_bResumeAfterSaveState)
				StartEmulation(g_hWnd, g_hWndCB);
		return true;
	}
	else
	{
		return false;
	}

}


bool StartEmulation(HWND hWnd, HWND hWndCB)
{
	
	ShowWindow(hWndCB, SW_HIDE);

    RECT rc;
	GetWindowRect(hWnd, &rc);
	rc.top    = 0;
	rc.bottom = 320;
	SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right, rc.bottom, 0);

    if (SHFullScreen(hWnd, SHFS_HIDESIPBUTTON|SHFS_HIDETASKBAR|SHFS_HIDESTARTICON))
	{
		SHSipPreference(hWnd, SIP_FORCEDOWN);
	}
	else
	{
		return false;
	}

    SetForegroundWindow(hWnd);

	// Try opening the Display for Fullscreen access
	if (GXOpenDisplay(hWnd, GX_FULLSCREEN) == 0)
	{
		return false;
	}

    if (GXSetViewport(0, 320, 0, 0) == 0)
    {
        return false;
    }

	// Initialize the Hardware Buttons
	GXOpenInput();

	// Get the Display properties
	g_gxdp		  = GXGetDisplayProperties();
	g_bGXBuffered = (GXIsDisplayDRAMBuffer() == TRUE);

	// Check for upgraded Aero and fix properties
	if (g_gxdp.cbxPitch == 0xf000)
	{
		g_gxdp.cbxPitch = 640;
	}

    // Check PDA model for device-specific fixes/optimizations
    unsigned short szTemp[64];

    SystemParametersInfo(SPI_GETOEMINFO, sizeof(szTemp), szTemp, 0);

	if (!_wcsicmp(szTemp, _T("Compaq iPAQ H3800")))		//iPAQ 38xx
    {
        g_bH3800        = true;
		g_pBuffer       = (unsigned short *) 0xac0755a0;
		g_gxdp.ffFormat = 168;
        g_gxdp.cxWidth  = 240;
        g_gxdp.cyHeight = 320;
        g_gxdp.cBPP     = 16;
        g_gxdp.cbyPitch = 2;
        g_gxdp.cbxPitch = -640;
		g_bGXBuffered = false;
    }
	else if (!_wcsicmp(szTemp, _T("Pocket PC J710")))	//Casio E-200
	{
		//nothing yet :(
	}

	// Get information about the Hardware Keys
	g_gxkl = GXGetDefaultKeys(GX_NORMALKEYS);

    if (Settings.APUEnabled)
    {
        Settings.NextAPUEnabled = Settings.APUEnabled;
        S9xInitSound(Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize);
    }

    if (!S9xGraphicsInit())
	{
		return false;
	}

    S9xTextMode();
	S9xGraphicsMode();

    if (!g_bH3800)
    	g_pBuffer = (unsigned short *) GXBeginDraw();

    DrawKeypad();
    S9xSetSoundMute(!Settings.APUEnabled);

    S9xSetTitle(Memory.ROMName);

	//Get the processor performance counter frequency
	QueryPerformanceFrequency(&g_liFreq);
	
	//Start drawing thread
	//InitializeCriticalSection(&g_cWaitForNextFrame);
	//EnterCriticalSection(&g_cWaitForNextFrame);
	//g_hDrawThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DrawThread, NULL, 0, NULL);
	//CeSetThreadQuantum(g_hDrawThread, 10);

#ifdef THREADCPU
	g_hS9xMainLoopThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)S9xMainLoopThread, NULL, 0, NULL);
	CeSetThreadQuantum(g_hS9xMainLoopThread, THREADCPUQUANTUM);
	SetThreadPriority(g_hS9xMainLoopThread, THREAD_PRIORITY_NORMAL);
#endif

	g_emMode = emRunning;

	return true;
}

HBITMAP			g_bmPausedBitmap;
HBITMAP			g_hbmPausedDIB;
HDC				g_hPausedDC;


bool PauseEmulation(HWND hWnd, HWND hWndCB)
{
    if(!g_bH3800) GXEndDraw();
#ifdef THREADCPU
	SuspendThread(g_hS9xMainLoopThread);
#endif
    GXCloseDisplay();
	GXCloseInput();

	HDC	hDC = GetDC(hWnd);

	g_hPausedDC = CreateCompatibleDC(hDC);

	typedef struct BMIWrapper
	{
		BITMAPINFOHEADER	bmiHeader;
		DWORD				bmiColors[3];
	} BMIWrapper;

	BMIWrapper	     bmInfo;
	unsigned short 	*pvBits = NULL;

	memset(&bmInfo, 0, sizeof(BMIWrapper));
	bmInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth       = 240;
	bmInfo.bmiHeader.biHeight      = 224;
	bmInfo.bmiHeader.biPlanes      = 1;
	bmInfo.bmiHeader.biBitCount    = 16;
	bmInfo.bmiHeader.biCompression = BI_BITFIELDS;
	bmInfo.bmiColors[0]			   = 0xf800;
	bmInfo.bmiColors[1]            = 0x07e0;
	bmInfo.bmiColors[2]            = 0x001f;

	g_hbmPausedDIB = CreateDIBSection(g_hPausedDC, (BITMAPINFO *) &bmInfo, DIB_RGB_COLORS, (void **) &pvBits, NULL, 0);

	if (g_hbmPausedDIB)
	{
		SelectObject(g_hPausedDC, g_hbmPausedDIB);

		static int      iXOffset   = (256 - g_gxdp.cxWidth) / 2;
		int				iGFXPitch2 = GFX.Pitch / 2;
		int				iYGFXPitch = iGFXPitch2 * 224;
		unsigned short 	iValue;

		for (int y = 0; y < 224; y++)
		{
			for (int x = 0; x < (int) g_gxdp.cxWidth; x++)
			{
				iValue = *((unsigned short *) GFX.Screen + (x + iXOffset) + iYGFXPitch);

				*pvBits = iValue;
				pvBits++;
			}

			iYGFXPitch -= iGFXPitch2;
		}
	}

	ReleaseDC(hWnd, hDC);

	RECT rc;
	GetWindowRect(hWnd, &rc);
	rc.top    = 26;
	rc.bottom = 320 - 26;
	MoveWindow(hWnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);

	if (!SHFullScreen(hWnd, SHFS_HIDESIPBUTTON|SHFS_SHOWTASKBAR|SHFS_SHOWSTARTICON))
	{
		return false;
	}
	
	ShowWindow(hWndCB, SW_SHOW);

    Memory.SaveSRAM(S9xGetSRAMFilename());
    S9xSetSoundMute(TRUE);

	g_emMode = emPaused;

	return true;
}

bool ResumeEmulation(HWND hWnd, HWND hWndCB)
{
	DeleteObject(g_bmPausedBitmap);
	DeleteObject(g_hPausedDC);

	GXOpenInput();

	RECT rc;
	GetWindowRect(hWnd, &rc);
	rc.top    = 0;
	rc.bottom = 320;
	SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right, rc.bottom, 0);

	if (SHFullScreen(hWnd, SHFS_HIDESIPBUTTON|SHFS_HIDETASKBAR|SHFS_HIDESTARTICON))
	{
		SHSipPreference(hWnd, SIP_FORCEDOWN);
	}
	else
	{
		return false;
	}

	ShowWindow(hWndCB, SW_HIDE);

	// Try opening the Display for Fullscreen access
	if (GXOpenDisplay(hWnd, GX_FULLSCREEN) == 0)
	{
		return false;
	}

    if (!g_bH3800)
    	g_pBuffer = (unsigned short *) GXBeginDraw();
#ifdef THREADCPU
	ResumeThread(g_hS9xMainLoopThread);
#endif

    DrawKeypad();
    S9xSetSoundMute(FALSE);

	g_emMode = emRunning;

	return true;
}

bool StopEmulation(HWND hWnd, HWND hWndCB)
{
	//Stop drawing thread
	//CloseHandle(g_hDrawThread);
	//DeleteCriticalSection(&g_cWaitForNextFrame);
#ifdef THREADCPU
	CloseHandle(g_hS9xMainLoopThread);
#endif

    if ((g_emMode == emRunning) || (g_emMode == emPaused))
    {
        Memory.SaveSRAM(S9xGetSRAMFilename());
        CloseSoundDevice();
    }

    if (g_emMode == emRunning)
	{
		GXCloseDisplay();

		RECT rc;
		GetWindowRect(hWnd, &rc);
		rc.top    = 0;
		rc.bottom = 320 - 26;
		SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right, rc.bottom, 0);

		if (!SHFullScreen(hWnd, SHFS_HIDESIPBUTTON|SHFS_SHOWTASKBAR|SHFS_SHOWSTARTICON))
		{
			return false;
		}

		ShowWindow(hWndCB, SW_SHOW);
    }
    
	g_emMode = emStopped;

	return true;
}


const char *S9xGetFilename (const char *ex)
{
    static char filename [PATH_MAX + 1];
    char drive [_MAX_DRIVE + 1];
    char dir [_MAX_DIR + 1];
    char fname [_MAX_FNAME + 1];
    char ext [_MAX_EXT + 1];

    _splitpath (Memory.ROMFilename, drive, dir, fname, ext);
    strcpy (filename, fname);
    strcat (filename, ex);

    return (filename);
}

void S9xLoadSDD1Data ()
{
}

void _makepath (char *path, const char *drive, const char *dir,
		const char *fname, const char *ext)
{
    if (drive && *drive)
    {
	*path = *drive;
	*(path + 1) = ':';
	*(path + 2) = 0;
    }
    else
	*path = 0;
	
    if (dir && *dir)
    {
	strcat (path, dir);
	if (strlen (dir) != 1 || *dir != '\\')
	    strcat (path, SLASH_STR);
    }
	
    strcat (path, fname);
    if (ext && *ext)
    {
        strcat (path, ".");
        strcat (path, ext);
    }
}

void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext)
{
    if (*path && *(path + 1) == ':')
    {
	*drive = toupper (*path);
	path += 2;
    }
    else
	*drive = 0;

    char *slash = strrchr (path, SLASH_CHAR);
    if (!slash)
	slash = strrchr (path, '/');
    char *dot = strrchr (path, '.');
    if (dot && slash && dot < slash)
	dot = NULL;

    if (!slash)
    {
	if (*drive)
	    strcpy (dir, "\\");
	else
	    strcpy (dir, "");
	strcpy (fname, path);
        if (dot)
        {
	    *(fname + (dot - path)) = 0;
	    strcpy (ext, dot + 1);
        }
	else
	    strcpy (ext, "");
    }
    else
    {
	if (*drive && *path != '\\')
	{
	    strcpy (dir, "\\");
	    strcat (dir, path);
	    *(dir + (slash - path) + 1) = 0;
	}
	else
	{
	    strcpy (dir, path);
	    if (slash - path == 0)
		*(dir + 1) = 0;
	    else
		*(dir + (slash - path)) = 0;
	}

	strcpy (fname, slash + 1);
        if (dot)
	{
	    *(fname + (dot - slash) - 1) = 0;
    	    strcpy (ext, dot + 1);
	}
	else
	    strcpy (ext, "");
    }
}

#ifdef THREADCPU
long WINAPI S9xMainLoopThread()
{
	struct SICPU		* icpu  = &ICPU;
	struct SCPUState	* cpu	= &CPU;
	struct SRegisters	* reg	= &Registers;
	for(;;)
	{
		while (!(CPU.Flags & FRAME_ADVANCE_FLAG))
		{
			S9xMainLoop(reg, icpu, cpu);
		}
		Sleep(0);
	}
	return 0;
}
#endif