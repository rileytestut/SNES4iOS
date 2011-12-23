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
#include "dsp1.h"
#include "missing.h"
#include "memmap.h"
#include <math.h>

#include "dsp1emu_gp32.c"

void S9xInitDSP1 ()
{
    static bool8 init = FALSE;
    
    if (!init)
    {
        InitDSP ();
        init = TRUE;
    }
}

void S9xResetDSP1 ()
{
    S9xInitDSP1 ();
    
    DSP1.waiting4command = TRUE;
    DSP1.in_count = 0;
    DSP1.out_count = 0;
    DSP1.in_index = 0;
    DSP1.out_index = 0;
    DSP1.first_parameter = TRUE;
}

uint8 S9xGetDSP (uint16 address)
{
    uint8 t;

#ifdef DEBUGGER
    if (Settings.TraceDSP)
    {
	sprintf (String, "DSP read: 0x%04X", address);
	S9xMessage (S9X_TRACE, S9X_TRACE_DSP1, String);
    }
#endif
    if ((address & 0xf000) == 0x6000 ||
	(address >= 0x8000 && address < 0xc000))
    {
	if (DSP1.out_count)
	{
	    if ((address & 1) == 0)
		t = (uint8) DSP1.output [DSP1.out_index];
	    else
	    {
		t = (uint8) (DSP1.output [DSP1.out_index] >> 8);
		DSP1.out_index++;
		if (--DSP1.out_count == 0)
		{
		    if (DSP1.command == 0x1a || DSP1.command == 0x0a)
		    {
			DSPOp0A ();
			DSP1.out_count = 4;
			DSP1.out_index = 0;
			DSP1.output [0] = Op0AA;
			DSP1.output [1] = Op0AB;
			DSP1.output [2] = Op0AC;
			DSP1.output [3] = Op0AD;
		    }
		}
		DSP1.waiting4command = TRUE;
	    }
	}
	else
	{
	    // Top Gear 3000 requires this value....
	    t = 0xff;
	}
    }
    else
	t = 0x80;

    return (t);
}

