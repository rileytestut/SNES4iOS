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
#ifndef _65c816_h_
#define _65c816_h_

#define AL A.B.l
#define AH A.B.h
#define XL X.B.l
#define XH X.B.h
#define YL Y.B.l
#define YH Y.B.h
#define SL S.B.l
#define SH S.B.h
#define DL D.B.l
#define DH D.B.h
#define PL P.B.l
#define PH P.B.h

#define Carry       1
#define Zero        2
#define IRQ         4
#define Decimal     8
#define IndexFlag  16
#define MemoryFlag 32
#define Overflow   64
#define Negative  128
#define Emulation 256

#define ClearCarry() (ICPU._Carry = 0)
#define SetCarry() (ICPU._Carry = 1)
#define SetZero() (ICPU._Zero = 0)
#define ClearZero() (ICPU._Zero = 1)
#define SetIRQ() (Registers.PL |= IRQ)
#define ClearIRQ() (Registers.PL &= ~IRQ)
#define SetDecimal() (Registers.PL |= Decimal)
#define ClearDecimal() (Registers.PL &= ~Decimal)
#define SetIndex() (Registers.PL |= IndexFlag)
#define ClearIndex() (Registers.PL &= ~IndexFlag)
#define SetMemory() (Registers.PL |= MemoryFlag)
#define ClearMemory() (Registers.PL &= ~MemoryFlag)
#define SetOverflow() (ICPU._Overflow = 1)
#define ClearOverflow() (ICPU._Overflow = 0)
#define SetNegative() (ICPU._Negative = 0x80)
#define ClearNegative() (ICPU._Negative = 0)

#define CheckZero() (ICPU._Zero == 0)
#define CheckCarry() (ICPU._Carry)
#define CheckIRQ() (Registers.PL & IRQ)
#define CheckDecimal() (Registers.PL & Decimal)
#define CheckIndex() (Registers.PL & IndexFlag)
#define CheckMemory() (Registers.PL & MemoryFlag)
#define CheckOverflow() (ICPU._Overflow)
#define CheckNegative() (ICPU._Negative & 0x80)
#define CheckEmulation() (Registers.P.W & Emulation)

#define ClearFlags(f) (Registers.P.W &= ~(f))
#define SetFlags(f)   (Registers.P.W |=  (f))
#define CheckFlag(f)  (Registers.PL & (f))

typedef union
{
#ifdef LSB_FIRST
    struct { uint8 l,h; } PACKING B;
#else
    struct { uint8 h,l; } PACKING B;
#endif
    uint16 W;
} ALIGN_BY_ONE pair;

struct SRegisters{
    uint8  PB;
    uint8  DB;
    pair   P;
    pair   A;
    pair   D;
    pair   X;
    pair   S;
    pair   Y;
    uint16 PC;
} PACKING;

#define Registers	CPU.Regs
//EXTERN_C struct SRegisters Registers;

#endif
