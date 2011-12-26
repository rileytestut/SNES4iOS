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
#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"

#include "sa1.h"
#define CPU SA1
#define ICPU SA1

#undef Registers

#define Registers SA1Registers
#define S9xGetByte S9xSA1GetByte
#define S9xGetWord S9xSA1GetWord
#define S9xSetByte S9xSA1SetByte
#define S9xSetWord S9xSA1SetWord
#define S9xSetPCBase S9xSA1SetPCBase
#define S9xOpcodesM1X1 S9xSA1OpcodesM1X1
#define S9xOpcodesM1X0 S9xSA1OpcodesM1X0
#define S9xOpcodesM0X1 S9xSA1OpcodesM0X1
#define S9xOpcodesM0X0 S9xSA1OpcodesM0X0
#define S9xOpcode_IRQ S9xSA1Opcode_IRQ
#define S9xOpcode_NMI S9xSA1Opcode_NMI
#define S9xUnpackStatus S9xSA1UnpackStatus
#define S9xPackStatus S9xSA1PackStatus
#define S9xFixCycles S9xSA1FixCycles
//CSNES Not even used
/*
#define Immediate8 SA1Immediate8
#define Immediate16 SA1Immediate16
#define Relative SA1Relative
#define RelativeLong SA1RelativeLong
#define AbsoluteIndexedIndirect SA1AbsoluteIndexedIndirect
#define AbsoluteIndirectLong SA1AbsoluteIndirectLong
#define AbsoluteIndirect SA1AbsoluteIndirect
#define Absolute SA1Absolute
#define AbsoluteLong SA1AbsoluteLong
#define Direct SA1Direct
#define DirectIndirectIndexed SA1DirectIndirectIndexed
#define DirectIndirectIndexedLong SA1DirectIndirectIndexedLong
#define DirectIndexedIndirect SA1DirectIndexedIndirect
#define DirectIndexedX SA1DirectIndexedX
#define DirectIndexedY SA1DirectIndexedY
#define AbsoluteIndexedX SA1AbsoluteIndexedX
#define AbsoluteIndexedY SA1AbsoluteIndexedY
#define AbsoluteLongIndexedX SA1AbsoluteLongIndexedX
#define DirectIndirect SA1DirectIndirect
#define DirectIndirectLong SA1DirectIndirectLong
#define StackRelative SA1StackRelative
#define StackRelativeIndirectIndexed SA1StackRelativeIndirectIndexed
*/

//#undef CPU_SHUTDOWN
#undef VAR_CYCLES
#define SA1_OPCODES

#include "cpuops.cpp"

void S9xSA1MainLoop ()
{
    int i;

#if 0
    if (SA1.Flags & NMI_FLAG)
    {
		SA1.Flags &= ~NMI_FLAG;
		if (SA1.WaitingForInterrupt)
		{
			SA1.WaitingForInterrupt = FALSE;
			SA1.PC++;
		}
		S9xSA1Opcode_NMI ();
    }
#endif
    if (SA1.Flags & IRQ_PENDING_FLAG)
    {
		if (SA1.IRQActive)
		{
			if (SA1.WaitingForInterrupt)
			{
				SA1.WaitingForInterrupt = FALSE;
				SA1.PC++;
			}
			if (!SA1CheckFlag (IRQ))
				S9xSA1Opcode_IRQ ();
		}
		else
			SA1.Flags &= ~IRQ_PENDING_FLAG;
    }
#ifdef DEBUGGER
    if (SA1.Flags & TRACE_FLAG)
    {
		for (i = 0; i < 3 && SA1.Executing; i++)
		{
			S9xSA1Trace ();
#ifdef CPU_SHUTDOWN
		    SA1.PCAtOpcodeStart = SA1.PC;
#endif
	    (*SA1.S9xOpcodes [*SA1.PC++].S9xOpcode) ();
		}
    }
    else
#endif
	    for (i = 0; i < 3 && SA1.Executing; i++)
		{
#ifdef CPU_SHUTDOWN
			SA1.PCAtOpcodeStart = SA1.PC;
#endif
	(*SA1.S9xOpcodes [*SA1.PC++].S9xOpcode) ();
		}
}
