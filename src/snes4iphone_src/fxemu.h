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
#ifndef _FXEMU_H_
#define _FXEMU_H_ 1

/* Types used by structures and code */
#ifndef snes9x_types_defined
#define snes9x_types_defined

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned char bool8;
typedef signed char int8;
typedef short int16;
typedef int int32;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* The FxInfo_s structure, the link between the FxEmulator and the Snes Emulator */
struct FxInit_s
{
    uint32	vFlags;
    uint8 *	pvRegisters;	/* 768 bytes located in the memory at address 0x3000 */
    uint32	nRamBanks;	/* Number of 64kb-banks in GSU-RAM/BackupRAM (banks 0x70-0x73) */
    uint8 *	pvRam;		/* Pointer to GSU-RAM */
    uint32	nRomBanks;	/* Number of 32kb-banks in Cart-ROM */
    uint8 *	pvRom;		/* Pointer to Cart-ROM */
};

/* Reset the FxChip */
extern void FxReset(struct FxInit_s *psFxInfo);

/* Execute until the next stop instruction */
extern int FxEmulate(uint32 nInstructions);

/* Write access to the cache */
extern void FxCacheWriteAccess(uint16 vAddress);
extern void FxFlushCache();	/* Callled when the G flag in SFR is set to zero */

/* Breakpoint */
extern void FxBreakPointSet(uint32 vAddress);
extern void FxBreakPointClear();

/* Step by step execution */
extern int FxStepOver(uint32 nInstructions);

/* Errors */
extern int FxGetErrorCode();
extern int FxGetIllegalAddress();

/* Access to internal registers */
extern uint32 FxGetColorRegister();
extern uint32 FxGetPlotOptionRegister();
extern uint32 FxGetSourceRegisterIndex();
extern uint32 FxGetDestinationRegisterIndex();

/* Get string for opcode currently in the pipe */
extern void FxPipeString(char * pvString);

/* Get the byte currently in the pipe */
extern uint8 FxPipe();

/* Option flags */
#define FX_FLAG_ADDRESS_CHECKING	0x01
#define FX_FLAG_ROM_BUFFER		0x02

/* Return codes from FxEmulate(), FxStepInto() or FxStepOver() */
#define FX_BREAKPOINT			-1
#define FX_ERROR_ILLEGAL_ADDRESS	-2

/* Return the number of bytes in an opcode */
#define OPCODE_BYTES(op) ((((op)>=0x05&&(op)<=0xf)||((op)>=0xa0&&(op)<=0xaf))?2:(((op)>=0xf0)?3:1))

extern void fx_computeScreenPointers ();

#endif
