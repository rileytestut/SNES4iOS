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
#ifndef _SNES9X_H_
#define _SNES9X_H_

//#define VERSION "1.39"

#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32__
#include "..\wsnes9x.h"
#include "..\zlib\zlib.h"
#endif

#include "port.h"
#include "65c816.h"
#include "messages.h"

#if defined(USE_GLIDE) && !defined(GFX_MULTI_FORMAT)
#define GFX_MULTI_FORMAT
#endif

#define ROM_NAME_LEN 23

#ifdef ZLIB
//#ifndef __WIN32__
#include "zlib.h"
//#endif
#define STREAM gzFile
#define READ_STREAM(p,l,s) gzread (s,p,l)
#define WRITE_STREAM(p,l,s) gzwrite (s,p,l)
#define OPEN_STREAM(f,m) gzopen (f,m)
#define CLOSE_STREAM(s) gzclose (s)
#define SEEK_STREAM(p,r,s) gzseek(s,p,r)
#else
#ifdef __GP32__
#define STREAM long * //F_HANDLE * 
#define READ_STREAM(p,l,s) gp32_fread ((unsigned char*)p,(long)l,s)
#define WRITE_STREAM(p,l,s) gp32_fwrite ((unsigned char*)p,(long)l,s)
#define OPEN_STREAM(f,m) gp32_fopen ((char*)f,(char*)m)
#define CLOSE_STREAM(s) gp32_fclose (s)
#define SEEK_STREAM(p,r,s) gp32_fseek(p,r,s)

#else
#define STREAM FILE *
#define READ_STREAM(p,l,s) fread (p,1,l,s)
#define WRITE_STREAM(p,l,s) fwrite (p,1,l,s)
#define OPEN_STREAM(f,m) fopen (f,m)
#define CLOSE_STREAM(s) fclose (s)
#define SEEK_STREAM(p,r,s) fseek(s,p,r)
#define FROM_CURRENT SEEK_CUR
#endif
#endif


/* SNES screen width and height */
#define SNES_WIDTH		256
#define SNES_HEIGHT		224
#define SNES_HEIGHT_EXTENDED	239
#define IMAGE_WIDTH		(Settings.SupportHiRes ? SNES_WIDTH * 2 : SNES_WIDTH)
#define IMAGE_HEIGHT		(Settings.SupportHiRes ? SNES_HEIGHT_EXTENDED * 2 : SNES_HEIGHT_EXTENDED)

#define SNES_MAX_NTSC_VCOUNTER  262
#define SNES_MAX_PAL_VCOUNTER   312
#define SNES_HCOUNTER_MAX	342
#define SPC700_TO_65C816_RATIO	2
#define AUTO_FRAMERATE		200

// NTSC master clock signal 21.47727MHz
// PPU: master clock / 4
// 1 / PPU clock * 342 -> 63.695us
// 63.695us / (1 / 3.579545MHz) -> 228 cycles per scanline
// From Earth Worm Jim: APU executes an average of 65.14285714 cycles per
// scanline giving an APU clock speed of 1.022731096MHz

// PAL master clock signal 21.28137MHz
// PPU: master clock / 4
// 1 / PPU clock * 342 -> 64.281us
// 64.281us / (1 / 3.546895MHz) -> 228 cycles per scanline.

//#define SNES_SCANLINE_TIME (63.695e-6)
//#define SNES_CLOCK_SPEED (3579545)

//#define SNES_CLOCK_LEN (1.0 / SNES_CLOCK_SPEED)

#ifdef VAR_CYCLES
//#define SNES_CYCLES_PER_SCANLINE ((uint32) ((SNES_SCANLINE_TIME / SNES_CLOCK_LEN) * 6 + 0.5))
#define SNES_CYCLES_PER_SCANLINE ((uint32)(228*6))
#else
#define SNES_CYCLES_PER_SCANLINE ((uint32) (SNES_SCANLINE_TIME / SNES_CLOCK_LEN + 0.5))
#endif

#define SNES_TR_MASK	    (1 << 4)
#define SNES_TL_MASK	    (1 << 5)
#define SNES_X_MASK	    (1 << 6)
#define SNES_A_MASK	    (1 << 7)
#define SNES_RIGHT_MASK	    (1 << 8)
#define SNES_LEFT_MASK	    (1 << 9)
#define SNES_DOWN_MASK	    (1 << 10)
#define SNES_UP_MASK	    (1 << 11)
#define SNES_START_MASK	    (1 << 12)
#define SNES_SELECT_MASK    (1 << 13)
#define SNES_Y_MASK	    (1 << 14)
#define SNES_B_MASK	    (1 << 15)

