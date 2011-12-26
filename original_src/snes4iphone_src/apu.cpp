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
#include "spc700.h"
#include "apu.h"
#include "soundux.h"
#include "cpuexec.h"

/* For note-triggered SPC dump support */
//#include "snapshot.h"

//extern int NoiseFreq [32];
#ifdef DEBUGGER
void S9xTraceSoundDSP (const char *s, int i1 = 0, int i2 = 0, int i3 = 0,
		       int i4 = 0, int i5 = 0, int i6 = 0, int i7 = 0);
#endif

#undef ABS
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define ENVX_SHIFT 24


unsigned long AttackRate [16] = {
	4100, 2600, 1500, 1000, 640, 380, 260, 160,
	96, 64, 40, 24, 16, 10, 6, 1
};

unsigned long DecayRate [8] = {
	1200, 740, 440, 290, 180, 110, 74, 37
};

unsigned long SustainRate [32] = {
	/*~0*/0xFFFFFFFF, 38000, 28000, 24000, 19000, 14000, 12000, 9400,
	7100, 5900, 4700, 3500, 2900, 2400, 1800, 1500,
	1200, 880, 740, 590, 440, 370, 290, 220,
	180, 150, 110, 92, 74, 55, 37, 18
};
	
unsigned long IncreaseRate [32] = {
	/*~0*/0xFFFFFFFF, 4100, 3100, 2600, 2000, 1500, 1300, 1000,
	770, 640, 510, 380, 320, 260, 190, 160,
	130, 96, 80, 64, 48, 40, 32, 24,
	20, 16, 12, 10, 8, 6, 4, 2
};

unsigned long DecreaseRateExp [32] = {
	/*~0*/0xFFFFFFFF, 38000, 28000, 24000, 19000, 14000, 12000, 9400,
	7100, 5900, 4700, 3500, 2900, 2400, 1800, 1500,
	1200, 880, 740, 590, 440, 370, 290, 220,
	180, 150, 110, 92, 74, 55, 37, 18
};	

// precalculated env rates for S9xSetEnvRate
unsigned long AttackERate     [16][10];
unsigned long DecayERate       [8][10];
unsigned long SustainERate    [32][10];
unsigned long IncreaseERate   [32][10];
unsigned long DecreaseERateExp[32][10];
unsigned long KeyOffERate[10];


static inline void S9xSetEnvelopeRate (int channel, unsigned long rate, int direction, int target, unsigned int mode)
{
    S9xSetEnvRate (&SoundData.channels [channel], rate, direction, target, mode);
}

static inline void S9xSetSoundADSR (int channel, int attack_ind, int decay_ind,
		      int sustain_ind, int sustain_level, int release_rate)
{
	int attack_rate = AttackRate [attack_ind];
	int decay_rate = DecayRate [decay_ind];
	int sustain_rate = SustainRate [sustain_ind];
	
	// Hack for ROMs that use a very short attack rate, key on a 
	// channel, then switch to decay mode. e.g. Final Fantasy II.
	if (attack_rate == 1)
		attack_rate = 0;

	SoundData.channels[channel].env_ind_attack = attack_ind;
    SoundData.channels[channel].env_ind_decay = decay_ind;
    SoundData.channels[channel].env_ind_sustain = sustain_ind;

	SoundData.channels[channel].attack_rate = attack_rate;
    SoundData.channels[channel].decay_rate = decay_rate;
    SoundData.channels[channel].sustain_rate = sustain_rate;
    SoundData.channels[channel].release_rate = release_rate;
    SoundData.channels[channel].sustain_level = sustain_level + 1;

    switch (SoundData.channels[channel].state)
    {
    case SOUND_ATTACK:
	S9xSetEnvelopeRate (channel, attack_rate, 1, 127, 0);
	break;

    case SOUND_DECAY:
	S9xSetEnvelopeRate (channel, decay_rate, -1,
			    (MAX_ENVELOPE_HEIGHT * (sustain_level + 1)) >> 3, 1<<28);
	break;
    case SOUND_SUSTAIN:
	S9xSetEnvelopeRate (channel, sustain_rate, -1, 0, 2<<28);
	break;
    }
}

static inline void S9xSetSoundVolume (int channel, short volume_left, short volume_right)
{
    Channel *ch = &SoundData.channels[channel];
    if (!so.stereo)
	volume_left = (ABS(volume_right) + ABS(volume_left)) / 2;

    ch->volume_left = volume_left;
    ch->volume_right = volume_right;
    ch-> left_vol_level = (ch->envx * volume_left) / 128;
    ch->right_vol_level = (ch->envx * volume_right) / 128;
}

