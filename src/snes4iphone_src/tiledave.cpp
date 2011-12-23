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
 
//#ifndef asmPPU
#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"


//#define asmPPU16
#define asmPPU8
//#define asmPPU16T

typedef union
{
    struct { uint8 b0,b1,b2,b3; } B;
    uint32 W;
} yo_uint32;

uint8 *SubScreenTranspBuffer;
uint8 tmpCache[64];
extern int squidgetranshackenable;


#ifdef asmPPU16
extern "C" void asmDrawTile16(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
extern "C" void asmDrawTileClipped16(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
#endif

#ifdef asmPPU8
extern "C" void asmDrawTile8(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
extern "C" void asmDrawTileClipped8(uint32 cache,uint32 solidbuf,uint32 OffsetGP32,uint32 Flip);
#endif

// RGB565
#define yoRGB_REMOVE_LOW_BITS_MASK ((30<<11)|(62<<5)|(30))
#define yoRGB_LOW_BITS_MASK ((1<<11)|(1<<5)|(1<<1))
#define yoRGB_HI_BITS_MASKx2 (((16<<11)|(32<<5)|(16))<<1)


#define COLOR_ADD(C1, C2) \
GFX.X2 [((((C1) & yoRGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & yoRGB_LOW_BITS_MASK)]


#define COLOR_ADD1_2(C1, C2) \
(((((C1) & yoRGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         ((C1) & (C2) & yoRGB_LOW_BITS_MASK) | ALPHA_BITS_MASK)

#define COLOR_SUB(C1, C2) \
GFX.ZERO_OR_X2 [(((C1) | yoRGB_HI_BITS_MASKx2) - \
		 ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1]
		 
#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | yoRGB_HI_BITS_MASKx2) - \
	   ((C2) & yoRGB_REMOVE_LOW_BITS_MASK)) >> 1]



__inline uint8 ConvertTile16New (uint8 *pCache,uint32 TileAddr,uint16 *ScreenColors)
{
#if 1
    register uint8 *tp = &Memory.VRAM[TileAddr];
    uint32 *p = (uint32*)tmpCache;
    uint32 non_zero,tile_opaque,tile_mono;
    uint8 line;
    uint32 p1;
    uint32 p2;
    register uint8 pix;
	    
    non_zero=0;
    switch (BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0;	    	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }	    	    
	    
	    *p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0;	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }	    
	    *p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    
	}
	break;

    case 2:    	
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0; 	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }	    	   
	    
		*p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    	
	}
	break;
	}		
	if (non_zero) 
	{	
		tile_opaque=1;
		uint8 *p3=tmpCache;
	    uint8 *q = pCache;
	    uint32 i;
		for (line = 8;line != 0; line--,q++,p3+=8)
		{
			if (!p3[0]) i=0x80;
			else i=0;
			if (!p3[1]) i|=0x40;
			if (!p3[2]) i|=0x20;
			if (!p3[3]) i|=0x10;
			if (!p3[4]) i|=0x08;
			if (!p3[5]) i|=0x04;
			if (!p3[6]) i|=0x02;
			if (!p3[7]) i|=0x01;					
			if (i) tile_opaque=0;
			*q=i;
		}
	    uint16 *r = (uint16*)(pCache)+4;
		for (line = 0;line <64;line++)
			r[line]=ScreenColors[tmpCache[line]];


		if (tile_opaque) return 2; //Tile is cached and opaque
		return 3; //Tile is cached and transp
	}
	else return 1; //Tile is totally transparent		
#else
	return 2;
#endif
}

#if 1
__inline uint8 ConvertTile8New (uint8 *pCache,uint32 TileAddr)
{
    register uint8 *tp = &Memory.VRAM[TileAddr];
    uint32 *p = (uint32*)pCache+((8+64)>>2);
    uint32 non_zero,tile_opaque,tile_mono;
    uint8 line;
    uint32 p1;
    uint32 p2;
    register uint8 pix;
	    
    non_zero=0;
    switch (BG.BitShift)
    {
    case 8:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0;	    	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 32)))
	    {
		p1 |= odd_high[2][pix >> 4];
		p2 |= odd_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 33)))
	    {
		p1 |= even_high[2][pix >> 4];
		p2 |= even_low[2][pix & 0xf];
	    }
	    if ((pix = *(tp + 48)))
	    {
		p1 |= odd_high[3][pix >> 4];
		p2 |= odd_low[3][pix & 0xf];
	    }
	    if ((pix = *(tp + 49)))
	    {
		p1 |= even_high[3][pix >> 4];
		p2 |= even_low[3][pix & 0xf];
	    }	    	    
	    
	    *p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    
	}
	break;

    case 4:
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0;	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 16)))
	    {
		p1 |= odd_high[1][pix >> 4];
		p2 |= odd_low[1][pix & 0xf];
	    }
	    if ((pix = *(tp + 17)))
	    {
		p1 |= even_high[1][pix >> 4];
		p2 |= even_low[1][pix & 0xf];
	    }	    
	    *p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    
	}
	break;

    case 2:    	
	for (line = 8; line != 0; line--, tp += 2)
	{
	    p1 = 0;
	    p2 = 0; 	    
	    if ((pix = *(tp + 0)))
	    {
		p1 |= odd_high[0][pix >> 4];
		p2 |= odd_low[0][pix & 0xf];
	    }
	    if ((pix = *(tp + 1)))
	    {
		p1 |= even_high[0][pix >> 4];
		p2 |= even_low[0][pix & 0xf];
	    }	    	   
	    
		*p++=p1;
		*p++=p2;
	    non_zero |= p1 | p2;	    	    	
	}
	break;
	}			
	if (non_zero) 
	{	
		tile_opaque=1;
		uint8 *p3=pCache+8+64;
	    uint8 *q = pCache;
	    uint32 i;
		for (line = 8;line != 0; line--,q++,p+=8)
		{
			/*if (!p3[0]) i=0x80;
			else i=0;
			if (!p3[1]) i|=0x40;
			if (!p3[2]) i|=0x20;
			if (!p3[3]) i|=0x10;
			if (!p3[4]) i|=0x08;
			if (!p3[5]) i|=0x04;
			if (!p3[6]) i|=0x02;
			if (!p3[7]) i|=0x01;*/
			
			if (p3[0]) i=0x01;
			else i=0;
			if (p3[1]) i|=0x02;
			if (p3[2]) i|=0x04;
			if (p3[3]) i|=0x08;
			if (p3[4]) i|=0x10;
			if (p3[5]) i|=0x20;
			if (p3[6]) i|=0x40;
			if (p3[7]) i|=0x80;
			
			//if (i==0) tile_opaque=0;
			*q=i;
		}

		//if (tile_opaque) return 2; //Tile is cached and opaque
		return 3; //Tile is cached and transp
	}		
	else return 1; //Tile is totally transparent		
}
#endif

