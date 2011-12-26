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

#ifndef __GP32__ 
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#endif
#if defined(__unix) || defined(__linux) || defined(__sun) || defined(__DJGPP)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "snapshot.h"
//#include "snaporig.h"
#include "memmap.h"
#include "snes9x.h"
#include "65c816.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "apu.h"
#include "soundux.h"
#ifdef USE_SA1
#include "sa1.h"
#endif
#include "srtc.h"
#include "sdd1.h"


// notaz: file i/o function pointers for states,
// changing funcs will allow to enable/disable gzipped saves
extern int  (*statef_open)(const char *fname, const char *mode);
extern int  (*statef_read)(void *p, int l);
extern int  (*statef_write)(void *p, int l);
extern void (*statef_close)();

extern uint8 *SRAM;

#ifdef ZSNES_FX
START_EXTERN_C
void S9xSuperFXPreSaveState ();
void S9xSuperFXPostSaveState ();
void S9xSuperFXPostLoadState ();
END_EXTERN_C
#endif

//bool8 S9xUnfreezeZSNES (const char *filename);

typedef struct {
    int offset;
    int size;
    int type;
} FreezeData;

enum {
    INT_V, uint8_ARRAY_V, uint16_ARRAY_V, uint32_ARRAY_V
};

#define Offset(field,structure) \
	((int) (((char *) (&(((structure)NULL)->field))) - ((char *) NULL)))

#define COUNT(ARRAY) (sizeof (ARRAY) / sizeof (ARRAY[0]))

#undef OFFSET
#define OFFSET(f) Offset(f,struct SCPUState *)

