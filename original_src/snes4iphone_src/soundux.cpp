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
#ifdef __DJGPP__
//#include <allegro.h>
#undef TRUE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined(_SNESPPC) || defined(__IPHONE__)
#include <errno.h>
#include <fcntl.h>
#endif

#define CLIP16(v) \
if ((v) < -32768) \
    (v) = -32768; \
else \
if ((v) > 32767) \
    (v) = 32767

#define CLIP16_latch(v,l) \
if ((v) < -32768) \
{ (v) = -32768; (l)++; }\
else \
if ((v) > 32767) \
{ (v) = 32767; (l)++; }

#define CLIP24(v) \
if ((v) < -8388608) \
    (v) = -8388608; \
else \
if ((v) > 8388607) \
    (v) = 8388607

/*
#define CLIP8(v) \
if ((v) < -128) \
    (v) = -128; \
else \
if ((v) > 127) \
    (v) = 127
*/


#include "snes9x.h"
#include "soundux.h"
#include "apu.h"
#include "memmap.h"
#include "cpuexec.h"


static int wave[SOUND_BUFFER_SIZE];

//extern int Echo [24000];
extern int MixBuffer [SOUND_BUFFER_SIZE];
//extern int EchoBuffer [SOUND_BUFFER_SIZE];
//extern int FilterTaps [8];
extern unsigned long Z;
//extern int Loop [16];

extern long FilterValues[4][2];
//extern int NoiseFreq [32];


//#define FIXED_POINT 0x10000UL
#define FIXED_POINT_REMAINDER 0xffffUL
#define FIXED_POINT_SHIFT 16

#define VOL_DIV8  0x8000
#define VOL_DIV16 0x0080
#define ENVX_SHIFT 24

extern "C" void DecodeBlockAsm (int8 *, int16 *, int32 *, int32 *);

// F is channel's current frequency and M is the 16-bit modulation waveform
// from the previous channel multiplied by the current envelope volume level.
#define PITCH_MOD(F,M) ((F) * ((((unsigned long) (M)) + 0x800000) >> 16) >> 7)
//#define PITCH_MOD(F,M) ((F) * ((((M) & 0x7fffff) >> 14) + 1) >> 8)

#define LAST_SAMPLE 0xffffff
#define JUST_PLAYED_LAST_SAMPLE(c) ((c)->sample_pointer >= LAST_SAMPLE)


static inline void S9xAPUSetEndOfSample (int i, Channel *ch)
{
    ch->state = SOUND_SILENT;
    ch->mode = MODE_NONE;
    APU.DSP [APU_ENDX] |= 1 << i;
    APU.DSP [APU_KON] &= ~(1 << i);
    APU.DSP [APU_KOFF] &= ~(1 << i);
    APU.KeyedChannels &= ~(1 << i);
}
#ifdef __DJGPP
END_OF_FUNCTION (S9xAPUSetEndOfSample)
#endif

static inline void S9xAPUSetEndX (int ch)
{
    APU.DSP [APU_ENDX] |= 1 << ch;
}
#ifdef __DJGPP
END_OF_FUNCTION (S9xAPUSetEndX)
#endif

void S9xSetEchoDelay (int delay)
{
    SoundData.echo_buffer_size = (512 * delay * so.playback_rate) >> 15; // notaz / 32000;
    if (so.stereo)
		SoundData.echo_buffer_size <<= 1;
    if (SoundData.echo_buffer_size) {
		while(SoundData.echo_ptr >= SoundData.echo_buffer_size)
			SoundData.echo_ptr -= SoundData.echo_buffer_size;
    } else
		SoundData.echo_ptr = 0;
    S9xSetEchoEnable (APU.DSP [APU_EON]);
}

void S9xSetSoundKeyOff (int channel)
{
    Channel *ch = &SoundData.channels[channel];

    if (ch->state != SOUND_SILENT)
    {
	ch->state = SOUND_RELEASE;
	ch->mode = MODE_RELEASE;
	S9xSetEnvRate (ch, 8, -1, 0, 5<<28);
    }
}

