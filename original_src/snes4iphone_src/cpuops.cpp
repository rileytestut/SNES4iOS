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
/**********************************************************************************************/
/* CPU-S9xOpcodes.CPP                                                                            */
/* This file contains all the opcodes                                                         */
/**********************************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "missing.h"
#include "apu.h"
#include "sa1.h"

START_EXTERN_C
extern uint8 A1, A2, A3, A4, W1, W2, W3, W4;
extern uint8 Ans8;
extern uint16 Ans16;
extern uint32 Ans32;
extern uint8 Work8;
extern uint16 Work16;
extern uint32 Work32;

extern signed char s9xInt8;
extern short s9xInt16;
extern long s9xInt32;
END_EXTERN_C

#include "cpuexec.h"
#include "cpuaddr.h"
#include "cpuops.h"
#include "cpumacro.h"
#include "apu.h"

/* ADC *************************************************************************************** */
static void Op69M1 (void)
{
    Immediate8 ();
    ADC8 ();
}

static void Op69M0 (void)
{
    Immediate16 ();
    ADC16 ();
}

static void Op65M1 (void)
{
    Direct ();
    ADC8 ();
}

static void Op65M0 (void)
{
    Direct ();
    ADC16 ();
}

static void Op75M1 (void)
{
    DirectIndexedX ();
    ADC8 ();
}

static void Op75M0 (void)
{
    DirectIndexedX ();
    ADC16 ();
}

static void Op72M1 (void)
{
    DirectIndirect ();
    ADC8 ();
}

static void Op72M0 (void)
{
    DirectIndirect ();
    ADC16 ();
}

static void Op61M1 (void)
{
    DirectIndexedIndirect ();
    ADC8 ();
}

static void Op61M0 (void)
{
    DirectIndexedIndirect ();
    ADC16 ();
}

static void Op71M1 (void)
{
    DirectIndirectIndexed ();
    ADC8 ();
}

static void Op71M0 (void)
{
    DirectIndirectIndexed ();
    ADC16 ();
}

static void Op67M1 (void)
{
    DirectIndirectLong ();
    ADC8 ();
}

static void Op67M0 (void)
{
    DirectIndirectLong ();
    ADC16 ();
}

static void Op77M1 (void)
{
    DirectIndirectIndexedLong ();
    ADC8 ();
}

static void Op77M0 (void)
{
    DirectIndirectIndexedLong ();
    ADC16 ();
}

static void Op6DM1 (void)
{
    Absolute ();
    ADC8 ();
}

static void Op6DM0 (void)
{
    Absolute ();
    ADC16 ();
}

static void Op7DM1 (void)
{
    AbsoluteIndexedX ();
    ADC8 ();
}

static void Op7DM0 (void)
{
    AbsoluteIndexedX ();
    ADC16 ();
}

static void Op79M1 (void)
{
    AbsoluteIndexedY ();
    ADC8 ();
}

static void Op79M0 (void)
{
    AbsoluteIndexedY ();
    ADC16 ();
}

static void Op6FM1 (void)
{
    AbsoluteLong ();
    ADC8 ();
}

static void Op6FM0 (void)
{
    AbsoluteLong ();
    ADC16 ();
}

static void Op7FM1 (void)
{
    AbsoluteLongIndexedX ();
    ADC8 ();
}

static void Op7FM0 (void)
{
    AbsoluteLongIndexedX ();
    ADC16 ();
}

static void Op63M1 (void)
{
    StackRelative ();
    ADC8 ();
}

static void Op63M0 (void)
{
    StackRelative ();
    ADC16 ();
}

static void Op73M1 (void)
{
    StackRelativeIndirectIndexed ();
    ADC8 ();
}

static void Op73M0 (void)
{
    StackRelativeIndirectIndexed ();
    ADC16 ();
}

/**********************************************************************************************/

/* AND *************************************************************************************** */
static void Op29M1 (void)
{
    Registers.AL &= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.AL);
}

static void Op29M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W &= *(uint16 *) CPU.PC;
#else
    Registers.A.W &= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.A.W);
}

static void Op25M1 (void)
{
    Direct ();
    AND8 ();
}

static void Op25M0 (void)
{
    Direct ();
    AND16 ();
}

static void Op35M1 (void)
{
    DirectIndexedX ();
    AND8 ();
}

static void Op35M0 (void)
{
    DirectIndexedX ();
    AND16 ();
}

static void Op32M1 (void)
{
    DirectIndirect ();
    AND8 ();
}

static void Op32M0 (void)
{
    DirectIndirect ();
    AND16 ();
}

static void Op21M1 (void)
{
    DirectIndexedIndirect ();
    AND8 ();
}

static void Op21M0 (void)
{
    DirectIndexedIndirect ();
    AND16 ();
}

static void Op31M1 (void)
{
    DirectIndirectIndexed ();
    AND8 ();
}

static void Op31M0 (void)
{
    DirectIndirectIndexed ();
    AND16 ();
}

static void Op27M1 (void)
{
    DirectIndirectLong ();
    AND8 ();
}

static void Op27M0 (void)
{
    DirectIndirectLong ();
    AND16 ();
}

static void Op37M1 (void)
{
    DirectIndirectIndexedLong ();
    AND8 ();
}

static void Op37M0 (void)
{
    DirectIndirectIndexedLong ();
    AND16 ();
}

static void Op2DM1 (void)
{
    Absolute ();
    AND8 ();
}

static void Op2DM0 (void)
{
    Absolute ();
    AND16 ();
}

static void Op3DM1 (void)
{
    AbsoluteIndexedX ();
    AND8 ();
}

static void Op3DM0 (void)
{
    AbsoluteIndexedX ();
    AND16 ();
}

static void Op39M1 (void)
{
    AbsoluteIndexedY ();
    AND8 ();
}

static void Op39M0 (void)
{
    AbsoluteIndexedY ();
    AND16 ();
}

static void Op2FM1 (void)
{
    AbsoluteLong ();
    AND8 ();
}

static void Op2FM0 (void)
{
    AbsoluteLong ();
    AND16 ();
}

static void Op3FM1 (void)
{
    AbsoluteLongIndexedX ();
    AND8 ();
}

static void Op3FM0 (void)
{
    AbsoluteLongIndexedX ();
    AND16 ();
}

static void Op23M1 (void)
{
    StackRelative ();
    AND8 ();
}

static void Op23M0 (void)
{
    StackRelative ();
    AND16 ();
}

static void Op33M1 (void)
{
    StackRelativeIndirectIndexed ();
    AND8 ();
}

static void Op33M0 (void)
{
    StackRelativeIndirectIndexed ();
    AND16 ();
}
/**********************************************************************************************/

/* ASL *************************************************************************************** */
static void Op0AM1 (void)
{
    A_ASL8 ();
}

static void Op0AM0 (void)
{
    A_ASL16 ();
}

static void Op06M1 (void)
{
    Direct ();
    ASL8 ();
}

static void Op06M0 (void)
{
    Direct ();
    ASL16 ();
}

static void Op16M1 (void)
{
    DirectIndexedX ();
    ASL8 ();
}

static void Op16M0 (void)
{
    DirectIndexedX ();
    ASL16 ();
}

static void Op0EM1 (void)
{
    Absolute ();
    ASL8 ();
}

static void Op0EM0 (void)
{
    Absolute ();
    ASL16 ();
}

static void Op1EM1 (void)
{
    AbsoluteIndexedX ();
    ASL8 ();
}

static void Op1EM0 (void)
{
    AbsoluteIndexedX ();
    ASL16 ();
}
/**********************************************************************************************/

/* BIT *************************************************************************************** */
static void Op89M1 (void)
{
    ICPU._Zero = Registers.AL & *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void Op89M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    ICPU._Zero = (Registers.A.W & *(uint16 *) CPU.PC) != 0;
#else
    ICPU._Zero = (Registers.A.W & (*CPU.PC + (*(CPU.PC + 1) << 8))) != 0;
#endif	
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    CPU.PC += 2;
}

static void Op24M1 (void)
{
    Direct ();
    BIT8 ();
}

static void Op24M0 (void)
{
    Direct ();
    BIT16 ();
}

static void Op34M1 (void)
{
    DirectIndexedX ();
    BIT8 ();
}

static void Op34M0 (void)
{
    DirectIndexedX ();
    BIT16 ();
}

static void Op2CM1 (void)
{
    Absolute ();
    BIT8 ();
}

static void Op2CM0 (void)
{
    Absolute ();
    BIT16 ();
}

static void Op3CM1 (void)
{
    AbsoluteIndexedX ();
    BIT8 ();
}

static void Op3CM0 (void)
{
    AbsoluteIndexedX ();
    BIT16 ();
}
/**********************************************************************************************/

