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
#ifndef _CPUEXEC_H_
#define _CPUEXEC_H_
#include "ppu.h"
#include "memmap.h"
#include "65c816.h"

#define DO_HBLANK_CHECK() \
    if (CPU.Cycles >= CPU.NextEvent) \
	S9xDoHBlankProcessing ();

struct SOpcodes {
#ifdef __WIN32__
	void (__cdecl *S9xOpcode)( void);
#else
	void (*S9xOpcode)( void);
#endif
};

struct SICPU
{
/*
    uint8  *Speed;
    struct SOpcodes *S9xOpcodes;
    uint8  _Carry;
    uint8  _Zero;
    uint8  _Negative;
    uint8  _Overflow;
*/
	bool8  CPUExecuting;
    uint32 ShiftedPB;
    uint32 ShiftedDB;
    uint32 Frame;
    uint32 Scanline;
    uint32 FrameAdvanceCount;
};

START_EXTERN_C
void S9xMainLoop (void);
void S9xReset (void);
void S9xDoHBlankProcessing ();
void S9xClearIRQ (uint32);
void S9xSetIRQ (uint32);

extern struct SOpcodes S9xOpcodesM1X1 [256];
extern struct SOpcodes S9xOpcodesM1X0 [256];
extern struct SOpcodes S9xOpcodesM0X1 [256];
extern struct SOpcodes S9xOpcodesM0X0 [256];

#ifndef VAR_CYCLES
extern uint8 S9xE1M1X1 [256];
extern uint8 S9xE0M1X0 [256];
extern uint8 S9xE0M1X1 [256];
extern uint8 S9xE0M0X0 [256];
extern uint8 S9xE0M0X1 [256];
#endif

extern struct SICPU ICPU;
END_EXTERN_C

STATIC inline void CLEAR_IRQ_SOURCE (uint32 M)
{
    CPU.IRQActive &= ~M;
    if (!CPU.IRQActive)
	CPU.Flags &= ~IRQ_PENDING_FLAG;
}

#if 0
STATIC inline void S9xUnpackStatus()
{
    ICPU._Zero = (Registers.PL & Zero) == 0;
    ICPU._Negative = (Registers.PL & Negative);
    ICPU._Carry = (Registers.PL & Carry);
    ICPU._Overflow = (Registers.PL & Overflow) >> 6;
}

STATIC inline void S9xPackStatus()
{
    Registers.PL &= ~(Zero | Negative | Carry | Overflow);
    Registers.PL |= ICPU._Carry | ((ICPU._Zero == 0) << 1) |
		    (ICPU._Negative & 0x80) | (ICPU._Overflow << 6);
}

STATIC inline void S9xFixCycles ()
{
    if (CheckEmulation ())
    {
#ifndef VAR_CYCLES
	ICPU.Speed = S9xE1M1X1;
#endif
	ICPU.S9xOpcodes = S9xOpcodesM1X1;
    }
    else
    if (CheckMemory ())
    {
	if (CheckIndex ())
	{
#ifndef VAR_CYCLES
	    ICPU.Speed = S9xE0M1X1;
#endif
	    ICPU.S9xOpcodes = S9xOpcodesM1X1;
	}
	else
	{
#ifndef VAR_CYCLES
	    ICPU.Speed = S9xE0M1X0;
#endif
	    ICPU.S9xOpcodes = S9xOpcodesM1X0;
	}
    }
    else
    {
	if (CheckIndex ())
	{
#ifndef VAR_CYCLES
	    ICPU.Speed = S9xE0M0X1;
#endif
	    ICPU.S9xOpcodes = S9xOpcodesM0X1;
	}
	else
	{
#ifndef VAR_CYCLES
	    ICPU.Speed = S9xE0M0X0;
#endif
	    ICPU.S9xOpcodes = S9xOpcodesM0X0;
	}
    }
}
#endif

STATIC inline void S9xReschedule ()
{
    uint8 which;
    long max;
    
    if (CPU.WhichEvent == HBLANK_START_EVENT ||
	CPU.WhichEvent == HTIMER_AFTER_EVENT)
    {
	which = HBLANK_END_EVENT;
	max = Settings.H_Max;
    }
    else
    {
	which = HBLANK_START_EVENT;
	max = Settings.HBlankStart;
    }

    if (PPU.HTimerEnabled &&
        (long) PPU.HTimerPosition < max &&
	(long) PPU.HTimerPosition > CPU.NextEvent &&
	(!PPU.VTimerEnabled ||
	 (PPU.VTimerEnabled && CPU.V_Counter == PPU.IRQVBeamPos)))
    {
	which = (long) PPU.HTimerPosition < Settings.HBlankStart ?
			HTIMER_BEFORE_EVENT : HTIMER_AFTER_EVENT;
	max = PPU.HTimerPosition;
    }
    CPU.NextEvent = max;
    CPU.WhichEvent = which;
}

#endif
