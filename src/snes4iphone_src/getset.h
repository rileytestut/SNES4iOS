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
#ifndef _GETSET_H_
#define _GETSET_H_

#include "ppu.h"
#include "dsp1.h"
#include "cpuexec.h"
#include "sa1.h"

#define __memcheck__
//#define __show_io__
extern int oppause;
extern uint16 mem_check;

INLINE uint8 S9xGetByte (uint32 Address)
{	
#ifdef __show_io__
	char str[64];
	sprintf(str,"rd @ %04X",Address);
	S9xMessage(0,0,str);
	gp32_pause();
#endif
#ifdef __memcheck__
	mem_check+=(Address>>16)+Address;
#endif	
#if defined(VAR_CYCLES) || defined(CPU_SHUTDOWN)
    int block;
    uint8 *GetAddress = Memory.Map [block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
    uint8 *GetAddress = Memory.Map [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif    
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += Memory.MemorySpeed [block];
#endif
#ifdef CPU_SHUTDOWN
	if (Memory.BlockIsRAM [block])
	    CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif
	return (*(GetAddress + (Address & 0xffff)));
    }

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
	if (!CPU.InDMA)
	    CPU.Cycles += ONE_CYCLE;
#endif	
	return (S9xGetPPU (Address & 0xffff));
    case CMemory::MAP_CPU:
#ifdef VAR_CYCLES
	CPU.Cycles += ONE_CYCLE;
#endif
	return (S9xGetCPU (Address & 0xffff));
    case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif	
	return (S9xGetDSP (Address & 0xffff));
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	return (*(Memory.SRAM + ((Address & CPU.Memory_SRAMMask))));

    case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	return (*(Memory.SRAM + (((Address & 0x7fff) - 0x6000 +
				  ((Address & 0xf0000) >> 3)) & CPU.Memory_SRAMMask)));

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("R(B) %06x\n", Address);
#endif

    case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	return (*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)));
//#ifndef __GP32__
    case CMemory::MAP_C4:
	return (S9xGetC4 (Address & 0xffff));
//#endif    
    default:
    case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
#ifdef DEBUGGER
	printf ("R(B) %06x\n", Address);
#endif
	return ((Address >> 8) & 0xff);
    }
}

INLINE uint16 S9xGetWord (uint32 Address)
{
#ifdef __show_io__
	char str[64];
	sprintf(str,"rd @ %04X",Address);
	S9xMessage(0,0,str);
	gp32_pause();
#endif
#ifdef __memcheck__
	mem_check+=(Address>>16)+Address;
#endif	
    if ((Address & 0x1fff) == 0x1fff)
    {
	return (S9xGetByte (Address) | (S9xGetByte (Address + 1) << 8));
    }
#if defined(VAR_CYCLES) || defined(CPU_SHUTDOWN)
    int block;
    uint8 *GetAddress = Memory.Map [block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
    uint8 *GetAddress = Memory.Map [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif    
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += Memory.MemorySpeed [block] << 1;
#endif
#ifdef CPU_SHUTDOWN
	if (Memory.BlockIsRAM [block])
	    CPU.WaitAddress = CPU.PCAtOpcodeStart;
#endif
#ifdef FAST_LSB_WORD_ACCESS
	return (*(uint16 *) (GetAddress + (Address & 0xffff)));
#else
	return (*(GetAddress + (Address & 0xffff)) |
		(*(GetAddress + (Address & 0xffff) + 1) << 8));
#endif	
    }

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
	if (!CPU.InDMA)
	    CPU.Cycles += TWO_CYCLES;
#endif	
	return (S9xGetPPU (Address & 0xffff) |
		(S9xGetPPU ((Address + 1) & 0xffff) << 8));
    case CMemory::MAP_CPU:
#ifdef VAR_CYCLES   
	CPU.Cycles += TWO_CYCLES;
#endif
	return (S9xGetCPU (Address & 0xffff) |
		(S9xGetCPU ((Address + 1) & 0xffff) << 8));
    case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif	
	return (S9xGetDSP (Address & 0xffff) |
		(S9xGetDSP ((Address + 1) & 0xffff) << 8));
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	return (*(Memory.SRAM + (Address & CPU.Memory_SRAMMask)) |
		(*(Memory.SRAM + ((Address + 1) & CPU.Memory_SRAMMask)) << 8));

    case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	return (*(Memory.SRAM +
		  (((Address & 0x7fff) - 0x6000 +
		    ((Address & 0xf0000) >> 3)) & CPU.Memory_SRAMMask)) |
		(*(Memory.SRAM +
		   ((((Address + 1) & 0x7fff) - 0x6000 +
		     (((Address + 1) & 0xf0000) >> 3)) & CPU.Memory_SRAMMask)) << 8));

    case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	return (*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) |
		(*(Memory.BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) << 8));

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("R(W) %06x\n", Address);
#endif

//#ifndef __GP32__
    case CMemory::MAP_C4:
	return (S9xGetC4 (Address & 0xffff) |	
		(S9xGetC4 ((Address + 1) & 0xffff) << 8));
//#endif    
    default:
    case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
#ifdef DEBUGGER
	printf ("R(W) %06x\n", Address);
#endif
	return (((Address >> 8) | (Address & 0xff00)) & 0xffff);
    }
}

