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
#ifndef _TILE_H_
#define _TILE_H_

#define TILE_PREAMBLE \
    uint8 *pCache; \
\
    uint32 TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift); \
    if ((Tile & 0x1ff) >= 256) \
	TileAddr += BG.NameSelect; \
\
    TileAddr &= 0xffff; \
\
    uint32 TileNumber; \
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) << 6]; \
\
    if (!BG.Buffered [TileNumber]) \
	BG.Buffered[TileNumber] = ConvertTile (pCache, TileAddr); \
\
    if (BG.Buffered [TileNumber] == BLANK_TILE) \
	return; \
\
    register uint32 l; \
    if (BG.DirectColourMode) \
    { \
	if (IPPU.DirectColourMapsNeedRebuild) \
            S9xBuildDirectColourMaps (); \
        gfx->ScreenColors = DirectColourMaps [(Tile >> 10) & BG.PaletteMask]; \
    } \
    else \
	gfx->ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette];

#define RENDER_TILE(NORMAL, FLIPPED, N) \
    if (!(Tile & (V_FLIP | H_FLIP))) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    if (*(uint32 *) bp) \
		NORMAL (Offset, bp, gfx); \
	    if (*(uint32 *) (bp + 4)) \
		NORMAL (Offset + N, bp + 4, gfx); \
	} \
    } \
    else \
    if (!(Tile & V_FLIP)) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    if (*(uint32 *) (bp + 4)) \
		FLIPPED (Offset, bp + 4, gfx); \
	    if (*(uint32 *) bp) \
		FLIPPED (Offset + N, bp, gfx); \
	} \
    } \
    else \
    if (Tile & H_FLIP) \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    if (*(uint32 *) (bp + 4)) \
		FLIPPED (Offset, bp + 4, gfx); \
	    if (*(uint32 *) bp) \
		FLIPPED (Offset + N, bp, gfx); \
	} \
    } \
    else \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    if (*(uint32 *) bp) \
		NORMAL (Offset, bp, gfx); \
	    if (*(uint32 *) (bp + 4)) \
		NORMAL (Offset + N, bp + 4, gfx); \
	} \
    }

#define TILE_CLIP_PREAMBLE \
    uint32 dd; \
    uint32 d1; \
    uint32 d2; \
\
    if (StartPixel < 4) \
    { \
	d1 = HeadMask [StartPixel]; \
	if (StartPixel + Width < 4) \
	    d1 &= TailMask [StartPixel + Width]; \
    } \
    else \
	d1 = 0; \
\
    if (StartPixel + Width > 4) \
    { \
	if (StartPixel > 4) \
	    d2 = HeadMask [StartPixel - 4]; \
	else \
	    d2 = 0xffffffff; \
\
	d2 &= TailMask [(StartPixel + Width - 4)]; \
    } \
    else \
	d2 = 0;


#define RENDER_CLIPPED_TILE(NORMAL, FLIPPED, N) \
    if (!(Tile & (V_FLIP | H_FLIP))) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    if ((dd = (*(uint32 *) bp) & d1)) \
		NORMAL (Offset, (uint8 *) &dd, gfx); \
	    if ((dd = (*(uint32 *) (bp + 4)) & d2)) \
		NORMAL (Offset + N, (uint8 *) &dd, gfx); \
	} \
    } \
    else \
    if (!(Tile & V_FLIP)) \
    { \
	bp = pCache + StartLine; \
	SWAP_DWORD (d1); \
	SWAP_DWORD (d2); \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    if ((dd = *(uint32 *) (bp + 4) & d1)) \
		FLIPPED (Offset, (uint8 *) &dd, gfx); \
	    if ((dd = *(uint32 *) bp & d2)) \
		FLIPPED (Offset + N, (uint8 *) &dd, gfx); \
	} \
    } \
    else \
    if (Tile & H_FLIP) \
    { \
	bp = pCache + 56 - StartLine; \
	SWAP_DWORD (d1); \
	SWAP_DWORD (d2); \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    if ((dd = *(uint32 *) (bp + 4) & d1)) \
		FLIPPED (Offset, (uint8 *) &dd, gfx); \
	    if ((dd = *(uint32 *) bp & d2)) \
		FLIPPED (Offset + N, (uint8 *) &dd, gfx); \
	} \
    } \
    else \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    if ((dd = (*(uint32 *) bp) & d1)) \
		NORMAL (Offset, (uint8 *) &dd, gfx); \
	    if ((dd = (*(uint32 *) (bp + 4)) & d2)) \
		NORMAL (Offset + N, (uint8 *) &dd, gfx); \
	} \
    }