void S9xFixSoundAfterSnapshotLoad ()
{
    SoundData.echo_write_enabled = !(APU.DSP [APU_FLG] & 0x20);
    SoundData.echo_channel_enable = APU.DSP [APU_EON];
    S9xSetEchoDelay (APU.DSP [APU_EDL] & 0xf);
    S9xSetEchoFeedback ((signed char) APU.DSP [APU_EFB]);

    S9xSetFilterCoefficient (0, (signed char) APU.DSP [APU_C0]);
    S9xSetFilterCoefficient (1, (signed char) APU.DSP [APU_C1]);
    S9xSetFilterCoefficient (2, (signed char) APU.DSP [APU_C2]);
    S9xSetFilterCoefficient (3, (signed char) APU.DSP [APU_C3]);
    S9xSetFilterCoefficient (4, (signed char) APU.DSP [APU_C4]);
    S9xSetFilterCoefficient (5, (signed char) APU.DSP [APU_C5]);
    S9xSetFilterCoefficient (6, (signed char) APU.DSP [APU_C6]);
    S9xSetFilterCoefficient (7, (signed char) APU.DSP [APU_C7]);
 
	for (int i = 0; i < 8; i++)
    {
		SoundData.channels[i].needs_decode = TRUE;
		S9xSetSoundFrequency (i, SoundData.channels[i].hertz);
		SoundData.channels [i].envxx = SoundData.channels [i].envx << ENVX_SHIFT;
		SoundData.channels [i].next_sample = 0;
		SoundData.channels [i].interpolate = 0;
		SoundData.channels [i].previous [0] = (int32) SoundData.channels [i].previous16 [0];
		SoundData.channels [i].previous [1] = (int32) SoundData.channels [i].previous16 [1];
    }
    SoundData.master_volume [0] = SoundData.master_volume_left;
    SoundData.master_volume [1] = SoundData.master_volume_right;
    SoundData.echo_volume [0] = SoundData.echo_volume_left;
    SoundData.echo_volume [1] = SoundData.echo_volume_right;
    IAPU.Scanline = 0;
}

void S9xSetEnvelopeHeight (int channel, int level)
{
    Channel *ch = &SoundData.channels[channel];

    ch->envx = level;
    ch->envxx = level << ENVX_SHIFT;

    ch->left_vol_level = (level * ch->volume_left) / 128;
    ch->right_vol_level = (level * ch->volume_right) / 128;

    if (ch->envx == 0 && ch->state != SOUND_SILENT && ch->state != SOUND_GAIN)
    {
	S9xAPUSetEndOfSample (channel, ch);
    }
}

#if 1
void S9xSetSoundSample (int, uint16) 
{
}
#else
void S9xSetSoundSample (int channel, uint16 sample_number)
{
    register Channel *ch = &SoundData.channels[channel];

    if (ch->state != SOUND_SILENT && 
	sample_number != ch->sample_number)
    {
	int keep = ch->state;
	ch->state = SOUND_SILENT;
	ch->sample_number = sample_number;
	ch->loop = FALSE;
	ch->needs_decode = TRUE;
	ch->last_block = FALSE;
	ch->previous [0] = ch->previous[1] = 0;
	ch->block_pointer = *S9xGetSampleAddress(sample_number);
	ch->sample_pointer = 0;
	ch->state = keep;
    }
}
#endif

static void DecodeBlock (Channel *ch)
{
    if (ch->block_pointer >= 0x10000 - 9)
    {
		ch->last_block = TRUE;
		ch->loop = FALSE;
		ch->block = ch->decoded;
		memset32 ((uint32_t *) ch->decoded, 0, 8);
		return;
    }
    signed char *compressed = (signed char *) &IAPU.RAM [ch->block_pointer];

    unsigned char filter = *compressed;
    if ((ch->last_block = filter & 1))
	ch->loop = (filter & 2) != 0;

    int16 *raw = ch->block = ch->decoded;

#ifdef ARM
	DecodeBlockAsm (compressed, raw, &ch->previous [0], &ch->previous [1]);
#else
	int32 out;
    unsigned char shift;
    signed char sample1, sample2;
    unsigned int i;
	
    compressed++;
    
    int32 prev0 = ch->previous [0];
    int32 prev1 = ch->previous [1];
    shift = filter >> 4;
	
    switch ((filter >> 2) & 3)
    {
    case 0:
		for (i = 8; i != 0; i--)
		{
			sample1 = *compressed++;
			sample2 = sample1 << 4;
			sample2 >>= 4;
			sample1 >>= 4;
			*raw++ = ((int32) sample1 << shift);
			*raw++ = ((int32) sample2 << shift);
		}
		prev1 = *(raw - 2);
		prev0 = *(raw - 1);
		break;
    case 1:
		for (i = 8; i != 0; i--)
		{
			sample1 = *compressed++;
			sample2 = sample1 << 4;
			sample2 >>= 4;
			sample1 >>= 4;
			prev0 = (int16) prev0;
			*raw++ = prev1 = ((int32) sample1 << shift) + prev0 - (prev0 >> 4);
			prev1 = (int16) prev1;
			*raw++ = prev0 = ((int32) sample2 << shift) + prev1 - (prev1 >> 4);
		}
		break;
    case 2:
		for (i = 8; i != 0; i--)
		{
			sample1 = *compressed++;
			sample2 = sample1 << 4;
			sample2 >>= 4;
			sample1 >>= 4;
			
			out = (sample1 << shift) - prev1 + (prev1 >> 4);
			prev1 = (int16) prev0;
			prev0 &= ~3;
			*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 5) - 
				(prev0 >> 4);
			
			out = (sample2 << shift) - prev1 + (prev1 >> 4);
			prev1 = (int16) prev0;
			prev0 &= ~3;
			*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 5) -
				(prev0 >> 4);
		}
		break;
    case 3:
		for (i = 8; i != 0; i--)
		{
			sample1 = *compressed++;
			sample2 = sample1 << 4;
			sample2 >>= 4;
			sample1 >>= 4;
			out = (sample1 << shift);
			
			out = out - prev1 + (prev1 >> 3) + (prev1 >> 4);
			prev1 = (int16) prev0;
			prev0 &= ~3;
			*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 3) - 
				(prev0 >> 4) - (prev1 >> 6);
			
			out = (sample2 << shift);
			out = out - prev1 + (prev1 >> 3) + (prev1 >> 4);
			prev1 = (int16) prev0;
			prev0 &= ~3;
			*raw++ = prev0 = out + (prev0 << 1) - (prev0 >> 3) - 
				(prev0 >> 4) - (prev1 >> 6);
		}
		break;
    }
    ch->previous [0] = prev0;
    ch->previous [1] = prev1;