/* CMP *************************************************************************************** */
static void OpC9M1 (void)
{
    s9xInt32 = (int) Registers.AL - (int) *CPU.PC++;
    ICPU._Carry = s9xInt32 >= 0;
    SetZN8 ((uint8) s9xInt32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC9M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS    
    s9xInt32 = (long) Registers.A.W - (long) *(uint16 *) CPU.PC;
#else
    s9xInt32 = (long) Registers.A.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC5M1 (void)
{
    Direct ();
    CMP8 ();
}

static void OpC5M0 (void)
{
    Direct ();
    CMP16 ();
}

static void OpD5M1 (void)
{
    DirectIndexedX ();
    CMP8 ();
}

static void OpD5M0 (void)
{
    DirectIndexedX ();
    CMP16 ();
}

static void OpD2M1 (void)
{
    DirectIndirect ();
    CMP8 ();
}

static void OpD2M0 (void)
{
    DirectIndirect ();
    CMP16 ();
}

static void OpC1M1 (void)
{
    DirectIndexedIndirect ();
    CMP8 ();
}

static void OpC1M0 (void)
{
    DirectIndexedIndirect ();
    CMP16 ();
}

static void OpD1M1 (void)
{
    DirectIndirectIndexed ();
    CMP8 ();
}

static void OpD1M0 (void)
{
    DirectIndirectIndexed ();
    CMP16 ();
}

static void OpC7M1 (void)
{
    DirectIndirectLong ();
    CMP8 ();
}

static void OpC7M0 (void)
{
    DirectIndirectLong ();
    CMP16 ();
}

static void OpD7M1 (void)
{
    DirectIndirectIndexedLong ();
    CMP8 ();
}

static void OpD7M0 (void)
{
    DirectIndirectIndexedLong ();
    CMP16 ();
}

static void OpCDM1 (void)
{
    Absolute ();
    CMP8 ();
}

static void OpCDM0 (void)
{
    Absolute ();
    CMP16 ();
}

static void OpDDM1 (void)
{
    AbsoluteIndexedX ();
    CMP8 ();
}

static void OpDDM0 (void)
{
    AbsoluteIndexedX ();
    CMP16 ();
}

static void OpD9M1 (void)
{
    AbsoluteIndexedY ();
    CMP8 ();
}

static void OpD9M0 (void)
{
    AbsoluteIndexedY ();
    CMP16 ();
}

static void OpCFM1 (void)
{
    AbsoluteLong ();
    CMP8 ();
}

static void OpCFM0 (void)
{
    AbsoluteLong ();
    CMP16 ();
}

static void OpDFM1 (void)
{
    AbsoluteLongIndexedX ();
    CMP8 ();
}

static void OpDFM0 (void)
{
    AbsoluteLongIndexedX ();
    CMP16 ();
}

static void OpC3M1 (void)
{
    StackRelative ();
    CMP8 ();
}

static void OpC3M0 (void)
{
    StackRelative ();
    CMP16 ();
}

static void OpD3M1 (void)
{
    StackRelativeIndirectIndexed ();
    CMP8 ();
}

static void OpD3M0 (void)
{
    StackRelativeIndirectIndexed ();
    CMP16 ();
}

/**********************************************************************************************/

/* CMX *************************************************************************************** */
static void OpE0X1 (void)
{
    s9xInt32 = (int) Registers.XL - (int) *CPU.PC++;
    ICPU._Carry = s9xInt32 >= 0;
    SetZN8 ((uint8) s9xInt32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpE0X0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS    
    s9xInt32 = (long) Registers.X.W - (long) *(uint16 *) CPU.PC;
#else
    s9xInt32 = (long) Registers.X.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpE4X1 (void)
{
    Direct ();
    CMX8 ();
}

static void OpE4X0 (void)
{
    Direct ();
    CMX16 ();
}

static void OpECX1 (void)
{
    Absolute ();
    CMX8 ();
}

static void OpECX0 (void)
{
    Absolute ();
    CMX16 ();
}

/**********************************************************************************************/

/* CMY *************************************************************************************** */
static void OpC0X1 (void)
{
    s9xInt32 = (int) Registers.YL - (int) *CPU.PC++;
    ICPU._Carry = s9xInt32 >= 0;
    SetZN8 ((uint8) s9xInt32);
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
}

static void OpC0X0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS    
    s9xInt32 = (long) Registers.Y.W - (long) *(uint16 *) CPU.PC;
#else
    s9xInt32 = (long) Registers.Y.W -
	    (long) (*CPU.PC + (*(CPU.PC + 1) << 8));
#endif
    ICPU._Carry = s9xInt32 >= 0;
    SetZN16 ((uint16) s9xInt32);
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
}

static void OpC4X1 (void)
{
    Direct ();
    CMY8 ();
}

static void OpC4X0 (void)
{
    Direct ();
    CMY16 ();
}

static void OpCCX1 (void)
{
    Absolute ();
    CMY8 ();
}

static void OpCCX0 (void)
{
    Absolute ();
    CMY16 ();
}

/**********************************************************************************************/

/* DEC *************************************************************************************** */
static void Op3AM1 (void)
{
    A_DEC8 ();
}

static void Op3AM0 (void)
{
    A_DEC16 ();
}

static void OpC6M1 (void)
{
    Direct ();
    DEC8 ();
}

static void OpC6M0 (void)
{
    Direct ();
    DEC16 ();
}

static void OpD6M1 (void)
{
    DirectIndexedX ();
    DEC8 ();
}

static void OpD6M0 (void)
{
    DirectIndexedX ();
    DEC16 ();
}

static void OpCEM1 (void)
{
    Absolute ();
    DEC8 ();
}

static void OpCEM0 (void)
{
    Absolute ();
    DEC16 ();
}

static void OpDEM1 (void)
{
    AbsoluteIndexedX ();
    DEC8 ();
}

static void OpDEM0 (void)
{
    AbsoluteIndexedX ();
    DEC16 ();
}

/**********************************************************************************************/

/* EOR *************************************************************************************** */
static void Op49M1 (void)
{
    Registers.AL ^= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.AL);
}

static void Op49M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W ^= *(uint16 *) CPU.PC;
#else
    Registers.A.W ^= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.A.W);
}

static void Op45M1 (void)
{
    Direct ();
    EOR8 ();
}

static void Op45M0 (void)
{
    Direct ();
    EOR16 ();
}

static void Op55M1 (void)
{
    DirectIndexedX ();
    EOR8 ();
}

static void Op55M0 (void)
{
    DirectIndexedX ();
    EOR16 ();
}

static void Op52M1 (void)
{
    DirectIndirect ();
    EOR8 ();
}

static void Op52M0 (void)
{
    DirectIndirect ();
    EOR16 ();
}

static void Op41M1 (void)
{
    DirectIndexedIndirect ();
    EOR8 ();
}

static void Op41M0 (void)
{
    DirectIndexedIndirect ();
    EOR16 ();
}

static void Op51M1 (void)
{
    DirectIndirectIndexed ();
    EOR8 ();
}

static void Op51M0 (void)
{
    DirectIndirectIndexed ();
    EOR16 ();
}

static void Op47M1 (void)
{
    DirectIndirectLong ();
    EOR8 ();
}

static void Op47M0 (void)
{
    DirectIndirectLong ();
    EOR16 ();
}

static void Op57M1 (void)
{
    DirectIndirectIndexedLong ();
    EOR8 ();
}

static void Op57M0 (void)
{
    DirectIndirectIndexedLong ();
    EOR16 ();
}

static void Op4DM1 (void)
{
    Absolute ();
    EOR8 ();
}

static void Op4DM0 (void)
{
    Absolute ();
    EOR16 ();
}

static void Op5DM1 (void)
{
    AbsoluteIndexedX ();
    EOR8 ();
}

static void Op5DM0 (void)
{
    AbsoluteIndexedX ();
    EOR16 ();
}

static void Op59M1 (void)
{
    AbsoluteIndexedY ();
    EOR8 ();
}

static void Op59M0 (void)
{
    AbsoluteIndexedY ();
    EOR16 ();
}

static void Op4FM1 (void)
{
    AbsoluteLong ();
    EOR8 ();
}

static void Op4FM0 (void)
{
    AbsoluteLong ();
    EOR16 ();
}

static void Op5FM1 (void)
{
    AbsoluteLongIndexedX ();
    EOR8 ();
}

static void Op5FM0 (void)
{
    AbsoluteLongIndexedX ();
    EOR16 ();
}

static void Op43M1 (void)
{
    StackRelative ();
    EOR8 ();
}

static void Op43M0 (void)
{
    StackRelative ();
    EOR16 ();
}

static void Op53M1 (void)
{
    StackRelativeIndirectIndexed ();
    EOR8 ();
}

static void Op53M0 (void)
{
    StackRelativeIndirectIndexed ();
    EOR16 ();
}

/**********************************************************************************************/

/* INC *************************************************************************************** */
static void Op1AM1 (void)
{
    A_INC8 ();
}

static void Op1AM0 (void)
{
    A_INC16 ();
}

static void OpE6M1 (void)
{
    Direct ();
    INC8 ();
}

static void OpE6M0 (void)
{
    Direct ();
    INC16 ();
}