#define RENDER_TILE_LARGE(PIXEL, FUNCTION) \
    if (!(Tile & (V_FLIP | H_FLIP))) \
    { \
	if ((pixel = *(pCache + StartLine + StartPixel))) \
	{ \
	    pixel = PIXEL; \
	    for (l = LineCount; l != 0; l--, sp += gfx->PPL, Depth += gfx->PPL) \
	    { \
		for (int z = Pixels - 1; z >= 0; z--) \
		    if (gfx->Z1 > Depth [z]) \
		    { \
			sp [z] = FUNCTION(sp + z, pixel); \
			Depth [z] = gfx->Z2; \
		    }\
	    } \
	} \
    } \
    else \
    if (!(Tile & V_FLIP)) \
    { \
	StartPixel = 7 - StartPixel; \
	if ((pixel = *(pCache + StartLine + StartPixel))) \
	{ \
	    pixel = PIXEL; \
	    for (l = LineCount; l != 0; l--, sp += gfx->PPL, Depth += gfx->PPL) \
	    { \
		for (int z = Pixels - 1; z >= 0; z--) \
		    if (gfx->Z1 > Depth [z]) \
		    { \
			sp [z] = FUNCTION(sp + z, pixel); \
			Depth [z] = gfx->Z2; \
		    }\
	    } \
	} \
    } \
    else \
    if (Tile & H_FLIP) \
    { \
	StartPixel = 7 - StartPixel; \
	if ((pixel = *(pCache + 56 - StartLine + StartPixel))) \
	{ \
	    pixel = PIXEL; \
	    for (l = LineCount; l != 0; l--, sp += gfx->PPL, Depth += gfx->PPL) \
	    { \
		for (int z = Pixels - 1; z >= 0; z--) \
		    if (gfx->Z1 > Depth [z]) \
		    { \
			sp [z] = FUNCTION(sp + z, pixel); \
			Depth [z] = gfx->Z2; \
		    }\
	    } \
	} \
    } \
    else \
    { \
	if ((pixel = *(pCache + 56 - StartLine + StartPixel))) \
	{ \
	    pixel = PIXEL; \
	    for (l = LineCount; l != 0; l--, sp += gfx->PPL, Depth += gfx->PPL) \
	    { \
		for (int z = Pixels - 1; z >= 0; z--) \
		    if (gfx->Z1 > Depth [z]) \
		    { \
			sp [z] = FUNCTION(sp + z, pixel); \
			Depth [z] = gfx->Z2; \
		    }\
	    } \
	} \
    }
	
#define RENDER_TILEHI(NORMAL, FLIPPED, N) \
    if (!(Tile & (V_FLIP | H_FLIP))) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    /*if (*(uint32 *) bp)*/if (((uint32)bp[0])|((uint32)bp[2])|((uint32)bp[4])|((uint32)bp[6])) \
		NORMAL (Offset, bp); \
	} \
    } \
    else \
    if (!(Tile & V_FLIP)) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    /*if (*(uint32 *) (bp + 4))*/if (((uint32)bp[0])|((uint32)bp[2])|((uint32)bp[4])|((uint32)bp[6])) \
		FLIPPED (Offset, bp); \
	} \
    } \
    else \
    if (Tile & H_FLIP) \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    /*if (*(uint32 *) (bp + 4))*/if (((uint32)bp[0])|((uint32)bp[2])|((uint32)bp[4])|((uint32)bp[6]))  \
		FLIPPED (Offset, bp); \
	} \
    } \
    else \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    /*if (*(uint32 *) bp)*/if (((uint32)bp[0])|((uint32)bp[2])|((uint32)bp[4])|((uint32)bp[6])) \
		NORMAL (Offset, bp); \
	} \
    }
    
    
    
#define RENDER_CLIPPED_TILEHI(NORMAL, FLIPPED, N) \
	d1=(d1&0xFF)|((d1&0xFF0000)>>8)|((d2&0xFF)<<16)|((d2&0xFF0000)<<8);\
    if (!(Tile & (V_FLIP | H_FLIP))) \
    { \
	bp = pCache + StartLine; \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    /*if ((dd = (*(uint32 *) bp) & d1))*/if (dd = (((((uint32)bp[6])<<24)|(((uint32)bp[4])<<16)|(((uint32)bp[2])<<8)|((uint32)bp[0]))&d1)) \
		NORMAL (Offset, (uint8 *) &dd); \
	} \
    } \
    else \
    if (!(Tile & V_FLIP)) \
    { \
	bp = pCache + StartLine; \
	SWAP_DWORD (d1); \
	/*SWAP_DWORD (d2);*/ \
	for (l = LineCount; l != 0; l--, bp += 8, Offset += gfx->PPL) \
	{ \
	    /*if ((dd = *(uint32 *) (bp + 4) & d1))*/if (dd = (((((uint32)bp[6])<<24)|(((uint32)bp[4])<<16)|(((uint32)bp[2])<<8)|((uint32)bp[0]))&d1)) \
		FLIPPED (Offset, (uint8 *) &dd); \
	} \
    } \
    else \
    if (Tile & H_FLIP) \
    { \
	bp = pCache + 56 - StartLine; \
	SWAP_DWORD (d1); \
	/*SWAP_DWORD (d2);*/ \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    /*if ((dd = *(uint32 *) (bp + 4) & d1))*/if (dd = (((((uint32)bp[6])<<24)|(((uint32)bp[4])<<16)|(((uint32)bp[2])<<8)|((uint32)bp[0]))&d1)) \
		FLIPPED (Offset, (uint8 *) &dd); \
	} \
    } \
    else \
    { \
	bp = pCache + 56 - StartLine; \
	for (l = LineCount; l != 0; l--, bp -= 8, Offset += gfx->PPL) \
	{ \
	    /*if ((dd = (*(uint32 *) bp) & d1))*/ if (dd = (((((uint32)bp[6])<<24)|(((uint32)bp[4])<<16)|(((uint32)bp[2])<<8)|((uint32)bp[0]))&d1)) \
		NORMAL (Offset, (uint8 *) &dd); \
	} \
    }    

#endif