static FreezeData SnapCPU [] = {
    {OFFSET (Flags), 4, INT_V},
    {OFFSET (BranchSkip), 1, INT_V},
    {OFFSET (NMIActive), 1, INT_V},
    {OFFSET (IRQActive), 1, INT_V},
    {OFFSET (WaitingForInterrupt), 1, INT_V},
    {OFFSET (WhichEvent), 1, INT_V},
    {OFFSET (Cycles), 4, INT_V},
    {OFFSET (NextEvent), 4, INT_V},
    {OFFSET (V_Counter), 4, INT_V},
    {OFFSET (MemSpeed), 4, INT_V},
    {OFFSET (MemSpeedx2), 4, INT_V},
    {OFFSET (FastROMSpeed), 4, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SRegisters *)

static FreezeData SnapRegisters [] = {
    {OFFSET (PB),  1, INT_V},
    {OFFSET (DB),  1, INT_V},
    {OFFSET (P.W), 2, INT_V},
    {OFFSET (A.W), 2, INT_V},
    {OFFSET (D.W), 2, INT_V},
    {OFFSET (S.W), 2, INT_V},
    {OFFSET (X.W), 2, INT_V},
    {OFFSET (Y.W), 2, INT_V},
    {OFFSET (PC),  2, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SPPU *)

static FreezeData SnapPPU [] = {
    {OFFSET (BGMode), 1, INT_V},
    {OFFSET (BG3Priority), 1, INT_V},
    {OFFSET (Brightness), 1, INT_V},
    {OFFSET (VMA.High), 1, INT_V},
    {OFFSET (VMA.Increment), 1, INT_V},
    {OFFSET (VMA.Address), 2, INT_V},
    {OFFSET (VMA.Mask1), 2, INT_V},
    {OFFSET (VMA.FullGraphicCount), 2, INT_V},
    {OFFSET (VMA.Shift), 2, INT_V},
    {OFFSET (BG[0].SCBase), 2, INT_V},
    {OFFSET (BG[0].VOffset), 2, INT_V},
    {OFFSET (BG[0].HOffset), 2, INT_V},
    {OFFSET (BG[0].BGSize), 1, INT_V},
    {OFFSET (BG[0].NameBase), 2, INT_V},
    {OFFSET (BG[0].SCSize), 2, INT_V},

    {OFFSET (BG[1].SCBase), 2, INT_V},
    {OFFSET (BG[1].VOffset), 2, INT_V},
    {OFFSET (BG[1].HOffset), 2, INT_V},
    {OFFSET (BG[1].BGSize), 1, INT_V},
    {OFFSET (BG[1].NameBase), 2, INT_V},
    {OFFSET (BG[1].SCSize), 2, INT_V},

    {OFFSET (BG[2].SCBase), 2, INT_V},
    {OFFSET (BG[2].VOffset), 2, INT_V},
    {OFFSET (BG[2].HOffset), 2, INT_V},
    {OFFSET (BG[2].BGSize), 1, INT_V},
    {OFFSET (BG[2].NameBase), 2, INT_V},
    {OFFSET (BG[2].SCSize), 2, INT_V},

    {OFFSET (BG[3].SCBase), 2, INT_V},
    {OFFSET (BG[3].VOffset), 2, INT_V},
    {OFFSET (BG[3].HOffset), 2, INT_V},
    {OFFSET (BG[3].BGSize), 1, INT_V},
    {OFFSET (BG[3].NameBase), 2, INT_V},
    {OFFSET (BG[3].SCSize), 2, INT_V},

    {OFFSET (CGFLIP), 1, INT_V},
    {OFFSET (CGDATA), 256, uint16_ARRAY_V},
    {OFFSET (FirstSprite), 1, INT_V},
#define O(N) \
    {OFFSET (OBJ[N].HPos), 2, INT_V}, \
    {OFFSET (OBJ[N].VPos), 2, INT_V}, \
    {OFFSET (OBJ[N].Name), 2, INT_V}, \
    {OFFSET (OBJ[N].VFlip), 1, INT_V}, \
    {OFFSET (OBJ[N].HFlip), 1, INT_V}, \
    {OFFSET (OBJ[N].Priority), 1, INT_V}, \
    {OFFSET (OBJ[N].Palette), 1, INT_V}, \
    {OFFSET (OBJ[N].Size), 1, INT_V}

    O(  0), O(  1), O(  2), O(  3), O(  4), O(  5), O(  6), O(  7),
    O(  8), O(  9), O( 10), O( 11), O( 12), O( 13), O( 14), O( 15),
    O( 16), O( 17), O( 18), O( 19), O( 20), O( 21), O( 22), O( 23),
    O( 24), O( 25), O( 26), O( 27), O( 28), O( 29), O( 30), O( 31),
    O( 32), O( 33), O( 34), O( 35), O( 36), O( 37), O( 38), O( 39),
    O( 40), O( 41), O( 42), O( 43), O( 44), O( 45), O( 46), O( 47),
    O( 48), O( 49), O( 50), O( 51), O( 52), O( 53), O( 54), O( 55),
    O( 56), O( 57), O( 58), O( 59), O( 60), O( 61), O( 62), O( 63),
    O( 64), O( 65), O( 66), O( 67), O( 68), O( 69), O( 70), O( 71),
    O( 72), O( 73), O( 74), O( 75), O( 76), O( 77), O( 78), O( 79),
    O( 80), O( 81), O( 82), O( 83), O( 84), O( 85), O( 86), O( 87),
    O( 88), O( 89), O( 90), O( 91), O( 92), O( 93), O( 94), O( 95),
    O( 96), O( 97), O( 98), O( 99), O(100), O(101), O(102), O(103),
    O(104), O(105), O(106), O(107), O(108), O(109), O(110), O(111),
    O(112), O(113), O(114), O(115), O(116), O(117), O(118), O(119),
    O(120), O(121), O(122), O(123), O(124), O(125), O(126), O(127),
#undef O
    {OFFSET (OAMPriorityRotation), 1, INT_V},
    {OFFSET (OAMAddr), 2, INT_V},
    {OFFSET (OAMFlip), 1, INT_V},
    {OFFSET (OAMTileAddress), 2, INT_V},
    {OFFSET (IRQVBeamPos), 2, INT_V},
    {OFFSET (IRQHBeamPos), 2, INT_V},
    {OFFSET (VBeamPosLatched), 2, INT_V},
    {OFFSET (HBeamPosLatched), 2, INT_V},
    {OFFSET (HBeamFlip), 1, INT_V},
    {OFFSET (VBeamFlip), 1, INT_V},
    {OFFSET (HVBeamCounterLatched), 1, INT_V},
    {OFFSET (MatrixA), 2, INT_V},
    {OFFSET (MatrixB), 2, INT_V},
    {OFFSET (MatrixC), 2, INT_V},
    {OFFSET (MatrixD), 2, INT_V},
    {OFFSET (CentreX), 2, INT_V},
    {OFFSET (CentreY), 2, INT_V},
    {OFFSET (Joypad1ButtonReadPos), 1, INT_V},
    {OFFSET (Joypad2ButtonReadPos), 1, INT_V},
    {OFFSET (Joypad3ButtonReadPos), 1, INT_V},
    {OFFSET (CGADD), 1, INT_V},
    {OFFSET (FixedColourRed), 1, INT_V},
    {OFFSET (FixedColourGreen), 1, INT_V},
    {OFFSET (FixedColourBlue), 1, INT_V},
    {OFFSET (SavedOAMAddr), 2, INT_V},
    {OFFSET (ScreenHeight), 2, INT_V},
    {OFFSET (WRAM), 4, INT_V},
    {OFFSET (ForcedBlanking), 1, INT_V},
    {OFFSET (OBJNameSelect), 2, INT_V},
    {OFFSET (OBJSizeSelect), 1, INT_V},
    {OFFSET (OBJNameBase), 2, INT_V},
    {OFFSET (OAMReadFlip), 1, INT_V},
    {OFFSET (VTimerEnabled), 1, INT_V},
    {OFFSET (HTimerEnabled), 1, INT_V},
    {OFFSET (HTimerPosition), 2, INT_V},
    {OFFSET (Mosaic), 1, INT_V},
    {OFFSET (Mode7HFlip), 1, INT_V},
    {OFFSET (Mode7VFlip), 1, INT_V},
    {OFFSET (Mode7Repeat), 1, INT_V},
    {OFFSET (Window1Left), 1, INT_V},
    {OFFSET (Window1Right), 1, INT_V},
    {OFFSET (Window2Left), 1, INT_V},
    {OFFSET (Window2Right), 1, INT_V},
#define O(N) \
    {OFFSET (ClipWindowOverlapLogic[N]), 1, INT_V}, \
    {OFFSET (ClipWindow1Enable[N]), 1, INT_V}, \
    {OFFSET (ClipWindow2Enable[N]), 1, INT_V}, \
    {OFFSET (ClipWindow1Inside[N]), 1, INT_V}, \
    {OFFSET (ClipWindow2Inside[N]), 1, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5),

#undef O

    {OFFSET (CGFLIPRead), 1, INT_V},
    {OFFSET (Need16x8Mulitply), 1, INT_V},
    {OFFSET (BGMosaic), 4, uint8_ARRAY_V},
    {OFFSET (OAMData), 512 + 32, uint8_ARRAY_V},
    {OFFSET (Need16x8Mulitply), 1, INT_V},
    {OFFSET (MouseSpeed), 2, uint8_ARRAY_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SDMA *)

static FreezeData SnapDMA [] = {
#define O(N) \
    {OFFSET (TransferDirection) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddressFixed) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddressDecrement) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (TransferMode) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (ABank) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (AAddress) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (Address) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (BAddress) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (TransferBytes) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (HDMAIndirectAddressing) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (IndirectAddress) + N * sizeof (struct SDMA), 2, INT_V}, \
    {OFFSET (IndirectBank) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (Repeat) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (LineCount) + N * sizeof (struct SDMA), 1, INT_V}, \
    {OFFSET (FirstLine) + N * sizeof (struct SDMA), 1, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7)
#undef O
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SAPU *)

static FreezeData SnapAPU [] = {
    {OFFSET (Cycles), 4, INT_V},
    {OFFSET (ShowROM), 1, INT_V},
    {OFFSET (Flags), 1, INT_V},
    {OFFSET (KeyedChannels), 1, INT_V},
    {OFFSET (OutPorts), 4, uint8_ARRAY_V},
    {OFFSET (DSP), 0x80, uint8_ARRAY_V},
    {OFFSET (ExtraRAM), 64, uint8_ARRAY_V},
    {OFFSET (Timer), 3, uint16_ARRAY_V},
    {OFFSET (TimerTarget), 3, uint16_ARRAY_V},
    {OFFSET (TimerEnabled), 3, uint8_ARRAY_V},
    {OFFSET (TimerValueWritten), 3, uint8_ARRAY_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SAPURegisters *)

static FreezeData SnapAPURegisters [] = {
    {OFFSET (P)   , 1, INT_V},
    {OFFSET (YA.W), 2, INT_V},
    {OFFSET (X)   , 1, INT_V},
    {OFFSET (S)   , 1, INT_V},
    {OFFSET (PC)  , 2, INT_V}
};

#undef OFFSET
#undef OFFSET1
#define OFFSET(f) Offset(f,SSoundData *)

static FreezeData SnapSoundData [] = {
    {OFFSET (master_volume_left), 2, INT_V},
    {OFFSET (master_volume_right), 2, INT_V},
    {OFFSET (echo_volume_left), 2, INT_V},
    {OFFSET (echo_volume_right), 2, INT_V},
    {OFFSET (echo_enable), 4, INT_V},
    {OFFSET (echo_feedback), 4, INT_V},
    {OFFSET (echo_ptr), 4, INT_V},
    {OFFSET (echo_buffer_size), 4, INT_V},
    {OFFSET (echo_write_enabled), 4, INT_V},
    {OFFSET (echo_channel_enable), 4, INT_V},
    {OFFSET (pitch_mod), 4, INT_V},
    {OFFSET (dummy), 3, uint32_ARRAY_V},
#define O(N) \
    {OFFSET (channels [N].state), 4, INT_V}, \
    {OFFSET (channels [N].type), 4, INT_V}, \
    {OFFSET (channels [N].volume_left), 2, INT_V}, \
    {OFFSET (channels [N].volume_right), 2, INT_V}, \
    {OFFSET (channels [N].hertz), 4, INT_V}, \
    {OFFSET (channels [N].count), 4, INT_V}, \
    {OFFSET (channels [N].loop), 1, INT_V}, \
    {OFFSET (channels [N].envx), 4, INT_V}, \
    {OFFSET (channels [N].left_vol_level), 2, INT_V}, \
    {OFFSET (channels [N].right_vol_level), 2, INT_V}, \
    {OFFSET (channels [N].envx_target), 2, INT_V}, \
    {OFFSET (channels [N].env_error), 4, INT_V}, \
    {OFFSET (channels [N].erate), 4, INT_V}, \
    {OFFSET (channels [N].direction), 4, INT_V}, \
    {OFFSET (channels [N].attack_rate), 4, INT_V}, \
    {OFFSET (channels [N].decay_rate), 4, INT_V}, \
    {OFFSET (channels [N].sustain_rate), 4, INT_V}, \
    {OFFSET (channels [N].release_rate), 4, INT_V}, \
    {OFFSET (channels [N].sustain_level), 4, INT_V}, \
    {OFFSET (channels [N].sample), 2, INT_V}, \
    {OFFSET (channels [N].decoded), 16, uint16_ARRAY_V}, \
    {OFFSET (channels [N].previous16), 2, uint16_ARRAY_V}, \
    {OFFSET (channels [N].sample_number), 2, INT_V}, \
    {OFFSET (channels [N].last_block), 1, INT_V}, \
    {OFFSET (channels [N].needs_decode), 1, INT_V}, \
    {OFFSET (channels [N].block_pointer), 4, INT_V}, \
    {OFFSET (channels [N].sample_pointer), 4, INT_V}, \
    {OFFSET (channels [N].mode), 4, INT_V}

    O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7)
#undef O
};

#ifdef USE_SA1

#undef OFFSET
#define OFFSET(f) Offset(f,struct SSA1Registers *)

static FreezeData SnapSA1Registers [] = {
    {OFFSET (PB),  1, INT_V},
    {OFFSET (DB),  1, INT_V},
    {OFFSET (P.W), 2, INT_V},
    {OFFSET (A.W), 2, INT_V},
    {OFFSET (D.W), 2, INT_V},
    {OFFSET (S.W), 2, INT_V},
    {OFFSET (X.W), 2, INT_V},
    {OFFSET (Y.W), 2, INT_V},
    {OFFSET (PC),  2, INT_V}
};

#undef OFFSET
#define OFFSET(f) Offset(f,struct SSA1 *)

static FreezeData SnapSA1 [] = {
    {OFFSET (Flags), 4, INT_V},
    {OFFSET (NMIActive), 1, INT_V},
    {OFFSET (IRQActive), 1, INT_V},
    {OFFSET (WaitingForInterrupt), 1, INT_V},
    {OFFSET (op1), 2, INT_V},
    {OFFSET (op2), 2, INT_V},
    {OFFSET (arithmetic_op), 4, INT_V},
    {OFFSET (sum), 8, INT_V},
    {OFFSET (overflow), 1, INT_V}
};
#endif

//static char ROMFilename [_MAX_PATH];
//static char SnapshotFilename [_MAX_PATH];

static void Freeze ();
static int Unfreeze ();
void FreezeStruct (char *name, void *base, FreezeData *fields,
		   int num_fields);
void FreezeBlock (char *name, uint8 *block, int size);

int UnfreezeStruct (char *name, void *base, FreezeData *fields,
		    int num_fields);
int UnfreezeBlock (char *name, uint8 *block, int size);

bool8 Snapshot (const char *filename)
{
    return (S9xFreezeGame (filename));
}

bool8 S9xFreezeGame (const char *filename)
{
    if(statef_open(filename, "wb"))
    {
		Freeze();
		statef_close();
		return (TRUE);
    }
    return (FALSE);
}


bool8 S9xUnfreezeGame (const char *filename)
{
    if(statef_open(filename, "rb"))
    {
		int result;
		if ((result = Unfreeze()) != SUCCESS)
		{
			switch (result)
			{
			case WRONG_FORMAT:
			S9xMessage (S9X_ERROR, S9X_WRONG_FORMAT, 
					"File not in Snes9x freeze format");
			S9xReset();
			break;
			case WRONG_VERSION:
			S9xMessage (S9X_ERROR, S9X_WRONG_VERSION,
					"Incompatable Snes9x freeze file format version");
			S9xReset();
			break;
			default:
			// should never happen
			break;
			}
			statef_close();
			return (FALSE);
		}
		statef_close();
		return (TRUE);
    }

    
    return (FALSE);
}

static void Freeze ()
{
    char buffer[1024];
    int i;

    S9xSetSoundMute (TRUE);
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPreSaveState ();
#endif

    S9xSRTCPreSaveState ();

    for (i = 0; i < 8; i++)
    {
	SoundData.channels [i].previous16 [0] = (int16) SoundData.channels [i].previous [0];
	SoundData.channels [i].previous16 [1] = (int16) SoundData.channels [i].previous [1];
    }
    sprintf (buffer, "%s:%04d\n", SNAPSHOT_MAGIC, SNAPSHOT_VERSION);
    statef_write(buffer, strlen (buffer));
    sprintf (buffer, "NAM:%06d:%s%c", strlen (Memory.ROMFilename) + 1,
	     Memory.ROMFilename, 0);
    statef_write(buffer, strlen (buffer) + 1);
    FreezeStruct ("CPU", &CPU, SnapCPU, COUNT (SnapCPU));
    FreezeStruct ("REG", &Registers, SnapRegisters, COUNT (SnapRegisters));
    FreezeStruct ("PPU", &PPU, SnapPPU, COUNT (SnapPPU));
    FreezeStruct ("DMA", DMA, SnapDMA, COUNT (SnapDMA));

// RAM and VRAM
    FreezeBlock ("VRA", Memory.VRAM, 0x10000);
    FreezeBlock ("RAM", Memory.RAM, 0x20000);
    FreezeBlock ("SRA", ::SRAM, 0x20000);
    FreezeBlock ("FIL", Memory.FillRAM, 0x8000);
    if (Settings.APUEnabled)
    {
// APU
	FreezeStruct ("APU", &APU, SnapAPU, COUNT (SnapAPU));
	// copy all SPC700 regs to savestate compatible struct
	SAPURegisters spcregs;
	spcregs.P  = IAPU.P;
	spcregs.YA.W = IAPU.YA.W;
	spcregs.X  = IAPU.X;
	spcregs.S  = IAPU.S;
	spcregs.PC = IAPU.PC - IAPU.RAM;
	FreezeStruct ("ARE", &spcregs, SnapAPURegisters,
		      COUNT (SnapAPURegisters));

	FreezeBlock  ("ARA", IAPU.RAM, 0x10000);
	FreezeStruct ("SOU", &SoundData, SnapSoundData,
		      COUNT (SnapSoundData));
    }
#ifdef USE_SA1
    if (Settings.SA1)
    {
	SA1Registers.PC = SA1.PC - SA1.PCBase;
	S9xSA1PackStatus ();
	FreezeStruct ("SA1", &SA1, SnapSA1, COUNT (SnapSA1));
	FreezeStruct ("SAR", &SA1Registers, SnapSA1Registers, 
		      COUNT (SnapSA1Registers));
    }
#endif
	S9xSetSoundMute (FALSE);
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPostSaveState ();
#endif
}

static int Unfreeze()
{
	// notaz: overflowing the damn Symbian stack again
    char buffer [16];
    char rom_filename [512];
    int result;

    int version;
    unsigned int len = strlen (SNAPSHOT_MAGIC) + 1 + 4 + 1;
    if (statef_read(buffer, len) != (int)len)
    {
		return (WRONG_FORMAT);
	}
    if (strncmp (buffer, SNAPSHOT_MAGIC, strlen (SNAPSHOT_MAGIC)) != 0)
    {
		return (WRONG_FORMAT);
	}
    if ((version = atoi (&buffer [strlen (SNAPSHOT_MAGIC) + 1])) > SNAPSHOT_VERSION)
	return (WRONG_VERSION);

    if ((result = UnfreezeBlock("NAM", (uint8 *) rom_filename, 512)) != SUCCESS)
	return (result);

    if (strcasecmp (rom_filename, Memory.ROMFilename) != 0 &&
	strcasecmp (S9xBasename (rom_filename), S9xBasename (Memory.ROMFilename)) != 0)
    {
	S9xMessage (S9X_WARNING, S9X_FREEZE_ROM_NAME,
		    "Current loaded ROM image doesn't match that required by freeze-game file.");
    }
    
    

    uint32 old_flags = CPU.Flags;
#ifdef USE_SA1
    uint32 sa1_old_flags = SA1.Flags;
#endif
	S9xReset ();
    S9xSetSoundMute (TRUE);

    if ((result = UnfreezeStruct("CPU", &CPU, SnapCPU, 
				  COUNT (SnapCPU))) != SUCCESS)
	return (result);
	
	
    Memory.FixROMSpeed ();
    CPU.Flags |= old_flags & (DEBUG_MODE_FLAG | TRACE_FLAG |
			      SINGLE_STEP_FLAG | FRAME_ADVANCE_FLAG);
    if ((result = UnfreezeStruct("REG", &Registers, SnapRegisters, COUNT (SnapRegisters))) != SUCCESS)
	return (result);
    if ((result = UnfreezeStruct("PPU", &PPU, SnapPPU, COUNT (SnapPPU))) != SUCCESS)
	return (result);
	

    IPPU.ColorsChanged = TRUE;
    IPPU.OBJChanged = TRUE;
    CPU.InDMA = FALSE;
    S9xFixColourBrightness ();
    IPPU.RenderThisFrame = FALSE;

    if ((result = UnfreezeStruct ("DMA", DMA, SnapDMA, 
				  COUNT (SnapDMA))) != SUCCESS)
	return (result);
	
    if ((result = UnfreezeBlock ("VRA", Memory.VRAM, 0x10000)) != SUCCESS)
	return (result);		
	
    if ((result = UnfreezeBlock ("RAM", Memory.RAM, 0x20000)) != SUCCESS)
	return (result);

    if ((result = UnfreezeBlock ("SRA", ::SRAM, 0x20000)) != SUCCESS)
	return (result);

    if ((result = UnfreezeBlock ("FIL", Memory.FillRAM, 0x8000)) != SUCCESS)
	return (result);

	
    if (UnfreezeStruct ("APU", &APU, SnapAPU, COUNT (SnapAPU)) == SUCCESS)
    {
		SAPURegisters spcregs;
		if ((result = UnfreezeStruct ("ARE", &spcregs, SnapAPURegisters,
				      COUNT (SnapAPURegisters))) != SUCCESS)
		    return (result);
		// reload all SPC700 regs from savestate compatible struct
		IAPU.P = spcregs.P;
		IAPU.YA.W = spcregs.YA.W;
		IAPU.X = spcregs.X;
		IAPU.S = spcregs.S;
		IAPU.PC = IAPU.RAM + spcregs.PC;

		if ((result = UnfreezeBlock ("ARA", IAPU.RAM, 0x10000)) != SUCCESS)
		    return (result);
		    
		if ((result = UnfreezeStruct ("SOU", &SoundData, SnapSoundData,
				      COUNT (SnapSoundData))) != SUCCESS)
		    return (result);
	    
	    // notaz: just to be sure
		for(int u=0; u<8; u++) {
			SoundData.channels[u].env_ind_attack &= 0xf;
			SoundData.channels[u].env_ind_decay  &= 0x7;
			SoundData.channels[u].env_ind_sustain&= 0x1f;
		}

		S9xSetSoundMute (FALSE);
		S9xAPUUnpackStatus ();
		if (APUCheckDirectPage ())
			IAPU.DirectPage = IAPU.RAM + 0x100;
		else
			IAPU.DirectPage = IAPU.RAM;
		Settings.APUEnabled = TRUE;
		/*IAPU.APUExecuting*/CPU.APU_APUExecuting = TRUE;
    }
    else
    {
	Settings.APUEnabled = FALSE;
	/*IAPU.APUExecuting*/CPU.APU_APUExecuting = FALSE;
	S9xSetSoundMute (TRUE);
    }
#ifdef USE_SA1
    if ((result = UnfreezeStruct ("SA1", &SA1, SnapSA1, 
				  COUNT(SnapSA1))) == SUCCESS)
    {
	if ((result = UnfreezeStruct ("SAR", &SA1Registers, 
				      SnapSA1Registers, COUNT (SnapSA1Registers))) != SUCCESS)
	    return (result);

	S9xFixSA1AfterSnapshotLoad ();
	SA1.Flags |= sa1_old_flags & (TRACE_FLAG);
    }
#endif
    S9xFixSoundAfterSnapshotLoad ();
    ICPU.ShiftedPB = Registers.PB << 16;
    ICPU.ShiftedDB = Registers.DB << 16;
    S9xSetPCBase (ICPU.ShiftedPB + Registers.PC);
    
    
    //S9xUnpackStatus (); // not needed
    //S9xFixCycles (); // also not needed?
    S9xReschedule ();
#ifdef ZSNES_FX
    if (Settings.SuperFX)
	S9xSuperFXPostLoadState ();
#endif

    S9xSRTCPostLoadState ();
    if (Settings.SDD1)	S9xSDD1PostLoadState ();
    
    return (SUCCESS);
}

int FreezeSize (int size, int type)
{
    switch (type)
    {
    case uint16_ARRAY_V:
	return (size * 2);
    case uint32_ARRAY_V:
	return (size * 4);
    default:
	return (size);
    }
}

void FreezeStruct(char *name, void *base, FreezeData *fields,
		   int num_fields)
{
    // Work out the size of the required block
    int len = 0;
    int i;
    int j;

    for (i = 0; i < num_fields; i++)
    {
	if (fields [i].offset + FreezeSize (fields [i].size, 
					    fields [i].type) > len)
	    len = fields [i].offset + FreezeSize (fields [i].size, 
						  fields [i].type);
    }

//    uint8 *block = new uint8 [len];
    uint8 *block = (uint8*)malloc(len);
    uint8 *ptr = block;
    uint16 word;
    uint32 dword;
    int64  qword;

    // Build the block ready to be streamed out
    for (i = 0; i < num_fields; i++)
    {
	switch (fields [i].type)
	{
	case INT_V:
	    switch (fields [i].size)
	    {
	    case 1:
		*ptr++ = *((uint8 *) base + fields [i].offset);
		break;
	    case 2:
		word = *((uint16 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (word >> 8);
		*ptr++ = (uint8) word;
		break;
	    case 4:
		dword = *((uint32 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (dword >> 24);
		*ptr++ = (uint8) (dword >> 16);
		*ptr++ = (uint8) (dword >> 8);
		*ptr++ = (uint8) dword;
		break;
	    case 8:
		qword = *((int64 *) ((uint8 *) base + fields [i].offset));
		*ptr++ = (uint8) (qword >> 56);
		*ptr++ = (uint8) (qword >> 48);
		*ptr++ = (uint8) (qword >> 40);
		*ptr++ = (uint8) (qword >> 32);
		*ptr++ = (uint8) (qword >> 24);
		*ptr++ = (uint8) (qword >> 16);
		*ptr++ = (uint8) (qword >> 8);
		*ptr++ = (uint8) qword;
		break;
	    }
	    break;
	case uint8_ARRAY_V:
	    memmove (ptr, (uint8 *) base + fields [i].offset, fields [i].size);
	    ptr += fields [i].size;
	    break;
	case uint16_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		word = *((uint16 *) ((uint8 *) base + fields [i].offset + j * 2));
		*ptr++ = (uint8) (word >> 8);
		*ptr++ = (uint8) word;
	    }
	    break;
	case uint32_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		dword = *((uint32 *) ((uint8 *) base + fields [i].offset + j * 4));
		*ptr++ = (uint8) (dword >> 24);
		*ptr++ = (uint8) (dword >> 16);
		*ptr++ = (uint8) (dword >> 8);
		*ptr++ = (uint8) dword;
	    }
	    break;
	}
    }

    FreezeBlock (name, block, len);

 	free(block);
}

void FreezeBlock (char *name, uint8 *block, int size)
{
    char buffer [512];
    sprintf (buffer, "%s:%06d:", name, size);
    statef_write(buffer, strlen (buffer));
    statef_write(block, size);
    
}

int UnfreezeStruct (char *name, void *base, FreezeData *fields,
		     int num_fields)
{
    // Work out the size of the required block
    int len = 0;
    int i;
    int j;

    for (i = 0; i < num_fields; i++)
    {
	if (fields [i].offset + FreezeSize (fields [i].size, 
					    fields [i].type) > len)
	    len = fields [i].offset + FreezeSize (fields [i].size, 
						  fields [i].type);
    }

	uint8 *block = (uint8*)malloc(len);
    uint8 *ptr = block;
    uint16 word;
    uint32 dword;
    int64  qword;
    int result;

    if ((result = UnfreezeBlock (name, block, len)) != SUCCESS)
    {
 	free(block);
	return (result);
    }

    // Unpack the block of data into a C structure
    for (i = 0; i < num_fields; i++)
    {
	switch (fields [i].type)
	{
	case INT_V:
	    switch (fields [i].size)
	    {
	    case 1:
		*((uint8 *) base + fields [i].offset) = *ptr++;
		break;
	    case 2:
		word  = *ptr++ << 8;
		word |= *ptr++;
		*((uint16 *) ((uint8 *) base + fields [i].offset)) = word;
		break;
	    case 4:
		dword  = *ptr++ << 24;
		dword |= *ptr++ << 16;
		dword |= *ptr++ << 8;
		dword |= *ptr++;
		*((uint32 *) ((uint8 *) base + fields [i].offset)) = dword;
		break;
	    case 8:
		qword  = (int64) *ptr++ << 56;
		qword |= (int64) *ptr++ << 48;
		qword |= (int64) *ptr++ << 40;
		qword |= (int64) *ptr++ << 32;
		qword |= (int64) *ptr++ << 24;
		qword |= (int64) *ptr++ << 16;
		qword |= (int64) *ptr++ << 8;
		qword |= (int64) *ptr++;
		*((int64 *) ((uint8 *) base + fields [i].offset)) = qword;
		break;
	    }
	    break;
	case uint8_ARRAY_V:
	    memmove ((uint8 *) base + fields [i].offset, ptr, fields [i].size);
	    ptr += fields [i].size;
	    break;
	case uint16_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		word  = *ptr++ << 8;
		word |= *ptr++;
		*((uint16 *) ((uint8 *) base + fields [i].offset + j * 2)) = word;
	    }
	    break;
	case uint32_ARRAY_V:
	    for (j = 0; j < fields [i].size; j++)
	    {
		dword  = *ptr++ << 24;
		dword |= *ptr++ << 16;
		dword |= *ptr++ << 8;
		dword |= *ptr++;
		*((uint32 *) ((uint8 *) base + fields [i].offset + j * 4)) = dword;
	    }
	    break;
	}
    }

//    delete block;
 	free(block);
    return (result);
}

int UnfreezeBlock(char *name, uint8 *block, int size)
{
    char buffer [20];
    int len = 0;
    int rem = 0;
    
    if (statef_read(buffer, 11) != 11 ||
	strncmp (buffer, name, 3) != 0 || buffer [3] != ':' ||
	(len = atoi (&buffer [4])) == 0)
    {
		return (WRONG_FORMAT);
    }
    
    if (len > size)
    {
		rem = len - size;
		len = size;
    }
    
    if (statef_read(block, len) != len)
    {
		return (WRONG_FORMAT);
	}
	
    if (rem)
    {
		char *junk = (char*)malloc(rem);
		statef_read(junk, rem);
		free(junk);
    }
	
    return (SUCCESS);
}