static inline void S9xSetMasterVolume (short volume_left, short volume_right)
{
    if (Settings.DisableMasterVolume)
    {
	SoundData.master_volume_left = 127;
	SoundData.master_volume_right = 127;
	SoundData.master_volume [0] = SoundData.master_volume [1] = 127;
    }
    else
    {
	if (!so.stereo)
	    volume_left = (ABS (volume_right) + ABS (volume_left)) / 2;
	SoundData.master_volume_left = volume_left;
	SoundData.master_volume_right = volume_right;
	SoundData.master_volume [0] = volume_left;
	SoundData.master_volume [1] = volume_right;
    }
}

static inline void S9xSetEchoVolume (short volume_left, short volume_right)
{
    if (!so.stereo)
	volume_left = (ABS (volume_right) + ABS (volume_left)) / 2;
    SoundData.echo_volume_left = volume_left;
    SoundData.echo_volume_right = volume_right;
    SoundData.echo_volume [0] = volume_left;
    SoundData.echo_volume [1] = volume_right;
}

static inline void S9xSetEchoWriteEnable (uint8 byte)
{
    SoundData.echo_write_enabled = byte;
    S9xSetEchoDelay (APU.DSP [APU_EDL] & 15);
}

static inline void S9xSetFrequencyModulationEnable (uint8 byte)
{
    SoundData.pitch_mod = byte & (0xFE);//~1;
}

static inline int S9xGetEnvelopeHeight (int channel)
{
    if ((Settings.SoundEnvelopeHeightReading ||
	 SNESGameFixes.SoundEnvelopeHeightReading2) &&
        SoundData.channels[channel].state != SOUND_SILENT &&
        SoundData.channels[channel].state != SOUND_GAIN)
    {
        return (SoundData.channels[channel].envx);
    }

    //siren fix from XPP
    if (SNESGameFixes.SoundEnvelopeHeightReading2 &&
        SoundData.channels[channel].state != SOUND_SILENT)
    {
        return (SoundData.channels[channel].envx);
    }

    return (0);
}

static inline void S9xSetSoundHertz (int channel, int hertz)
{
    SoundData.channels[channel].hertz = hertz;
    S9xSetSoundFrequency (channel, hertz);
}

static inline void S9xSetSoundType (int channel, int type_of_sound)
{
    SoundData.channels[channel].type = type_of_sound;
}

static inline bool8 S9xSetSoundMode (int channel, int mode)
{
    Channel *ch = &SoundData.channels[channel];

    switch (mode)
    {
    case MODE_RELEASE:
	if (ch->mode != MODE_NONE)
	{
	    ch->mode = MODE_RELEASE;
	    return (TRUE);
	}
	break;
	
    case MODE_DECREASE_LINEAR:
    case MODE_DECREASE_EXPONENTIAL:
    case MODE_GAIN:
	if (ch->mode != MODE_RELEASE)
	{
	    ch->mode = mode;
	    if (ch->state != SOUND_SILENT)
		ch->state = mode;

	    return (TRUE);
	}
	break;

    case MODE_INCREASE_LINEAR:
    case MODE_INCREASE_BENT_LINE:
	if (ch->mode != MODE_RELEASE)
	{
	    ch->mode = mode;
	    if (ch->state != SOUND_SILENT)
		ch->state = mode;

	    return (TRUE);
	}
	break;

    case MODE_ADSR:
	if (ch->mode == MODE_NONE || ch->mode == MODE_ADSR)
	{
	    ch->mode = mode;
	    return (TRUE);
	}
    }

    return (FALSE);
}