#endif
    ch->block_pointer += 9;
}


static void MixStereo (int sample_count)
{
    int pitch_mod = SoundData.pitch_mod & (0xFFFFFFFF^APU.DSP[APU_NON]);//~APU.DSP[APU_NON];

    for (uint32 J = 0; J < NUM_CHANNELS; J++) 
    {
	int32 VL, VR;
	Channel *ch = &SoundData.channels[J];
	unsigned long freq0 = ch->frequency;

	if (ch->state == SOUND_SILENT || !(so.sound_switch & (1 << J)))
	    continue;

//		freq0 = (unsigned long) ((double) freq0 * 0.985);//uncommented by jonathan gevaryahu, as it is necessary for most cards in linux

	bool8 mod = pitch_mod & (1 << J);

	if (ch->needs_decode) 
	{
	    DecodeBlock(ch);
	    ch->needs_decode = FALSE;
	    ch->sample = ch->block[0];
	    ch->sample_pointer = freq0 >> FIXED_POINT_SHIFT;
	    if (ch->sample_pointer == 0)
		ch->sample_pointer = 1;
	    if (ch->sample_pointer > SOUND_DECODE_LENGTH)
		ch->sample_pointer = SOUND_DECODE_LENGTH - 1;

	    ch->next_sample = ch->block[ch->sample_pointer];
	    ch->interpolate = 0;

	    if (Settings.InterpolatedSound && freq0 < FIXED_POINT && !mod)
		ch->interpolate = ((ch->next_sample - ch->sample) * 
				   (long) freq0) / (long) FIXED_POINT;
	}
	VL = (ch->sample * ch-> left_vol_level) / 128;
	VR = (ch->sample * ch->right_vol_level) / 128;

	for (uint32 I = 0; I < (uint32) sample_count; I += 2)
	{
	    unsigned long freq = freq0;

	    if (mod)
		freq = PITCH_MOD(freq, wave [I / 2]);

	    ch->env_error += ch->erate;
	    if (ch->env_error >= FIXED_POINT) 
	    {
		uint32 step = ch->env_error >> FIXED_POINT_SHIFT;

		switch (ch->state)
		{
		case SOUND_ATTACK:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx += step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_DECAY;
			if (ch->sustain_level != 8) 
			{
			    S9xSetEnvRate (ch, ch->decay_rate, -1,
						(MAX_ENVELOPE_HEIGHT * ch->sustain_level) >> 3, 1<<28);
			    break;
			}
			ch->state = SOUND_SUSTAIN;
			S9xSetEnvRate (ch, ch->sustain_rate, -1, 0, 2<<28);
		    }
		    break;
		
		case SOUND_DECAY:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= ch->envx_target)
		    {
			if (ch->envx <= 0)
			{
			    S9xAPUSetEndOfSample (J, ch);
			    goto stereo_exit;
			}
			ch->state = SOUND_SUSTAIN;
			S9xSetEnvRate (ch, ch->sustain_rate, -1, 0, 2<<28);
		    }
		    break;

		case SOUND_SUSTAIN:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto stereo_exit;
		    }
		    break;
		    
		case SOUND_RELEASE:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx -= (MAX_ENVELOPE_HEIGHT << ENVX_SHIFT) / 256;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto stereo_exit;
		    }
		    break;
		
		case SOUND_INCREASE_LINEAR:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx += step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_GAIN;
			ch->mode = MODE_GAIN;
			S9xSetEnvRate (ch, 0, -1, 0, 0);
		    }
		    break;

		case SOUND_INCREASE_BENT_LINE:
		    if (ch->envx >= (MAX_ENVELOPE_HEIGHT * 3) / 4)
		    {
			while (ch->env_error >= FIXED_POINT)
			{
			    ch->envxx += (MAX_ENVELOPE_HEIGHT << ENVX_SHIFT) / 256;
			    ch->env_error -= FIXED_POINT;
			}
			ch->envx = ch->envxx >> ENVX_SHIFT;
		    }
		    else
		    {
			ch->env_error &= FIXED_POINT_REMAINDER;
			ch->envx += step << 1;
			ch->envxx = ch->envx << ENVX_SHIFT;
		    }

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_GAIN;
			ch->mode = MODE_GAIN;
			S9xSetEnvRate (ch, 0, -1, 0, 0);
		    }
		    break;

		case SOUND_DECREASE_LINEAR:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx -= step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto stereo_exit;
		    }
		    break;

		case SOUND_DECREASE_EXPONENTIAL:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto stereo_exit;
		    }
		    break;
		
		case SOUND_GAIN:
		    S9xSetEnvRate (ch, 0, -1, 0, 0);
		    break;
		}
		ch-> left_vol_level = (ch->envx * ch->volume_left) / 128;
		ch->right_vol_level = (ch->envx * ch->volume_right) / 128;
		VL = (ch->sample * ch-> left_vol_level) / 128;
		VR = (ch->sample * ch->right_vol_level) / 128;
	    }

	    ch->count += freq;
	    if (ch->count >= FIXED_POINT)
	    {
			VL = ch->count >> FIXED_POINT_SHIFT;
			ch->sample_pointer += VL;
			ch->count &= FIXED_POINT_REMAINDER;

			ch->sample = ch->next_sample;
			if (ch->sample_pointer >= SOUND_DECODE_LENGTH)
			{
				if (JUST_PLAYED_LAST_SAMPLE(ch))
				{
					S9xAPUSetEndOfSample (J, ch);
					goto stereo_exit;
				}
				do
				{
					ch->sample_pointer -= SOUND_DECODE_LENGTH;
					if (ch->last_block)
					{
						if (!ch->loop)
						{
							ch->sample_pointer = LAST_SAMPLE;
							ch->next_sample = ch->sample;
							break;
						}
						else
						{
							S9xAPUSetEndX (J);
							ch->last_block = FALSE;
							uint16 *dir = S9xGetSampleAddress (ch->sample_number);
							ch->block_pointer = *(dir + 1);
						}
					}
					DecodeBlock (ch);
				} while (ch->sample_pointer >= SOUND_DECODE_LENGTH);
				if (!JUST_PLAYED_LAST_SAMPLE (ch))
					ch->next_sample = ch->block [ch->sample_pointer];
			}
			else
				ch->next_sample = ch->block [ch->sample_pointer];

			if (ch->type == SOUND_SAMPLE)
			{
				if (Settings.InterpolatedSound && freq < FIXED_POINT && !mod)
				{
					ch->interpolate = ((ch->next_sample - ch->sample) * 
							   (long) freq) / (long) FIXED_POINT;
					ch->sample = (int16) (ch->sample + (((ch->next_sample - ch->sample) * 
							   (long) (ch->count)) / (long) FIXED_POINT));
				}		  
				else
					ch->interpolate = 0;
			}
			else
			{
				for (;VL > 0; VL--)
					if ((so.noise_gen <<= 1) & 0x80000000L)
						so.noise_gen ^= 0x0040001L;
				ch->sample = (so.noise_gen << 17) >> 17;
				ch->interpolate = 0;
			}

			VL = (ch->sample * ch-> left_vol_level) / 128;
			VR = (ch->sample * ch->right_vol_level) / 128;
		}
	    else
	    {
			if (ch->interpolate)
			{
				int32 s = (int32) ch->sample + ch->interpolate;
				
				CLIP16(s);
				ch->sample = (int16) s;
				VL = (ch->sample * ch-> left_vol_level) / 128;
				VR = (ch->sample * ch->right_vol_level) / 128;
			}
	    }

	    if (pitch_mod & (1 << (J + 1)))
			wave [I / 2] = ch->sample * ch->envx;

		MixBuffer [I]   += VL;
		MixBuffer [I+1] += VR;
		if (ch->echo_buf_ptr)
		{
			ch->echo_buf_ptr [I]   += VL;
			ch->echo_buf_ptr [I+1] += VR;
		}
        }