INLINE void S9xSetByte (uint8 Byte, uint32 Address)
{
#ifdef __show_io__
	char str[64];
	sprintf(str,"wr @ %04X %02X",Address,Byte);
	S9xMessage(0,0,str);
	gp32_pause();
#endif
#ifdef __memcheck__
	mem_check+=Byte;
#endif	

#if defined(CPU_SHUTDOWN)
    CPU.WaitAddress = NULL;
#endif
#if defined(VAR_CYCLES)
    int block;
    uint8 *SetAddress = Memory.WriteMap [block = ((Address >> MEMMAP_SHIFT) & MEMMAP_MASK)];
#else
    uint8 *SetAddress = Memory.WriteMap [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif

    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += Memory.MemorySpeed [block];
#endif
#ifdef CPU_SHUTDOWN
	SetAddress += Address & 0xffff;
#ifdef USE_SA1
	if (SetAddress == SA1.WaitByteAddress1 ||
	    SetAddress == SA1.WaitByteAddress2)
	{
	    SA1.Executing = SA1.S9xOpcodes != NULL;
	    SA1.WaitCounter = 0;
	}
#endif
	*SetAddress = Byte;
#else
	*(SetAddress + (Address & 0xffff)) = Byte;
#endif
	return;
    }

    switch ((int) SetAddress)
    {
    case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
	if (!CPU.InDMA)
	    CPU.Cycles += ONE_CYCLE;
#endif	
	S9xSetPPU (Byte, Address & 0xffff);
	return;

    case CMemory::MAP_CPU:
#ifdef VAR_CYCLES   
	CPU.Cycles += ONE_CYCLE;
#endif
	S9xSetCPU (Byte, Address & 0xffff);
	return;

    case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif	
	S9xSetDSP (Byte, Address & 0xffff);
	return;

    case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	if (CPU.Memory_SRAMMask)
	{
	    *(Memory.SRAM + (Address & CPU.Memory_SRAMMask)) = Byte;
	    CPU.SRAMModified = TRUE;
	}
	return;

    case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	if (CPU.Memory_SRAMMask)
	{
	    *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 +
			      ((Address & 0xf0000) >> 3)) & CPU.Memory_SRAMMask)) = Byte;
	    CPU.SRAMModified = TRUE;
	}
	return;

    case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
	CPU.SRAMModified = TRUE;
	return;

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("W(B) %06x\n", Address);
#endif

    case CMemory::MAP_SA1RAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	*(Memory.SRAM + (Address & 0xffff)) = Byte;
	SA1.Executing = !SA1.Waiting;
	break;
//#ifndef __GP32__
    case CMemory::MAP_C4:
	S9xSetC4 (Byte, Address & 0xffff);
	return;
//#endif	
    default:
    case CMemory::MAP_NONE:
#ifdef VAR_CYCLES    
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif	
#ifdef DEBUGGER
	printf ("W(B) %06x\n", Address);
#endif
	return;
    }
}