static inline void S9xPlaySample (int channel)
{
    Channel *ch = &SoundData.channels[channel];
    
    ch->state = SOUND_SILENT;
    ch->mode = MODE_NONE;
    ch->envx = 0;
    ch->envxx = 0;

	ch->g_index=0;
	ch->gaussian[0]=ch->gaussian[1]=ch->gaussian[2]=ch->gaussian[3]=0;

    S9xFixEnvelope (channel,
		    APU.DSP [APU_GAIN  + (channel << 4)], 
		    APU.DSP [APU_ADSR1 + (channel << 4)],
		    APU.DSP [APU_ADSR2 + (channel << 4)]);

    ch->sample_number = APU.DSP [APU_SRCN + channel * 0x10];
    if (APU.DSP [APU_NON] & (1 << channel))
	ch->type = SOUND_NOISE;
    else
	ch->type = SOUND_SAMPLE;

    S9xSetSoundFrequency (channel, ch->hertz);
    ch->loop = FALSE;
    ch->needs_decode = TRUE;
    ch->last_block = FALSE;
    ch->previous [0] = ch->previous[1] = 0;
    ch->block_pointer = *S9xGetSampleAddress(ch->sample_number);
    ch->sample_pointer = 0;
    ch->env_error = 0;
    ch->next_sample = 0;
    ch->interpolate = 0;
    ch->last_valid_header=0;
    switch (ch->mode)
    {
    case MODE_ADSR:
	if (ch->attack_rate == 0)
	{
	    if (ch->decay_rate == 0 || ch->sustain_level == 8)
	    {
		ch->state = SOUND_SUSTAIN;
		ch->envx = (MAX_ENVELOPE_HEIGHT * ch->sustain_level) >> 3;
		S9xSetEnvRate (ch, ch->sustain_rate, -1, 0, 2<<28);
	    }
	    else
	    {
		ch->state = SOUND_DECAY;
		ch->envx = MAX_ENVELOPE_HEIGHT;
		S9xSetEnvRate (ch, ch->decay_rate, -1, 
				    (MAX_ENVELOPE_HEIGHT * ch->sustain_level) >> 3, 1<<28);
	    }
	    ch-> left_vol_level = (ch->envx * ch->volume_left) / 128;
	    ch->right_vol_level = (ch->envx * ch->volume_right) / 128;
	}
	else
	{
	    ch->state = SOUND_ATTACK;
	    ch->envx = 0;
	    ch->left_vol_level = 0;
	    ch->right_vol_level = 0;
	    S9xSetEnvRate (ch, ch->attack_rate, 1, MAX_ENVELOPE_HEIGHT, 0);
	}
	ch->envxx = ch->envx << ENVX_SHIFT;
	break;

    case MODE_GAIN:
	ch->state = SOUND_GAIN;
	break;

    case MODE_INCREASE_LINEAR:
	ch->state = SOUND_INCREASE_LINEAR;
	break;

    case MODE_INCREASE_BENT_LINE:
	ch->state = SOUND_INCREASE_BENT_LINE;
	break;

    case MODE_DECREASE_LINEAR:
	ch->state = SOUND_DECREASE_LINEAR;
	break;

    case MODE_DECREASE_EXPONENTIAL:
	ch->state = SOUND_DECREASE_EXPONENTIAL;
	break;

    default:
	break;
    }

    S9xFixEnvelope (channel,
		    APU.DSP [APU_GAIN  + (channel << 4)], 
		    APU.DSP [APU_ADSR1 + (channel << 4)],
		    APU.DSP [APU_ADSR2 + (channel << 4)]);
}

#ifdef ASM_SPC700
extern "C" uint32 Spc700JumpTab;
#endif

bool8 S9xInitAPU ()
{
	// notaz
	memset(&IAPU, 0, sizeof(IAPU));
	IAPU.ExtraRAM = APU.ExtraRAM;
#ifdef ASM_SPC700
	IAPU.asmJumpTab = &Spc700JumpTab;
#endif

	IAPU.RAM = (uint8 *) malloc (0x10000);
    IAPU.ShadowRAM = NULL;//(uint8 *) malloc (0x10000);
    IAPU.CachedSamples = NULL;//(uint8 *) malloc (0x40000);
    
    if (!IAPU.RAM /*|| !IAPU.ShadowRAM || !IAPU.CachedSamples*/)
    {
	S9xDeinitAPU ();
	return (FALSE);
    }

    return (TRUE);
}

void S9xDeinitAPU ()
{
    if (IAPU.RAM)
    {
	free ((char *) IAPU.RAM);
	IAPU.RAM = NULL;
    }
    if (IAPU.ShadowRAM)
    {
	free ((char *) IAPU.ShadowRAM);
	IAPU.ShadowRAM = NULL;
    }
    if (IAPU.CachedSamples)
    {
	free ((char *) IAPU.CachedSamples);
	IAPU.CachedSamples = NULL;
    }
}

EXTERN_C uint8 APUROM [64];