void S9xSetDSP (uint8 byte, uint16 address)
{
#ifdef DEBUGGER
    missing.unknowndsp_write = address;
    if (Settings.TraceDSP)
    {
	sprintf (String, "DSP write: 0x%04X=0x%02X", address, byte);
	S9xMessage (S9X_TRACE, S9X_TRACE_DSP1, String);
    }
#endif
    if ((address & 0xf000) == 0x6000 ||
	(address >= 0x8000 && address < 0xc000))
    {
	if ((address & 1) == 0)
	{
	    if (DSP1.waiting4command)
	    {
		DSP1.command = byte;
		DSP1.in_index = 0;
		DSP1.waiting4command = FALSE;
		DSP1.first_parameter = TRUE;

		// Mario Kart uses 0x00, 0x02, 0x06, 0x0c, 0x28, 0x0a
		switch (byte)
		{
		    case 0x00: DSP1.in_count = 2;	break;
		    case 0x10: DSP1.in_count = 2;	break;
		    case 0x04: DSP1.in_count = 2;	break;
		    case 0x08: DSP1.in_count = 3;	break;
		    case 0x18: DSP1.in_count = 4;	break;
		    case 0x28: DSP1.in_count = 3;	break;
		    case 0x0c: DSP1.in_count = 3;	break;
		    case 0x1c: DSP1.in_count = 6;	break;
		    case 0x02: DSP1.in_count = 7;	break;
		    case 0x0a: DSP1.in_count = 1;	break;
		    case 0x1a: DSP1.in_count = 1;	break;
		    case 0x06: DSP1.in_count = 3;	break;
		    case 0x0e: DSP1.in_count = 2;	break;
		    case 0x01: DSP1.in_count = 4;	break;
		    case 0x11: DSP1.in_count = 4;	break;
		    case 0x21: DSP1.in_count = 4;	break;
		    case 0x0d: DSP1.in_count = 3;	break;
		    case 0x1d: DSP1.in_count = 3;	break;
		    case 0x2d: DSP1.in_count = 3;	break;
		    case 0x03: DSP1.in_count = 3;	break;
		    case 0x13: DSP1.in_count = 3;	break;
		    case 0x23: DSP1.in_count = 3;	break;
		    case 0x0b: DSP1.in_count = 3;	break;
		    case 0x1b: DSP1.in_count = 3;	break;
		    case 0x2b: DSP1.in_count = 3;	break;
		    case 0x14: DSP1.in_count = 6;	break;
//		    case 0x80: DSP1.in_count = 2;	break;

		    default:
		    case 0x80:
			DSP1.in_count = 0;
			DSP1.waiting4command = TRUE;
			DSP1.first_parameter = TRUE;
			break;
		}
	    }
	    else
	    {
		DSP1.parameters [DSP1.in_index] = byte;
		DSP1.first_parameter = FALSE;
	    }
	}
	else
	{
	    if (DSP1.waiting4command ||
		(DSP1.first_parameter && byte == 0x80))
	    {
		DSP1.waiting4command = TRUE;
		DSP1.first_parameter = FALSE;
	    }
	    else
	    if (DSP1.first_parameter)
	    {
	    }
	    else
	    {
		if (DSP1.in_count)
		{
		    DSP1.parameters [DSP1.in_index] |= (byte << 8);
		    if (--DSP1.in_count == 0)
		    {
			// Actually execute the command
			DSP1.waiting4command = TRUE;
			DSP1.out_index = 0;
			switch (DSP1.command)
			{
			    case 0x00:	// Multiple
				Op00Multiplicand = (int16) DSP1.parameters [0];
				Op00Multiplier = (int16) DSP1.parameters [1];

				DSPOp00 ();

				DSP1.out_count = 1;
				DSP1.output [0] = Op00Result;
				break;

			    case 0x10:	// Inverse
				Op10Coefficient = (int16) DSP1.parameters [0];
				Op10Exponent = (int16) DSP1.parameters [1];

				DSPOp10 ();

				DSP1.out_count = 2;
				DSP1.output [0] = (uint16) (int16) Op10CoefficientR;
				DSP1.output [1] = (uint16) (int16) Op10ExponentR;
				break;

			    case 0x04:	// Sin and Cos of angle
				Op04Angle = (int16) DSP1.parameters [0];
				Op04Radius = (uint16) DSP1.parameters [1];

				DSPOp04 ();

				DSP1.out_count = 2;
				DSP1.output [0] = (uint16) Op04Sin;
				DSP1.output [1] = (uint16) Op04Cos;
				break;

			    case 0x08:	// Radius
				Op08X = (int16) DSP1.parameters [0];
				Op08Y = (int16) DSP1.parameters [1];
				Op08Z = (int16) DSP1.parameters [2];

				DSPOp08 ();

				DSP1.out_count = 2;
				DSP1.output [0] = (int16) Op08Ll; 
				DSP1.output [1] = (int16) Op08Lh;
				break;

			    case 0x18:	// Range
				
				Op18X = (int16) DSP1.parameters [0];
				Op18Y = (int16) DSP1.parameters [1];
				Op18Z = (int16) DSP1.parameters [2];
				Op18R = (int16) DSP1.parameters [3];

				DSPOp18 ();

				DSP1.out_count = 1;
				DSP1.output [0] = Op18D;
				break;

			    case 0x28:	// Distance (vector length)
				Op28X = (int16) DSP1.parameters [0];
				Op28Y = (int16) DSP1.parameters [1];
				Op28Z = (int16) DSP1.parameters [2];

				DSPOp28 ();

				DSP1.out_count = 1;
				DSP1.output [0] = (uint16) Op28R;
				break;

			    case 0x0c:	// Rotate (2D rotate)
				Op0CA = (int16) DSP1.parameters [0];
				Op0CX1 = (int16) DSP1.parameters [1];
				Op0CY1 = (int16) DSP1.parameters [2];

				DSPOp0C ();

				DSP1.out_count = 2;
				DSP1.output [0] = (uint16) Op0CX2;
				DSP1.output [1] = (uint16) Op0CY2;
				break;

			    case 0x1c:	// Polar (3D rotate)
				Op1CZ = DSP1.parameters [0];
				Op1CX = DSP1.parameters [1];
				Op1CY = DSP1.parameters [2];
				Op1CXBR = DSP1.parameters [3];
				Op1CYBR = DSP1.parameters [4];
				Op1CZBR = DSP1.parameters [5];

				DSPOp1C ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op1CXAR;
				DSP1.output [1] = (uint16) Op1CYAR;
				DSP1.output [2] = (uint16) Op1CZAR;
				break;

			    case 0x02:	// Parameter (Projection)
				Op02FX = DSP1.parameters [0];
				Op02FY = DSP1.parameters [1];
				Op02FZ = DSP1.parameters [2];
				Op02LFE = DSP1.parameters [3];
				Op02LES = DSP1.parameters [4];
				Op02AAS = DSP1.parameters [5];
				Op02AZS = DSP1.parameters [6];

				DSPOp02 ();

				DSP1.out_count = 4;
				DSP1.output [0] = Op02VOF;
				DSP1.output [1] = Op02VVA;
				DSP1.output [2] = Op02CX;
				DSP1.output [3] = Op02CY;
				break;

			    case 0x1a:	// Raster mode 7 matrix data
			    case 0x0a:
				Op0AVS = DSP1.parameters [0];

				DSPOp0A ();

				DSP1.out_count = 4;
				DSP1.output [0] = Op0AA;
				DSP1.output [1] = Op0AB;
				DSP1.output [2] = Op0AC;
				DSP1.output [3] = Op0AD;
				break;

			    case 0x06:	// Project object
				Op06X = (int16) DSP1.parameters [0];
				Op06Y = (int16) DSP1.parameters [1];
				Op06Z = (int16) DSP1.parameters [2];

				DSPOp06 ();

				DSP1.out_count = 3;
				DSP1.output [0] = Op06H;
				DSP1.output [1] = Op06V;
				DSP1.output [2] = Op06S;
				break;
			    
			    case 0x0e:	// Target
				Op0EH = (int16) DSP1.parameters [0];
				Op0EV = (int16) DSP1.parameters [1];

				DSPOp0E ();

				DSP1.out_count = 2;
				DSP1.output [0] = Op0EX;
				DSP1.output [1] = Op0EY;
				break;

			    // Extra commands used by Pilot Wings
			    case 0x01: // Set attitude matrix A
				Op01m = (int16) DSP1.parameters [0];
				Op01Zr = (int16) DSP1.parameters [1];
				Op01Xr = (int16) DSP1.parameters [2];
				Op01Yr = (int16) DSP1.parameters [3];

				DSPOp01 ();
				break;

			    case 0x11:	// Set attitude matrix B
				Op11m = (int16) DSP1.parameters [0];
				Op11Zr = (int16) DSP1.parameters [1];
				Op11Xr = (int16) DSP1.parameters [2];
				Op11Yr = (int16) DSP1.parameters [3];

				DSPOp11 ();
				break;

			    case 0x21:	// Set attitude matrix C
				Op21m = (int16) DSP1.parameters [0];
				Op21Zr = (int16) DSP1.parameters [1];
				Op21Xr = (int16) DSP1.parameters [2];
				Op21Yr = (int16) DSP1.parameters [3];

				DSPOp21 ();
				break;

			    case 0x0d:	// Objective matrix A
				Op0DX = (int16) DSP1.parameters [0];
				Op0DY = (int16) DSP1.parameters [1];
				Op0DZ = (int16) DSP1.parameters [2];

				DSPOp0D ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op0DF;
				DSP1.output [1] = (uint16) Op0DL;
				DSP1.output [2] = (uint16) Op0DU;
				break;

			    case 0x1d:	// Objective matrix B
				Op1DX = (int16) DSP1.parameters [0];
				Op1DY = (int16) DSP1.parameters [1];
				Op1DZ = (int16) DSP1.parameters [2];

				DSPOp1D ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op1DF;
				DSP1.output [1] = (uint16) Op1DL;
				DSP1.output [2] = (uint16) Op1DU;
				break;

			    case 0x2d:	// Objective matrix C
				Op2DX = (int16) DSP1.parameters [0];
				Op2DY = (int16) DSP1.parameters [1];
				Op2DZ = (int16) DSP1.parameters [2];

				DSPOp2D ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op2DF;
				DSP1.output [1] = (uint16) Op2DL;
				DSP1.output [2] = (uint16) Op2DU;
				break;

			    case 0x03:	// Subjective matrix A
				Op03F = (int16) DSP1.parameters [0];
				Op03L = (int16) DSP1.parameters [1];
				Op03U = (int16) DSP1.parameters [2];

				DSPOp03 ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op03X;
				DSP1.output [1] = (uint16) Op03Y;
				DSP1.output [2] = (uint16) Op03Z;
				break;

			    case 0x13:	// Subjective matrix B
				Op13F = (int16) DSP1.parameters [0];
				Op13L = (int16) DSP1.parameters [1];
				Op13U = (int16) DSP1.parameters [2];

				DSPOp13 ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op13X;
				DSP1.output [1] = (uint16) Op13Y;
				DSP1.output [2] = (uint16) Op13Z;
				break;

			    case 0x23:	// Subjective matrix C
				Op23F = (int16) DSP1.parameters [0];
				Op23L = (int16) DSP1.parameters [1];
				Op23U = (int16) DSP1.parameters [2];

				DSPOp23 ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op23X;
				DSP1.output [1] = (uint16) Op23Y;
				DSP1.output [2] = (uint16) Op23Z;
				break;

			    case 0x0b:
				Op0BX = (int16) DSP1.parameters [0];
				Op0BY = (int16) DSP1.parameters [1];
				Op0BZ = (int16) DSP1.parameters [2];

				DSPOp0B ();

				DSP1.out_count = 1;
				DSP1.output [0] = (uint16) Op0BS;
				break;

			    case 0x1b:
				Op1BX = (int16) DSP1.parameters [0];
				Op1BY = (int16) DSP1.parameters [1];
				Op1BZ = (int16) DSP1.parameters [2];

				DSPOp1B ();

				DSP1.out_count = 1;
				DSP1.output [0] = (uint16) Op1BS;
				break;
				
			    case 0x2b:
				Op2BX = (int16) DSP1.parameters [0];
				Op2BY = (int16) DSP1.parameters [1];
				Op2BZ = (int16) DSP1.parameters [2];

				DSPOp0B ();

				DSP1.out_count = 1;
				DSP1.output [0] = (uint16) Op2BS;
				break;

			    case 0x14:	// Gyrate
				Op14Zr = (int16) DSP1.parameters [0];
				Op14Xr = (int16) DSP1.parameters [1];
				Op14Yr = (int16) DSP1.parameters [2];
				Op14U = (int16) DSP1.parameters [3];
				Op14F = (int16) DSP1.parameters [4];
				Op14L = (int16) DSP1.parameters [5];

				DSPOp14 ();

				DSP1.out_count = 3;
				DSP1.output [0] = (uint16) Op14Zrr;
				DSP1.output [1] = (uint16) Op14Xrr;
				DSP1.output [2] = (uint16) Op14Yrr;
				break;

			    default:
			        break;
			}
		    }
		    else
			DSP1.in_index++;
		}
	    }
	}
    }
}
