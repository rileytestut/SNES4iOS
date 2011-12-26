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
#include <stdlib.h>
#include <stdio.h>

#include "snes9x.h"
#include "memmap.h"
#include "display.h"
#include "cheats.h"

#ifdef DEBUGGER
extern FILE *trace;
#endif

void S9xUsage ()
{
    S9xMessage (S9X_INFO, S9X_USAGE, "snes9x: S9xUsage: snes9x <options> <rom image filename>\n\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "Where <options> can be:\n");
    
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-cycles or -h <num>       Percentage of CPU cycles to execute every scan line (default 90)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-frameskip or -f <num>    Screen update frame skip rate (default 2)\n");
    S9xExtraUsage ();
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-forcehirom or -F or -FH  Force Hi-ROM memory map, useful for hacked ROM imagess.\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-forcelorom or -FL        Force Lo-ROM memory map, useful for hacked ROM images.\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-old or -o                Enable old-style SNES joypad emulation\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-noold or -no             Disbale old-style SNES joypad emulation\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-soundskip or -ss <num>   Sound CPU skip-waiting method, 0 - 3 (default 0)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-sound or -S              Enable digital sound output (default: enabled)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-nosound or -NS           Disable digital sound output\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-sound or -S              Enable digital sound output (default: off)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-soundquality or -r <num> Sound sample playback rate/quality, 0-7 (default 4)\n");

#ifdef __sgi
/* BS: changed the sample rate values to match the IRIX options */
    S9xMessage (S9X_INFO, S9X_USAGE, "\
                          0 - off, 1 - 8192, 2 - 11025, 3 - 16000,\n\
                          4 - 22050 (default), 5 - 32000, 6 - 44100,\n\
                          7 - 48000\n");
#else
    S9xMessage (S9X_INFO, S9X_USAGE, "\
                          0 - off, 1 - 8192, 2 - 11025, 3 - 16500,\n\
                          4 - 22050 (default), 5 - 29300, 6 - 36600,\n\
                          7 - 44000\n");
#endif

    S9xMessage (S9X_INFO, S9X_USAGE, "\
-stereo                   Enable stereo sound (default: mono sound)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-mono                     Enable mono sound (default: mono sound)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-buffersize or -B         Sound playback buffer size (default auto for playback rate)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-nospeedhacks or -N       Disable some internal speed ups that break a few  ROMs\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-speedhacks or -SH        Enable some internal speed ups that break a few ROMs\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-loadsnapshot or -l <filename>\n\
                          Load saved game position snapshot file & required ROM\n\
                          image.\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-swapjoypads or -s        Swap joypad 1 and 2 around\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-pal or -p                Fool ROM into thinking that this is a PAL SNES system\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-ntsc or -n               Fool ROM into thinking that this is a NTCS SNES system\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-interleaved or -i        ROM image is in interleaved format.\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-interleaved2 or -i2      ROM image is in interleaved 2 format\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-nohdma or -H             Disable H-DMA emulation (default: enabled)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-hdma or -NH              Enable H-DMA emulation (default: enabled)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-layering or -L           Swap some background priority levels - helps some games\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-graphicwindows           Enable graphic window effects (default: enabled)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-nographicwindows or -nw  Disable graphic window effects (default: enabled)\n");
#ifdef DEBUGGER
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-noirq or -I              Disable processor IRQ (for debugging)\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-debug or -d              Enter debug mode once ROM has loaded\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-trace or -t              Trace CPU instructions to file (WARNING: file gets very large!)\n");
#endif    

#ifdef JOYSTICK_SUPPORT
#ifdef __linux
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-joydevX /dev/jsY         Use joystick device /dev/jsY for emulation of gamepad X\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-joymapX 0 1 2 3 4 5 6 7  Joystick buttons which should be assigned to gamepad X - A B X Y TL TR Start and Select\n");
#else
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-four or -4               Single standard PC joystick has four buttons\n");
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-six or -6                Single standard PC joystick has six buttons\n");
#endif
    S9xMessage (S9X_INFO, S9X_USAGE, "\
-nojoy or -j              Disable joystick support\n");
#endif    

    S9xMessage (S9X_INFO, S9X_USAGE, "\
\nROM image needs to be in Super MagiCom (*.smc), Super FamiCom (*.sfc),\n\
*.fig, or split (*.1, *.2, or sf32527a, sf32527b, etc) format and can be\n\
compressed with gzip or compress.\n");

    exit (1);
}

#ifdef STORM
extern int dofps;
extern int hicolor;
extern int secondjoy;
extern int minimal;
int prelude=0;
extern int unit;
#endif

char *S9xParseArgs (char **argv, int argc)
{
    char *rom_filename = NULL;

    for (int i = 1; i < argc; i++)
    {
	if (*argv[i] == '-')
	{
	    if (strcasecmp (argv [i], "-so") == 0 ||
		     strcasecmp (argv [i], "-sound") == 0)
	    {
		Settings.NextAPUEnabled = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-ns") == 0 ||
		     strcasecmp (argv [i], "-nosound") == 0)
	    {
		Settings.NextAPUEnabled = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-soundskip") == 0 ||
		     strcasecmp (argv [i], "-sk") == 0)
	    {
		if (i + 1 < argc)
		    Settings.SoundSkipMethod = atoi (argv [++i]);
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-ra") == 0 ||
		     strcasecmp (argv [i], "-ratio") == 0)
	    {
		if (i + 1 < argc)
		{
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-h") == 0 ||
		     strcasecmp (argv [i], "-cycles") == 0)
	    {
		if (i + 1 < argc)
		{
		    int p = atoi (argv [++i]);
		    if (p > 0 && p < 200)
			Settings.CyclesPercentage = p;
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-nh") == 0 ||
		     strcasecmp (argv [i], "-nohdma") == 0)
	    {
		Settings.DisableHDMA = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-ha") == 0 ||
		     strcasecmp (argv [i], "-hdma") == 0)
	    {
		Settings.DisableHDMA = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-n") == 0 ||
		     strcasecmp (argv [i], "-nospeedhacks") == 0)
	    {
		Settings.ShutdownMaster = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-sh") == 0 ||
		     strcasecmp (argv [i], "-speedhacks") == 0)
	    {
		Settings.ShutdownMaster = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-p") == 0 ||
		     strcasecmp (argv [i], "-pal") == 0)
	    {
		Settings.ForcePAL = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-n") == 0 ||
		     strcasecmp (argv [i], "-ntsc") == 0)
	    {
		Settings.ForceNTSC = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-f") == 0 ||
		     strcasecmp (argv [i], "-frameskip") == 0)
	    {
		if (i + 1 < argc)
		    Settings.SkipFrames = atoi (argv [++i]) + 1;
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-fh") == 0 ||
		     strcasecmp (argv [i], "-hr") == 0 ||
		     strcasecmp (argv [i], "-hirom") == 0)
		Settings.ForceHiROM = TRUE;
	    else if (strcasecmp (argv [i], "-fl") == 0 ||
		     strcasecmp (argv [i], "-lr") == 0 ||
		     strcasecmp (argv [i], "-lorom") == 0)
		Settings.ForceLoROM = TRUE;
	    else if (strcasecmp (argv [i], "-hd") == 0 ||
		     strcasecmp (argv [i], "-header") == 0 ||
		     strcasecmp (argv [i], "-he") == 0)
	    {
		Settings.ForceHeader = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-nhd") == 0 ||
		     strcasecmp (argv [i], "-noheader") == 0)
	    {
		Settings.ForceNoHeader = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-bs") == 0)
	    {
		Settings.BS = TRUE;
	    }
#ifdef DEBUGGER
	    else if (strcasecmp (argv [i], "-d") == 0 ||
		     strcasecmp (argv [i], "-debug") == 0)
	    {
		CPU.Flags |= DEBUG_MODE_FLAG;
	    }
	    else if (strcasecmp (argv [i], "-t") == 0 ||
		     strcasecmp (argv [i], "-trace") == 0)
	    {
		trace = fopen ("trace.log", "wb");
		CPU.Flags |= TRACE_FLAG;
	    }
#endif
	    else if (strcasecmp (argv [i], "-L") == 0 ||
		     strcasecmp (argv [i], "-layering") == 0)
		Settings.BGLayering = TRUE;
	    else if (strcasecmp (argv [i], "-nl") == 0 ||
		     strcasecmp (argv [i], "-nolayering") == 0)
		Settings.BGLayering = FALSE;
	    else if (strcasecmp (argv [i], "-O") == 0 ||
		     strcasecmp (argv [i], "-tileredraw") == 0)
	    {
	    }
	    else if (strcasecmp (argv [i], "-no") == 0 ||
		     strcasecmp (argv [i], "-lineredraw") == 0)
	    {
	    }
	    else if (strcasecmp (argv [i], "-tr") == 0 ||
		     strcasecmp (argv [i], "-transparency") == 0)
	    {
		Settings.ForceTransparency = TRUE;
		Settings.ForceNoTransparency = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-nt") == 0 ||
		     strcasecmp (argv [i], "-notransparency") == 0)
	    {
		Settings.ForceNoTransparency = TRUE;
		Settings.ForceTransparency = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-hi") == 0 ||   
		     strcasecmp (argv [i], "-hires") == 0)
	    {
		Settings.SupportHiRes = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-16") == 0 ||
		     strcasecmp (argv [i], "-sixteen") == 0)
	    {
		Settings.SixteenBit = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-displayframerate") == 0 ||
		     strcasecmp (argv [i], "-dfr") == 0)
	    {
		Settings.DisplayFrameRate = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-s") == 0 ||
		     strcasecmp (argv [i], "-swapjoypads") == 0 ||
		     strcasecmp (argv [i], "-sw") == 0)
		Settings.SwapJoypads = TRUE;
	    else if (strcasecmp (argv [i], "-i") == 0 ||
		     strcasecmp (argv [i], "-interleaved") == 0)
		Settings.ForceInterleaved = TRUE;
	    else if (strcasecmp (argv [i], "-i2") == 0 ||
		     strcasecmp (argv [i], "-interleaved2") == 0)
		Settings.ForceInterleaved2 = TRUE;
	    else if (strcasecmp (argv [i], "-ni") == 0 ||
		     strcasecmp (argv [i], "-nointerleave") == 0)
		Settings.ForceNotInterleaved = TRUE;
	    else if (strcasecmp (argv [i], "-noirq") == 0)
		Settings.DisableIRQ = TRUE;
	    else if (strcasecmp (argv [i], "-nw") == 0 ||
		     strcasecmp (argv [i], "-nowindows") == 0)
	    {
		Settings.DisableGraphicWindows = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-windows") == 0)
	    {
		Settings.DisableGraphicWindows = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-im7") == 0)
	    {
		Settings.Mode7Interpolate = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-gg") == 0 ||
		     strcasecmp (argv [i], "-gamegenie") == 0)
	    {
		if (i + 1 < argc)
		{
		    uint32 address;
		    uint8 byte;
		    const char *error;
		    if ((error = S9xGameGenieToRaw (argv [++i], address, byte)) == NULL)
			S9xAddCheat (TRUE, FALSE, address, byte);
		    else
			S9xMessage (S9X_ERROR, S9X_GAME_GENIE_CODE_ERROR,
				    error);
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-ar") == 0 ||
		     strcasecmp (argv [i], "-actionreplay") == 0)
	    {
		if (i + 1 < argc)
		{
		    uint32 address;
		    uint8 byte;
		    const char *error;
		    if ((error = S9xProActionReplayToRaw (argv [++i], address, byte)) == NULL)
			S9xAddCheat (TRUE, FALSE, address, byte);
		    else
			S9xMessage (S9X_ERROR, S9X_ACTION_REPLY_CODE_ERROR,
				    error);
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-gf") == 0 ||
		     strcasecmp (argv [i], "-goldfinger") == 0)
	    {
		if (i + 1 < argc)
		{
		    uint32 address;
		    uint8 bytes [3];
		    bool8 sram;
		    uint8 num_bytes;
		    const char *error;
		    if ((error = S9xGoldFingerToRaw (argv [++i], address, sram,
						     num_bytes, bytes)) == NULL)
		    {
			for (int c = 0; c < num_bytes; c++)
			    S9xAddCheat (TRUE, FALSE, address + c, bytes [c]);
		    }
		    else
			S9xMessage (S9X_ERROR, S9X_GOLD_FINGER_CODE_ERROR,
				    error);
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv[i], "-ft") == 0 ||
		     strcasecmp (argv [i], "-frametime") == 0)
	    {
		if (i + 1 < argc)
		{
		    double ft;
		    if (sscanf (argv [++i], "%lf", &ft) == 1)
		    {
#ifdef __WIN32__
			Settings.FrameTimePAL = (int32) (ft * 1000);
			Settings.FrameTimeNTSC = (int32) (ft * 1000);
#else
			Settings.FrameTimePAL = (int32) ft;
			Settings.FrameTimeNTSC = (int32) ft;
#endif

		    }
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-e") == 0 ||
		     strcasecmp (argv [i], "-echo") == 0)
		Settings.DisableSoundEcho = FALSE;
	    else if (strcasecmp (argv [i], "-ne") == 0 ||
		     strcasecmp (argv [i], "-noecho") == 0)
		Settings.DisableSoundEcho = TRUE;
	    else if (strcasecmp (argv [i], "-r") == 0 ||
		     strcasecmp (argv [i], "-soundquality") == 0 ||
		     strcasecmp (argv [i], "-sq") == 0)
	    {
		if (i + 1 < argc)
		    Settings.SoundPlaybackRate = atoi (argv [++i]) & 7;
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-stereo") == 0 ||
		     strcasecmp (argv [i], "-st") == 0)
	    {
		Settings.Stereo = TRUE;
		Settings.APUEnabled = TRUE;
		Settings.NextAPUEnabled = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-mono") == 0)
	    {
		Settings.Stereo = FALSE;
		Settings.NextAPUEnabled = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-envx") == 0 ||
		     strcasecmp (argv [i], "-ex") == 0)
	    {
		Settings.SoundEnvelopeHeightReading = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-nosamplecaching") == 0 ||
		     strcasecmp (argv [i], "-nsc") == 0 ||
		     strcasecmp (argv [i], "-nc") == 0)
	    {
		Settings.DisableSampleCaching = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-nomastervolume") == 0 ||
		     strcasecmp (argv [i], "-nmv") == 0)
	    {
		Settings.DisableMasterVolume = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-soundsync") == 0 ||
		     strcasecmp (argv [i], "-sy") == 0)
	    {
		Settings.SoundSync = TRUE;
		Settings.SoundEnvelopeHeightReading = TRUE;
		Settings.InterpolatedSound = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-soundsync2") == 0 ||
		     strcasecmp (argv [i], "-sy2") == 0)
	    {
		Settings.SoundSync = 2;
		Settings.SoundEnvelopeHeightReading = TRUE;
		Settings.InterpolatedSound = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-interpolatedsound") == 0 ||
		     strcasecmp (argv [i], "-is") == 0)
	    {
		Settings.InterpolatedSound = TRUE;
	    }
#ifdef USE_THREADS
	    else if (strcasecmp (argv [i], "-threadsound") == 0 ||
		     strcasecmp (argv [i], "-ts") == 0)
	    {
		Settings.ThreadSound = TRUE;
	    }
#endif
	    else if (strcasecmp (argv [i], "-alt") == 0 ||
		     strcasecmp (argv [i], "-altsampledecode") == 0)
	    {
		Settings.AltSampleDecode = 1;
	    }
	    else if (strcasecmp (argv [i], "-fix") == 0)
	    {
		Settings.FixFrequency = 1;
	    }
	    else if (strcasecmp (argv [i], "-nosuperfx") == 0 ||
		     strcasecmp (argv [i], "-nosfx") == 0)
		Settings.ForceNoSuperFX = TRUE;
	    else if (strcasecmp (argv [i], "-superfx") == 0 ||
		     strcasecmp (argv [i], "-sfx") == 0)
		Settings.ForceSuperFX = TRUE;
	    else if (strcasecmp (argv [i], "-dsp1") == 0)
		Settings.ForceDSP1 = TRUE;
	    else if (strcasecmp (argv [i], "-nodsp1") == 0)
		Settings.ForceNoDSP1 = TRUE;
	    else if (strcasecmp (argv [i], "-nomultiplayer5") == 0 ||
		     strcasecmp (argv [i], "-nmp") == 0)
		Settings.MultiPlayer5 = FALSE;
	    else if (strcasecmp (argv [i], "-multiplayer5") == 0 ||
		     strcasecmp (argv [i], "-mp") == 0)
	    {
		Settings.MultiPlayer5 = TRUE;
		Settings.ControllerOption = SNES_MULTIPLAYER5;
	    }
	    else if (strcasecmp (argv [i], "-mouse") == 0 ||
		     strcasecmp (argv [i], "-mo") == 0)
	    {
		Settings.ControllerOption = SNES_MOUSE_SWAPPED;
		Settings.Mouse = TRUE;
	    }
	    else if (strcasecmp (argv [i], "-nomouse") == 0 ||
		     strcasecmp (argv [i], "-nm") == 0)
	    {
		Settings.Mouse = FALSE;
	    }
	    else if (strcasecmp (argv [i], "-superscope") == 0 ||
		     strcasecmp (argv [i], "-ss") == 0)
	    {
		Settings.SuperScope = TRUE;
		Settings.ControllerOption = SNES_SUPERSCOPE;
	    }
	    else if (strcasecmp (argv [i], "-nosuperscope") == 0 ||
		     strcasecmp (argv [i], "-nss") == 0)
	    {
		Settings.SuperScope = FALSE;
	    }
#ifdef NETPLAY_SUPPORT
	    else if (strcasecmp (argv [i], "-port") == 0 ||
		     strcasecmp (argv [i], "-po") == 0)
	    {
		if (i + 1 < argc)
		{
		    Settings.NetPlay = TRUE;
		    Settings.Port = -atoi (argv [++i]);
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-server") == 0 ||
		     strcasecmp (argv [i], "-srv") == 0)
	    {
		if (i + 1 < argc)
		{
		    Settings.NetPlay = TRUE;
		    strncpy (Settings.ServerName, argv [++i], 127);
		    Settings.ServerName [127] = 0;
		}
		else
		    S9xUsage ();
	    }
	    else if (strcasecmp (argv [i], "-net") == 0)
	    {
		Settings.NetPlay = TRUE;
	    }
#endif
#ifdef STORM
            else if (strcasecmp(argv[i],"-nosecondjoy")==0){secondjoy=0;}
            else if (strcasecmp(argv[i],"-showfps")==0){dofps=1;}
            else if (strcasecmp(argv[i],"-hicolor")==0){hicolor=1;}
            else if (strcasecmp(argv[i],"-minimal")==0){minimal=1;printf("Keyboard with exception of ESC switched off!\n");}
            else if (strcasecmp(argv[i],"-ahiunit")==0)
            {
             if (i+1<argc)
             {
              fprintf(stderr,"AHI Unit set to: Unit %i\n",atoi(argv[++i]));
              unit=atoi(argv[++i]);
             }
            }
#endif

	    else
		S9xParseArg (argv, i, argc);
	}
	else
	    rom_filename = argv [i];
    }

    return (rom_filename);
}

void S9xParseCheatsFile (const char *rom_filename)
{
    FILE *f;
    char dir [_MAX_DIR];
    char drive [_MAX_DRIVE];
    char name [_MAX_FNAME];
    char ext [_MAX_EXT];
    char fname [_MAX_PATH];
    char buf [80];
    uint32 address;
    uint8 byte;
    uint8 bytes [3];
    bool8 sram;
    uint8 num_bytes;
    const char *error;
    char *p;

    _splitpath (rom_filename, drive, dir, name, ext);
    _makepath (fname, drive, dir, name, "pat");

    if ((f = fopen(fname, "r")) != NULL)
    {
        while(fgets(buf, 80, f) != NULL)
        {
	    if ((p = strrchr (buf, '\n')) != NULL) 
		*p = '\0';
/*	    if (((error = S9xGameGenieToRaw (buf, address, byte)) == NULL) ||
		((error = S9xProActionReplayToRaw (buf, address, byte)) == NULL))
	    {
		S9xAddCheat (TRUE, FALSE, address, byte);
	    }
	    else
	    if ((error = S9xGoldFingerToRaw (buf, address, sram,
					     num_bytes, bytes)) == NULL)
	    {
		for (int c = 0; c < num_bytes; c++)
		    S9xAddCheat (TRUE, FALSE, address + c, bytes [c]);
	    }
	    else
		S9xMessage (S9X_ERROR, S9X_GAME_GENIE_CODE_ERROR, error);*/
        }
        fclose(f);
    }
}