/********************************/
__inline void NORMAL16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
#if 1
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;

   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen++; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)
   	    #undef FN
#endif
}

__inline void NORMAL16_O (uint32 Offset,uint16 *Pixels)
{
#if 1
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
		*Screen++=*Pixels++;
   	    *Screen++=*Pixels++;
		*Screen++=*Pixels++;
		*Screen++=*Pixels++;
		*Screen++=*Pixels++;
		*Screen++=*Pixels++;
		*Screen++=*Pixels++;
		*Screen++=*Pixels++;
#endif
}

__inline void FLIPPED16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;

   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen--; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;

    	*Screen--=*Pixels++;
   	    *Screen--=*Pixels++;
		*Screen--=*Pixels++;
		*Screen--=*Pixels++;
		*Screen--=*Pixels++;
		*Screen--=*Pixels++;
		*Screen--=*Pixels++;
		*Screen--=*Pixels++;
}



__inline void NORMAL16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; } \
   	    	Screen += 320; Pixels++; ZB += 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
	   	    Screen += 320; Pixels++; ZB += 320;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; }\
   	    	Screen -= 320; Pixels++; ZB -= 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)
   	    #undef FN
}

__inline void FLIPPED16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
   		    Screen -= 320; Pixels++; ZB -= 320;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}




__inline void NORMAL_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 320; SubScreen += 320; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 320; SubScreen += 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void FLIPPED_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 320; SubScreen -= 320; Pixels++;
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)
   	    #undef FN
}

__inline void FLIPPED_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 320; SubScreen -= 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void NORMAL_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

//////////////////// fooble
__inline void NORMAL_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 320; SubScreen += 320; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 320; SubScreen += 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void FLIPPED_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (7 *  320);
   	    SubScreen += (7 * 320);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 320; SubScreen -= 320; Pixels++;
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 320; SubScreen -= 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void NORMAL_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

__inline void NORMAL_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 320; SubScreen += 320; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 320; SubScreen += 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void FLIPPED_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 320; SubScreen -= 320; Pixels++;
   	    	
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 320; SubScreen -= 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void NORMAL_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