static void OpF6M1 (void)
{
    DirectIndexedX ();
    INC8 ();
}

static void OpF6M0 (void)
{
    DirectIndexedX ();
    INC16 ();
}

static void OpEEM1 (void)
{
    Absolute ();
    INC8 ();
}

static void OpEEM0 (void)
{
    Absolute ();
    INC16 ();
}

static void OpFEM1 (void)
{
    AbsoluteIndexedX ();
    INC8 ();
}

static void OpFEM0 (void)
{
    AbsoluteIndexedX ();
    INC16 ();
}

/**********************************************************************************************/
/* LDA *************************************************************************************** */
static void OpA9M1 (void)
{
    Registers.AL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.AL);
}

static void OpA9M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W = *(uint16 *) CPU.PC;
#else
    Registers.A.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.A.W);
}

static void OpA5M1 (void)
{
    Direct ();
    LDA8 ();
}

static void OpA5M0 (void)
{
    Direct ();
    LDA16 ();
}

static void OpB5M1 (void)
{
    DirectIndexedX ();
    LDA8 ();
}

static void OpB5M0 (void)
{
    DirectIndexedX ();
    LDA16 ();
}

static void OpB2M1 (void)
{
    DirectIndirect ();
    LDA8 ();
}

static void OpB2M0 (void)
{
    DirectIndirect ();
    LDA16 ();
}

static void OpA1M1 (void)
{
    DirectIndexedIndirect ();
    LDA8 ();
}

static void OpA1M0 (void)
{
    DirectIndexedIndirect ();
    LDA16 ();
}

static void OpB1M1 (void)
{
    DirectIndirectIndexed ();
    LDA8 ();
}

static void OpB1M0 (void)
{
    DirectIndirectIndexed ();
    LDA16 ();
}

static void OpA7M1 (void)
{
    DirectIndirectLong ();
    LDA8 ();
}

static void OpA7M0 (void)
{
    DirectIndirectLong ();
    LDA16 ();
}

static void OpB7M1 (void)
{
    DirectIndirectIndexedLong ();
    LDA8 ();
}

static void OpB7M0 (void)
{
    DirectIndirectIndexedLong ();
    LDA16 ();
}

static void OpADM1 (void)
{
    Absolute ();
    LDA8 ();
}

static void OpADM0 (void)
{
    Absolute ();
    LDA16 ();
}

static void OpBDM1 (void)
{
    AbsoluteIndexedX ();
    LDA8 ();
}

static void OpBDM0 (void)
{
    AbsoluteIndexedX ();
    LDA16 ();
}

static void OpB9M1 (void)
{
    AbsoluteIndexedY ();
    LDA8 ();
}

static void OpB9M0 (void)
{
    AbsoluteIndexedY ();
    LDA16 ();
}

static void OpAFM1 (void)
{
    AbsoluteLong ();
    LDA8 ();
}

static void OpAFM0 (void)
{
    AbsoluteLong ();
    LDA16 ();
}

static void OpBFM1 (void)
{
    AbsoluteLongIndexedX ();
    LDA8 ();
}

static void OpBFM0 (void)
{
    AbsoluteLongIndexedX ();
    LDA16 ();
}

static void OpA3M1 (void)
{
    StackRelative ();
    LDA8 ();
}

static void OpA3M0 (void)
{
    StackRelative ();
    LDA16 ();
}

static void OpB3M1 (void)
{
    StackRelativeIndirectIndexed ();
    LDA8 ();
}

static void OpB3M0 (void)
{
    StackRelativeIndirectIndexed ();
    LDA16 ();
}

/**********************************************************************************************/

/* LDX *************************************************************************************** */
static void OpA2X1 (void)
{
    Registers.XL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.XL);
}

static void OpA2X0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.X.W = *(uint16 *) CPU.PC;
#else
    Registers.X.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.X.W);
}

static void OpA6X1 (void)
{
    Direct ();
    LDX8 ();
}

static void OpA6X0 (void)
{
    Direct ();
    LDX16 ();
}

static void OpB6X1 (void)
{
    DirectIndexedY ();
    LDX8 ();
}

static void OpB6X0 (void)
{
    DirectIndexedY ();
    LDX16 ();
}

static void OpAEX1 (void)
{
    Absolute ();
    LDX8 ();
}

static void OpAEX0 (void)
{
    Absolute ();
    LDX16 ();
}

static void OpBEX1 (void)
{
    AbsoluteIndexedY ();
    LDX8 ();
}

static void OpBEX0 (void)
{
    AbsoluteIndexedY ();
    LDX16 ();
}
/**********************************************************************************************/

/* LDY *************************************************************************************** */
static void OpA0X1 (void)
{
    Registers.YL = *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.YL);
}

static void OpA0X0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.Y.W = *(uint16 *) CPU.PC;
#else
    Registers.Y.W = *CPU.PC + (*(CPU.PC + 1) << 8);
#endif

    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.Y.W);
}

static void OpA4X1 (void)
{
    Direct ();
    LDY8 ();
}

static void OpA4X0 (void)
{
    Direct ();
    LDY16 ();
}

static void OpB4X1 (void)
{
    DirectIndexedX ();
    LDY8 ();
}

static void OpB4X0 (void)
{
    DirectIndexedX ();
    LDY16 ();
}

static void OpACX1 (void)
{
    Absolute ();
    LDY8 ();
}

static void OpACX0 (void)
{
    Absolute ();
    LDY16 ();
}

static void OpBCX1 (void)
{
    AbsoluteIndexedX ();
    LDY8 ();
}

static void OpBCX0 (void)
{
    AbsoluteIndexedX ();
    LDY16 ();
}
/**********************************************************************************************/

/* LSR *************************************************************************************** */
static void Op4AM1 (void)
{
    A_LSR8 ();
}

static void Op4AM0 (void)
{
    A_LSR16 ();
}

static void Op46M1 (void)
{
    Direct ();
    LSR8 ();
}

static void Op46M0 (void)
{
    Direct ();
    LSR16 ();
}

static void Op56M1 (void)
{
    DirectIndexedX ();
    LSR8 ();
}

static void Op56M0 (void)
{
    DirectIndexedX ();
    LSR16 ();
}

static void Op4EM1 (void)
{
    Absolute ();
    LSR8 ();
}

static void Op4EM0 (void)
{
    Absolute ();
    LSR16 ();
}

static void Op5EM1 (void)
{
    AbsoluteIndexedX ();
    LSR8 ();
}

static void Op5EM0 (void)
{
    AbsoluteIndexedX ();
    LSR16 ();
}

/**********************************************************************************************/

/* ORA *************************************************************************************** */
static void Op09M1 (void)
{
    Registers.AL |= *CPU.PC++;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed;
#endif
    SetZN8 (Registers.AL);
}

static void Op09M0 (void)
{
#ifdef FAST_LSB_WORD_ACCESS
    Registers.A.W |= *(uint16 *) CPU.PC;
#else
    Registers.A.W |= *CPU.PC + (*(CPU.PC + 1) << 8);
#endif
    CPU.PC += 2;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2;
#endif
    SetZN16 (Registers.A.W);
}

static void Op05M1 (void)
{
    Direct ();
    ORA8 ();
}

static void Op05M0 (void)
{
    Direct ();
    ORA16 ();
}

static void Op15M1 (void)
{
    DirectIndexedX ();
    ORA8 ();
}

static void Op15M0 (void)
{
    DirectIndexedX ();
    ORA16 ();
}

static void Op12M1 (void)
{
    DirectIndirect ();
    ORA8 ();
}

static void Op12M0 (void)
{
    DirectIndirect ();
    ORA16 ();
}

static void Op01M1 (void)
{
    DirectIndexedIndirect ();
    ORA8 ();
}

static void Op01M0 (void)
{
    DirectIndexedIndirect ();
    ORA16 ();
}

static void Op11M1 (void)
{
    DirectIndirectIndexed ();
    ORA8 ();
}

static void Op11M0 (void)
{
    DirectIndirectIndexed ();
    ORA16 ();
}

static void Op07M1 (void)
{
    DirectIndirectLong ();
    ORA8 ();
}

static void Op07M0 (void)
{
    DirectIndirectLong ();
    ORA16 ();
}

static void Op17M1 (void)
{
    DirectIndirectIndexedLong ();
    ORA8 ();
}

static void Op17M0 (void)
{
    DirectIndirectIndexedLong ();
    ORA16 ();
}

static void Op0DM1 (void)
{
    Absolute ();
    ORA8 ();
}

static void Op0DM0 (void)
{
    Absolute ();
    ORA16 ();
}

static void Op1DM1 (void)
{
    AbsoluteIndexedX ();
    ORA8 ();
}

static void Op1DM0 (void)
{
    AbsoluteIndexedX ();
    ORA16 ();
}

static void Op19M1 (void)
{
    AbsoluteIndexedY ();
    ORA8 ();
}

static void Op19M0 (void)
{
    AbsoluteIndexedY ();
    ORA16 ();
}

static void Op0FM1 (void)
{
    AbsoluteLong ();
    ORA8 ();
}