INLINE void S9xSetWord (uint16 Word, uint32 Address)
{
#ifdef __show_io__
	char str[64];
	sprintf(str,"wr @ %04X %04X",Address,Word);
	S9xMessage(0,0,str);
	gp32_pause();
#endif
#ifdef __memcheck__
	mem_check+=Word;
#endif
#if defined(CPU_SHUTDOWN)
    CPU.WaitAddress = NULL;
#endif
#if defined (VAR_CYCLES)
    int block;
    uint8 *SetAddress = Memory.WriteMap [block = ((Address >> MEMMAP_SHIFT) & MEMMAP_MASK)];
#else
    uint8 *SetAddress = Memory.WriteMap [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif

    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef VAR_CYCLES
	CPU.Cycles += Memory.MemorySpeed [block] << 1;
#endif
#if defined(CPU_SHUTDOWN) && defined(USE_SA1)
	uint8 *SetAddressSA1 = (uint8 *)(Address & 0xffff);
	if (SetAddressSA1 == SA1.WaitByteAddress1 ||
	    SetAddressSA1 == SA1.WaitByteAddress2)
	{
	    SA1.Executing = SA1.S9xOpcodes != NULL;
	    SA1.WaitCounter = 0;
	}
#endif
#ifdef FAST_LSB_WORD_ACCESS
	*(uint16 *) (SetAddress + (Address & 0xffff)) = Word;
#else
	*(SetAddress + (Address & 0xffff)) = (uint8) Word;
	*(SetAddress + ((Address + 1) & 0xffff)) = Word >> 8;
#endif
	return;
    }

    switch ((int) SetAddress)
    {
    case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
	if (!CPU.InDMA)
	    CPU.Cycles += TWO_CYCLES;
#endif	
	S9xSetPPU ((uint8) Word, Address & 0xffff);
	S9xSetPPU (Word >> 8, (Address & 0xffff) + 1);
	return;

    case CMemory::MAP_CPU:
#ifdef VAR_CYCLES   
	CPU.Cycles += TWO_CYCLES;
#endif
	S9xSetCPU ((uint8) Word, (Address & 0xffff));
	S9xSetCPU (Word >> 8, (Address & 0xffff) + 1);
	return;

    case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif	
	S9xSetDSP ((uint8) Word, (Address & 0xffff));
	S9xSetDSP (Word >> 8, (Address & 0xffff) + 1);
	return;

    case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	if (CPU.Memory_SRAMMask)
	{
	    *(Memory.SRAM + (Address & CPU.Memory_SRAMMask)) = (uint8) Word;
	    *(Memory.SRAM + ((Address + 1) & CPU.Memory_SRAMMask)) = Word >> 8;
	    CPU.SRAMModified = TRUE;
	}
	return;

    case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	if (CPU.Memory_SRAMMask)
	{
	    *(Memory.SRAM + 
	      (((Address & 0x7fff) - 0x6000 +
		((Address & 0xf0000) >> MEMMAP_SHIFT) & CPU.Memory_SRAMMask))) = (uint8) Word;
	    *(Memory.SRAM + 
	      ((((Address + 1) & 0x7fff) - 0x6000 +
		(((Address + 1) & 0xf0000) >> MEMMAP_SHIFT) & CPU.Memory_SRAMMask))) = (uint8) (Word >> 8);
	    CPU.SRAMModified = TRUE;
	}
	return;

    case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
	*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = (uint8) Word;
	*(Memory.BWRAM + (((Address + 1) & 0x7fff) - 0x6000)) = (uint8) (Word >> 8);
	CPU.SRAMModified = TRUE;
	return;

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("W(W) %06x\n", Address);
#endif

    case CMemory::MAP_SA1RAM:
#ifdef VAR_CYCLES
	CPU.Cycles += SLOW_ONE_CYCLE;
#endif
	*(Memory.SRAM + (Address & 0xffff)) = (uint8) Word;
	*(Memory.SRAM + ((Address + 1) & 0xffff)) = (uint8) (Word >> 8);
	SA1.Executing = !SA1.Waiting;
	break;
//#ifndef __GP32__
    case CMemory::MAP_C4:
	S9xSetC4 (Word & 0xff, Address & 0xffff);
	S9xSetC4 ((uint8) (Word >> 8), (Address + 1) & 0xffff);
	return;
//#endif	
    default:
    case CMemory::MAP_NONE:
#ifdef VAR_CYCLES    
	CPU.Cycles += SLOW_ONE_CYCLE * 2;
#endif
#ifdef DEBUGGER
	printf ("W(W) %06x\n", Address);
#endif
	return;
    }
}