void S9xResetAPU ()
{
//    Settings.APUEnabled = Settings.NextAPUEnabled;

    memset (IAPU.RAM, Settings.APURAMInitialValue, 0x10000);
    //memset (IAPU.ShadowRAM, Settings.APURAMInitialValue, 0x10000);
    
    //ZeroMemory (IAPU.CachedSamples, 0x40000);
    ZeroMemory (APU.OutPorts, 4);
    IAPU.DirectPage = IAPU.RAM;
    memmove (&IAPU.RAM [0xffc0], APUROM, sizeof (APUROM));
    memmove (APU.ExtraRAM, APUROM, sizeof (APUROM));
    IAPU.PC = IAPU.RAM + IAPU.RAM [0xfffe] + (IAPU.RAM [0xffff] << 8);
    CPU.APU_Cycles = 0;
    IAPU.YA.W = 0;
    IAPU.X = 0;
    IAPU.S = 0xff;
    IAPU.P = 0;
    S9xAPUUnpackStatus ();
    CPU.APU_APUExecuting = Settings.APUEnabled;
#ifdef SPC700_SHUTDOWN
    IAPU.WaitAddress1 = NULL;
    IAPU.WaitAddress2 = NULL;
    IAPU.WaitCounter = 0;
#endif
    APU.ShowROM = TRUE;
    IAPU.RAM [0xf1] = 0x80;

    int i;

    for (i = 0; i < 3; i++)
    {
	APU.TimerEnabled [i] = FALSE;
	APU.TimerValueWritten [i] = 0;
	APU.TimerTarget [i] = 0;
	APU.Timer [i] = 0;
    }
    for (int j = 0; j < 0x80; j++)
	APU.DSP [j] = 0;

    IAPU.TwoCycles = IAPU.OneCycle * 2;

    for (i = 0; i < 256; i++)
	S9xAPUCycles [i] = S9xAPUCycleLengths [i] * IAPU.OneCycle;

    APU.DSP [APU_ENDX] = 0;
    APU.DSP [APU_KOFF] = 0;
    APU.DSP [APU_KON] = 0;
    APU.DSP [APU_FLG] = APU_MUTE | APU_ECHO_DISABLED;
    APU.KeyedChannels = 0;

    S9xResetSound (TRUE);
    S9xSetEchoEnable (0);
}

extern int framecpto;
void S9xSetAPUDSP (uint8 byte)
{
    uint8 reg = IAPU.RAM [0xf2];
	static uint8 KeyOn;
	static uint8 KeyOnPrev;
    int i;
    
/*    char str[64];
    if (byte!=0)
    {
		sprintf(str,"fr : %d\nwrite dsp %d\ncpu cycle=%d pc=%04X",framecpto,byte,CPU.Cycles,CPU.PC-CPU.PCBase);
		S9xMessage(0,0,str);
		gp32_pause();
	}*/

	//extern uint8 spc_dump_dsp[0x100];

	//spc_dump_dsp[reg] = byte;

    switch (reg)
    {
    case APU_FLG:
	if (byte & APU_SOFT_RESET)
	{
	    APU.DSP [reg] = APU_MUTE | APU_ECHO_DISABLED | (byte & 0x1f);
	    APU.DSP [APU_ENDX] = 0;
	    APU.DSP [APU_KOFF] = 0;
	    APU.DSP [APU_KON] = 0;
	    S9xSetEchoWriteEnable (FALSE);
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] DSP reset\n", ICPU.Scanline);
#endif
	    // Kill sound
	    S9xResetSound (FALSE);
	}
	else
	{
	    S9xSetEchoWriteEnable (!(byte & APU_ECHO_DISABLED));
	    if (byte & APU_MUTE)
	    {
#ifdef DEBUGGER
		if (Settings.TraceSoundDSP)
		    S9xTraceSoundDSP ("[%d] Mute sound\n", ICPU.Scanline);
#endif
		S9xSetSoundMute (TRUE);
	    }
	    else
		S9xSetSoundMute (FALSE);

	    SoundData.noise_hertz = NoiseFreq [byte & 0x1f];
	    for (i = 0; i < 8; i++)
	    {
		if (SoundData.channels [i].type == SOUND_NOISE)
		    S9xSetSoundFrequency (i, SoundData.noise_hertz);
	    }
	}
	break;
    case APU_NON:
	if (byte != APU.DSP [APU_NON])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Noise:", ICPU.Scanline);
#endif
	    uint8 mask = 1;
	    for (int c = 0; c < 8; c++, mask <<= 1)
	    {
		int type;
		if (byte & mask)
		{
		    type = SOUND_NOISE;
#ifdef DEBUGGER
		    if (Settings.TraceSoundDSP)
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d,", c);
			else
			    S9xTraceSoundDSP ("%d(on),", c);
		    }
#endif
		}
		else
		{
		    type = SOUND_SAMPLE;
#ifdef DEBUGGER
		    if (Settings.TraceSoundDSP)
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d(off),", c);
		    }
#endif
		}
		S9xSetSoundType (c, type);
	    }
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("\n");
#endif
	}
	break;
    case APU_MVOL_LEFT:
	if (byte != APU.DSP [APU_MVOL_LEFT])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Master volume left:%d\n", 
				  ICPU.Scanline, (signed char) byte);