enum {
    SNES_MULTIPLAYER5,
    SNES_JOYPAD,
    SNES_MOUSE_SWAPPED,
    SNES_MOUSE,
    SNES_SUPERSCOPE,
	SNES_JUSTIFIER,
	SNES_JUSTIFIER_2,
    SNES_MAX_CONTROLLER_OPTIONS
};

#define DEBUG_MODE_FLAG	    (1 << 0)
#define TRACE_FLAG			(1 << 1)
#define SINGLE_STEP_FLAG    (1 << 2)
#define BREAK_FLAG			(1 << 3)
#define SCAN_KEYS_FLAG	    (1 << 4)
#define SAVE_SNAPSHOT_FLAG  (1 << 5)
#define DELAYED_NMI_FLAG    (1 << 6)
#define NMI_FLAG			(1 << 7)
#define PROCESS_SOUND_FLAG  (1 << 8)
#define FRAME_ADVANCE_FLAG  (1 << 9)
#define DELAYED_NMI_FLAG2   (1 << 10)
#define IRQ_PENDING_FLAG    (1 << 11)

#ifdef VAR_CYCLES
#define ONE_CYCLE 6
#define SLOW_ONE_CYCLE 8
#define TWO_CYCLES 12
#else
#define ONE_CYCLE 1
#define SLOW_ONE_CYCLE 1
#define TWO_CYCLES 2
#endif


#undef MEMMAP_BLOCK_SIZE
#define MEMMAP_BLOCK_SIZE (0x1000)
#undef MEMMAP_NUM_BLOCKS
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)

struct SCPUState{
    uint32  Flags;					//0
    bool8   BranchSkip;				//4
    bool8   NMIActive;				//5
    bool8   IRQActive;				//6
    bool8   WaitingForInterrupt;	//7
    struct SRegisters Regs;			//8
		//uint8  PB;				//8
		//uint8  DB;				//9
		//pair   P;					//10
		//pair   A;					//12
		//pair   D;					//14
		//pair   X;					//16
		//pair   S;					//18
		//pair   Y;					//20
	    //uint16 PC;				//22
    uint8   *PC;					//24
    int32   Cycles;					//28
    uint8   *PCBase;				//32
    uint8   *PCAtOpcodeStart;		//36
    uint8   *WaitAddress;			//40
    uint32  WaitCounter;			//44
    int32   NextEvent;				//48
    int32   V_Counter;				//52
    int32   MemSpeed;				//56
    int32   MemSpeedx2;				//60
	int32   FastROMSpeed;			//64
    uint32 AutoSaveTimer;    		//68
    uint32 NMITriggerPoint;    		//72
    uint32 NMICycleCount;			//76
    uint32 IRQCycleCount;			//80

    bool8   InDMA;					//84
    uint8   WhichEvent;				//85
	bool8  SRAMModified;			//86
	bool8  BRKTriggered;			//87
	uint32	_ARM_asm_reserved_1;	//88  to stock current jmp table
    bool8  TriedInterleavedMode2;	//92
    bool8  _ARM_asm_padding1[3];	//93
    
    uint8*	Memory_Map;				//96
    uint8*	Memory_WriteMap;		//100
    uint8*	Memory_MemorySpeed;		//104
    uint8*	Memory_BlockIsRAM;		//108
    uint8*	Memory_SRAM;			//112
    uint8*	Memory_BWRAM;			//116
    uint16	Memory_SRAMMask;		//120
    bool8	APU_APUExecuting;		//122
    bool8	_ARM_asm_padding2;		//123
    uint32	_PALMSOS_R9;			//124
    uint32	_PALMSOS_R10;    		//128
  	int32	APU_Cycles;				//132 notaz
    uint32	jump1;			//136
    uint32	jump2;    		//140
    uint32	jump3;    		//144
    uint32	jump4;    		//148
};


#define HBLANK_START_EVENT 0
#define HBLANK_END_EVENT 1
#define HTIMER_BEFORE_EVENT 2
#define HTIMER_AFTER_EVENT 3
#define NO_EVENT 4

struct SSettings{
    // CPU options
    bool8  APUEnabled;
    bool8  Shutdown;
    uint8  SoundSkipMethod;
    long   H_Max;
    long   HBlankStart;
    long   CyclesPercentage;
    bool8  DisableIRQ;
    bool8  Paused;
    bool8  ForcedPause;
    bool8  StopEmulation;

    // Tracing options
    bool8  TraceDMA;
    bool8  TraceHDMA;
    bool8  TraceVRAM;
    bool8  TraceUnknownRegisters;
    bool8  TraceDSP;

    // Joystick options
    bool8  SwapJoypads;
    bool8  JoystickEnabled;

    // ROM timing options (see also H_Max above)
    bool8  ForcePAL;
    bool8  ForceNTSC;
    bool8  PAL;
    uint32 FrameTimePAL;
    uint32 FrameTimeNTSC;
    uint32 FrameTime;
    uint32 SkipFrames;