stereo_exit: ;
    }
}

static void MixMono (int sample_count)
{
    int pitch_mod = SoundData.pitch_mod & (0xFFFFFFFF^APU.DSP[APU_NON]);

    for (uint32 J = 0; J < NUM_CHANNELS; J++) 
    {
	Channel *ch = &SoundData.channels[J];
	unsigned long freq0 = ch->frequency;

	if (ch->state == SOUND_SILENT || !(so.sound_switch & (1 << J)))
	    continue;

//	freq0 = (unsigned long) ((double) freq0 * 0.985);

	bool8 mod = pitch_mod & (1 << J);

	if (ch->needs_decode) 
	{
	    DecodeBlock(ch);
	    ch->needs_decode = FALSE;
	    ch->sample = ch->block[0];
	    ch->sample_pointer = freq0 >> FIXED_POINT_SHIFT;
	    if (ch->sample_pointer == 0)
		ch->sample_pointer = 1;
	    if (ch->sample_pointer > SOUND_DECODE_LENGTH)
		ch->sample_pointer = SOUND_DECODE_LENGTH - 1;
	    ch->next_sample = ch->block[ch->sample_pointer];
	    ch->interpolate = 0;

	    if (Settings.InterpolatedSound && freq0 < FIXED_POINT && !mod)
		ch->interpolate = ((ch->next_sample - ch->sample) * 
				   (long) freq0) / (long) FIXED_POINT;
	}
	int32 V = (ch->sample * ch->left_vol_level) / 128;

	for (uint32 I = 0; I < (uint32) sample_count; I++)
	{
	    unsigned long freq = freq0;

	    if (mod)
		freq = PITCH_MOD(freq, wave [I]);

	    ch->env_error += ch->erate;
	    if (ch->env_error >= FIXED_POINT) 
	    {
		uint32 step = ch->env_error >> FIXED_POINT_SHIFT;

		switch (ch->state)
		{
		case SOUND_ATTACK:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx += step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_DECAY;
			if (ch->sustain_level != 8) 
			{
			    S9xSetEnvRate (ch, ch->decay_rate, -1,
						(MAX_ENVELOPE_HEIGHT * ch->sustain_level) >> 3, 1<<28);
			    break;
			}
			ch->state = SOUND_SUSTAIN;
			S9xSetEnvRate (ch, ch->sustain_rate, -1, 0, 2<<28);
		    }
		    break;
		
		case SOUND_DECAY:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= ch->envx_target)
		    {
			if (ch->envx <= 0)
			{
			    S9xAPUSetEndOfSample (J, ch);
			    goto mono_exit;
			}
			ch->state = SOUND_SUSTAIN;
			S9xSetEnvRate (ch, ch->sustain_rate, -1, 0, 2<<28);
		    }
		    break;

		case SOUND_SUSTAIN:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto mono_exit;
		    }
		    break;
		    
		case SOUND_RELEASE:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx -= (MAX_ENVELOPE_HEIGHT << ENVX_SHIFT) / 256;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto mono_exit;
		    }
		    break;
		
		case SOUND_INCREASE_LINEAR:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx += step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_GAIN;
			ch->mode = MODE_GAIN;
			S9xSetEnvRate (ch, 0, -1, 0, 0);
		    }
		    break;

		case SOUND_INCREASE_BENT_LINE:
		    if (ch->envx >= (MAX_ENVELOPE_HEIGHT * 3) / 4)
		    {
			while (ch->env_error >= FIXED_POINT)
			{
			    ch->envxx += (MAX_ENVELOPE_HEIGHT << ENVX_SHIFT) / 256;
			    ch->env_error -= FIXED_POINT;
			}
			ch->envx = ch->envxx >> ENVX_SHIFT;
		    }
		    else
		    {
			ch->env_error &= FIXED_POINT_REMAINDER;
			ch->envx += step << 1;
			ch->envxx = ch->envx << ENVX_SHIFT;
		    }

		    if (ch->envx >= 126)
		    {
			ch->envx = 127;
			ch->envxx = 127 << ENVX_SHIFT;
			ch->state = SOUND_GAIN;
			ch->mode = MODE_GAIN;
			S9xSetEnvRate (ch, 0, -1, 0, 0);
		    }
		    break;

		case SOUND_DECREASE_LINEAR:
		    ch->env_error &= FIXED_POINT_REMAINDER;
		    ch->envx -= step << 1;
		    ch->envxx = ch->envx << ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto mono_exit;
		    }
		    break;

		case SOUND_DECREASE_EXPONENTIAL:
		    while (ch->env_error >= FIXED_POINT)
		    {
			ch->envxx = (ch->envxx >> 8) * 255;
			ch->env_error -= FIXED_POINT;
		    }
		    ch->envx = ch->envxx >> ENVX_SHIFT;
		    if (ch->envx <= 0)
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto mono_exit;
		    }
		    break;
		
		case SOUND_GAIN:
		    S9xSetEnvRate (ch, 0, -1, 0, 0);
		    break;
		}
		ch->left_vol_level = (ch->envx * ch->volume_left) / 128;
		V = (ch->sample * ch->left_vol_level) / 128;
	    }

	    ch->count += freq;
	    if (ch->count >= FIXED_POINT)
	    {
		V = ch->count >> FIXED_POINT_SHIFT;
		ch->sample_pointer += V;
		ch->count &= FIXED_POINT_REMAINDER;

		ch->sample = ch->next_sample;
		if (ch->sample_pointer >= SOUND_DECODE_LENGTH)
		{
		    if (JUST_PLAYED_LAST_SAMPLE(ch))
		    {
			S9xAPUSetEndOfSample (J, ch);
			goto mono_exit;
		    }
		    do
		    {
			ch->sample_pointer -= SOUND_DECODE_LENGTH;
			if (ch->last_block)
			{
			    if (!ch->loop)
			    {
				ch->sample_pointer = LAST_SAMPLE;
				ch->next_sample = ch->sample;
				break;
			    }
			    else
			    {
				ch->last_block = FALSE;
				uint16 *dir = S9xGetSampleAddress (ch->sample_number);
				ch->block_pointer = *(dir + 1);
				S9xAPUSetEndX (J);
			    }
			}
			DecodeBlock (ch);
		    } while (ch->sample_pointer >= SOUND_DECODE_LENGTH);
		    if (!JUST_PLAYED_LAST_SAMPLE (ch))
			ch->next_sample = ch->block [ch->sample_pointer];
		}
		else
		    ch->next_sample = ch->block [ch->sample_pointer];

		if (ch->type == SOUND_SAMPLE)
		{
		    if (Settings.InterpolatedSound && freq < FIXED_POINT && !mod)
		    {
			ch->interpolate = ((ch->next_sample - ch->sample) * 
					   (long) freq) / (long) FIXED_POINT;
			ch->sample = (int16) (ch->sample + (((ch->next_sample - ch->sample) * 
					   (long) (ch->count)) / (long) FIXED_POINT));
		    }		  
		    else
			ch->interpolate = 0;
		}
		else
		{
		    for (;V > 0; V--)
			if ((so.noise_gen <<= 1) & 0x80000000L)
			    so.noise_gen ^= 0x0040001L;
		    ch->sample = (so.noise_gen << 17) >> 17;
		    ch->interpolate = 0;
		}
		V = (ch->sample * ch-> left_vol_level) / 128;
            }
	    else
	    {
		if (ch->interpolate)
		{
		    int32 s = (int32) ch->sample + ch->interpolate;

		    CLIP16(s);
		    ch->sample = (int16) s;
		    V = (ch->sample * ch-> left_vol_level) / 128;
		}
	    }

	    MixBuffer [I] += V;
		if (ch->echo_buf_ptr)
		    ch->echo_buf_ptr [I] += V;

	    if (pitch_mod & (1 << (J + 1)))
		wave [I] = ch->sample * ch->envx;
        }
