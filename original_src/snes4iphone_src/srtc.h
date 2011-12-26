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
#ifndef _srtc_h_
#define _srtc_h_

#if !defined(_SNESPPC) && !defined(__GIZ__) && !defined(__GP2X__) && !defined(__IPHONE__)
#include <time.h>
#endif

#ifdef __IPHONE__
#include <sys/types.h>
#endif

#define MAX_RTC_INDEX       0xC

#define MODE_READ           0
#define MODE_LOAD_RTC       1
#define MODE_COMMAND        2
#define MODE_COMMAND_DONE   3

#define COMMAND_LOAD_RTC    0
#define COMMAND_CLEAR_RTC   4


/***   The format of the rtc_data structure is:

Index Description     Range (nibble)
----- --------------  ---------------------------------------

  0   Seconds low     0-9
  1   Seconds high    0-5

  2   Minutes low     0-9
  3   Minutes high    0-5

  4   Hour low        0-9
  5   Hour high       0-2

  6   Day low         0-9
  7   Day high        0-3

  8   Month           1-C (0xC is December, 12th month)

  9   Year ones       0-9
  A   Year tens       0-9
  B   Year High       9-B  (9=19xx, A=20xx, B=21xx)

  C   Day of week     0-6  (0=Sunday, 1=Monday,...,6=Saturday)

***/

typedef struct
{
    bool8_32 needs_init;
    bool8_32 count_enable;	// Does RTC mark time or is it frozen
    uint8 data [MAX_RTC_INDEX+1];
    int8  index;
    uint8 mode;

    time_t system_timestamp;	// Of latest RTC load time
    uint32 pad;
} SRTC_DATA;

extern SRTC_DATA           rtc;

void    S9xUpdateSrtcTime ();
void	S9xSetSRTC (uint8 data, uint16 Address);
uint8	S9xGetSRTC (uint16 Address);
void	S9xSRTCPreSaveState ();
void	S9xSRTCPostLoadState ();
void	S9xResetSRTC ();
void	S9xHardResetSRTC ();

#define SRTC_SRAM_PAD (4 + 8 + 1 + MAX_RTC_INDEX)

#endif	// _srtc_h
