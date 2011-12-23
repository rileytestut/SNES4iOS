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
#ifndef _CPUADDR_H_
#define _CPUADDR_H_

//EXTERN_C long OpAddress;

STATIC INLINE long FASTCALL Immediate8 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = icpu->ShiftedPB + cpu->PC - cpu->PCBase;
    cpu->PC++;
	return OpAddress;
}

STATIC INLINE long FASTCALL Immediate16 (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = icpu->ShiftedPB + cpu->PC - cpu->PCBase;
    cpu->PC += 2;
	return OpAddress;
}

STATIC INLINE long FASTCALL Relative (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    int8 Int8 = *cpu->PC++;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif    
	return ((int) (cpu->PC - cpu->PCBase) + Int8) & 0xffff;
}

STATIC INLINE long FASTCALL RelativeLong (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) cpu->PC;
#else
    long OpAddress = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + ONE_CYCLE;
#endif
    cpu->PC += 2;
    OpAddress += (cpu->PC - cpu->PCBase);
    OpAddress &= 0xffff;
	return OpAddress;
}

STATIC INLINE long FASTCALL AbsoluteIndexedIndirect (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (reg->X.W + *(uint16 *) cpu->PC) & 0xffff;
#else
    long OpAddress = (reg->X.W + *cpu->PC + (*(cpu->PC + 1) << 8)) & 0xffff;
#endif
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    cpu->PC += 2;
    return S9xGetWord (icpu->ShiftedPB + OpAddress, cpu);
}

STATIC INLINE long FASTCALL AbsoluteIndirectLong (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) cpu->PC;
#else
    long OpAddress = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    cpu->PC += 2;
    return S9xGetWord (OpAddress, cpu) | (S9xGetByte (OpAddress + 2, cpu) << 16);
}

STATIC INLINE long FASTCALL AbsoluteIndirect (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) cpu->PC;
#else
    long OpAddress = *cpu->PC + (*(cpu->PC + 1) << 8);
#endif

#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    cpu->PC += 2;
    return S9xGetWord (OpAddress, cpu) + icpu->ShiftedPB;
}

STATIC INLINE long FASTCALL Absolute (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = *(uint16 *) cpu->PC + icpu->ShiftedDB;
#else
    long OpAddress = *cpu->PC + (*(cpu->PC + 1) << 8) + icpu->ShiftedDB;
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL AbsoluteLong (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (*(uint32 *) cpu->PC) & 0xffffff;
#else
    long OpAddress = *cpu->PC + (*(cpu->PC + 1) << 8) + (*(cpu->PC + 2) << 16);
#endif
    cpu->PC += 3;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + cpu->MemSpeed;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL Direct(struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
//    if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndirectIndexed (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif

    OpAddress = icpu->ShiftedDB + S9xGetWord (OpAddress, cpu) + reg->Y.W;

//    if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
    // XXX: always add one if STA
    // XXX: else Add one cycle if crosses page boundary
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndirectIndexedLong (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif

    OpAddress = S9xGetWord (OpAddress, cpu) + (S9xGetByte (OpAddress + 2, cpu) << 16) +
		reg->Y.W;
//    if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndexedIndirect(struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W + reg->X.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif

    OpAddress = S9xGetWord (OpAddress, cpu) + icpu->ShiftedDB;

#ifdef VAR_CYCLES
//    if (reg->DL != 0)
//	cpu->Cycles += TWO_CYCLES;
//    else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndexedX (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W + reg->X.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif

#ifdef VAR_CYCLES
//    if (reg->DL != 0)
//	cpu->Cycles += TWO_CYCLES;
//    else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndexedY (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W + reg->Y.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif

#ifdef VAR_CYCLES
//    if (reg->DL != 0)
//	cpu->Cycles += TWO_CYCLES;
//    else
	cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL AbsoluteIndexedX (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = icpu->ShiftedDB + *(uint16 *) cpu->PC + reg->X.W;
#else
    long OpAddress = icpu->ShiftedDB + *cpu->PC + (*(cpu->PC + 1) << 8) +
		reg->X.W;
#endif
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    // XXX: always add one cycle for ROL, LSR, etc
    // XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC INLINE long FASTCALL AbsoluteIndexedY (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = icpu->ShiftedDB + *(uint16 *) cpu->PC + reg->Y.W;
#else
    long OpAddress = icpu->ShiftedDB + *cpu->PC + (*(cpu->PC + 1) << 8) +
		reg->Y.W;
#endif    
    cpu->PC += 2;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2;
#endif
    // XXX: always add cycle for STA
    // XXX: else is cross page boundary add one cycle
	return OpAddress;
}

STATIC INLINE long FASTCALL AbsoluteLongIndexedX (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
#ifdef FAST_LSB_WORD_ACCESS
    long OpAddress = (*(uint32 *) cpu->PC + reg->X.W) & 0xffffff;
#else
    long OpAddress = (*cpu->PC + (*(cpu->PC + 1) << 8) + (*(cpu->PC + 2) << 16) + reg->X.W) & 0xffffff;
#endif
    cpu->PC += 3;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeedx2 + cpu->MemSpeed;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndirect (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    OpAddress = S9xGetWord (OpAddress, cpu) + icpu->ShiftedDB;

//    if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC INLINE long FASTCALL DirectIndirectLong (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->D.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
#endif
    OpAddress = S9xGetWord (OpAddress, cpu) +
		(S9xGetByte (OpAddress + 2, cpu) << 16);
//    if (reg->DL != 0) cpu->Cycles += ONE_CYCLE;
	return OpAddress;
}

STATIC INLINE long FASTCALL StackRelative (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->S.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
    cpu->Cycles += ONE_CYCLE;
#endif
	return OpAddress;
}

STATIC INLINE long FASTCALL StackRelativeIndirectIndexed (struct SRegisters * reg, struct SICPU * icpu, struct SCPUState * cpu)
{
    long OpAddress = (*cpu->PC++ + reg->S.W) & 0xffff;
#ifdef VAR_CYCLES
    cpu->Cycles += cpu->MemSpeed;
    cpu->Cycles += TWO_CYCLES;
#endif
    OpAddress = (S9xGetWord (OpAddress, cpu) + icpu->ShiftedDB +
		 reg->Y.W) & 0xffffff;
	return OpAddress;
}
#endif
