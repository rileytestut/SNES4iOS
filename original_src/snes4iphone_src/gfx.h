/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#ifndef _GFX_H_
#define _GFX_H_

#include "port.h"

struct SGFX{
    // Initialize these variables
    uint8  *Screen;
    uint8  *SubScreen;
    uint8  *ZBuffer;
    uint8  *SubZBuffer;
    uint32 Pitch;

    // Setup in call to S9xGraphicsInit()
    int   Delta;
    uint16 *X2;
    uint16 *ZERO_OR_X2;
    uint16 *ZERO;
    uint32 RealPitch; // True pitch of Screen buffer.
    uint32 Pitch2;    // Same as RealPitch except while using speed up hack for Glide.
    uint32 ZPitch;    // Pitch of ZBuffer
    uint32 PPL;	      // Number of pixels on each of Screen buffer
    uint32 PPLx2;
    uint32 PixSize;
    uint8  *S;
    uint8  *DB;
    uint16 *ScreenColors;
    uint32 DepthDelta;
    uint8  Z1;
    uint8  Z2;
    uint32 FixedColour;
    const char *InfoString;
    uint32 InfoStringTimeout;
    uint32 StartY;
    uint32 EndY;
    struct ClipData *pCurrentClip;
    uint32 Mode7Mask;
    uint32 Mode7PriorityMask;
    int	   OBJList [129];
    uint32 Sizes [129];
    int    VPositions [129];

    uint8  r212c;
    uint8  r212d;
    uint8  r2130;
    uint8  r2131;
    bool8_32  Pseudo;
    
#ifdef GFX_MULTI_FORMAT
    uint32 PixelFormat;
    uint32 (*BuildPixel) (uint32 R, uint32 G, uint32 B);
    uint32 (*BuildPixel2) (uint32 R, uint32 G, uint32 B);
    void   (*DecomposePixel) (uint32 Pixel, uint32 &R, uint32 &G, uint32 &B);
#endif
};

struct SLineData {
    struct {
	uint16 VOffset;
	uint16 HOffset;
    } BG [4];
};

#define H_FLIP 0x4000
#define V_FLIP 0x8000
#define BLANK_TILE 2

struct SBG
{
    uint32 TileSize;
    uint32 BitShift;
    uint32 TileShift;
    uint32 TileAddress;
    uint32 NameSelect;
    uint32 SCBase;

    uint32 StartPalette;
    uint32 PaletteShift;
    uint32 PaletteMask;
    
    uint8 *Buffer;
    uint8 *Buffered;
    bool8_32  DirectColourMode;
};

struct SLineMatrixData
{
    short MatrixA;
    short MatrixB;
    short MatrixC;
    short MatrixD;
    short CentreX;
    short CentreY;
};

extern uint32 odd_high [4][16];
extern uint32 odd_low [4][16];
extern uint32 even_high [4][16];
extern uint32 even_low [4][16];
extern SBG BG;
extern uint16 DirectColourMaps [8][256];

//extern uint8 add32_32 [32][32];
//extern uint8 add32_32_half [32][32];
//extern uint8 sub32_32 [32][32];
//extern uint8 sub32_32_half [32][32];
extern uint8 mul_brightness [16][32];

// Could use BSWAP instruction on Intel port...
#define SWAP_DWORD(dw) dw = ((dw & 0xff) << 24) | ((dw & 0xff00) << 8) | \
		            ((dw & 0xff0000) >> 8) | ((dw & 0xff000000) >> 24)

#ifdef FAST_LSB_WORD_ACCESS
#define READ_2BYTES(s) (*(uint16 *) (s))
#define WRITE_2BYTES(s, d) *(uint16 *) (s) = (d)
#else
#ifdef LSB_FIRST
#define READ_2BYTES(s) (*(uint8 *) (s) | (*((uint8 *) (s) + 1) << 8))
#define WRITE_2BYTES(s, d) *(uint8 *) (s) = (d), \
			   *((uint8 *) (s) + 1) = (d) >> 8
#else  // else MSB_FISRT
#define READ_2BYTES(s) (*(uint8 *) (s) | (*((uint8 *) (s) + 1) << 8))
#define WRITE_2BYTES(s, d) *(uint8 *) (s) = (d), \
			   *((uint8 *) (s) + 1) = (d) >> 8
#endif // LSB_FIRST
#endif // i386

#define SUB_SCREEN_DEPTH 0
#define MAIN_SCREEN_DEPTH 32

#if defined(OLD_COLOUR_BLENDING)
#define COLOR_ADD(C1, C2) \
GFX.X2 [((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & RGB_LOW_BITS_MASK)]
#else
#define COLOR_ADD(C1, C2) \
(GFX.X2 [((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	 ((C1) & (C2) & RGB_LOW_BITS_MASK)] | \
 (((C1) ^ (C2)) & RGB_LOW_BITS_MASK))	   
#endif

#define COLOR_ADD1_2(C1, C2) \
(((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         ((C1) & (C2) & RGB_LOW_BITS_MASK) | ALPHA_BITS_MASK)

#if defined(OLD_COLOUR_BLENDING)
#define COLOR_SUB(C1, C2) \
GFX.ZERO_OR_X2 [(((C1) | RGB_HI_BITS_MASKx2) - \
		 ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]
#else
#define COLOR_SUB(C1, C2) \
(GFX.ZERO_OR_X2 [(((C1) | RGB_HI_BITS_MASKx2) - \
                  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1] + \
((C1) & RGB_LOW_BITS_MASK) - ((C2) & RGB_LOW_BITS_MASK))
#endif

#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | RGB_HI_BITS_MASKx2) - \
	   ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]

typedef void (*NormalTileRenderer) (uint32 Tile, uint32 Offset, 
				    uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
typedef void (*ClippedTileRenderer) (uint32 Tile, uint32 Offset,
				     uint32 StartPixel, uint32 Width,
				     uint32 StartLine, uint32 LineCount, struct SGFX * gfx);
typedef void (*LargePixelRenderer) (uint32 Tile, uint32 Offset,
				    uint32 StartPixel, uint32 Pixels,
				    uint32 StartLine, uint32 LineCount, struct SGFX * gfx);

START_EXTERN_C
void S9xStartScreenRefresh ();
void S9xDrawScanLine (uint8 Line);
void S9xEndScreenRefresh ();
void S9xSetupOBJ (struct SOBJ *);
void S9xUpdateScreen ();
void RenderLine (uint8 line);
void S9xBuildDirectColourMaps ();

// External port interface which must be implemented or initialised for each
// port.
extern struct SGFX GFX;

bool8_32 S9xGraphicsInit ();
void S9xGraphicsDeinit();
bool8_32 S9xInitUpdate (void);
bool8_32 S9xDeinitUpdate (int Width, int Height, bool8_32 sixteen_bit);
void S9xSetPalette ();
void S9xSyncSpeed ();

#ifdef GFX_MULTI_FORMAT
bool8_32 S9xSetRenderPixelFormat (int format);
#endif

END_EXTERN_C

#endif