#endif
		S9xSetMasterVolume ((signed char) byte,
				    (signed char) APU.DSP [APU_MVOL_RIGHT]);
	}
	break;
    case APU_MVOL_RIGHT:
	if (byte != APU.DSP [APU_MVOL_RIGHT])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Master volume right:%d\n",
				  ICPU.Scanline, (signed char) byte);
#endif
		S9xSetMasterVolume ((signed char) APU.DSP [APU_MVOL_LEFT],
				    (signed char) byte);
	}
	break;
    case APU_EVOL_LEFT:
	if (byte != APU.DSP [APU_EVOL_LEFT])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Echo volume left:%d\n",
				  ICPU.Scanline, (signed char) byte);
#endif
		S9xSetEchoVolume ((signed char) byte,
				  (signed char) APU.DSP [APU_EVOL_RIGHT]);
	}
	break;
    case APU_EVOL_RIGHT:
	if (byte != APU.DSP [APU_EVOL_RIGHT])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Echo volume right:%d\n",
				  ICPU.Scanline, (signed char) byte);
#endif
		S9xSetEchoVolume ((signed char) APU.DSP [APU_EVOL_LEFT],
				  (signed char) byte);
	}
	break;
    case APU_ENDX:
#ifdef DEBUGGER
	if (Settings.TraceSoundDSP)
	    S9xTraceSoundDSP ("[%d] Reset ENDX\n", ICPU.Scanline);
#endif
	byte = 0;
	break;

    case APU_KOFF:
		//		if (byte)
	{
	    uint8 mask = 1;
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Key off:", ICPU.Scanline);
#endif
	    for (int c = 0; c < 8; c++, mask <<= 1)
	    {
		if ((byte & mask) != 0)
		{
#ifdef DEBUGGER

		    if (Settings.TraceSoundDSP)
			S9xTraceSoundDSP ("%d,", c);
#endif		    
		    if (APU.KeyedChannels & mask)
		    {
			{
							KeyOnPrev&=~mask;
			    APU.KeyedChannels &= ~mask;
			    APU.DSP [APU_KON] &= ~mask;
			    //APU.DSP [APU_KOFF] |= mask;
			    S9xSetSoundKeyOff (c);
			}
		    }
		}
				else if((KeyOnPrev&mask)!=0)
				{
					KeyOnPrev&=~mask;
					APU.KeyedChannels |= mask;
					//APU.DSP [APU_KON] |= mask;
					APU.DSP [APU_KOFF] &= ~mask;
					APU.DSP [APU_ENDX] &= ~mask;
					S9xPlaySample (c);
				}
	    }
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("\n");
#endif
	}
		//KeyOnPrev=0;
	APU.DSP [APU_KOFF] = byte;
	return;
    case APU_KON:

	if (byte)
	{
	    uint8 mask = 1;
#ifdef DEBUGGER

	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] Key on:", ICPU.Scanline);
#endif
	    for (int c = 0; c < 8; c++, mask <<= 1)
	    {
		if ((byte & mask) != 0)
		{
#ifdef DEBUGGER
		    if (Settings.TraceSoundDSP)
			S9xTraceSoundDSP ("%d,", c);
#endif		    
		    // Pac-In-Time requires that channels can be key-on
		    // regardeless of their current state.
					if((APU.DSP [APU_KOFF] & mask) ==0)
					{
						KeyOnPrev&=~mask;
		    APU.KeyedChannels |= mask;
						//APU.DSP [APU_KON] |= mask;
						//APU.DSP [APU_KOFF] &= ~mask;
		    APU.DSP [APU_ENDX] &= ~mask;
		    S9xPlaySample (c);
		}
					else KeyOn|=mask;
				}
	    }
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("\n");
#endif
	}
	//spc_is_dumping_temp = byte;
	return;
	
    case APU_VOL_LEFT + 0x00:
    case APU_VOL_LEFT + 0x10:
    case APU_VOL_LEFT + 0x20:
    case APU_VOL_LEFT + 0x30:
    case APU_VOL_LEFT + 0x40:
    case APU_VOL_LEFT + 0x50:
    case APU_VOL_LEFT + 0x60:
    case APU_VOL_LEFT + 0x70:
// At Shin Megami Tensei suggestion 6/11/00
//	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d volume left: %d\n", 
				  ICPU.Scanline, reg>>4, (signed char) byte);
#endif
		S9xSetSoundVolume (reg >> 4, (signed char) byte,
				   (signed char) APU.DSP [reg + 1]);
	}
	break;
    case APU_VOL_RIGHT + 0x00:
    case APU_VOL_RIGHT + 0x10:
    case APU_VOL_RIGHT + 0x20:
    case APU_VOL_RIGHT + 0x30:
    case APU_VOL_RIGHT + 0x40:
    case APU_VOL_RIGHT + 0x50:
    case APU_VOL_RIGHT + 0x60:
    case APU_VOL_RIGHT + 0x70:
// At Shin Megami Tensei suggestion 6/11/00
//	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d volume right: %d\n", 
				  ICPU.Scanline, reg >>4, (signed char) byte);
#endif
		S9xSetSoundVolume (reg >> 4, (signed char) APU.DSP [reg - 1],
				   (signed char) byte);
	}
	break;

    case APU_P_LOW + 0x00:
    case APU_P_LOW + 0x10:
    case APU_P_LOW + 0x20:
    case APU_P_LOW + 0x30:
    case APU_P_LOW + 0x40:
    case APU_P_LOW + 0x50:
    case APU_P_LOW + 0x60:
    case APU_P_LOW + 0x70:
#ifdef DEBUGGER
	if (Settings.TraceSoundDSP)
	    S9xTraceSoundDSP ("[%d] %d freq low: %d\n",
			      ICPU.Scanline, reg>>4, byte);
#endif
	    S9xSetSoundHertz (reg >> 4, (((byte + (APU.DSP [reg + 1] << 8)) & FREQUENCY_MASK) * 32000) >> 12);
	break;

    case APU_P_HIGH + 0x00:
    case APU_P_HIGH + 0x10:
    case APU_P_HIGH + 0x20:
    case APU_P_HIGH + 0x30:
    case APU_P_HIGH + 0x40:
    case APU_P_HIGH + 0x50:
    case APU_P_HIGH + 0x60:
    case APU_P_HIGH + 0x70:
#ifdef DEBUGGER
	if (Settings.TraceSoundDSP)
	    S9xTraceSoundDSP ("[%d] %d freq high: %d\n",
			      ICPU.Scanline, reg>>4, byte);
#endif
	    S9xSetSoundHertz (reg >> 4, 
			(((byte << 8) + APU.DSP [reg - 1]) & FREQUENCY_MASK) * 8);
	break;

    case APU_SRCN + 0x00:
    case APU_SRCN + 0x10:
    case APU_SRCN + 0x20:
    case APU_SRCN + 0x30:
    case APU_SRCN + 0x40:
    case APU_SRCN + 0x50:
    case APU_SRCN + 0x60:
    case APU_SRCN + 0x70:
	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d sample number: %d\n",
				  ICPU.Scanline, reg>>4, byte);
#endif
	    //S9xSetSoundSample (reg >> 4, byte); // notaz: seems to be unused?
	}
	break;
	
    case APU_ADSR1 + 0x00:
    case APU_ADSR1 + 0x10:
    case APU_ADSR1 + 0x20:
    case APU_ADSR1 + 0x30:
    case APU_ADSR1 + 0x40:
    case APU_ADSR1 + 0x50:
    case APU_ADSR1 + 0x60:
    case APU_ADSR1 + 0x70:
	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d adsr1: %02x\n",
				  ICPU.Scanline, reg>>4, byte);
#endif
	    {
		S9xFixEnvelope (reg >> 4, APU.DSP [reg + 2], byte, 
			     APU.DSP [reg + 1]);
	    }
	}
	break;

    case APU_ADSR2 + 0x00:
    case APU_ADSR2 + 0x10:
    case APU_ADSR2 + 0x20:
    case APU_ADSR2 + 0x30:
    case APU_ADSR2 + 0x40:
    case APU_ADSR2 + 0x50:
    case APU_ADSR2 + 0x60:
    case APU_ADSR2 + 0x70:
	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d adsr2: %02x\n", 
				  ICPU.Scanline, reg>>4, byte);
#endif
	    {
		S9xFixEnvelope (reg >> 4, APU.DSP [reg + 1], APU.DSP [reg - 1],
			     byte);
	    }
	}
	break;

    case APU_GAIN + 0x00:
    case APU_GAIN + 0x10:
    case APU_GAIN + 0x20:
    case APU_GAIN + 0x30:
    case APU_GAIN + 0x40:
    case APU_GAIN + 0x50:
    case APU_GAIN + 0x60:
    case APU_GAIN + 0x70:
	if (byte != APU.DSP [reg])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
		S9xTraceSoundDSP ("[%d] %d gain: %02x\n",
				  ICPU.Scanline, reg>>4, byte);