INLINE uint8 *GetBasePointer (uint32 Address)
{
    uint8 *GetAddress = Memory.Map [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (GetAddress);

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (Memory.FillRAM - 0x2000);
    case CMemory::MAP_CPU:
	return (Memory.FillRAM - 0x4000);
    case CMemory::MAP_DSP:
	return (Memory.FillRAM - 0x6000);
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	return (Memory.SRAM);
    case CMemory::MAP_BWRAM:
	return (Memory.BWRAM - 0x6000);
    case CMemory::MAP_HIROM_SRAM:
	return (Memory.SRAM - 0x6000);
//#ifndef __GP32__	
    case CMemory::MAP_C4:
	return (Memory.C4RAM - 0x6000);
//#endif	
    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("GBP %06x\n", Address);
#endif

    default:
    case CMemory::MAP_NONE:
#ifdef DEBUGGER
	printf ("GBP %06x\n", Address);
#endif
	return (0);
    }
}

INLINE uint8 *S9xGetMemPointer (uint32 Address)
{
    uint8 *GetAddress = Memory.Map [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (GetAddress + (Address & 0xffff));

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (Memory.FillRAM - 0x2000 + (Address & 0xffff));
    case CMemory::MAP_CPU:
	return (Memory.FillRAM - 0x4000 + (Address & 0xffff));
    case CMemory::MAP_DSP:
	return (Memory.FillRAM - 0x6000 + (Address & 0xffff));
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	return (Memory.SRAM + (Address & 0xffff));
    case CMemory::MAP_BWRAM:
	return (Memory.BWRAM - 0x6000 + (Address & 0xffff));
    case CMemory::MAP_HIROM_SRAM:
	return (Memory.SRAM - 0x6000 + (Address & 0xffff));
//#ifndef __GP32__	
    case CMemory::MAP_C4:
	return (Memory.C4RAM - 0x6000 + (Address & 0xffff));
//#endif
    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("GMP %06x\n", Address);
#endif
    default:
    case CMemory::MAP_NONE:
#ifdef DEBUGGER
	printf ("GMP %06x\n", Address);
#endif
	return (0);
    }
}

INLINE void S9xSetPCBase (uint32 Address)
{
#ifdef VAR_CYCLES
    int block;
    uint8 *GetAddress = Memory.Map [block = (Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#else
    uint8 *GetAddress = Memory.Map [(Address >> MEMMAP_SHIFT) & MEMMAP_MASK];
#endif    
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef VAR_CYCLES
	CPU.MemSpeed = Memory.MemorySpeed [block];
	CPU.MemSpeedx2 = CPU.MemSpeed << 1;
#endif
	CPU.PCBase = GetAddress;
	CPU.PC = GetAddress + (Address & 0xffff);
	return;
    }

    switch ((int) GetAddress)
    {
    case CMemory::MAP_PPU:
#ifdef VAR_CYCLES
	CPU.MemSpeed = ONE_CYCLE;
	CPU.MemSpeedx2 = TWO_CYCLES;
#endif	
	CPU.PCBase = Memory.FillRAM - 0x2000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
	
    case CMemory::MAP_CPU:
#ifdef VAR_CYCLES   
	CPU.MemSpeed = ONE_CYCLE;
	CPU.MemSpeedx2 = TWO_CYCLES;
#endif
	CPU.PCBase = Memory.FillRAM - 0x4000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
	
    case CMemory::MAP_DSP:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif	
	CPU.PCBase = Memory.FillRAM - 0x6000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
	
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
	CPU.PCBase = Memory.SRAM;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;

    case CMemory::MAP_BWRAM:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
	CPU.PCBase = Memory.BWRAM - 0x6000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
    case CMemory::MAP_HIROM_SRAM:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
	CPU.PCBase = Memory.SRAM - 0x6000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
//#ifndef __GP32__
    case CMemory::MAP_C4:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
	CPU.PCBase = Memory.C4RAM - 0x6000;
	CPU.PC = CPU.PCBase + (Address & 0xffff);
	return;
//#endif
    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("SBP %06x\n", Address);
#endif
	
    default:
    case CMemory::MAP_NONE:
#ifdef VAR_CYCLES
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
#endif
#ifdef DEBUGGER
	printf ("SBP %06x\n", Address);
#endif
	CPU.PCBase = Memory.SRAM;
	CPU.PC = Memory.SRAM + (Address & 0xffff);
	return;
    }
}
#endif
