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
#ifndef _SPC700_H_
#define _SPC700_H_

#ifdef SPCTOOL
#define NO_CHANNEL_STRUCT
#include "spctool/dsp.h"
#include "spctool/spc700.h"
#include "spctool/soundmod.h"
#endif


#define Carry       1
#define Zero        2
#define Interrupt   4
#define HalfCarry   8
#define BreakFlag  16
#define DirectPageFlag 32
#define Overflow   64
#define Negative  128

#define APUClearCarry() (IAPU._Carry = 0)
#define APUSetCarry() (IAPU._Carry = 1)
#define APUSetInterrupt() (IAPU.P |= Interrupt)
#define APUClearInterrupt() (IAPU.P &= ~Interrupt)
#define APUSetHalfCarry() (IAPU.P |= HalfCarry)
#define APUClearHalfCarry() (IAPU.P &= ~HalfCarry)
#define APUSetBreak() (IAPU.P |= BreakFlag)
#define APUClearBreak() (IAPU.P &= ~BreakFlag)
#define APUSetDirectPage() (IAPU.P |= DirectPageFlag)
#define APUClearDirectPage() (IAPU.P &= ~DirectPageFlag)
#define APUSetOverflow() (IAPU._Overflow = 1)
#define APUClearOverflow() (IAPU._Overflow = 0)

#define APUCheckZero() (IAPU._Zero == 0)
#define APUCheckCarry() (IAPU._Carry)
#define APUCheckInterrupt() (IAPU.P & Interrupt)
#define APUCheckHalfCarry() (IAPU.P & HalfCarry)
#define APUCheckBreak() (IAPU.P & BreakFlag)
#define APUCheckDirectPage() (IAPU.P & DirectPageFlag)
#define APUCheckOverflow() (IAPU._Overflow)
#define APUCheckNegative() (IAPU._Zero & 0x80)

//#define APUClearFlags(f) (IAPU.P &= ~(f))
//#define APUSetFlags(f)   (IAPU.P |=  (f))
//#define APUCheckFlag(f)  (IAPU.P &   (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 A, Y; } B;
#else
    struct { uint8 Y, A; } B;
#endif
    uint16 W;
	uint32 _padder; // make sure this whole thing takes 4 bytes
} YAndA;

struct SAPURegisters{
    uint8  P;
    YAndA YA;
    uint8  X;
    uint8  S;
    uint16  PC;
};

//EXTERN_C struct SAPURegisters APURegisters;

// Needed by ILLUSION OF GAIA
//#define ONE_APU_CYCLE 14
#define ONE_APU_CYCLE 21

// Needed by all games written by the software company called Human
//#define ONE_APU_CYCLE_HUMAN 17
#define ONE_APU_CYCLE_HUMAN 21

// 1.953us := 1.024065.54MHz

#ifdef SPCTOOL
EXTERN_C int32 ESPC (int32);

#define APU_EXECUTE() \
{ \
    int32 l = (CPU.Cycles - CPU.APU_Cycles) / 14; \
    if (l > 0) \
    { \
        l -= _EmuSPC(l); \
        CPU.APU_Cycles += l * 14; \
    } \
}

#else

#ifdef ASM_SPC700

// return cycles left (always negative)
extern "C" int spc700_execute(int cycles);

#define APU_EXECUTE1() \
{ \
	CPU.APU_Cycles -= spc700_execute(0); \
}

// notaz: CPU.APU_APUExecuting: 0 == disabled, 1 == enabled normal, 3 == enabled in hack mode

#define APU_EXECUTE(mode) \
if (CPU.APU_APUExecuting == mode) \
{\
	if(CPU.APU_Cycles <= CPU.Cycles) { \
		int cycles = CPU.Cycles - CPU.APU_Cycles; \
		CPU.APU_Cycles += cycles - spc700_execute(cycles); \
	} \
}

#else

#define APU_EXECUTE1() \
{ \
    CPU.APU_Cycles += S9xAPUCycles [*IAPU.PC]; \
    (*S9xApuOpcodes[*IAPU.PC]) (); \
}

#define APU_EXECUTE(x) \
if (CPU.APU_APUExecuting) \
{\
    while (CPU.APU_Cycles <= CPU.Cycles) \
	APU_EXECUTE1(); \
}

#endif // ASM_SPC700

#endif // SPCTOOL

#endif