/////
__inline void NORMAL_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 320; SubScreen += 320; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)
   	    #undef FN
}

__inline void NORMAL_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 320; SubScreen += 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void FLIPPED_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 320; SubScreen -= 320; Pixels++;

   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (7 * 320);
   	    SubScreen += (7 * 320);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 320; SubScreen -= 320; Pixels++;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void NORMAL_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 320; SubScreen += 320; Pixels++; ZB += 320;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 320; SubScreen -= 320; Pixels++; ZB -= 320;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (7 * 320);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (7 * 320);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (7 * 320);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 320; SubScreen -= 320; Pixels++; ZB  -= 320;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

// Some of these are pointless, but what the hell... 

__inline void SqTrans_NORMAL16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
#if 1
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;

   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen+=640; Pixels+=2;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
#endif
}

__inline void SqTrans_NORMAL16_O (uint32 Offset,uint16 *Pixels)
{
#if 1
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
   	    *Screen=*Pixels;
   	    Screen+=640; Pixels+=2;
   	    *Screen=*Pixels;
   	    Screen+=640; Pixels+=2;
   	    *Screen=*Pixels;
   	    Screen+=640; Pixels+=2;
   	    *Screen=*Pixels;
#endif
}

__inline void SqTrans_FLIPPED16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
   	    Screen += 6 * 320;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen -= 640; Pixels+=2;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;
   	    
   	    Screen += 6 * 320;

	    	*Screen=*Pixels;
   	    Screen-=640; Pixels+=2;
   	    *Screen=*Pixels;
   	    Screen-=640; Pixels+=2;
   	    *Screen=*Pixels;
   	    Screen-=640; Pixels+=2;
   	    *Screen=*Pixels;
}



__inline void SqTrans_NORMAL16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; } \
   	    	Screen += 640; Pixels+=2; ZB += 640;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
	   	    Screen += 640; Pixels+=2; ZB += 640;
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void SqTrans_FLIPPED16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.S + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; }\
   	    	Screen -= 640; Pixels+=2; ZB -= 640;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
		uint16 *Screen = (uint16 *) GFX.S + Offset + (6 * 640);
		uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

		#define FN \
			if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
			Screen -= 640; Pixels+=2; ZB -= 640;
		FN
		FN
		FN
		FN
		#undef FN
}


__inline void SqTrans_NORMAL_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
		uint16 *Screen = (uint16 *) GFX.Screen + Offset;
		uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
		#define FN(N) \
			if (!(solid&(1<<(7-N)))) {\
				if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
				else *Screen=*Pixels;}\
			Screen += 640; SubScreen += 640; Pixels+=2;
		FN(0)
		FN(2)
		FN(4)
		FN(6)
		#undef FN
}

__inline void SqTrans_NORMAL_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
		uint16 *Screen = (uint16 *) GFX.Screen + Offset;
		uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
		#define FN \
			if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
			else *Screen=*Pixels; \
			Screen += 640; SubScreen += 640; Pixels+=2;
		FN
		FN
		FN
		FN
		#undef FN
}

__inline void SqTrans_FLIPPED_ADD_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
		uint16 *Screen = (uint16 *) GFX.Screen + Offset;
		uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2;
   	    	
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_ADD_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 640; SubScreen -= 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void SqTrans_NORMAL_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
	   	FN
	   	FN
	   	FN
		FN
		#undef FN
}

__inline void SqTrans_FLIPPED_ADD_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_ADD_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
		FN
   		FN
   		FN
   		FN
   		#undef FN
}

//////////////////// fooble
__inline void SqTrans_NORMAL_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 640; SubScreen += 640; Pixels+=2;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 640; SubScreen += 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
		FN
	   	#undef FN
}

__inline void SqTrans_FLIPPED_ADD1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (6 *  640);
   	    SubScreen += (6 * 640);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2;
   	    	
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_ADD1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 640; SubScreen -= 640; Pixels+=2;
	   	FN
	   	FN
		FN
	   	FN
	   	#undef FN
}



__inline void SqTrans_NORMAL_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void SqTrans_FLIPPED_ADD1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_ADD1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_ADD1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

__inline void SqTrans_NORMAL_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 640; SubScreen += 640; Pixels+=2;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 640; SubScreen += 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void SqTrans_FLIPPED_SUB_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2;
   	    	
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_SUB_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 640; SubScreen -= 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void SqTrans_NORMAL_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void SqTrans_FLIPPED_SUB_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_SUB_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}