mono_exit: ;
    }
}


// For backwards compatibility with older port specific code
void S9xMixSamples (signed short *buffer, int sample_count)
{
    S9xMixSamplesO (buffer, sample_count, 0);
}


void S9xMixSamplesO (signed short *buffer, int sample_count, int sample_offset)
{
	// 16-bit sound only
	int J;

	buffer += sample_offset;
/*
	if (so.mute_sound)
	{
		memset16((uint16_t*)buffer, 0, sample_count);
		return;
	}
*/
	memset32 ((uint32_t*)MixBuffer, 0, sample_count);
	if (SoundData.echo_enable)
		memset32 ((uint32_t*)EchoBuffer, 0, sample_count);

	if (so.stereo)
		MixStereo (sample_count);
	else
		MixMono (sample_count);

    /* Mix and convert waveforms */
	if (SoundData.echo_enable && SoundData.echo_buffer_size)
	{
		if (so.stereo)
		{
			int l, r;
			int master_vol_l = SoundData.master_volume[0];
			int master_vol_r = SoundData.master_volume[1];
			int echo_vol_l = SoundData.echo_volume[0];
			int echo_vol_r = SoundData.echo_volume[1];

			// 16-bit stereo sound with echo enabled ...
			if (SoundData.no_filter)
			{
				// ... but no filter defined.
				for (J = 0; J < sample_count; J+=2)
				{
					int E = Echo [SoundData.echo_ptr];

					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J];
					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J+1];

					if (SoundData.echo_ptr >= SoundData.echo_buffer_size)
						SoundData.echo_ptr = 0;

					l = (MixBuffer[J]   * master_vol_l + E * echo_vol_l) / VOL_DIV16;
					r = (MixBuffer[J+1] * master_vol_r + E * echo_vol_r) / VOL_DIV16;

					CLIP16(l);
					CLIP16(r);
					buffer[J]   = l;
					buffer[J+1] = r;
				}
			}
			else
			{
				// ... with filter defined.
				for (J = 0; J < sample_count; J+=2)
				{
					register int E = Echo [SoundData.echo_ptr];

					Loop [(Z - 0) & 15] = E;
					E =  E                    * FilterTaps [0];
					E += Loop [(Z -  2) & 15] * FilterTaps [1];
					E += Loop [(Z -  4) & 15] * FilterTaps [2];
					E += Loop [(Z -  6) & 15] * FilterTaps [3];
					E += Loop [(Z -  8) & 15] * FilterTaps [4];
					E += Loop [(Z - 10) & 15] * FilterTaps [5];
					E += Loop [(Z - 12) & 15] * FilterTaps [6];
					E += Loop [(Z - 14) & 15] * FilterTaps [7];
					E /= 128;
					Z++;

					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J];
					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J+1];

					if (SoundData.echo_ptr >= SoundData.echo_buffer_size)
						SoundData.echo_ptr = 0;

					l = (MixBuffer[J]   * master_vol_l + E * echo_vol_l) / VOL_DIV16;
					r = (MixBuffer[J+1] * master_vol_r + E * echo_vol_r) / VOL_DIV16;

					CLIP16(l);
					CLIP16(r);
					buffer[J]   = l;
					buffer[J+1] = r;
				}
			}
		}
		else
		{
			int I;
			int master_vol_l = SoundData.master_volume[0];
			int echo_vol_l = SoundData.echo_volume[0];

			// 16-bit mono sound with echo enabled...
			if (SoundData.no_filter)
			{
				// ... no filter defined
				for (J = 0; J < sample_count; J++)
				{
					int E = Echo [SoundData.echo_ptr];

					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J];

					if (SoundData.echo_ptr >= SoundData.echo_buffer_size)
						SoundData.echo_ptr = 0;

					I = (MixBuffer[J] * master_vol_l + E * echo_vol_l) / VOL_DIV16;
					CLIP16(I);
					buffer[J] = I;
				}
			}
			else
			{
				// ... with filter defined
				for (J = 0; J < sample_count; J++)
				{
					register int E = Echo [SoundData.echo_ptr];

					Loop [(Z - 0) & 7] = E;
					E =  E                  * FilterTaps [0];
					E += Loop [(Z - 1) & 7] * FilterTaps [1];
					E += Loop [(Z - 2) & 7] * FilterTaps [2];
					E += Loop [(Z - 3) & 7] * FilterTaps [3];
					E += Loop [(Z - 4) & 7] * FilterTaps [4];
					E += Loop [(Z - 5) & 7] * FilterTaps [5];
					E += Loop [(Z - 6) & 7] * FilterTaps [6];
					E += Loop [(Z - 7) & 7] * FilterTaps [7];
					E /= 128;
					Z++;

					Echo[SoundData.echo_ptr++] = (E * SoundData.echo_feedback) / 128 + EchoBuffer[J];

					if (SoundData.echo_ptr >= SoundData.echo_buffer_size)
						SoundData.echo_ptr = 0;

					I = (MixBuffer[J] * master_vol_l + E * echo_vol_l) / VOL_DIV16;
					CLIP16(I);
					buffer[J] = I;
				}
			}
		}
	}
	else
	{
		int l, master_vol_l = SoundData.master_volume[0];

		if (so.stereo)
		{
			int r, master_vol_r = SoundData.master_volume[1];

			// 16-bit stereo sound, no echo
			for (J = 0; J < sample_count; J+=2)
			{
				l = (MixBuffer[J]   * master_vol_l) / VOL_DIV16;
				r = (MixBuffer[J+1] * master_vol_r) / VOL_DIV16;

				CLIP16(l);
				CLIP16(r);
				buffer[J]   = l;
				buffer[J+1] = r;
			}
		}
		else
		{
			// 16-bit mono sound, no echo
			for (J = 0; J < sample_count; J++)
			{
				l = (MixBuffer[J] * master_vol_l) / VOL_DIV16;

				CLIP16(l);
				buffer[J] = l;
			}
		}
	}
}