    // ROM image options
    bool8  ForceLoROM;
    bool8  ForceHiROM;
    bool8  ForceHeader;
    bool8  ForceNoHeader;
    bool8  ForceInterleaved;
    bool8  ForceInterleaved2;
    bool8  ForceNotInterleaved;

    // Peripherial options
    bool8  ForceSuperFX;
    bool8  ForceNoSuperFX;
    bool8  ForceDSP1;
    bool8  ForceNoDSP1;
    bool8  ForceSA1;
    bool8  ForceNoSA1;
    bool8  ForceC4;
    bool8  ForceNoC4;
    bool8  ForceSDD1;
    bool8  ForceNoSDD1;
    bool8  MultiPlayer5;
    bool8  Mouse;
    bool8  SuperScope;
    bool8  SRTC;
    uint32 ControllerOption;
    
    bool8  ShutdownMaster;
    bool8  MultiPlayer5Master;
    bool8  SuperScopeMaster;
    bool8  MouseMaster;
    bool8  SuperFX;
    bool8  DSP1Master;
    bool8  SA1;
    bool8  C4;
    bool8  SDD1;

    // Sound options
    uint32 SoundPlaybackRate;
    bool8  TraceSoundDSP;
    bool8  Stereo;
    bool8  ReverseStereo;
    bool8  SixteenBitSound;
    int    SoundBufferSize;
    int    SoundMixInterval;
    bool8  SoundEnvelopeHeightReading;
    bool8  DisableSoundEcho;
    bool8  DisableSampleCaching;
    bool8  DisableMasterVolume;
    bool8  SoundSync;
    bool8  InterpolatedSound;
    bool8  ThreadSound;
    bool8  Mute;
//    bool8  NextAPUEnabled;
    uint8  AltSampleDecode;
    bool8  FixFrequency;
    
    // Graphics options
    bool8  SixteenBit;
    bool8  Transparency;
    bool8  SupportHiRes;
    bool8  Mode7Interpolate;

    // SNES graphics options
    bool8  BGLayering;
    bool8  DisableGraphicWindows;
    bool8  ForceTransparency;
    bool8  ForceNoTransparency;
    bool8  DisableHDMA;
    bool8  DisplayFrameRate;

    // Others
    bool8  NetPlay;
    bool8  NetPlayServer;
    char   ServerName [128];
    int    Port;
    bool8  GlideEnable;
    bool8  OpenGLEnable;
    int32  AutoSaveDelay; // Time in seconds before S-RAM auto-saved if modified.
    bool8  ApplyCheats;
    bool8  TurboMode;
    uint32 TurboSkipFrames;
    uint32 AutoMaxSkipFrames;
    
// Fixes for individual games
    uint32 StrikeGunnerOffsetHack;
    bool8  ChuckRock;
    bool8  StarfoxHack;
    bool8  WinterGold;
    bool8  Dezaemon;
    bool8  WrestlemaniaArcade;
    bool8  BS;	// Japanese Satellite System games.
    bool8  DaffyDuck;
    uint8  APURAMInitialValue;
    bool8  SDD1Pack;
    
	// notaz
	uint32 GfxLayerMask;
#ifdef __WIN32__
    int    SoundDriver;
#endif
};

struct SSNESGameFixes
{
    uint8 NeedInit0x2137;
    uint8 umiharakawaseFix;
    uint8 alienVSpredetorFix;
    uint8 APU_OutPorts_ReturnValueFix;
    uint8 Old_Read0x4200;
    uint8 _0x213E_ReturnValue;
    uint8 TouhaidenControllerFix;
    uint8 SoundEnvelopeHeightReading2;
    uint8 SRAMInitialValue;
};

START_EXTERN_C
extern struct SSettings Settings;
extern struct SCPUState CPU;
extern struct SSNESGameFixes SNESGameFixes;
extern char String [513];

void S9xExit ();
void S9xMessage (int type, int number, const char *message);
void S9xLoadSDD1Data ();
END_EXTERN_C

enum {
    PAUSE_NETPLAY_CONNECT = (1 << 0),
    PAUSE_TOGGLE_FULL_SCREEN = (1 << 1),
    PAUSE_EXIT = (1 << 2),
    PAUSE_MENU = (1 << 3),
    PAUSE_INACTIVE_WINDOW = (1 << 4),
    PAUSE_WINDOW_ICONISED = (1 << 5),
    PAUSE_RESTORE_GUI = (1 << 6),
    PAUSE_FREEZE_FILE = (1 << 7)
};
void S9xSetPause (uint32 mask);
void S9xClearPause (uint32 mask);

#endif