/////
__inline void SqTrans_NORMAL_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen += 640; SubScreen += 640; Pixels+=2;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen += 640; SubScreen += 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
	   	FN   	    
	   	#undef FN
}

__inline void SqTrans_FLIPPED_SUB1_2_16_T (uint32 Offset,uint16 *Pixels,uint32 solid)
{   	    
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) {\
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen); \
   	    		else *Screen=*Pixels;}\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2;

   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_SUB1_2_16_O (uint32 Offset,uint16 *Pixels)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;    
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;       	       	    
   	    Screen += (6 * 640);
   	    SubScreen += (6 * 640);
   	    #define FN \
   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
	   	    else *Screen=*Pixels; \
	   	    Screen -= 640; SubScreen -= 640; Pixels+=2;
	   	FN
	   	FN
	   	FN
	   	FN
	   	#undef FN
}



__inline void SqTrans_NORMAL_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
   	    		*ZB=index_spr; \
   	    	} \
   	    	Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
   	    FN(0)
   	    FN(2)
   	    FN(4)
   	    FN(6)
   	    #undef FN
}

__inline void SqTrans_NORMAL_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset;
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
	   	    Screen += 640; SubScreen += 640; Pixels+=2; ZB += 640;
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void SqTrans_FLIPPED_SUB1_2_16_SPR_T (uint32 Offset,uint16 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { \
   	    		if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   		*ZB=index_spr; }\
   	    	Screen -= 640; SubScreen -= 640; Pixels+=2; ZB -= 640;
   	    FN(1)
   	    FN(3)
   	    FN(5)
   	    FN(7)
   	    #undef FN
}

__inline void SqTrans_FLIPPED_SUB1_2_16_SPR_O (uint32 Offset,uint16 *Pixels,uint32 index_spr)
{
   	    uint16 *Screen = (uint16 *) GFX.Screen + Offset + (6 * 640);
   	    uint16 *SubScreen = (uint16 *) GFX.SubScreen + Offset + (6 * 640);
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset + (6 * 640);

   	    #define FN \
	   	    if (*ZB>index_spr) {\
	   	    	if (*SubScreen) *Screen = COLOR_SUB1_2(*Pixels,*SubScreen);\
		   	    else *Screen=*Pixels;\
		   	    *ZB=index_spr;} \
   		    Screen -= 640; SubScreen -= 640; Pixels+=2; ZB  -= 640;
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}



/********************************/

#if 1
__inline void NORMAL8_T (uint32 Offset,uint8 *Pixels,uint32 solid)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;    
   	    
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen--; Pixels++;   	    	   	    
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}
__inline void NORMAL8_O (uint32 Offset,uint8 *Pixels)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;    
   	    
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;
   	    Screen--; Pixels++;
   	    *Screen=*Pixels;

}

__inline void FLIPPED8_T (uint32 Offset,uint8 *Pixels,uint32 solid)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;
   	    
   	    Screen -= 7;
   	    #define FN(N) \
   	    	if (!(solid&(1<<(7-N)))) *Screen = *Pixels; \
   	    	Screen++; Pixels++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED8_O (uint32 Offset,uint8 *Pixels)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;
   	    
   	    Screen -= 7;

    	*Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
   	    Screen++; Pixels++;
   	    *Screen=*Pixels;
}


__inline void NORMAL8_SPR_T (uint32 Offset,uint8 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;

   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; } \
   	    	Screen--; Pixels++; ZB--;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	       	    	
   	    #undef FN
}

__inline void NORMAL8_SPR_O (uint32 Offset,uint8 *Pixels,uint32 index_spr)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset;    
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
	   	    Screen--; Pixels++; ZB--;
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
	   	FN
		#undef FN
}

__inline void FLIPPED8_SPR_T (uint32 Offset,uint8 *Pixels,uint32 solid,uint32 index_spr)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN(N) \
   	    	if ((!(solid&(1<<(7-N))))&&(*ZB>index_spr)) { *Screen = *Pixels; *ZB=index_spr; }\
   	    	Screen++; Pixels++; ZB++;
   	    FN(0)
   	    FN(1)
   	    FN(2)
   	    FN(3)
   	    FN(4)
   	    FN(5)
   	    FN(6)
   	    FN(7)   	    
   	    #undef FN
}