#endif
	    {
		S9xFixEnvelope (reg >> 4, byte, APU.DSP [reg - 2],
			     APU.DSP [reg - 1]);
	    }
	}
	break;

    case APU_ENVX + 0x00:
    case APU_ENVX + 0x10:
    case APU_ENVX + 0x20:
    case APU_ENVX + 0x30:
    case APU_ENVX + 0x40:
    case APU_ENVX + 0x50:
    case APU_ENVX + 0x60:
    case APU_ENVX + 0x70:
	break;

    case APU_OUTX + 0x00:
    case APU_OUTX + 0x10:
    case APU_OUTX + 0x20:
    case APU_OUTX + 0x30:
    case APU_OUTX + 0x40:
    case APU_OUTX + 0x50:
    case APU_OUTX + 0x60:
    case APU_OUTX + 0x70:
	break;
    
    case APU_DIR:
#ifdef DEBUGGER
	if (Settings.TraceSoundDSP)
	    S9xTraceSoundDSP ("[%d] Sample directory to: %02x\n",
			      ICPU.Scanline, byte);
#endif
	break;

    case APU_PMON:
	if (byte != APU.DSP [APU_PMON])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
	    {
		S9xTraceSoundDSP ("[%d] FreqMod:", ICPU.Scanline);
		uint8 mask = 1;
		for (int c = 0; c < 8; c++, mask <<= 1)
		{
		    if (byte & mask)
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d", c);
			else
			    S9xTraceSoundDSP ("%d(on),", c);
		    }
		    else
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d(off),", c);
		    }
		}
		S9xTraceSoundDSP ("\n");
	    }
#endif
		S9xSetFrequencyModulationEnable (byte);
	}
	break;

    case APU_EON:
	if (byte != APU.DSP [APU_EON])
	{
#ifdef DEBUGGER
	    if (Settings.TraceSoundDSP)
	    {
		S9xTraceSoundDSP ("[%d] Echo:", ICPU.Scanline);
		uint8 mask = 1;
		for (int c = 0; c < 8; c++, mask <<= 1)
		{
		    if (byte & mask)
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d", c);
			else
			    S9xTraceSoundDSP ("%d(on),", c);
		    }
		    else
		    {
			if (APU.DSP [reg] & mask)
			    S9xTraceSoundDSP ("%d(off),", c);
		    }
		}
		S9xTraceSoundDSP ("\n");
	    }
#endif
		S9xSetEchoEnable (byte);
	}
	break;

    case APU_EFB:
	S9xSetEchoFeedback ((signed char) byte);
	break;

    case APU_ESA:
	break;

    case APU_EDL:
	S9xSetEchoDelay (byte & 0xf);
	break;

    case APU_C0:
    case APU_C1:
    case APU_C2:
    case APU_C3:
    case APU_C4:
    case APU_C5:
    case APU_C6:
    case APU_C7:
	S9xSetFilterCoefficient (reg >> 4, (signed char) byte);
	break;
    default:
// XXX
//printf ("Write %02x to unknown APU register %02x\n", byte, reg);
	break;
    }

	KeyOnPrev|=KeyOn;
	KeyOn=0;
	
    if (reg < 0x80)
	APU.DSP [reg] = byte;
}

void S9xFixEnvelope (int channel, uint8 gain, uint8 adsr1, uint8 adsr2)
{
    if (adsr1 & 0x80)
    {
		// ADSR mode
		
		// XXX: can DSP be switched to ADSR mode directly from GAIN/INCREASE/
		// DECREASE mode? And if so, what stage of the sequence does it start
		// at?
		if (S9xSetSoundMode (channel, MODE_ADSR))
		{
			S9xSetSoundADSR (channel, adsr1 & 0xf, (adsr1 >> 4) & 7, adsr2 & 0x1f, (adsr2 >> 5) & 7, 8);
		}
    }
    else
    {
		// Gain mode
		if ((gain & 0x80) == 0)
		{
			if (S9xSetSoundMode (channel, MODE_GAIN))
			{
			S9xSetEnvelopeRate (channel, 0, 0, gain & 0x7f, 0);
			S9xSetEnvelopeHeight (channel, gain & 0x7f);
			}
		}
		else
		{
			
			if (gain & 0x40)
			{
				// Increase mode
				if (S9xSetSoundMode (channel, (gain & 0x20) ?
							  MODE_INCREASE_BENT_LINE :
							  MODE_INCREASE_LINEAR))
				{
					S9xSetEnvelopeRate (channel, IncreaseRate [gain & 0x1f], 1, 127, (3<<28)|gain);
				}
			}
			else
			{
				if(gain & 0x20) {
					if (S9xSetSoundMode (channel, MODE_DECREASE_EXPONENTIAL))
						S9xSetEnvelopeRate (channel, DecreaseRateExp [gain & 0x1f] / 2, -1, 0, (4<<28)|gain);
				} else {
					if (S9xSetSoundMode (channel, MODE_DECREASE_LINEAR))
						S9xSetEnvelopeRate (channel, IncreaseRate [gain & 0x1f], -1, 0, (3<<28)|gain);
				}
			}
		}
    }
}