static void Op0FM0 (void)
{
    AbsoluteLong ();
    ORA16 ();
}

static void Op1FM1 (void)
{
    AbsoluteLongIndexedX ();
    ORA8 ();
}

static void Op1FM0 (void)
{
    AbsoluteLongIndexedX ();
    ORA16 ();
}

static void Op03M1 (void)
{
    StackRelative ();
    ORA8 ();
}

static void Op03M0 (void)
{
    StackRelative ();
    ORA16 ();
}

static void Op13M1 (void)
{
    StackRelativeIndirectIndexed ();
    ORA8 ();
}

static void Op13M0 (void)
{
    StackRelativeIndirectIndexed ();
    ORA16 ();
}

/**********************************************************************************************/

/* ROL *************************************************************************************** */
static void Op2AM1 (void)
{
    A_ROL8 ();
}

static void Op2AM0 (void)
{
    A_ROL16 ();
}

static void Op26M1 (void)
{
    Direct ();
    ROL8 ();
}

static void Op26M0 (void)
{
    Direct ();
    ROL16 ();
}

static void Op36M1 (void)
{
    DirectIndexedX ();
    ROL8 ();
}

static void Op36M0 (void)
{
    DirectIndexedX ();
    ROL16 ();
}

static void Op2EM1 (void)
{
    Absolute ();
    ROL8 ();
}

static void Op2EM0 (void)
{
    Absolute ();
    ROL16 ();
}

static void Op3EM1 (void)
{
    AbsoluteIndexedX ();
    ROL8 ();
}

static void Op3EM0 (void)
{
    AbsoluteIndexedX ();
    ROL16 ();
}
/**********************************************************************************************/

/* ROR *************************************************************************************** */
static void Op6AM1 (void)
{
    A_ROR8 ();
}

static void Op6AM0 (void)
{
    A_ROR16 ();
}

static void Op66M1 (void)
{
    Direct ();
    ROR8 ();
}

static void Op66M0 (void)
{
    Direct ();
    ROR16 ();
}

static void Op76M1 (void)
{
    DirectIndexedX ();
    ROR8 ();
}

static void Op76M0 (void)
{
    DirectIndexedX ();
    ROR16 ();
}

static void Op6EM1 (void)
{
    Absolute ();
    ROR8 ();
}

static void Op6EM0 (void)
{
    Absolute ();
    ROR16 ();
}

static void Op7EM1 (void)
{
    AbsoluteIndexedX ();
    ROR8 ();
}

static void Op7EM0 (void)
{
    AbsoluteIndexedX ();
    ROR16 ();
}
/**********************************************************************************************/

/* SBC *************************************************************************************** */
static void OpE9M1 (void)
{
    Immediate8 ();
    SBC8 ();
}

static void OpE9M0 (void)
{
    Immediate16 ();
    SBC16 ();
}

static void OpE5M1 (void)
{
    Direct ();
    SBC8 ();
}

static void OpE5M0 (void)
{
    Direct ();
    SBC16 ();
}

static void OpF5M1 (void)
{
    DirectIndexedX ();
    SBC8 ();
}

static void OpF5M0 (void)
{
    DirectIndexedX ();
    SBC16 ();
}

static void OpF2M1 (void)
{
    DirectIndirect ();
    SBC8 ();
}

static void OpF2M0 (void)
{
    DirectIndirect ();
    SBC16 ();
}

static void OpE1M1 (void)
{
    DirectIndexedIndirect ();
    SBC8 ();
}

static void OpE1M0 (void)
{
    DirectIndexedIndirect ();
    SBC16 ();
}

static void OpF1M1 (void)
{
    DirectIndirectIndexed ();
    SBC8 ();
}

static void OpF1M0 (void)
{
    DirectIndirectIndexed ();
    SBC16 ();
}

static void OpE7M1 (void)
{
    DirectIndirectLong ();
    SBC8 ();
}

static void OpE7M0 (void)
{
    DirectIndirectLong ();
    SBC16 ();
}

static void OpF7M1 (void)
{
    DirectIndirectIndexedLong ();
    SBC8 ();
}

static void OpF7M0 (void)
{
    DirectIndirectIndexedLong ();
    SBC16 ();
}

static void OpEDM1 (void)
{
    Absolute ();
    SBC8 ();
}

static void OpEDM0 (void)
{
    Absolute ();
    SBC16 ();
}

static void OpFDM1 (void)
{
    AbsoluteIndexedX ();
    SBC8 ();
}

static void OpFDM0 (void)
{
    AbsoluteIndexedX ();
    SBC16 ();
}

static void OpF9M1 (void)
{
    AbsoluteIndexedY ();
    SBC8 ();
}

static void OpF9M0 (void)
{
    AbsoluteIndexedY ();
    SBC16 ();
}

static void OpEFM1 (void)
{
    AbsoluteLong ();
    SBC8 ();
}

static void OpEFM0 (void)
{
    AbsoluteLong ();
    SBC16 ();
}

static void OpFFM1 (void)
{
    AbsoluteLongIndexedX ();
    SBC8 ();
}

static void OpFFM0 (void)
{
    AbsoluteLongIndexedX ();
    SBC16 ();
}

static void OpE3M1 (void)
{
    StackRelative ();
    SBC8 ();
}

static void OpE3M0 (void)
{
    StackRelative ();
    SBC16 ();
}

static void OpF3M1 (void)
{
    StackRelativeIndirectIndexed ();
    SBC8 ();
}

static void OpF3M0 (void)
{
    StackRelativeIndirectIndexed ();
    SBC16 ();
}
/**********************************************************************************************/

/* STA *************************************************************************************** */
static void Op85M1 (void)
{
    Direct ();
    STA8 ();
}

static void Op85M0 (void)
{
    Direct ();
    STA16 ();
}

static void Op95M1 (void)
{
    DirectIndexedX ();
    STA8 ();
}

static void Op95M0 (void)
{
    DirectIndexedX ();
    STA16 ();
}

static void Op92M1 (void)
{
    DirectIndirect ();
    STA8 ();
}

static void Op92M0 (void)
{
    DirectIndirect ();
    STA16 ();
}