__inline void FLIPPED8_SPR_O (uint32 Offset,uint8 *Pixels,uint32 index_spr)
{
   	    uint8 *Screen = (uint8 *) GFX.S + Offset - 7;
   	    uint8 *ZB = (uint8 *)GFX.ZBuffer + Offset - 7;
   	    
   	    #define FN \
	   	    if (*ZB>index_spr) {*Screen=*Pixels; *ZB=index_spr;} \
   		    Screen++; Pixels++; ZB++;
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		FN
   		#undef FN
}



void DrawHiResTile8New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;

    register uint32 l;    
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
/*		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];*/
        Col = 0;
    }
    else 
    {
/*    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];*/
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
   
    if (!BG.Buffered [TileNumber<<1]) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile8New (pCache, TileAddr);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	   	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
    }
    else
    if (BG.Buffered [(TileNumber<<1)|1]!=Col)
    {
    	//update cache
    	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
	   	BG.Buffered [(TileNumber<<1)|1]=Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;

			for (l = 4; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL8_O (Offset, (uint8*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL8_O (Offset, (uint8*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP

			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED8_O (Offset, (uint8*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		

			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED8_O (Offset, (uint8*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2, headerbp+=2, Offset+=1)
			   	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2, headerbp+=2, Offset-=1)
		    	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2, headerbp+=2, Offset-=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2, headerbp+=2, Offset+=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile8New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;

    register uint32 l;    
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
/*		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];*/
        Col = 0;
    }
    else 
    {
/*    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];*/
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    if (!BG.Buffered [TileNumber<<1])
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile8New (pCache, TileAddr);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	   	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
    }
    else
    if (BG.Buffered [(TileNumber<<1)|1]!=Col)
    {
    	//update cache
    	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
	   	BG.Buffered [(TileNumber<<1)|1]=Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*8;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL8_O (Offset, (uint8*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*8;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL8_O (Offset, (uint8*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*8;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED8_O (Offset, (uint8*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*8;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED8_O (Offset, (uint8*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*8;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2, headerbp+=2, Offset+=1)
			   	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*8;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2, headerbp+=2, Offset-=1)
		    	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*8;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2, headerbp+=2, Offset-=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*8;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2, headerbp+=2, Offset+=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);

		}
	}
}



void DrawTile8New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
/*		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];*/
        Col = 0;
    }
    else 
    {
/*    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];*/
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
   
    if (!BG.Buffered [TileNumber<<1]) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile8New (pCache, TileAddr);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	   	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
    }
    else
    if (BG.Buffered [(TileNumber<<1)|1]!=Col)
    {
    	//update cache
    	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
	   	BG.Buffered [(TileNumber<<1)|1]=Col;
    }
   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
#ifndef asmPPU8
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}

	if (solid_lineclip==0xFF) return;
#endif
	
	{
#ifndef asmPPU8
		Offset -= (StartLine * 320);; //align to tile multiple
#endif		
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset+=1)
			   	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),2);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset-=1)
		    	NORMAL8_T (Offset, (uint8*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),3);
#else			
			uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset-=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP

#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),1);
#else		
			uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset+=1)
		    	FLIPPED8_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile8New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    



	DrawTile8New (Tile, Offset, StartLine, LineCount);
	return;
	
	
}

void DrawTile8NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
/*		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];*/
        Col = 0;
    }
    else 
    {
/*    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];*/
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
   	if (!BG.Buffered [TileNumber<<1]) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile8New (pCache, TileAddr);    	
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	   	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
    }
    else
    if (BG.Buffered [(TileNumber<<1)|1]!=Col)
    {
    	//update cache
    	int i;
	   	uint8 *p=pCache+8;
	   	for (i=0;i<64;i++,p++) *p=p[64]|Col;
	   	BG.Buffered [(TileNumber<<1)|1]=Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
#ifndef asmPPU8
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}

	if (solid_lineclip==0xFF) return;
#endif	
	
	{
#ifndef asmPPU8
		Offset -= (StartLine * 320);; //align to tile multiple
#endif		  
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),0);
#else	
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset+=1)
			   	NORMAL8_SPR_T (Offset, (uint8*)bp,solid_lineclip|*headerbp,index_spr);
#endif
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),2);
#else		
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset-=1)
		    	NORMAL8_SPR_T (Offset, (uint8*)bp,solid_lineclip|*headerbp,index_spr);
#endif
		}
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),3);
#else		
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset-=1)
		    	FLIPPED8_SPR_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp,index_spr);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU8
			asmDrawTile8((uint32)pCache,StartLine|(LineCount<<16),(uint32)(GFX.S+(Offset)),1);