void S9xSetAPUControl (uint8 byte)
{
//if (byte & 0x40)
//printf ("*** Special SPC700 timing enabled\n");
    if ((byte & 1) != 0 && !APU.TimerEnabled [0])
    {
	APU.Timer [0] = 0;
	IAPU.RAM [0xfd] = 0;
	if ((APU.TimerTarget [0] = IAPU.RAM [0xfa]) == 0)
	    APU.TimerTarget [0] = 0x100;
    }
    if ((byte & 2) != 0 && !APU.TimerEnabled [1])
    {
	APU.Timer [1] = 0;
	IAPU.RAM [0xfe] = 0;
	if ((APU.TimerTarget [1] = IAPU.RAM [0xfb]) == 0)
	    APU.TimerTarget [1] = 0x100;
    }
    if ((byte & 4) != 0 && !APU.TimerEnabled [2])
    {
	APU.Timer [2] = 0;
	IAPU.RAM [0xff] = 0;
	if ((APU.TimerTarget [2] = IAPU.RAM [0xfc]) == 0)
	    APU.TimerTarget [2] = 0x100;
    }
    APU.TimerEnabled [0] = byte & 1;
    APU.TimerEnabled [1] = (byte & 2) >> 1;
    APU.TimerEnabled [2] = (byte & 4) >> 2;

    if (byte & 0x10)
	IAPU.RAM [0xF4] = IAPU.RAM [0xF5] = 0;

    if (byte & 0x20)
	IAPU.RAM [0xF6] = IAPU.RAM [0xF7] = 0;

    if (byte & 0x80)
    {
	if (!APU.ShowROM)
	{
	    memmove (&IAPU.RAM [0xffc0], APUROM, sizeof (APUROM));
	    APU.ShowROM = TRUE;
	}
    }
    else
    {
	if (APU.ShowROM)
	{
	    APU.ShowROM = FALSE;
	    memmove (&IAPU.RAM [0xffc0], APU.ExtraRAM, sizeof (APUROM));
	}
    }
    IAPU.RAM [0xf1] = byte;
}

void S9xSetAPUTimer (uint16 Address, uint8 byte)
{
    IAPU.RAM [Address] = byte;

    switch (Address)
    {
    case 0xfa:
	if ((APU.TimerTarget [0] = IAPU.RAM [0xfa]) == 0)
	    APU.TimerTarget [0] = 0x100;
	APU.TimerValueWritten [0] = TRUE;
	break;
    case 0xfb:
	if ((APU.TimerTarget [1] = IAPU.RAM [0xfb]) == 0)
	    APU.TimerTarget [1] = 0x100;
	APU.TimerValueWritten [1] = TRUE;
	break;
    case 0xfc:
	if ((APU.TimerTarget [2] = IAPU.RAM [0xfc]) == 0)
	    APU.TimerTarget [2] = 0x100;
	APU.TimerValueWritten [2] = TRUE;
	break;
    }
}

uint8 S9xGetAPUDSP ()
{
    uint8 reg = IAPU.RAM [0xf2] & 0x7f;
    uint8 byte = APU.DSP [reg];

    switch (reg)
    {
    case APU_KON:
	break;
    case APU_KOFF:
	break;
    case APU_OUTX + 0x00:
    case APU_OUTX + 0x10:
    case APU_OUTX + 0x20:
    case APU_OUTX + 0x30:
    case APU_OUTX + 0x40:
    case APU_OUTX + 0x50:
    case APU_OUTX + 0x60:
    case APU_OUTX + 0x70:
	if (SoundData.channels [reg >> 4].state == SOUND_SILENT)
	    return (0);
	return ((SoundData.channels [reg >> 4].sample >> 8) |
		(SoundData.channels [reg >> 4].sample & 0xff));

    case APU_ENVX + 0x00:
    case APU_ENVX + 0x10:
    case APU_ENVX + 0x20:
    case APU_ENVX + 0x30:
    case APU_ENVX + 0x40:
    case APU_ENVX + 0x50:
    case APU_ENVX + 0x60:
    case APU_ENVX + 0x70:
		return 0;
//		return ((uint8) S9xGetEnvelopeHeight (reg >> 4));

    case APU_ENDX:
// To fix speech in Magical Drop 2 6/11/00
//	APU.DSP [APU_ENDX] = 0;
	break;
    default:
	break;
    }
    return (byte);
}