static void Op81M1 (void)
{
    DirectIndexedIndirect ();
    STA8 ();
#ifdef noVAR_CYCLES
    if (CheckIndex ())
	CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op81M0 (void)
{
    DirectIndexedIndirect ();
    STA16 ();
#ifdef noVAR_CYCLES
    if (CheckIndex ())
	CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op91M1 (void)
{
    DirectIndirectIndexed ();
    STA8 ();
}

static void Op91M0 (void)
{
    DirectIndirectIndexed ();
    STA16 ();
}

static void Op87M1 (void)
{
    DirectIndirectLong ();
    STA8 ();
}

static void Op87M0 (void)
{
    DirectIndirectLong ();
    STA16 ();
}

static void Op97M1 (void)
{
    DirectIndirectIndexedLong ();
    STA8 ();
}

static void Op97M0 (void)
{
    DirectIndirectIndexedLong ();
    STA16 ();
}

static void Op8DM1 (void)
{
    Absolute ();
    STA8 ();
}

static void Op8DM0 (void)
{
    Absolute ();
    STA16 ();
}

static void Op9DM1 (void)
{
    AbsoluteIndexedX ();
    STA8 ();
}

static void Op9DM0 (void)
{
    AbsoluteIndexedX ();
    STA16 ();
}

static void Op99M1 (void)
{
    AbsoluteIndexedY ();
    STA8 ();
}

static void Op99M0 (void)
{
    AbsoluteIndexedY ();
    STA16 ();
}

static void Op8FM1 (void)
{
    AbsoluteLong ();
    STA8 ();
}

static void Op8FM0 (void)
{
    AbsoluteLong ();
    STA16 ();
}

static void Op9FM1 (void)
{
    AbsoluteLongIndexedX ();
    STA8 ();
}

static void Op9FM0 (void)
{
    AbsoluteLongIndexedX ();
    STA16 ();
}

static void Op83M1 (void)
{
    StackRelative ();
    STA8 ();
}

static void Op83M0 (void)
{
    StackRelative ();
    STA16 ();
}

static void Op93M1 (void)
{
    StackRelativeIndirectIndexed ();
    STA8 ();
}

static void Op93M0 (void)
{
    StackRelativeIndirectIndexed ();
    STA16 ();
}
/**********************************************************************************************/

/* STX *************************************************************************************** */
static void Op86X1 (void)
{
    Direct ();
    STX8 ();
}

static void Op86X0 (void)
{
    Direct ();
    STX16 ();
}

static void Op96X1 (void)
{
    DirectIndexedY ();
    STX8 ();
}

static void Op96X0 (void)
{
    DirectIndexedY ();
    STX16 ();
}

static void Op8EX1 (void)
{
    Absolute ();
    STX8 ();
}

static void Op8EX0 (void)
{
    Absolute ();
    STX16 ();
}
/**********************************************************************************************/

/* STY *************************************************************************************** */
static void Op84X1 (void)
{
    Direct ();
    STY8 ();
}

static void Op84X0 (void)
{
    Direct ();
    STY16 ();
}

static void Op94X1 (void)
{
    DirectIndexedX ();
    STY8 ();
}

static void Op94X0 (void)
{
    DirectIndexedX ();
    STY16 ();
}

static void Op8CX1 (void)
{
    Absolute ();
    STY8 ();
}

static void Op8CX0 (void)
{
    Absolute ();
    STY16 ();
}
/**********************************************************************************************/

/* STZ *************************************************************************************** */
static void Op64M1 (void)
{
    Direct ();
    STZ8 ();
}

static void Op64M0 (void)
{
    Direct ();
    STZ16 ();
}

static void Op74M1 (void)
{
    DirectIndexedX ();
    STZ8 ();
}

static void Op74M0 (void)
{
    DirectIndexedX ();
    STZ16 ();
}

static void Op9CM1 (void)
{
    Absolute ();
    STZ8 ();
}

static void Op9CM0 (void)
{
    Absolute ();
    STZ16 ();
}

static void Op9EM1 (void)
{
    AbsoluteIndexedX ();
    STZ8 ();
}

static void Op9EM0 (void)
{
    AbsoluteIndexedX ();
    STZ16 ();
}

/**********************************************************************************************/

/* TRB *************************************************************************************** */
static void Op14M1 (void)
{
    Direct ();
    TRB8 ();
}

static void Op14M0 (void)
{
    Direct ();
    TRB16 ();
}

static void Op1CM1 (void)
{
    Absolute ();
    TRB8 ();
}

static void Op1CM0 (void)
{
    Absolute ();
    TRB16 ();
}
/**********************************************************************************************/

/* TSB *************************************************************************************** */
static void Op04M1 (void)
{
    Direct ();
    TSB8 ();
}

static void Op04M0 (void)
{
    Direct ();
    TSB16 ();
}

static void Op0CM1 (void)
{
    Absolute ();
    TSB8 ();
}

static void Op0CM0 (void)
{
    Absolute ();
    TSB16 ();
}

/**********************************************************************************************/

/* Branch Instructions *********************************************************************** */
#ifndef SA1_OPCODES
#define BranchCheck0()\
    if( CPU.BranchSkip)\
    {\
	CPU.BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod)\
	    if( CPU.PC - CPU.PCBase > OpAddress)\
	        return;\
    }

#define BranchCheck1()\
    if( CPU.BranchSkip)\
    {\
	CPU.BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod) {\
	    if( CPU.PC - CPU.PCBase > OpAddress)\
	        return;\
	} else \
	if (Settings.SoundSkipMethod == 1)\
	    return;\
	if (Settings.SoundSkipMethod == 3)\
	    if( CPU.PC - CPU.PCBase > OpAddress)\
	        return;\
	    else\
		CPU.PC = CPU.PCBase + OpAddress;\
    }

#define BranchCheck2()\
    if( CPU.BranchSkip)\
    {\
	CPU.BranchSkip = FALSE;\
	if (!Settings.SoundSkipMethod) {\
	    if( CPU.PC - CPU.PCBase > OpAddress)\
	        return;\
	} else \
	if (Settings.SoundSkipMethod == 1)\
	    CPU.PC = CPU.PCBase + OpAddress;\
	if (Settings.SoundSkipMethod == 3)\
	    if (CPU.PC - CPU.PCBase > OpAddress)\
	        return;\
	    else\
		CPU.PC = CPU.PCBase + OpAddress;\
    }
#else
#define BranchCheck0()
#define BranchCheck1()
#define BranchCheck2()
#endif

#ifdef CPU_SHUTDOWN
#ifndef SA1_OPCODES
inline void CPUShutdown()
{
    if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
    {
	// Don't skip cycles with a pending NMI or IRQ - could cause delayed
	// interrupt. Interrupts are delayed for a few cycles already, but
	// the delay could allow the shutdown code to cycle skip again.
	// Was causing screen flashing on Top Gear 3000.

	if (CPU.WaitCounter == 0 && 
	    !(CPU.Flags & (IRQ_PENDING_FLAG | NMI_FLAG)))
	{
	    CPU.WaitAddress = NULL;
	    if (Settings.SA1)
		S9xSA1ExecuteDuringSleep ();
	    CPU.Cycles = CPU.NextEvent;
	    if (CPU.APU_APUExecuting)
	    {
			ICPU.CPUExecuting = FALSE;
			do
			{
		    	APU_EXECUTE1();
			} while (CPU.APU_Cycles < CPU.NextEvent);
			ICPU.CPUExecuting = TRUE;
	    }
	}
	else
		if (CPU.WaitCounter >= 2)
	   	 CPU.WaitCounter = 1;
		else
	    	 CPU.WaitCounter--;
    }
}
#else
inline void CPUShutdown()
{
    if (Settings.Shutdown && CPU.PC == CPU.WaitAddress)
    {
	if (CPU.WaitCounter >= 1)
	{
	    SA1.Executing = FALSE;
	    SA1.CPUExecuting = FALSE;
	}
	else
	    CPU.WaitCounter++;
    }
}
#endif
#else
#define CPUShutdown()
#endif

/* BCC */
static void Op90 (void)
{
    Relative ();
    BranchCheck0 ();
    if (!CheckCarry ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BCS */
static void OpB0 (void)
{
    Relative ();
    BranchCheck0 ();
    if (CheckCarry ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BEQ */
static void OpF0 (void)
{
    Relative ();
    BranchCheck2 ();
    if (CheckZero ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BMI */
static void Op30 (void)
{
    Relative ();
    BranchCheck1 ();
    if (CheckNegative ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BNE */
static void OpD0 (void)
{
    Relative ();
    BranchCheck1 ();
    if (!CheckZero ())
    {
	CPU.PC = CPU.PCBase + OpAddress;

#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BPL */
static void Op10 (void)
{
    Relative ();
    BranchCheck1 ();
    if (!CheckNegative ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BRA */
static void Op80 (void)
{
    Relative ();
    CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
    CPU.Cycles++;
#endif
#endif
    CPUShutdown ();
}

/* BVC */
static void Op50 (void)
{
    Relative ();
    BranchCheck0 ();
    if (!CheckOverflow ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}

/* BVS */
static void Op70 (void)
{
    Relative ();
    BranchCheck0 ();
    if (CheckOverflow ())
    {
	CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles++;
#endif
#endif
	CPUShutdown ();
    }
}
/**********************************************************************************************/

/* ClearFlag Instructions ******************************************************************** */
/* CLC */
static void Op18 (void)
{
    ClearCarry ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLD */
static void OpD8 (void)
{
    ClearDecimal ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* CLI */
static void Op58 (void)
{
    ClearIRQ ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
/*    CHECK_FOR_IRQ(); */
}

/* CLV */
static void OpB8 (void)
{
    ClearOverflow ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* DEX/DEY *********************************************************************************** */
static void OpCAX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.XL--;
    SetZN8 (Registers.XL);
}

static void OpCAX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.X.W--;
    SetZN16 (Registers.X.W);
}

static void Op88X1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.YL--;
    SetZN8 (Registers.YL);
}

static void Op88X0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.Y.W--;
    SetZN16 (Registers.Y.W);
}
/**********************************************************************************************/

/* INX/INY *********************************************************************************** */
static void OpE8X1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.XL++;
    SetZN8 (Registers.XL);
}

static void OpE8X0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.X.W++;
    SetZN16 (Registers.X.W);
}

static void OpC8X1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.YL++;
    SetZN8 (Registers.YL);
}

static void OpC8X0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
#ifdef CPU_SHUTDOWN
    CPU.WaitAddress = NULL;
#endif

    Registers.Y.W++;
    SetZN16 (Registers.Y.W);
}

/**********************************************************************************************/

/* NOP *************************************************************************************** */
static void OpEA (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif

}
/**********************************************************************************************/

/* PUSH Instructions ************************************************************************* */
#define PushW(w) \
    S9xSetWord (w, Registers.S.W - 1);\
    Registers.S.W -= 2;
#define PushB(b)\
    S9xSetByte (b, Registers.S.W--);

static void OpF4 (void)
{
    Absolute ();
    PushW ((unsigned short)OpAddress);
}

static void OpD4 (void)
{
    DirectIndirect ();
    PushW ((unsigned short)OpAddress);
}

static void Op62 (void)
{
    RelativeLong ();
    PushW ((unsigned short)OpAddress);
}

static void Op48M1 (void)
{
    PushB (Registers.AL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op48M0 (void)
{
    PushW (Registers.A.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op8B (void)
{
    PushB (Registers.DB);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op0B (void)
{
    PushW (Registers.D.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op4B (void)
{
    PushB (Registers.PB);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op08 (void)
{
    S9xPackStatus ();
    PushB (Registers.PL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX1 (void)
{
    PushB (Registers.XL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpDAX0 (void)
{
    PushW (Registers.X.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX1 (void)
{
    PushB (Registers.YL);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op5AX0 (void)
{
    PushW (Registers.Y.W);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* PULL Instructions ************************************************************************* */
#define PullW(w) \
	w = S9xGetWord (Registers.S.W + 1); \
	Registers.S.W += 2;

#define PullB(b)\
	b = S9xGetByte (++Registers.S.W);

static void Op68M1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.AL);
    SetZN8 (Registers.AL);
}

static void Op68M0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.A.W);
    SetZN16 (Registers.A.W);
}

static void OpAB (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.DB);
    SetZN8 (Registers.DB);
    ICPU.ShiftedDB = Registers.DB << 16;
}

/* PHP */
static void Op2B (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.D.W);
    SetZN16 (Registers.D.W);
}

/* PLP */
static void Op28 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.PL);
    S9xUnpackStatus ();

    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
/*     CHECK_FOR_IRQ();*/
}

static void OpFAX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.XL);
    SetZN8 (Registers.XL);
}

static void OpFAX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.X.W);
    SetZN16 (Registers.X.W);
}

static void Op7AX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullB (Registers.YL);
    SetZN8 (Registers.YL);
}

static void Op7AX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    PullW (Registers.Y.W);
    SetZN16 (Registers.Y.W);
}

/**********************************************************************************************/

/* SetFlag Instructions ********************************************************************** */
/* SEC */
static void Op38 (void)
{
    SetCarry ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

/* SED */
static void OpF8 (void)
{
    SetDecimal ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    missing.decimal_mode = 1;
}

/* SEI */
static void Op78 (void)
{
    SetIRQ ();
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* Transfer Instructions ********************************************************************* */
/* TAX8 */
static void OpAAX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.AL;
    SetZN8 (Registers.XL);
}

/* TAX16 */
static void OpAAX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.A.W;
    SetZN16 (Registers.X.W);
}

/* TAY8 */
static void OpA8X1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.YL = Registers.AL;
    SetZN8 (Registers.YL);
}

/* TAY16 */
static void OpA8X0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.Y.W = Registers.A.W;
    SetZN16 (Registers.Y.W);
}

static void Op5B (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.D.W = Registers.A.W;
    SetZN16 (Registers.D.W);
}

static void Op1B (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.S.W = Registers.A.W;
    if (CheckEmulation())
	Registers.SH = 1;
}

static void Op7B (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.D.W;
    SetZN16 (Registers.A.W);
}

static void Op3B (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.S.W;
    SetZN16 (Registers.A.W);
}

static void OpBAX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.SL;
    SetZN8 (Registers.XL);
}

static void OpBAX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.S.W;
    SetZN16 (Registers.X.W);
}

static void Op8AM1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.AL = Registers.XL;
    SetZN8 (Registers.AL);
}

static void Op8AM0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.X.W;
    SetZN16 (Registers.A.W);
}

static void Op9A (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.S.W = Registers.X.W;
    if (CheckEmulation())
	Registers.SH = 1;
}

static void Op9BX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.YL = Registers.XL;
    SetZN8 (Registers.YL);
}

static void Op9BX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.Y.W = Registers.X.W;
    SetZN16 (Registers.Y.W);
}

static void Op98M1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.AL = Registers.YL;
    SetZN8 (Registers.AL);
}

static void Op98M0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.A.W = Registers.Y.W;
    SetZN16 (Registers.A.W);
}