#else		
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8, headerbp++, Offset+=1)
		    	FLIPPED8_SPR_T (Offset, (uint8*)bp,solid_lineclipI|*headerbp,index_spr);
#endif
		}
	}
}

void DrawClippedTile8NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    DrawTile8NewSprite (Tile, Offset, StartLine, LineCount, index_spr);
}
#endif

/*********************************************************************/

void DrawTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
#if 1
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;

    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;

	bp = pCache+8;
         // do this one first!
	StartLine=0;
	Offset-=(StartLine*320);
			for (l = 0; l < 1; l++, bp += 8*2, Offset+=320)
			   	NORMAL16_O (Offset, (uint16*)bp);
				
	return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2))
	{

	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);
#else
			bp = pCache+8;
         // do this one first!
			for (l = 0; l < LineCount; l++, bp += 8*2, Offset+=320)
			   	NORMAL16_O (Offset, (uint16*)bp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);
#else			
			bp = pCache+8;	
			Offset += 7;
         // do this one when the first one works!
	    	for (l = 0; l < LineCount; l++, bp += 8*2, Offset+=320)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else
			bp = pCache+8;
			Offset += (LineCount * 320)+7;
	    	for (l = 0; l < LineCount; l++, bp += 8*2, Offset-=320)
		    	FLIPPED16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else
			bp = pCache+8;
			Offset += (LineCount * 320);
	    	for (l = 0; l < LineCount; l++, bp += 8*2, Offset-=320)
		    	NORMAL16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else
		    headerbp = pCache;
			bp = pCache+8;
         // do this one when the first one works!
			for (l = 0; l < LineCount; l++, bp += 8*2, headerbp++, Offset+=320)
			   	NORMAL16_T (Offset, (uint16*)bp,*headerbp);
#endif
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);
#else
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
			// do this one when the first one works!
			for (l = 0; l < LineCount; l++, bp += 8*2, headerbp++, Offset+=320)
		    	FLIPPED16_T (Offset, (uint16*)bp,*headerbp);
#endif
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else
		    headerbp = pCache;
			bp = pCache+8;
			Offset += (LineCount * 320)+7;
			for (l = 0; l < LineCount; l++, bp += 8*2, headerbp++, Offset-=320)
		    	FLIPPED16_T (Offset, (uint16*)bp,*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else
		    headerbp = pCache;
			bp = pCache+8;
			Offset += (LineCount * 320);
	    	for (l = 0; l < LineCount; l++, bp += 8*2, headerbp++, Offset-=320)
		    	NORMAL16_T (Offset, (uint16*)bp,*headerbp);
#endif
		}
	}
#endif
}

void DrawClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    DrawTile16New (Tile, Offset, StartLine, LineCount);
}

void DrawTile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col))
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}

	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP


			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP

			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP

			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;

         // do this one when the first one works!
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;

         // do this one when the first one works!
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** ADD ***********************/
void DrawTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);

    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16ADDNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16ADDNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_ADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_ADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_ADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_ADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_ADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** ADD1_2 ***********************/
void DrawTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_ADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_ADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_ADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_ADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}


/****************** SUB ***********************/
void DrawTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16SUBNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16SUBNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_SUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_SUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_SUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_SUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_SUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** SUB1_2 ***********************/
void DrawTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void DrawClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void DrawTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawClippedTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	NORMAL_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	NORMAL_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	FLIPPED_SUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void DrawHiResTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void DrawHiResClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	NORMAL_SUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	FLIPPED_SUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320); //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	NORMAL_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	FLIPPED_SUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_Tile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
#if 1
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;

    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;

	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320); //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);
#else
			bp = pCache+8;
         // do this one first!
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);
#else			
			bp = pCache+8;			
			Offset += 7;
         // do this one when the first one works!
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320); //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else
		    headerbp = pCache;
			bp = pCache+8;
         // do this one when the first one works!
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);
#else
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
         // do this one when the first one works!
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU16
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
#endif
}

void SqTrans_ClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;

    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320); //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	SqTrans_NORMAL16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
         // do this one when the first one works!
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU16
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void SqTrans_Tile16NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col))
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}

	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset++)
			   	SqTrans_NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP


			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset++)
		    	SqTrans_FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	SqTrans_NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	SqTrans_FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_ClippedTile16NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP

			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, Offset++)
			   	SqTrans_NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_NORMAL16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP

			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset--)
		    	SqTrans_FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset++)
		    	SqTrans_FLIPPED16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
			   	SqTrans_NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_NORMAL16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset--)
		    	SqTrans_FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset++)
		    	SqTrans_FLIPPED16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_HiResTile16New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;

         // do this one when the first one works!
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP

			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;

         // do this one when the first one works!
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
			//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_HiResClippedTile16New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMAL16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMAL16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPED16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMAL16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
			solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPED16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}