#ifdef __DJGPP
END_OF_FUNCTION(S9xMixSamplesO);
#endif

void S9xResetSound (bool8 full)
{
    for (int i = 0; i < 8; i++)
    {
	SoundData.channels[i].state = SOUND_SILENT;
	SoundData.channels[i].mode = MODE_NONE;
	SoundData.channels[i].type = SOUND_SAMPLE;
	SoundData.channels[i].volume_left = 0;
	SoundData.channels[i].volume_right = 0;
	SoundData.channels[i].hertz = 0;
	SoundData.channels[i].count = 0;
	SoundData.channels[i].loop = FALSE;
	SoundData.channels[i].envx_target = 0;
	SoundData.channels[i].env_error = 0;
	SoundData.channels[i].erate = 0;
	SoundData.channels[i].envx = 0;
	SoundData.channels[i].envxx = 0;
	SoundData.channels[i].left_vol_level = 0;
	SoundData.channels[i].right_vol_level = 0;
	SoundData.channels[i].direction = 0;
	SoundData.channels[i].attack_rate = 0;
	SoundData.channels[i].decay_rate = 0;
	SoundData.channels[i].sustain_rate = 0;
	SoundData.channels[i].release_rate = 0;
	SoundData.channels[i].sustain_level = 0;
	// notaz
	SoundData.channels[i].env_ind_attack = 0;
	SoundData.channels[i].env_ind_decay = 0;
	SoundData.channels[i].env_ind_sustain = 0;
	SoundData.echo_ptr = 0;
	SoundData.echo_feedback = 0;
	SoundData.echo_buffer_size = 1;
    }
    FilterTaps [0] = 127;
    FilterTaps [1] = 0;
    FilterTaps [2] = 0;
    FilterTaps [3] = 0;
    FilterTaps [4] = 0;
    FilterTaps [5] = 0;
    FilterTaps [6] = 0;
    FilterTaps [7] = 0;
    so.mute_sound = FALSE;
    so.noise_gen = 1;
    so.sound_switch = 255;

    if (full)
    {
		SoundData.master_volume_left = 0;
		SoundData.master_volume_right = 0;
		SoundData.echo_volume_left = 0;
		SoundData.echo_volume_right = 0;
		SoundData.echo_enable = 0;
		SoundData.echo_write_enabled = 0;
		SoundData.echo_channel_enable = 0;
		SoundData.pitch_mod = 0;
		SoundData.dummy[0] = 0;
		SoundData.dummy[1] = 0;
		SoundData.dummy[2] = 0;
		SoundData.master_volume[0] = 0;
		SoundData.master_volume[1] = 0;
		SoundData.echo_volume[0] = 0;
		SoundData.echo_volume[1] = 0;
		SoundData.noise_hertz = 0;
    }

    SoundData.master_volume_left = 127;
    SoundData.master_volume_right = 127;
    SoundData.master_volume [0] = SoundData.master_volume [1] = 127;
    SoundData.no_filter = TRUE;
}