static void OpBBX1 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.XL = Registers.YL;
    SetZN8 (Registers.XL);
}

static void OpBBX0 (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
    Registers.X.W = Registers.Y.W;
    SetZN16 (Registers.X.W);
}

/**********************************************************************************************/

/* XCE *************************************************************************************** */
static void OpFB (void)
{
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif

    A1 = ICPU._Carry;
    A2 = Registers.PH;
    ICPU._Carry = A2 & 1;
    Registers.PH = A1;

    if (CheckEmulation())
    {
	SetFlags (MemoryFlag | IndexFlag);
	Registers.SH = 1;
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
}
/**********************************************************************************************/

/* BRK *************************************************************************************** */
static void Op00 (void)
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** BRK");
#endif

#ifndef SA1_OPCODES
    CPU.BRKTriggered = TRUE;
#endif

    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase + 1);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE6));
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFFE));
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* BRL ************************************************************************************** */
static void Op82 (void)
{
    RelativeLong ();
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
}
/**********************************************************************************************/

/* IRQ *************************************************************************************** */
void S9xOpcode_IRQ (void)
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** IRQ");
#endif
    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEE));
#endif
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2207] |
			 (Memory.FillRAM [0x2208] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x40))
	    S9xSetPCBase (Memory.FillRAM [0x220e] | 
			  (Memory.FillRAM [0x220f] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFE));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}

/**********************************************************************************************/

/* NMI *************************************************************************************** */
void S9xOpcode_NMI (void)
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** NMI");
#endif
    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFEA));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
#ifdef SA1_OPCODES
	S9xSA1SetPCBase (Memory.FillRAM [0x2205] |
			 (Memory.FillRAM [0x2206] << 8));
#else
	if (Settings.SA1 && (Memory.FillRAM [0x2209] & 0x20))
	    S9xSetPCBase (Memory.FillRAM [0x220c] |
			  (Memory.FillRAM [0x220d] << 8));
	else
	    S9xSetPCBase (S9xGetWord (0xFFFA));
#endif
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* COP *************************************************************************************** */
static void Op02 (void)
{
#ifdef DEBUGGER
    if (CPU.Flags & TRACE_FLAG)
	S9xTraceMessage ("*** COP");
#endif	
    if (!CheckEmulation())
    {
	PushB (Registers.PB);
	PushW (CPU.PC - CPU.PCBase + 1);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFE4));
#ifdef VAR_CYCLES
        CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 8;
#endif
#endif
    }
    else
    {
	PushW (CPU.PC - CPU.PCBase);
	S9xPackStatus ();
	PushB (Registers.PL);
	ClearDecimal ();
	SetIRQ ();

	Registers.PB = 0;
	ICPU.ShiftedPB = 0;
	S9xSetPCBase (S9xGetWord (0xFFF4));
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 6;
#endif
#endif
    }
}
/**********************************************************************************************/

/* JML *************************************************************************************** */
static void OpDC (void)
{
    AbsoluteIndirectLong ();
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}

static void Op5C (void)
{
    AbsoluteLong ();
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
}
/**********************************************************************************************/

/* JMP *************************************************************************************** */
static void Op4C (void)
{
    Absolute ();
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
#if defined(CPU_SHUTDOWN) && defined(SA1_OPCODES)
    CPUShutdown ();
#endif
}

static void Op6C (void)
{
    AbsoluteIndirect ();
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
}

static void Op7C (void)
{
    AbsoluteIndexedIndirect ();
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}
/**********************************************************************************************/

/* JSL/RTL *********************************************************************************** */
static void Op22 (void)
{
    AbsoluteLong ();
    PushB (Registers.PB);
    PushW (CPU.PC - CPU.PCBase - 1);
    Registers.PB = (uint8) (OpAddress >> 16);
    ICPU.ShiftedPB = OpAddress & 0xff0000;
    S9xSetPCBase (OpAddress);
}