#if 0
/****************** ADD ***********************/
void SqTrans_Tile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
	uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);

    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
			//NO FLIP
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);
#endif
		}
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
			{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void SqTrans_ClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
	{
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
			for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void SqTrans_Tile16ADDNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
		else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_ClippedTile16ADDNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
	}

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
			//VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
			//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_HiResTile16ADDNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP

			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
				SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
				if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_HiResClippedTile16ADDNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
	uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALADD_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALADD_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

			}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDADD_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** ADD1_2 ***********************/
void SqTrans_Tile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
	uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
	switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
				SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void SqTrans_ClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
		else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
			//NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
				solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void SqTrans_Tile16ADD1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
			//VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_ClippedTile16ADD1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
	//Tile is not blank, 'have to draw it


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
			//NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
			{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_HiResTile16ADD1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
			//NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
				solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_HiResClippedTile16ADD1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
	uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALADD1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDADD1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}


/****************** SUB ***********************/
void SqTrans_Tile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
	uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


	register uint8 *bp,*headerbp;
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
		{
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
				if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void SqTrans_ClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
		Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);
#endif
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
		if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
			for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
				SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void SqTrans_Tile16SUBNewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
	}


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
		else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
		{
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_ClippedTile16SUBNewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

	if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
		if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
			solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_HiResTile16SUBNew (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
		if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   		

			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
			for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_HiResClippedTile16SUBNew (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
		{
		    //NO FLIP		  
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALSUB_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
				SqTrans_FLIPPEDSUB_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALSUB_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
				solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDSUB_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

/****************** SUB1_2 ***********************/
void SqTrans_Tile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }
    

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),0);			
#else
			bp = pCache+8;
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
			bp = pCache+8;			
			Offset += 7;			
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)+7*2),3);	   		
#else	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);
#endif
	    }
		else
	    {
		    //VFLIP
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,0,(uint32)(GFX.S+(Offset<<1)),1);
#else	   		
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);
#endif
		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    		    
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)),0);
#else			
		    headerbp = pCache;
			bp = pCache+8;					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    	    	
			asmDrawTile16((uint32)pCache,solid_lineclip,(uint32)(GFX.S+(Offset<<1)+7*2),2);			
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+7*2),3);
#else			
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
				solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    	   		
			asmDrawTile16((uint32)pCache,solid_lineclipI,(uint32)(GFX.S+(Offset<<1)),1);
#else			
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif
		}
	}
}

void SqTrans_ClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
		GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }   
    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else					  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
				SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);
#else			
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8),(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);
#else			
	   	
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);
#endif		    	
		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
#ifdef asmPPU		    			   	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),0);			
#else			
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
#endif			   	
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclip,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),2);			
#else			
	    	
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
#endif		    	
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
			solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+(StartPixel+Width-1)*2),3);			
#else
	   		
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
#ifdef asmPPU		    		    	
			asmDrawTileClipped16((uint32)pCache,(StartPixel<<16)|(Width<<8)|solid_lineclipI,(uint32)(GFX.S+(Offset<<1)+StartPixel*2),1);			
#else		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);
#endif		    	
		}
	}
}

void SqTrans_Tile16SUB1_2NewSprite (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
		BG.Buffered[(TileNumber<<1)|1] = Col;
    }


    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;

	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	    else
	    {
		    //VFLIP
	   		
		 
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
		if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 7;

	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 8; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_ClippedTile16SUB1_2NewSprite (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount, uint32 index_spr)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }

    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
   	{
   		BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
    	BG.Buffered[(TileNumber<<1)|1] = Col;
    }

    if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		  
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

	    }
	    else
	    {
		    //VFLIP

			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_O (Offset, (uint16*)bp,index_spr);

		}
	}
	else
	{

		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += StartPixel;				
			for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclip|*headerbp,index_spr);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
			uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel+Width-1);

	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += StartPixel;
	    	for (l = Width; l != 0; l--, bp += 8*2, headerbp++, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_SPR_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp,index_spr);

		}
	}
}