extern unsigned long AttackRate [16];
extern unsigned long DecayRate [8];
extern unsigned long SustainRate [32];
extern unsigned long IncreaseRate [32];
extern unsigned long DecreaseRateExp [32];	


void S9xSetPlaybackRate (uint32 playback_rate)
{
    so.playback_rate = playback_rate;

	if(playback_rate) {
		// notaz: calclulate a value (let's call it freqbase) to simplify channel freq calculations later.
		so.freqbase = (FIXED_POINT<<11) / playback_rate;
		// now precalculate env rates for S9xSetEnvRate
		static int steps [] =
		{
			//0, 64, 1238, 1238, 256, 1, 64, 109, 64, 1238
			0, 64, 619, 619, 128, 1, 64, 55, 64, 619
		};
		int i, u;
		for(i=0; i < 16; i++)
			for(u=0; u < 10; u++)
				AttackERate[i][u] = (unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
													(AttackRate[i] * playback_rate));
		for(i=0; i < 8; i++)
			for(u=0; u < 10; u++)
				DecayERate[i][u]  = (unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
													(DecayRate[i]  * playback_rate));

		for(i=0; i < 32; i++)
			for(u=0; u < 10; u++)
				SustainERate[i][u]= (unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
													(SustainRate[i]  * playback_rate));

		for(i=0; i < 32; i++)
			for(u=0; u < 10; u++)
				IncreaseERate[i][u]=(unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
													(IncreaseRate[i] * playback_rate));

		for(i=0; i < 32; i++)
			for(u=0; u < 10; u++)
				DecreaseERateExp[i][u] = (unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
													(DecreaseRateExp[i] / 2 * playback_rate));

		for(u=0; u < 10; u++)
			KeyOffERate[u] = (unsigned long) (((int64) FIXED_POINT * 1000 * steps[u]) /
												(8 * playback_rate));
	}

    S9xSetEchoDelay (APU.DSP [APU_EDL] & 0xf);
    for (int i = 0; i < 8; i++)
	S9xSetSoundFrequency (i, SoundData.channels [i].hertz);
}

bool8 S9xInitSound (void)
{
    so.playback_rate = 22050;
    so.stereo = 0;
	so.sound_switch = 255;

    S9xResetSound (TRUE);
    S9xSetSoundMute (TRUE);

    return (1);
}