static void Op6B (void)
{
    PullW (Registers.PC);
    PullB (Registers.PB);
    ICPU.ShiftedPB = Registers.PB << 16;
    S9xSetPCBase (ICPU.ShiftedPB + ((Registers.PC + 1) & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* JSR/RTS *********************************************************************************** */
static void Op20 (void)
{
    Absolute ();
    PushW (CPU.PC - CPU.PCBase - 1);
    S9xSetPCBase (ICPU.ShiftedPB + (OpAddress & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void OpFC (void)
{
    AbsoluteIndexedIndirect ();
    PushW (CPU.PC - CPU.PCBase - 1);
    S9xSetPCBase (ICPU.ShiftedPB + OpAddress);
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE;
#endif
}

static void Op60 (void)
{
    PullW (Registers.PC);
    S9xSetPCBase (ICPU.ShiftedPB + ((Registers.PC + 1) & 0xffff));
#ifdef VAR_CYCLES
    CPU.Cycles += ONE_CYCLE * 3;
#endif
}

/**********************************************************************************************/

/* MVN/MVP *********************************************************************************** */
static void Op54X1 (void)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = Registers.DB << 16;
    SrcBank = *CPU.PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W), 
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.XL++;
    Registers.YL++;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op54X0 (void)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif
    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = Registers.DB << 16;
    SrcBank = *CPU.PC++;

    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W), 
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.X.W++;
    Registers.Y.W++;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op44X1 (void)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = Registers.DB << 16;
    SrcBank = *CPU.PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W), 
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.XL--;
    Registers.YL--;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

static void Op44X0 (void)
{
    uint32 SrcBank;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeedx2 + TWO_CYCLES;
#endif    
    Registers.DB = *CPU.PC++;
    ICPU.ShiftedDB = Registers.DB << 16;
    SrcBank = *CPU.PC++;
    S9xSetByte (S9xGetByte ((SrcBank << 16) + Registers.X.W), 
	     ICPU.ShiftedDB + Registers.Y.W);

    Registers.X.W--;
    Registers.Y.W--;
    Registers.A.W--;
    if (Registers.A.W != 0xffff)
	CPU.PC -= 3;
}

/**********************************************************************************************/

/* REP/SEP *********************************************************************************** */
static void OpC2 (void)
{
    Work8 = ~*CPU.PC++;
    Registers.PL &= Work8;
    ICPU._Carry &= Work8;
    ICPU._Overflow &= (Work8 >> 6);
    ICPU._Negative &= Work8;
    ICPU._Zero |= ~Work8 & Zero;

#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
    if (CheckEmulation())
    {
	SetFlags (MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
/*    CHECK_FOR_IRQ(); */
}

static void OpE2 (void)
{
    Work8 = *CPU.PC++;
    Registers.PL |= Work8;
    ICPU._Carry |= Work8 & 1;
    ICPU._Overflow |= (Work8 >> 6) & 1;
    ICPU._Negative |= Work8;
    if (Work8 & Zero)
	ICPU._Zero = 0;
#ifdef VAR_CYCLES
    CPU.Cycles += CPU.MemSpeed + ONE_CYCLE;
#endif
    if (CheckEmulation())
    {
	SetFlags (MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
    S9xFixCycles();
}
/**********************************************************************************************/

/* XBA *************************************************************************************** */
static void OpEB (void)
{
    Work8 = Registers.AL;
    Registers.AL = Registers.AH;
    Registers.AH = Work8;

    SetZN8 (Registers.AL);
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
}
/**********************************************************************************************/

/* RTI *************************************************************************************** */
static void Op40 (void)
{
    PullB (Registers.PL);
    S9xUnpackStatus ();
    PullW (Registers.PC);
    if (!CheckEmulation())
    {
	PullB (Registers.PB);
	ICPU.ShiftedPB = Registers.PB << 16;
    }
    else
    {
	SetFlags (MemoryFlag | IndexFlag);
	missing.emulate6502 = 1;
    }
    S9xSetPCBase (ICPU.ShiftedPB + Registers.PC);
    
    if (CheckIndex ())
    {
	Registers.XH = 0;
	Registers.YH = 0;
    }
#ifdef VAR_CYCLES
    CPU.Cycles += TWO_CYCLES;
#endif
    S9xFixCycles();
/*    CHECK_FOR_IRQ(); */
}

/**********************************************************************************************/

/* STP/WAI/DB ******************************************************************************** */
// WAI
static void OpCB (void)
{
    if (CPU.IRQActive)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += TWO_CYCLES;
#else
#ifndef SA1_OPCODES
	CPU.Cycles += 2;
#endif
#endif
    }
    else
    {
	CPU.WaitingForInterrupt = TRUE;
	CPU.PC--;
#ifdef CPU_SHUTDOWN
#ifndef SA1_OPCODES
	if (Settings.Shutdown)
	{
	    CPU.Cycles = CPU.NextEvent;
	    if (CPU.APU_APUExecuting)
	    {
		ICPU.CPUExecuting = FALSE;
		do
		{
		    APU_EXECUTE1 ();
		} while (CPU.APU_Cycles < CPU.NextEvent);
		ICPU.CPUExecuting = TRUE;
	    }
	}
#else
	if (Settings.Shutdown)
	{
	    SA1.CPUExecuting = FALSE;
	    SA1.Executing = FALSE;
	}
#endif
#endif
    }
}

// STP
static void OpDB (void)
{
    CPU.PC--;
    CPU.Flags |= DEBUG_MODE_FLAG;
}

// Reserved S9xOpcode
static void Op42 (void) {	
#ifndef SA1_OPCODES	
	uint8 b;
	
	CPU.WaitAddress = NULL;
	if (Settings.SA1)	S9xSA1ExecuteDuringSleep ();
	
	CPU.Cycles = CPU.NextEvent;
	if (CPU.APU_APUExecuting)
	{
		ICPU.CPUExecuting = FALSE;
		do
		{
			APU_EXECUTE1();
		} while (CPU.APU_Cycles < CPU.NextEvent);
		ICPU.CPUExecuting = TRUE;
	}
	
	//debug_log("toto");
	b=*CPU.PC++;
	
	//relative
	s9xInt8=0xF0|(b&0xF);
#ifdef VAR_CYCLES
	CPU.Cycles += CPU.MemSpeed;
#endif    
	OpAddress = ((int) (CPU.PC - CPU.PCBase) + s9xInt8) & 0xffff;
	
	switch (b&0xF0) {		
		case 0x10: //BPL
			BranchCheck1 ();
			if (!CheckNegative ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0x30: //BMI
			BranchCheck1 ();
			if (CheckNegative ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0x50: //BVC
			BranchCheck0 ();
			if (!CheckOverflow ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0x70: //BVS
			BranchCheck0 ();
			if (CheckOverflow ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0x80: //BRA			
			//op80
			CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
    		CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
    		CPU.Cycles++;
#endif
#endif
			CPUShutdown ();
			return;
		case 0x90: //BCC
			BranchCheck0 ();
			if (!CheckCarry ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0xB0: //BCS
			BranchCheck0 ();
			if (CheckCarry ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0xD0: //BNE
			BranchCheck1 ();
			if (!CheckZero ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
#else
#ifndef SA1_OPCODES
				CPU.Cycles++;
#endif
#endif
				CPUShutdown ();
			}
			return;
		case 0xF0: //BEQ
			BranchCheck2 ();
			if (CheckZero ()) {
				CPU.PC = CPU.PCBase + OpAddress;
#ifdef VAR_CYCLES
				CPU.Cycles += ONE_CYCLE;
				#else
				#ifndef SA1_OPCODES
				CPU.Cycles++;
				#endif
				#endif
				CPUShutdown ();
			}
    	return;
	}
#endif	
}

/**********************************************************************************************/

/**********************************************************************************************/
/* CPU-S9xOpcodes Definitions                                                                    */
/**********************************************************************************************/
struct SOpcodes S9xOpcodesM1X1[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X1},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X1},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X1},    {Op85M1},    {Op86X1},
    {Op87M1},    {Op88X1},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX1},    {Op8DM1},    {Op8EX1},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X1},    {Op95M1},
    {Op96X1},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX1},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X1},    {OpA1M1},    {OpA2X1},    {OpA3M1},    {OpA4X1},
    {OpA5M1},    {OpA6X1},    {OpA7M1},    {OpA8X1},    {OpA9M1},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM1},    {OpAEX1},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X1},    {OpB5M1},    {OpB6X1},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM1},
    {OpBEX1},    {OpBFM1},    {OpC0X1},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X1},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X1},    {OpC9M1},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X1},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X1},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X1},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM1X0[256] =
{
    {Op00},	 {Op01M1},    {Op02},      {Op03M1},    {Op04M1},
    {Op05M1},    {Op06M1},    {Op07M1},    {Op08},      {Op09M1},
    {Op0AM1},    {Op0B},      {Op0CM1},    {Op0DM1},    {Op0EM1},
    {Op0FM1},    {Op10},      {Op11M1},    {Op12M1},    {Op13M1},
    {Op14M1},    {Op15M1},    {Op16M1},    {Op17M1},    {Op18},
    {Op19M1},    {Op1AM1},    {Op1B},      {Op1CM1},    {Op1DM1},
    {Op1EM1},    {Op1FM1},    {Op20},      {Op21M1},    {Op22},
    {Op23M1},    {Op24M1},    {Op25M1},    {Op26M1},    {Op27M1},
    {Op28},      {Op29M1},    {Op2AM1},    {Op2B},      {Op2CM1},
    {Op2DM1},    {Op2EM1},    {Op2FM1},    {Op30},      {Op31M1},
    {Op32M1},    {Op33M1},    {Op34M1},    {Op35M1},    {Op36M1},
    {Op37M1},    {Op38},      {Op39M1},    {Op3AM1},    {Op3B},
    {Op3CM1},    {Op3DM1},    {Op3EM1},    {Op3FM1},    {Op40},
    {Op41M1},    {Op42},      {Op43M1},    {Op44X0},    {Op45M1},
    {Op46M1},    {Op47M1},    {Op48M1},    {Op49M1},    {Op4AM1},
    {Op4B},      {Op4C},      {Op4DM1},    {Op4EM1},    {Op4FM1},
    {Op50},      {Op51M1},    {Op52M1},    {Op53M1},    {Op54X0},
    {Op55M1},    {Op56M1},    {Op57M1},    {Op58},      {Op59M1},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM1},    {Op5EM1},
    {Op5FM1},    {Op60},      {Op61M1},    {Op62},      {Op63M1},
    {Op64M1},    {Op65M1},    {Op66M1},    {Op67M1},    {Op68M1},
    {Op69M1},    {Op6AM1},    {Op6B},      {Op6C},      {Op6DM1},
    {Op6EM1},    {Op6FM1},    {Op70},      {Op71M1},    {Op72M1},
    {Op73M1},    {Op74M1},    {Op75M1},    {Op76M1},    {Op77M1},
    {Op78},      {Op79M1},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM1},    {Op7EM1},    {Op7FM1},    {Op80},      {Op81M1},
    {Op82},      {Op83M1},    {Op84X0},    {Op85M1},    {Op86X0},
    {Op87M1},    {Op88X0},    {Op89M1},    {Op8AM1},    {Op8B},
    {Op8CX0},    {Op8DM1},    {Op8EX0},    {Op8FM1},    {Op90},
    {Op91M1},    {Op92M1},    {Op93M1},    {Op94X0},    {Op95M1},
    {Op96X0},    {Op97M1},    {Op98M1},    {Op99M1},    {Op9A},
    {Op9BX0},    {Op9CM1},    {Op9DM1},    {Op9EM1},    {Op9FM1},
    {OpA0X0},    {OpA1M1},    {OpA2X0},    {OpA3M1},    {OpA4X0},
    {OpA5M1},    {OpA6X0},    {OpA7M1},    {OpA8X0},    {OpA9M1},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM1},    {OpAEX0},
    {OpAFM1},    {OpB0},      {OpB1M1},    {OpB2M1},    {OpB3M1},
    {OpB4X0},    {OpB5M1},    {OpB6X0},    {OpB7M1},    {OpB8},
    {OpB9M1},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM1},
    {OpBEX0},    {OpBFM1},    {OpC0X0},    {OpC1M1},    {OpC2},
    {OpC3M1},    {OpC4X0},    {OpC5M1},    {OpC6M1},    {OpC7M1},
    {OpC8X0},    {OpC9M1},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM1},    {OpCEM1},    {OpCFM1},    {OpD0},      {OpD1M1},
    {OpD2M1},    {OpD3M1},    {OpD4},      {OpD5M1},    {OpD6M1},
    {OpD7M1},    {OpD8},      {OpD9M1},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM1},    {OpDEM1},    {OpDFM1},    {OpE0X0},
    {OpE1M1},    {OpE2},      {OpE3M1},    {OpE4X0},    {OpE5M1},
    {OpE6M1},    {OpE7M1},    {OpE8X0},    {OpE9M1},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM1},    {OpEEM1},    {OpEFM1},
    {OpF0},      {OpF1M1},    {OpF2M1},    {OpF3M1},    {OpF4},
    {OpF5M1},    {OpF6M1},    {OpF7M1},    {OpF8},      {OpF9M1},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM1},    {OpFEM1},
    {OpFFM1}
};