void SqTrans_HiResTile16SUB1_2New (uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    
    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

	if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;	        
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{

		Offset -= (StartLine * 320);; //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		  
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP

		 
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache;
			bp = pCache+8;
					
			for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
			solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
		   		solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;   			
	   		}
		    headerbp = pCache;
			bp = pCache+8;
			Offset += 3;

	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	  			solid_lineclipI<<=1;
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
		    headerbp = pCache;
			bp = pCache+8;
	    	for (l = 4; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}

void SqTrans_HiResClippedTile16SUB1_2New (uint32 Tile, uint32 Offset,
			uint32 StartPixel, uint32 Width,
			uint32 StartLine, uint32 LineCount)
{
    uint8 *pCache;
    uint32 Col;
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift);
    if ((Tile & 0x1ff) >= 256) TileAddr += BG.NameSelect;
    TileAddr &= 0xffff;
    
    register uint32 l;
    if (BG.DirectColourMode)
    {
        //Did the palette changed ?
		if (IPPU.DirectColourMapsNeedRebuild) S9xBuildDirectColourMaps ();
        GFX.ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask];
        Col = 0;
    }
    else 
    {
    	GFX.ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];
    	Col = (((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette;
    }
    
    uint32 TileNumber;
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) *(128+8)];
    

    
	if ((!BG.Buffered [TileNumber<<1])|(BG.Buffered [(TileNumber<<1)|1]!=Col)) 
    {
    	BG.Buffered[TileNumber<<1] = ConvertTile16New (pCache, TileAddr,GFX.ScreenColors);
	   	BG.Buffered[(TileNumber<<1)|1] = Col;
	}
    

	if (BG.Buffered [TileNumber<<1] == 1) //BLANK_TILE
		return;
    //Tile is not blank, 'have to draw it        


    register uint8 *bp,*headerbp;    	
    uint32 solid_lineclip;
    switch (StartLine)
	{
		case 0:solid_lineclip=0x00;break;
		case 1:solid_lineclip=0x80;break;
		case 2:solid_lineclip=0xC0;break;
		case 3:solid_lineclip=0xE0;break;
		case 4:solid_lineclip=0xF0;break;
		case 5:solid_lineclip=0xF8;break;
		case 6:solid_lineclip=0xFC;break;
		case 7:solid_lineclip=0xFE;break;
	}
	switch (StartLine+LineCount) //EndLine
	{
		case 1:solid_lineclip|=0x7F;break;
		case 2:solid_lineclip|=0x3F;break;
		case 3:solid_lineclip|=0x1F;break;
		case 4:	solid_lineclip|=0x0F;break;
		case 5:	solid_lineclip|=0x07;break;
		case 6:	solid_lineclip|=0x03;break;
		case 7:	solid_lineclip|=0x01;break;

	}
	
	if (solid_lineclip==0xFF) return;
	
	if ( (BG.Buffered [TileNumber<<1] == 2)&&(!solid_lineclip))
	{
		Offset -= (StartLine * 320);; //align to tile multiple    
		if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP		  
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_O (Offset, (uint16*)bp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);

	    }
	    else
	    {
		    //VFLIP
	   	
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_O (Offset, (uint16*)bp);

		}
	}
	else
	{

		Offset -= (StartLine * 320); //align to tile multiple
	    if (!(Tile & (V_FLIP | H_FLIP)))
	    {
		    //NO FLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8 + StartPixel*16;
			Offset += (StartPixel>>1);				
			for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
			   	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);						
	    }
	    else
	    if (!(Tile & V_FLIP))
	    {
	    	//HFLIP
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_NORMALSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclip|*headerbp);
	    }
	    else
	    if (Tile & H_FLIP)
	    {
	    	//VFLIP & HFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
			{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;

	   		}
		    headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += ((StartPixel+Width-1)>>1);

	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset-=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

	    }
	    else
	    {
		    //VFLIP
	   		uint8 solid_lineclipI;
	   		solid_lineclipI=0;
	   		for (int i=0;i<8;i++)
	   		{
	   			solid_lineclipI<<=1;   		
	   			if (solid_lineclip & (1<<i)) solid_lineclipI|=1;
	   		}
		    
			headerbp = pCache+StartPixel;
			bp = pCache+8+StartPixel*16;
			Offset += (StartPixel>>1);
	    	for (l = Width>>1; l != 0; l--, bp += 8*2*2, headerbp+=2, Offset+=1)
		    	SqTrans_FLIPPEDSUB1_2_16_T (Offset, (uint16*)bp,solid_lineclipI|*headerbp);

		}
	}
}
#endif