struct SOpcodes S9xOpcodesM0X0[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X0},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X0},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX0},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX0},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X0},    {Op85M0},    {Op86X0},
    {Op87M0},    {Op88X0},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX0},    {Op8DM0},    {Op8EX0},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X0},    {Op95M0},
    {Op96X0},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX0},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X0},    {OpA1M0},    {OpA2X0},    {OpA3M0},    {OpA4X0},
    {OpA5M0},    {OpA6X0},    {OpA7M0},    {OpA8X0},    {OpA9M0},
    {OpAAX0},    {OpAB},      {OpACX0},    {OpADM0},    {OpAEX0},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X0},    {OpB5M0},    {OpB6X0},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX0},    {OpBBX0},    {OpBCX0},    {OpBDM0},
    {OpBEX0},    {OpBFM0},    {OpC0X0},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X0},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X0},    {OpC9M0},    {OpCAX0},    {OpCB},      {OpCCX0},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX0},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X0},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X0},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X0},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX0},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX0},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};

struct SOpcodes S9xOpcodesM0X1[256] =
{
    {Op00},	 {Op01M0},    {Op02},      {Op03M0},    {Op04M0},
    {Op05M0},    {Op06M0},    {Op07M0},    {Op08},      {Op09M0},
    {Op0AM0},    {Op0B},      {Op0CM0},    {Op0DM0},    {Op0EM0},
    {Op0FM0},    {Op10},      {Op11M0},    {Op12M0},    {Op13M0},
    {Op14M0},    {Op15M0},    {Op16M0},    {Op17M0},    {Op18},
    {Op19M0},    {Op1AM0},    {Op1B},      {Op1CM0},    {Op1DM0},
    {Op1EM0},    {Op1FM0},    {Op20},      {Op21M0},    {Op22},
    {Op23M0},    {Op24M0},    {Op25M0},    {Op26M0},    {Op27M0},
    {Op28},      {Op29M0},    {Op2AM0},    {Op2B},      {Op2CM0},
    {Op2DM0},    {Op2EM0},    {Op2FM0},    {Op30},      {Op31M0},
    {Op32M0},    {Op33M0},    {Op34M0},    {Op35M0},    {Op36M0},
    {Op37M0},    {Op38},      {Op39M0},    {Op3AM0},    {Op3B},
    {Op3CM0},    {Op3DM0},    {Op3EM0},    {Op3FM0},    {Op40},
    {Op41M0},    {Op42},      {Op43M0},    {Op44X1},    {Op45M0},
    {Op46M0},    {Op47M0},    {Op48M0},    {Op49M0},    {Op4AM0},
    {Op4B},      {Op4C},      {Op4DM0},    {Op4EM0},    {Op4FM0},
    {Op50},      {Op51M0},    {Op52M0},    {Op53M0},    {Op54X1},
    {Op55M0},    {Op56M0},    {Op57M0},    {Op58},      {Op59M0},
    {Op5AX1},    {Op5B},      {Op5C},      {Op5DM0},    {Op5EM0},
    {Op5FM0},    {Op60},      {Op61M0},    {Op62},      {Op63M0},
    {Op64M0},    {Op65M0},    {Op66M0},    {Op67M0},    {Op68M0},
    {Op69M0},    {Op6AM0},    {Op6B},      {Op6C},      {Op6DM0},
    {Op6EM0},    {Op6FM0},    {Op70},      {Op71M0},    {Op72M0},
    {Op73M0},    {Op74M0},    {Op75M0},    {Op76M0},    {Op77M0},
    {Op78},      {Op79M0},    {Op7AX1},    {Op7B},      {Op7C},
    {Op7DM0},    {Op7EM0},    {Op7FM0},    {Op80},      {Op81M0},
    {Op82},      {Op83M0},    {Op84X1},    {Op85M0},    {Op86X1},
    {Op87M0},    {Op88X1},    {Op89M0},    {Op8AM0},    {Op8B},
    {Op8CX1},    {Op8DM0},    {Op8EX1},    {Op8FM0},    {Op90},
    {Op91M0},    {Op92M0},    {Op93M0},    {Op94X1},    {Op95M0},
    {Op96X1},    {Op97M0},    {Op98M0},    {Op99M0},    {Op9A},
    {Op9BX1},    {Op9CM0},    {Op9DM0},    {Op9EM0},    {Op9FM0},
    {OpA0X1},    {OpA1M0},    {OpA2X1},    {OpA3M0},    {OpA4X1},
    {OpA5M0},    {OpA6X1},    {OpA7M0},    {OpA8X1},    {OpA9M0},
    {OpAAX1},    {OpAB},      {OpACX1},    {OpADM0},    {OpAEX1},
    {OpAFM0},    {OpB0},      {OpB1M0},    {OpB2M0},    {OpB3M0},
    {OpB4X1},    {OpB5M0},    {OpB6X1},    {OpB7M0},    {OpB8},
    {OpB9M0},    {OpBAX1},    {OpBBX1},    {OpBCX1},    {OpBDM0},
    {OpBEX1},    {OpBFM0},    {OpC0X1},    {OpC1M0},    {OpC2},
    {OpC3M0},    {OpC4X1},    {OpC5M0},    {OpC6M0},    {OpC7M0},
    {OpC8X1},    {OpC9M0},    {OpCAX1},    {OpCB},      {OpCCX1},
    {OpCDM0},    {OpCEM0},    {OpCFM0},    {OpD0},      {OpD1M0},
    {OpD2M0},    {OpD3M0},    {OpD4},      {OpD5M0},    {OpD6M0},
    {OpD7M0},    {OpD8},      {OpD9M0},    {OpDAX1},    {OpDB},
    {OpDC},      {OpDDM0},    {OpDEM0},    {OpDFM0},    {OpE0X1},
    {OpE1M0},    {OpE2},      {OpE3M0},    {OpE4X1},    {OpE5M0},
    {OpE6M0},    {OpE7M0},    {OpE8X1},    {OpE9M0},    {OpEA},
    {OpEB},      {OpECX1},    {OpEDM0},    {OpEEM0},    {OpEFM0},
    {OpF0},      {OpF1M0},    {OpF2M0},    {OpF3M0},    {OpF4},
    {OpF5M0},    {OpF6M0},    {OpF7M0},    {OpF8},      {OpF9M0},
    {OpFAX1},    {OpFB},      {OpFC},      {OpFDM0},    {OpFEM0},
    {OpFFM0}
};
