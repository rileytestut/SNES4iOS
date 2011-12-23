//Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later
//version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __GP32__ 
#include <stdio.h>
#include <stdarg.h>
#endif
#include <math.h>

#ifndef __GP32__ 
#include <string.h>
#include <stdlib.h>
#endif
//#define DebugDSP1

#define funcSADDMULT1616(a,b,c,d) (((int64)a*(int64)b+(int64)c*(int64)d)>>16);

#define SADDMULT1616(res,a,b,c,d) {\
	res=funcSADDMULT1616(a,b,c,d);\
	}
#define SMULT1616(res,a,b) {\
	res=funcSADDMULT1616(a,b,0,0);\
	}

// uncomment some lines to test
//#define printinfo
//#define debug02
//#define debug0A
//#define debug06

//#ifdef __GP32__
//for the SMUL1616 & UMUL1616
//#include "gp32_func.h"
//#endif

#define __OPT__
#define __OPT01__
#define __OPT02__
#define __OPT04__
#define __OPT06__
#define __OPT0C__    // this optimisation may break pilotwings
#define __OPT11__
#define __OPT21__
#define __OPT1C__

#ifdef DebugDSP1

FILE * LogFile = NULL;

void Log_Message (char *Message, ...)
{
	char Msg[400];
	va_list ap;

   va_start(ap,Message);
   vsprintf(Msg,Message,ap );
   va_end(ap);
	
   strcat(Msg,"\r\n\0");
   fwrite(Msg,strlen(Msg),1,LogFile);
   fflush (LogFile);
}

void Start_Log (void)
{
	char LogFileName[255];
//  [4/15/2001]	char *p;

   strcpy(LogFileName,"dsp1emu.log\0");
	
   LogFile = fopen(LogFileName,"wb");
}

void Stop_Log (void)
{
   if (LogFile)
   {
      fclose(LogFile);
      LogFile = NULL;
	}
}

#endif

const unsigned short DSP1ROM[1024] = {
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0001,	0x0002,	0x0004,	0x0008,	0x0010,	0x0020,	
	0x0040,	0x0080,	0x0100,	0x0200,	0x0400,	0x0800,	0x1000,	0x2000,	
	0x4000,	0x7fff,	0x4000,	0x2000,	0x1000,	0x0800,	0x0400,	0x0200,	
	0x0100,	0x0080,	0x0040,	0x0020,	0x0001,	0x0008,	0x0004,	0x0002,	
	0x0001,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	0x0000,	
	0x0000,	0x0000,	0x8000,	0xffe5,	0x0100,	0x7fff,	0x7f02,	0x7e08,	
	0x7d12,	0x7c1f,	0x7b30,	0x7a45,	0x795d,	0x7878,	0x7797,	0x76ba,	
	0x75df,	0x7507,	0x7433,	0x7361,	0x7293,	0x71c7,	0x70fe,	0x7038,	
	0x6f75,	0x6eb4,	0x6df6,	0x6d3a,	0x6c81,	0x6bca,	0x6b16,	0x6a64,	
	0x69b4,	0x6907,	0x685b,	0x67b2,	0x670b,	0x6666,	0x65c4,	0x6523,	
	0x6484,	0x63e7,	0x634c,	0x62b3,	0x621c,	0x6186,	0x60f2,	0x6060,	
	0x5fd0,	0x5f41,	0x5eb5,	0x5e29,	0x5d9f,	0x5d17,	0x5c91,	0x5c0c,	
	0x5b88,	0x5b06,	0x5a85,	0x5a06,	0x5988,	0x590b,	0x5890,	0x5816,	
	0x579d,	0x5726,	0x56b0,	0x563b,	0x55c8,	0x5555,	0x54e4,	0x5474,	
	0x5405,	0x5398,	0x532b,	0x52bf,	0x5255,	0x51ec,	0x5183,	0x511c,	
	0x50b6,	0x5050,	0x4fec,	0x4f89,	0x4f26,	0x4ec5,	0x4e64,	0x4e05,	
	0x4da6,	0x4d48,	0x4cec,	0x4c90,	0x4c34,	0x4bda,	0x4b81,	0x4b28,	
	0x4ad0,	0x4a79,	0x4a23,	0x49cd,	0x4979,	0x4925,	0x48d1,	0x487f,	
	0x482d,	0x47dc,	0x478c,	0x473c,	0x46ed,	0x469f,	0x4651,	0x4604,	
	0x45b8,	0x456c,	0x4521,	0x44d7,	0x448d,	0x4444,	0x43fc,	0x43b4,	
	0x436d,	0x4326,	0x42e0,	0x429a,	0x4255,	0x4211,	0x41cd,	0x4189,	
	0x4146,	0x4104,	0x40c2,	0x4081,	0x4040,	0x3fff,	0x41f7,	0x43e1,	
	0x45bd,	0x478d,	0x4951,	0x4b0b,	0x4cbb,	0x4e61,	0x4fff,	0x5194,	
	0x5322,	0x54a9,	0x5628,	0x57a2,	0x5914,	0x5a81,	0x5be9,	0x5d4a,	
	0x5ea7,	0x5fff,	0x6152,	0x62a0,	0x63ea,	0x6530,	0x6672,	0x67b0,	
	0x68ea,	0x6a20,	0x6b53,	0x6c83,	0x6daf,	0x6ed9,	0x6fff,	0x7122,	
	0x7242,	0x735f,	0x747a,	0x7592,	0x76a7,	0x77ba,	0x78cb,	0x79d9,	
	0x7ae5,	0x7bee,	0x7cf5,	0x7dfa,	0x7efe,	0x7fff,	0x0000,	0x0324,	
	0x0647,	0x096a,	0x0c8b,	0x0fab,	0x12c8,	0x15e2,	0x18f8,	0x1c0b,	
	0x1f19,	0x2223,	0x2528,	0x2826,	0x2b1f,	0x2e11,	0x30fb,	0x33de,	
	0x36ba,	0x398c,	0x3c56,	0x3f17,	0x41ce,	0x447a,	0x471c,	0x49b4,	
	0x4c3f,	0x4ebf,	0x5133,	0x539b,	0x55f5,	0x5842,	0x5a82,	0x5cb4,	
	0x5ed7,	0x60ec,	0x62f2,	0x64e8,	0x66cf,	0x68a6,	0x6a6d,	0x6c24,	
	0x6dca,	0x6f5f,	0x70e2,	0x7255,	0x73b5,	0x7504,	0x7641,	0x776c,	
	0x7884,	0x798a,	0x7a7d,	0x7b5d,	0x7c29,	0x7ce3,	0x7d8a,	0x7e1d,	
	0x7e9d,	0x7f09,	0x7f62,	0x7fa7,	0x7fd8,	0x7ff6,	0x7fff,	0x7ff6,	
	0x7fd8,	0x7fa7,	0x7f62,	0x7f09,	0x7e9d,	0x7e1d,	0x7d8a,	0x7ce3,	
	0x7c29,	0x7b5d,	0x7a7d,	0x798a,	0x7884,	0x776c,	0x7641,	0x7504,	
	0x73b5,	0x7255,	0x70e2,	0x6f5f,	0x6dca,	0x6c24,	0x6a6d,	0x68a6,	
	0x66cf,	0x64e8,	0x62f2,	0x60ec,	0x5ed7,	0x5cb4,	0x5a82,	0x5842,	
	0x55f5,	0x539b,	0x5133,	0x4ebf,	0x4c3f,	0x49b4,	0x471c,	0x447a,	
	0x41ce,	0x3f17,	0x3c56,	0x398c,	0x36ba,	0x33de,	0x30fb,	0x2e11,	
	0x2b1f,	0x2826,	0x2528,	0x2223,	0x1f19,	0x1c0b,	0x18f8,	0x15e2,	
	0x12c8,	0x0fab,	0x0c8b,	0x096a,	0x0647,	0x0324,	0x7fff,	0x7ff6,	
	0x7fd8,	0x7fa7,	0x7f62,	0x7f09,	0x7e9d,	0x7e1d,	0x7d8a,	0x7ce3,	
	0x7c29,	0x7b5d,	0x7a7d,	0x798a,	0x7884,	0x776c,	0x7641,	0x7504,	
	0x73b5,	0x7255,	0x70e2,	0x6f5f,	0x6dca,	0x6c24,	0x6a6d,	0x68a6,	
	0x66cf,	0x64e8,	0x62f2,	0x60ec,	0x5ed7,	0x5cb4,	0x5a82,	0x5842,	
	0x55f5,	0x539b,	0x5133,	0x4ebf,	0x4c3f,	0x49b4,	0x471c,	0x447a,	
	0x41ce,	0x3f17,	0x3c56,	0x398c,	0x36ba,	0x33de,	0x30fb,	0x2e11,	
	0x2b1f,	0x2826,	0x2528,	0x2223,	0x1f19,	0x1c0b,	0x18f8,	0x15e2,	
	0x12c8,	0x0fab,	0x0c8b,	0x096a,	0x0647,	0x0324,	0x0000,	0xfcdc,	
	0xf9b9,	0xf696,	0xf375,	0xf055,	0xed38,	0xea1e,	0xe708,	0xe3f5,	
	0xe0e7,	0xdddd,	0xdad8,	0xd7da,	0xd4e1,	0xd1ef,	0xcf05,	0xcc22,	
	0xc946,	0xc674,	0xc3aa,	0xc0e9,	0xbe32,	0xbb86,	0xb8e4,	0xb64c,	
	0xb3c1,	0xb141,	0xaecd,	0xac65,	0xaa0b,	0xa7be,	0xa57e,	0xa34c,	
	0xa129,	0x9f14,	0x9d0e,	0x9b18,	0x9931,	0x975a,	0x9593,	0x93dc,	
	0x9236,	0x90a1,	0x8f1e,	0x8dab,	0x8c4b,	0x8afc,	0x89bf,	0x8894,	
	0x877c,	0x8676,	0x8583,	0x84a3,	0x83d7,	0x831d,	0x8276,	0x81e3,	
	0x8163,	0x80f7,	0x809e,	0x8059,	0x8028,	0x800a,	0x6488,	0x0080,	
	0x03ff,	0x0116,	0x0002,	0x0080,	0x4000,	0x3fd7,	0x3faf,	0x3f86,	
	0x3f5d,	0x3f34,	0x3f0c,	0x3ee3,	0x3eba,	0x3e91,	0x3e68,	0x3e40,	
	0x3e17,	0x3dee,	0x3dc5,	0x3d9c,	0x3d74,	0x3d4b,	0x3d22,	0x3cf9,	
	0x3cd0,	0x3ca7,	0x3c7f,	0x3c56,	0x3c2d,	0x3c04,	0x3bdb,	0x3bb2,	
	0x3b89,	0x3b60,	0x3b37,	0x3b0e,	0x3ae5,	0x3abc,	0x3a93,	0x3a69,	
	0x3a40,	0x3a17,	0x39ee,	0x39c5,	0x399c,	0x3972,	0x3949,	0x3920,	
	0x38f6,	0x38cd,	0x38a4,	0x387a,	0x3851,	0x3827,	0x37fe,	0x37d4,	
	0x37aa,	0x3781,	0x3757,	0x372d,	0x3704,	0x36da,	0x36b0,	0x3686,	
	0x365c,	0x3632,	0x3609,	0x35df,	0x35b4,	0x358a,	0x3560,	0x3536,	
	0x350c,	0x34e1,	0x34b7,	0x348d,	0x3462,	0x3438,	0x340d,	0x33e3,	
	0x33b8,	0x338d,	0x3363,	0x3338,	0x330d,	0x32e2,	0x32b7,	0x328c,	
	0x3261,	0x3236,	0x320b,	0x31df,	0x31b4,	0x3188,	0x315d,	0x3131,	
	0x3106,	0x30da,	0x30ae,	0x3083,	0x3057,	0x302b,	0x2fff,	0x2fd2,	
	0x2fa6,	0x2f7a,	0x2f4d,	0x2f21,	0x2ef4,	0x2ec8,	0x2e9b,	0x2e6e,	
	0x2e41,	0x2e14,	0x2de7,	0x2dba,	0x2d8d,	0x2d60,	0x2d32,	0x2d05,	
	0x2cd7,	0x2ca9,	0x2c7b,	0x2c4d,	0x2c1f,	0x2bf1,	0x2bc3,	0x2b94,	
	0x2b66,	0x2b37,	0x2b09,	0x2ada,	0x2aab,	0x2a7c,	0x2a4c,	0x2a1d,	
	0x29ed,	0x29be,	0x298e,	0x295e,	0x292e,	0x28fe,	0x28ce,	0x289d,	
	0x286d,	0x283c,	0x280b,	0x27da,	0x27a9,	0x2777,	0x2746,	0x2714,	
	0x26e2,	0x26b0,	0x267e,	0x264c,	0x2619,	0x25e7,	0x25b4,	0x2581,	
	0x254d,	0x251a,	0x24e6,	0x24b2,	0x247e,	0x244a,	0x2415,	0x23e1,	
	0x23ac,	0x2376,	0x2341,	0x230b,	0x22d6,	0x229f,	0x2269,	0x2232,	
	0x21fc,	0x21c4,	0x218d,	0x2155,	0x211d,	0x20e5,	0x20ad,	0x2074,	
	0x203b,	0x2001,	0x1fc7,	0x1f8d,	0x1f53,	0x1f18,	0x1edd,	0x1ea1,	
	0x1e66,	0x1e29,	0x1ded,	0x1db0,	0x1d72,	0x1d35,	0x1cf6,	0x1cb8,	
	0x1c79,	0x1c39,	0x1bf9,	0x1bb8,	0x1b77,	0x1b36,	0x1af4,	0x1ab1,	
	0x1a6e,	0x1a2a,	0x19e6,	0x19a1,	0x195c,	0x1915,	0x18ce,	0x1887,	
	0x183f,	0x17f5,	0x17ac,	0x1761,	0x1715,	0x16c9,	0x167c,	0x162e,	
	0x15df,	0x158e,	0x153d,	0x14eb,	0x1497,	0x1442,	0x13ec,	0x1395,	
	0x133c,	0x12e2,	0x1286,	0x1228,	0x11c9,	0x1167,	0x1104,	0x109e,	
	0x1036,	0x0fcc,	0x0f5f,	0x0eef,	0x0e7b,	0x0e04,	0x0d89,	0x0d0a,	
	0x0c86,	0x0bfd,	0x0b6d,	0x0ad6,	0x0a36,	0x098d,	0x08d7,	0x0811,	
	0x0736,	0x063e,	0x0519,	0x039a,	0x0000,	0x7fff,	0x0100,	0x0080,	
	0x021d,	0x00c8,	0x00ce,	0x0048,	0x0a26,	0x277a,	0x00ce,	0x6488,	
	0x14ac,	0x0001,	0x00f9,	0x00fc,	0x00ff,	0x00fc,	0x00f9,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	
	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff,	0xffff};



/***************************************************************************\
*  Math tables                                                              *
\***************************************************************************/

#define INCR 2048  //<<11
#define INCR_SHIFT 5 //16-11 coz (>>INCR_SHIFT) = (*INCR/65536)
#define _FIX_SHIFT_ 16
//double *CosTable2;
int32 CosTable2[INCR];
//double *SinTable2;
int32 SinTable2[INCR];

//#define Angle(x) (((x)/(65536/INCR)) & (INCR-1))
#define Angle(x) (((int32)(x)>>INCR_SHIFT) & (INCR-1) )
//#define Cos(x) ((double) CosTable2[x])
#define Cos(x) ((int32) CosTable2[x])
//#define Sin(x) ((double) SinTable2[x])
#define Sin(x) ((int32) SinTable2[x])
//#define PI 3.14159265358979323846264338327
#define PI_float 3.14159265358979323846264338327f
#define PI 205887 //3.14159265358979323846264338327 << _FIX_SHIFT_

//double Atan(double x)
int32 Atan(int32 x)
{
//	if ((x>=1) || (x<=1))    //stupid ?
	if ((x>=(1<<_FIX_SHIFT_)) || (x<=(1<<_FIX_SHIFT_)))    //stupid ?
		//return (x/(1+0.28*x*x));
		{int32 t;SMULT1616(t,x,x)
		 return (  ( (int64)x<<_FIX_SHIFT_ )  /(1+0.28f*t) );}
	else
		//return ( PI/2 - Atan(1/x));
		return ( PI/2 - Atan( ((int64)1<<(_FIX_SHIFT_*2))/x) );
}

/***************************************************************************\
*  DSP1 code                                                                *
\***************************************************************************/


void InitDSP(void)
{
#ifdef __OPT__
        unsigned int i;
	//CosTable2 = (double *) malloc(INCR*sizeof(double));
	//CosTable2 = (int32 *) malloc(INCR*sizeof(int32));
	//SinTable2 = (double *) malloc(INCR*sizeof(double));
	//SinTable2 = (int32 *) malloc(INCR*sizeof(int32));
	for (i=0; i<INCR; i++){
		//CosTable2[i] = (cos((double)(2*PI*i/INCR)));
		CosTable2[i] = (int32)((cos((double)(2*PI_float*i/INCR)))*(1<<_FIX_SHIFT_));
		//SinTable2[i] = (sin((double)(2*PI*i/INCR)));
		SinTable2[i] = (int32)((sin((double)(2*PI_float*i/INCR)))*(1<<_FIX_SHIFT_));
	}
#endif
#ifdef DebugDSP1
	Start_Log();
#endif
}


short Op00Multiplicand;
short Op00Multiplier;
short Op00Result;

void DSPOp00()
{
   //Op00Result=Op00Multiplicand*Op00Multiplier/32768;
   Op00Result=Op00Multiplicand*Op00Multiplier>>15;
   #ifdef DebugDSP1
      Log_Message("OP00 MULT %d*%d/32768=%d",Op00Multiplicand,Op00Multiplier,Op00Result);
   #endif
}

short Op20Multiplicand;
short Op20Multiplier;
short Op20Result;

void DSPOp20()
{
   Op20Result= Op20Multiplicand * Op20Multiplier >> 15;
   Op20Result++;

   #ifdef DebugDSP1
      Log_Message("OP20 MULT %d*%d/32768=%d",Op20Multiplicand,Op20Multiplier,Op20Result);
   #endif
}


signed short Op10Coefficient;
signed short Op10Exponent;
signed short Op10CoefficientR;
signed short Op10ExponentR;
//float Op10Temp;
int32 Op10Temp;

void DSPOp10()
{
        Op10ExponentR=-Op10Exponent;
        //Op10Temp = Op10Coefficient / 32768.0;
        Op10Temp = (Op10Coefficient<<(_FIX_SHIFT_-15));
	if (Op10Temp == 0) {
		Op10CoefficientR = 0;
	} else
		//Op10Temp = 1/Op10Temp;	
		Op10Temp =  ((int64)(1)<<(_FIX_SHIFT_*2)) /Op10Temp ;	
        if (Op10Temp > 0) 
                //while (Op10Temp>=1.0) {
                while (Op10Temp>=(1<<_FIX_SHIFT_)) {
                        //Op10Temp=Op10Temp/2.0;
                        Op10Temp=Op10Temp>>1;
                        Op10ExponentR++;
                }
        else
                //while (Op10Temp<-1.0) {
                while (Op10Temp<-(1<<_FIX_SHIFT_)) {
                        //Op10Temp=Op10Temp/2.0;
                        Op10Temp=Op10Temp>>1;
                        Op10ExponentR++;
                }
        //Op10CoefficientR = Op10Temp*32768;
        Op10CoefficientR = Op10Temp>>(_FIX_SHIFT_-15);
	#ifdef DebugDSP1
        Log_Message("OP10 INV %d*2^%d = %d*2^%d", Op10Coefficient, Op10Exponent, Op10CoefficientR, Op10ExponentR);
	#endif
}


short Op04Angle;
unsigned short Op04Radius;
short Op04Sin;
short Op04Cos;

#ifdef __OPT04__

void DSPOp04()
{
   int angle;
   
   angle = Angle(Op04Angle);

   //Op04Sin = Sin(angle) * Op04Radius;
   //Op04Cos = Cos(angle) * Op04Radius;
   SMULT1616(Op04Sin,Sin(angle),Op04Radius)
   SMULT1616(Op04Cos,Cos(angle),Op04Radius)

   #ifdef DebugDSP1
      Log_Message("OP04 Angle:%d Radius:%d",(Op04Angle/256)&255,Op04Radius);
      Log_Message("OP04 SIN:%d COS:%d",Op04Sin,Op04Cos);
   #endif
}
#else

void DSPOp04()
{
   double angle;
   
   angle = Op04Angle*2*PI/65536.0;

   Op04Sin = sin(angle) * Op04Radius;
   Op04Cos = cos(angle) * Op04Radius;

   #ifdef DebugDSP1
      Log_Message("OP04 Angle:%d Radius:%d",(Op04Angle/256)&255,Op04Radius);
      Log_Message("OP04 SIN:%d COS:%d",Op04Sin,Op04Cos);
   #endif
}
#endif 

unsigned short Op0CA;
short Op0CX1;
short Op0CY1;
short Op0CX2;
short Op0CY2;

#ifdef __OPT0C__
void DSPOp0C()
{
   //Op0CX2=Op0CX1*Cos(Angle(Op0CA))+Op0CY1*Sin(Angle(Op0CA));
   //Op0CY2=Op0CX1*-Sin(Angle(Op0CA))+Op0CY1*Cos(Angle(Op0CA));
   Op0CX2=((int32)Op0CX1*Cos(Angle(Op0CA))+(int32)Op0CY1*Sin(Angle(Op0CA)))>>_FIX_SHIFT_;
   Op0CY2=((int32)Op0CX1*-Sin(Angle(Op0CA))+(int32)Op0CY1*Cos(Angle(Op0CA)))>>_FIX_SHIFT_;
   
   #ifdef DebugDSP1
      Log_Message("OP0C Angle:%d X:%d Y:%d CX:%d CY:%d",(Op0CA/256)&255,Op0CX1,Op0CY1,Op0CX2,Op0CY2);
   #endif
}
#else
void DSPOp0C()
{
	
   Op0CX2=(Op0CX1*cos(Op0CA*2*PI/65536.0)+Op0CY1*sin(Op0CA*2*PI/65536.0));
   Op0CY2=(Op0CX1*-sin(Op0CA*2*PI/65536.0)+Op0CY1*cos(Op0CA*2*PI/65536.0));
   #ifdef DebugDSP1
      Log_Message("OP0C Angle:%d X:%d Y:%d CX:%d CY:%d",(Op0CA/256)&255,Op0CX1,Op0CY1,Op0CX2,Op0CY2);
   #endif
}

#endif

short Op02FX;
short Op02FY;
short Op02FZ;
short Op02LFE;
short Op02LES;
unsigned short Op02AAS;
unsigned short Op02AZS;
unsigned short Op02VOF;
unsigned short Op02VVA;

short Op02CX;
short Op02CY;
/*double Op02CXF;
double Op02CYF;
double ViewerX0;
double ViewerY0;
double ViewerZ0;
double ViewerX1;
double ViewerY1;
double ViewerZ1;
double ViewerX;
double ViewerY;
double ViewerZ;*/
int32 Op02CXF;
int32 Op02CYF;
int32 ViewerX0;
int32 ViewerY0;
int32 ViewerZ0;
int32 ViewerX1;
int32 ViewerY1;
int32 ViewerZ1;
int32 ViewerX;
int32 ViewerY;
int32 ViewerZ;
int ViewerAX;
int ViewerAY;
int ViewerAZ;
/*double NumberOfSlope;
double ScreenX;
double ScreenY;
double ScreenZ;
double TopLeftScreenX;
double TopLeftScreenY;
double TopLeftScreenZ;
double BottomRightScreenX;
double BottomRightScreenY;
double BottomRightScreenZ;
double Ready;
double RasterLX;
double RasterLY;
double RasterLZ;
double ScreenLX1;
double ScreenLY1;
double ScreenLZ1;*/
int32 NumberOfSlope;
int32 ScreenX;
int32 ScreenY;
int32 ScreenZ;
int32 TopLeftScreenX;
int32 TopLeftScreenY;
int32 TopLeftScreenZ;
int32 BottomRightScreenX;
int32 BottomRightScreenY;
int32 BottomRightScreenZ;
int32 Ready;
int32 RasterLX;
int32 RasterLY;
int32 RasterLZ;
int32 ScreenLX1;
int32 ScreenLY1;
int32 ScreenLZ1;
int    ReversedLES;
short Op02LESb;
/*double NAzsB,NAasB;
double ViewerXc;
double ViewerYc;
double ViewerZc;
double CenterX,CenterY;*/
int32 NAzsB,NAasB;
int32 ViewerXc;
int32 ViewerYc;
int32 ViewerZc;
int32 CenterX,CenterY;
short Op02CYSup,Op02CXSup;
//double CXdistance;
int32 CXdistance;

#define VofAngle 0x3880

short TValDebug,TValDebug2;
short ScrDispl;


#ifdef __OPT02__
void DSPOp02()
{
	ViewerZ1=-Cos(Angle(Op02AZS));
/*  ViewerX1=Sin(Angle(Op02AZS))*Sin(Angle(Op02AAS));
	ViewerY1=Sin(Angle(Op02AZS))*Cos(Angle(Op02AAS));*/
	SMULT1616(ViewerX1,Sin(Angle(Op02AZS)),Sin(Angle(Op02AAS)))
	SMULT1616(ViewerY1,Sin(Angle(Op02AZS)),Cos(Angle(Op02AAS)))

	
   #ifdef debug02
   printf("\nViewerX1 : %f ViewerY1 : %f ViewerZ1 : %f\n",ViewerX1,ViewerY1,
                                                                   ViewerZ1);
   getch();
   #endif
   /*ViewerX=Op02FX-ViewerX1*Op02LFE;
   ViewerY=Op02FY-ViewerY1*Op02LFE;
   ViewerZ=Op02FZ-ViewerZ1*Op02LFE;

   ScreenX=Op02FX+ViewerX1*(Op02LES-Op02LFE);
   ScreenY=Op02FY+ViewerY1*(Op02LES-Op02LFE);
   ScreenZ=Op02FZ+ViewerZ1*(Op02LES-Op02LFE);*/
   ViewerX=((int32)Op02FX<<_FIX_SHIFT_)-ViewerX1*(int32)Op02LFE;
   ViewerY=((int32)Op02FY<<_FIX_SHIFT_)-ViewerY1*(int32)Op02LFE;
   ViewerZ=((int32)Op02FZ<<_FIX_SHIFT_)-ViewerZ1*(int32)Op02LFE;

   ScreenX=((int32)Op02FX<<_FIX_SHIFT_)+ViewerX1*(int32)(Op02LES-Op02LFE);
   ScreenY=((int32)Op02FY<<_FIX_SHIFT_)+ViewerY1*(int32)(Op02LES-Op02LFE);
   ScreenZ=((int32)Op02FZ<<_FIX_SHIFT_)+ViewerZ1*(int32)(Op02LES-Op02LFE);

   #ifdef debug02
   printf("ViewerX : %f ViewerY : %f ViewerZ : %f\n",ViewerX,ViewerY,ViewerZ);
   printf("Op02FX : %d Op02FY : %d Op02FZ : %d\n",Op02FX,Op02FY,Op02FZ);
   printf("ScreenX : %f ScreenY : %f ScreenZ : %f\n",ScreenX,ScreenY,ScreenZ);
   getch();
   #endif
   if (ViewerZ1==0)ViewerZ1++;
   NumberOfSlope=((int64)ViewerZ<<_FIX_SHIFT_)/(-ViewerZ1);

   //Op02CX=(short)(Op02CXF=ViewerX+ViewerX1*NumberOfSlope);
   //Op02CY=(short)(Op02CYF=ViewerY+ViewerY1*NumberOfSlope);
   int32 t;
   SMULT1616(t,ViewerX1,NumberOfSlope)
   Op02CX=(short)(Op02CXF=(ViewerX+t)>>_FIX_SHIFT_);
   SMULT1616(t,ViewerY1,NumberOfSlope)
   Op02CY=(short)(Op02CYF=(ViewerY+t)>>_FIX_SHIFT_);

   Op02VOF=0x0000;
   ReversedLES=0;
   Op02LESb=Op02LES;
   //if ((Op02LES>=VofAngle+16384.0) && (Op02LES<VofAngle+32768.0)) {
   if ((Op02LES>=VofAngle+16384) && (Op02LES<VofAngle+32768)) {
     ReversedLES=1;
     Op02LESb=VofAngle+0x4000-(Op02LES-(VofAngle+0x4000));
   }
   //to be optimized here : tan
   Op02VVA = (short)(Op02LESb * tan((Op02AZS-0x4000)*6.2832/65536.0));
   if ((Op02LESb>=VofAngle) && (Op02LESb<=VofAngle+0x4000)) {
      Op02VOF= (short)(Op02LESb * tan((Op02AZS-0x4000-VofAngle)*6.2832/65536.0));
      Op02VVA-=Op02VOF;
   }
   if (ReversedLES){
     Op02VOF=-Op02VOF;
   }

   //NAzsB = (Op02AZS-0x4000)*6.2832/65536.0;
   NAzsB = (int32)(Op02AZS-0x4000);
   //NAasB = Op02AAS*6.2832/65536.0;
   NAasB = (int32)(Op02AAS);

   //if (tan(NAzsB)==0) NAzsB=0.1;
   if (Sin(Angle(NAzsB))==0) NAzsB=1043; //0.1*65536/(2*pi)

   ScrDispl=0;
   //if (NAzsB>-0.15) {NAzsB=-0.15;ScrDispl=Op02VVA-0xFFDA;}
   if (NAzsB>-1565 /*0.15*65536/2/pi*/) {NAzsB=-1565;ScrDispl=Op02VVA-0xFFDA;}

   //CXdistance=1/tan(NAzsB);
   CXdistance=((int64)Cos(Angle(NAzsB))<<_FIX_SHIFT_)/Sin(Angle((NAzsB)));


   ViewerXc=(int32)Op02FX<<_FIX_SHIFT_;
   ViewerYc=(int32)Op02FY<<_FIX_SHIFT_;
   ViewerZc=(int32)Op02FZ<<_FIX_SHIFT_;

   //CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   //CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   //Op02CX = (short)CenterX;
   //Op02CY = (short)CenterY;
   SMULT1616(t,-Sin(Angle(NAasB)),ViewerZc)
   SMULT1616(t,t,CXdistance)
   CenterX = t+ViewerXc;
   SMULT1616(t,Cos(Angle(NAasB)),ViewerZc)
   SMULT1616(t,t,CXdistance)
   CenterY = t+ViewerYc;
   Op02CX=CenterX>>_FIX_SHIFT_;
   Op02CY=CenterY>>_FIX_SHIFT_;

   ViewerXc=ViewerX;//-Op02FX);
   ViewerYc=ViewerY;//-Op02FY);
   ViewerZc=ViewerZ;//-Op02FZ);

   //CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   SMULT1616(t,-Sin(Angle(NAasB)),ViewerZc)
   SMULT1616(t,t,CXdistance)
   CenterX = t+ViewerXc;
   
   /*if (CenterX<-32768) CenterX = -32768; if (CenterX>32767) CenterX=32767;
   CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   if (CenterY<-32768) CenterY = -32768; if (CenterY>32767) CenterY=32767;*/
   
   //BUG en puissance : overflow
   if (CenterX<(-32768<<_FIX_SHIFT_)) CenterX = (-32768<<_FIX_SHIFT_); if (CenterX>(32767<<_FIX_SHIFT_)) CenterX=(32767<<_FIX_SHIFT_);
   SMULT1616(t,Cos(Angle(NAasB)),ViewerZc)
   SMULT1616(t,t,CXdistance)
   CenterY = t+ViewerYc;
   if (CenterY<(-32768<<_FIX_SHIFT_)) CenterY = (-32768<<_FIX_SHIFT_); if (CenterY>(32767<<_FIX_SHIFT_)) CenterY=(32767<<_FIX_SHIFT_);

//   TValDebug = (NAzsB*65536/6.28);
 //  TValDebug2 = ScrDispl;

//   if (Op02CY < 0) {Op02CYSup = Op02CY/256; Op02CY = 0;}
//   if (Op02CX < 0) {Op02CXSup = Op02CX/256; Op02CX = 0;}

//  [4/15/2001]   (ViewerX+ViewerX1*NumberOfSlope);
//  [4/15/2001]   (ViewerY+ViewerY1*NumberOfSlope);

//   if(Op02LFE==0x2200)Op02VVA=0xFECD;
//   else Op02VVA=0xFFB2;


   #ifdef DebugDSP1
      Log_Message("OP02 FX:%d FY:%d FZ:%d LFE:%d LES:%d",Op02FX,Op02FY,Op02FZ,Op02LFE,Op02LES);
      Log_Message("     AAS:%d AZS:%d VOF:%d VVA:%d",Op02AAS,Op02AZS,Op02VOF,Op02VVA);
      Log_Message("     VX:%d VY:%d VZ:%d",(short)ViewerX,(short)ViewerY,(short)ViewerZ);
   #endif

}
#else

void DSPOp02()
{
   ViewerZ1=-cos(Op02AZS*6.2832/65536.0);
   ViewerX1=sin(Op02AZS*6.2832/65536.0)*sin(Op02AAS*6.2832/65536.0);
   ViewerY1=sin(Op02AZS*6.2832/65536.0)*cos(-Op02AAS*6.2832/65536.0);

   #ifdef debug02
   printf("\nViewerX1 : %f ViewerY1 : %f ViewerZ1 : %f\n",ViewerX1,ViewerY1,
                                                                   ViewerZ1);
   getch();
   #endif
   ViewerX=Op02FX-ViewerX1*Op02LFE;
   ViewerY=Op02FY-ViewerY1*Op02LFE;
   ViewerZ=Op02FZ-ViewerZ1*Op02LFE;

   ScreenX=Op02FX+ViewerX1*(Op02LES-Op02LFE);
   ScreenY=Op02FY+ViewerY1*(Op02LES-Op02LFE);
   ScreenZ=Op02FZ+ViewerZ1*(Op02LES-Op02LFE);

   #ifdef debug02
   printf("ViewerX : %f ViewerY : %f ViewerZ : %f\n",ViewerX,ViewerY,ViewerZ);
   printf("Op02FX : %d Op02FY : %d Op02FZ : %d\n",Op02FX,Op02FY,Op02FZ);
   printf("ScreenX : %f ScreenY : %f ScreenZ : %f\n",ScreenX,ScreenY,ScreenZ);
   getch();
   #endif
   if (ViewerZ1==0)ViewerZ1++;
   NumberOfSlope=ViewerZ/-ViewerZ1;

   Op02CX=(short)(Op02CXF=ViewerX+ViewerX1*NumberOfSlope);
   Op02CY=(short)(Op02CYF=ViewerY+ViewerY1*NumberOfSlope);

   ViewerXc=ViewerX;//-Op02FX);
   ViewerYc=ViewerY;//-Op02FY);
   ViewerZc=ViewerZ;//-Op02FZ);

   Op02VOF=0x0000;
   ReversedLES=0;
   Op02LESb=Op02LES;
   if ((Op02LES>=VofAngle+16384.0) && (Op02LES<VofAngle+32768.0)) {
     ReversedLES=1;
     Op02LESb=VofAngle+0x4000-(Op02LES-(VofAngle+0x4000));
   }
   Op02VVA = (short)(Op02LESb * tan((Op02AZS-0x4000)*6.2832/65536.0));
   if ((Op02LESb>=VofAngle) && (Op02LESb<=VofAngle+0x4000)) {
      Op02VOF= (short)(Op02LESb * tan((Op02AZS-0x4000-VofAngle)*6.2832/65536.0));
      Op02VVA-=Op02VOF;
   }
   if (ReversedLES){
     Op02VOF=-Op02VOF;
   }

   NAzsB = (Op02AZS-0x4000)*6.2832/65536.0;
   NAasB = Op02AAS*6.2832/65536.0;

   if (tan(NAzsB)==0) NAzsB=0.1;

   ScrDispl=0;
   if (NAzsB>-0.15) {NAzsB=-0.15;ScrDispl=Op02VVA-0xFFDA;}

   CXdistance=1/tan(NAzsB);

   CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   if (CenterX<-32768) CenterX = -32768; if (CenterX>32767) CenterX=32767;
   Op02CX = (short)CenterX;
   CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   if (CenterY<-32768) CenterY = -32768; if (CenterY>32767) CenterY=32767;
   Op02CY = (short)CenterY;

//   TValDebug = (NAzsB*65536/6.28);
 //  TValDebug2 = ScrDispl;

//   if (Op02CY < 0) {Op02CYSup = Op02CY/256; Op02CY = 0;}
//   if (Op02CX < 0) {Op02CXSup = Op02CX/256; Op02CX = 0;}

//  [4/15/2001]   (ViewerX+ViewerX1*NumberOfSlope);
//  [4/15/2001]   (ViewerY+ViewerY1*NumberOfSlope);

//   if(Op02LFE==0x2200)Op02VVA=0xFECD;
//   else Op02VVA=0xFFB2;


   #ifdef DebugDSP1
      Log_Message("OP02 FX:%d FY:%d FZ:%d LFE:%d LES:%d",Op02FX,Op02FY,Op02FZ,Op02LFE,Op02LES);
      Log_Message("     AAS:%d AZS:%d VOF:%d VVA:%d",Op02AAS,Op02AZS,Op02VOF,Op02VVA);
      Log_Message("     VX:%d VY:%d VZ:%d",(short)ViewerX,(short)ViewerY,(short)ViewerZ);
   #endif

}
#endif

short Op0AVS;
short Op0AA;
short Op0AB;
short Op0AC;
short Op0AD;

/*double RasterRX;
double RasterRY;
double RasterRZ;
double RasterLSlopeX;
double RasterLSlopeY;
double RasterLSlopeZ;
double RasterRSlopeX;
double RasterRSlopeY;
double RasterRSlopeZ;
double GroundLX;
double GroundLY;
double GroundRX;
double GroundRY;
double Distance;

double NAzs,NAas;
double RVPos,RHPos,RXRes,RYRes;*/

int32 RasterRX;
int32 RasterRY;
int32 RasterRZ;
int32 RasterLSlopeX;
int32 RasterLSlopeY;
int32 RasterLSlopeZ;
int32 RasterRSlopeX;
int32 RasterRSlopeY;
int32 RasterRSlopeZ;
int32 GroundLX;
int32 GroundLY;
int32 GroundRX;
int32 GroundRY;
int32 Distance;

int32 NAzs,NAas;
int32 RVPos,RHPos,RXRes,RYRes;



void GetRXYPos(){
   int32 scalar;   

   if (Op02LES==0) return;


   NAzs = NAzsB - Atan((RVPos) / (int32)Op02LES);
   NAas = NAasB;// + Atan(RHPos) / (double)Op02LES);

  /* if (cos(NAzs)==0) NAzs+=0.001;
   if (tan(NAzs)==0) NAzs+=0.001;*/
   if (Cos(Angle(NAzs))==0) NAzs+=10;
   if (Sin(Angle(NAzs))==0) NAzs+=10;

   /*RXRes = (-sin(NAas)*ViewerZc/(tan(NAzs))+ViewerXc);
   RYRes = (cos(NAas)*ViewerZc/(tan(NAzs))+ViewerYc);
   scalar = ((ViewerZc/sin(NAzs))/(double)Op02LES);
   RXRes += scalar*-sin(NAas+PI_float/2)*RHPos;
   RYRes += scalar*cos(NAas+PI_float/2)*RHPos;*/
   RXRes = ((int64)-Sin(Angle(NAas))*(int64)ViewerZc/ ((int64)(Sin(Angle(NAzs))<<_FIX_SHIFT_)/(int64)Cos(Angle(NAzs)) )+ViewerXc);
   RYRes = ((int64)Cos(Angle(NAas))*(int64)ViewerZc/ ((int64)(Sin(Angle(NAzs))<<_FIX_SHIFT_)/(int64)Cos(Angle(NAzs)) )+ViewerYc);
   scalar = ((ViewerZc/Sin(Angle(NAzs)))/(int32)Op02LES);
   int32 t;
   SMULT1616(t,-Sin(Angle(NAas+PI/2)),RHPos)
   RXRes += scalar*t;
   SMULT1616(t,Cos(Angle(NAas+PI/2)),RHPos)
   RYRes += scalar*t;
}

void DSPOp0A()
{
  //double x2,y2,x3,y3,x4,y4,m,ypos;
  int32 x2,y2,x3,y3,x4,y4,m,ypos;


   if(Op0AVS==0) {Op0AVS++; return;}
   ypos=(int32)(Op0AVS-ScrDispl)<<_FIX_SHIFT_;
   // CenterX,CenterX = Center (x1,y1)
   // Get (0,Vs) coords (x2,y2)
   RVPos = ypos; RHPos = 0;
   GetRXYPos(); x2 = RXRes; y2 = RYRes;
   // Get (-128,Vs) coords (x3,y3)
   RVPos = ypos; RHPos = -128<<_FIX_SHIFT_;
   GetRXYPos(); x3 = RXRes; y3 = RYRes;
   // Get (127,Vs) coords (x4,y4)
   RVPos = ypos; RHPos = 127<<_FIX_SHIFT_;
   GetRXYPos(); x4 = RXRes; y4 = RYRes;

   // A = (x4-x3)/256
   //m = (x4-x3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
   m = (x4-x3)>>16; if (m>32767) m=32767; if (m<-32768) m=-32768;
   Op0AA = (short)(m);
   // C = (y4-y3)/256
   //m = (y4-y3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
   m = (y4-y3)>>16; if (m>32767) m=32767; if (m<-32768) m=-32768;
   Op0AC = (short)(m);
   if (ypos==0){
     Op0AB = 0;
     Op0AD = 0;
   }
   else {
     // B = (x2-x1)/Vs
     m = (x2-CenterX)/ypos*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
     Op0AB = (short)(m);
     // D = (y2-y1)/Vs
     m = (y2-CenterY)/ypos*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
     Op0AD = (short)(m);
   }

   Op0AVS+=1;
}

short Op06X;
short Op06Y;
short Op06Z;
short Op06H;
short Op06V;
unsigned short Op06S;

/*double ObjPX;
double ObjPY;
double ObjPZ;
double ObjPX1;
double ObjPY1;
double ObjPZ1;
double ObjPX2;
double ObjPY2;
double ObjPZ2;
double DivideOp06;*/
int32 ObjPX;
int32 ObjPY;
int32 ObjPZ;
int32 ObjPX1;
int32 ObjPY1;
int32 ObjPZ1;
int32 ObjPX2;
int32 ObjPY2;
int32 ObjPZ2;
int32 DivideOp06;
int Temp;
int tanval2;

#ifdef __OPT06__
void DSPOp06()
{

   ObjPX=Op06X-Op02FX;
   ObjPY=Op06Y-Op02FY;
   ObjPZ=Op06Z-Op02FZ;



   // rotate around Z
   tanval2 = Angle(-Op02AAS+32768);
//   tanval2 = (-Op02AAS+32768)/(65536/INCR);
   //ObjPX1=(ObjPX*Cos(tanval2)+ObjPY*-Sin(tanval2));
   SADDMULT1616(ObjPX1,ObjPX,Cos(tanval2),ObjPY,-Sin(tanval2))
   //ObjPY1=(ObjPX*Sin(tanval2)+ObjPY*Cos(tanval2));
   SADDMULT1616(ObjPY1,ObjPX,Sin(tanval2),ObjPY,Cos(tanval2))
   ObjPZ1=ObjPZ;


   // rotate around X
//   tanval2 = (-Op02AZS/(65536/INCR)) & 1023;
   tanval2 = Angle(-Op02AZS);
//   tanval2 = (-Op02AZS)/256;
   ObjPX2=ObjPX1;
   //ObjPY2=(ObjPY1*Cos(tanval2)+ObjPZ1*-Sin(tanval2));
   SADDMULT1616(ObjPY2,ObjPY1,Cos(tanval2),ObjPZ1,-Sin(tanval2))
   //ObjPZ2=(ObjPY1*Sin(tanval2)+ObjPZ1*Cos(tanval2));
   SADDMULT1616(ObjPZ2,ObjPY1,Sin(tanval2),ObjPZ1,Cos(tanval2))

   #ifdef debug06
   Log_Message("ObjPX2: %f ObjPY2: %f ObjPZ2: %f\n",ObjPX2,ObjPY2,ObjPZ2);
   #endif

   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPZ2<0)
   {
      Op06H=(short)(-(int64)ObjPX2*(int64)Op02LES/-(ObjPZ2)); //-ObjPX2*256/-ObjPZ2;
      Op06V=(short)(-(int64)ObjPY2*(int64)Op02LES/-(ObjPZ2)); //-ObjPY2*256/-ObjPZ2;
      Op06S=(unsigned short)(256*(int64)(Op02LES<<_FIX_SHIFT_)/-ObjPZ2);
   }
   else
   {
      Op06H=0;
      Op06V=14*16;
      Op06S=0xFFFF;
   }


   #ifdef DebugDSP1
      Log_Message("OP06 X:%d Y:%d Z:%d",Op06X,Op06Y,Op06Z);
      Log_Message("OP06 H:%d V:%d S:%d",Op06H,Op06V,Op06S);
   #endif
}
#else

void DSPOp06()
{
   ObjPX=Op06X-Op02FX;
   ObjPY=Op06Y-Op02FY;
   ObjPZ=Op06Z-Op02FZ;

   // rotate around Z
   tanval = (-Op02AAS+32768)/65536.0*6.2832;
   ObjPX1=(ObjPX*cos(tanval)+ObjPY*-sin(tanval));
   ObjPY1=(ObjPX*sin(tanval)+ObjPY*cos(tanval));
   ObjPZ1=ObjPZ;

   #ifdef debug06
   Log_Message("Angle : %f", tanval);
   Log_Message("ObjPX1: %f ObjPY1: %f ObjPZ1: %f\n",ObjPX1,ObjPY1,ObjPZ1);
   Log_Message("cos(tanval) : %f  sin(tanval) : %f", cos(tanval), sin(tanval));
   #endif

   // rotate around X
   tanval = (-Op02AZS)/65536.0*6.2832;
   ObjPX2=ObjPX1;
   ObjPY2=(ObjPY1*cos(tanval)+ObjPZ1*-sin(tanval));
   ObjPZ2=(ObjPY1*sin(tanval)+ObjPZ1*cos(tanval));

   #ifdef debug06
   Log_Message("ObjPX2: %f ObjPY2: %f ObjPZ2: %f\n",ObjPX2,ObjPY2,ObjPZ2);
   #endif

   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPZ2<0)
   {
      Op06H=(short)(-ObjPX2*Op02LES/-(ObjPZ2)); //-ObjPX2*256/-ObjPZ2;
      Op06V=(short)(-ObjPY2*Op02LES/-(ObjPZ2)); //-ObjPY2*256/-ObjPZ2;
      Op06S=(unsigned short)(256*(double)Op02LES/-ObjPZ2);
   }
   else
   {
      Op06H=0;
      Op06V=14*16;
      Op06S=0xFFFF;
   }


   #ifdef DebugDSP1
      Log_Message("OP06 X:%d Y:%d Z:%d",Op06X,Op06Y,Op06Z);
      Log_Message("OP06 H:%d V:%d S:%d",Op06H,Op06V,Op06S);
   #endif
}
#endif 



/*double matrixB[3][3];
double matrixB2[3][3];
double matrixB3[3][3];

double matrixA[3][3];
double matrixA2[3][3];
double matrixA3[3][3];*/
int32 matrixB[3][3];
int32 matrixB2[3][3];
int32 matrixB3[3][3];

int32 matrixA[3][3];
int32 matrixA2[3][3];
int32 matrixA3[3][3];


void MultMatrixB(int32 result[3][3],int32 mat1[3][3],int32 mat2[3][3])
{
   result[0][0]=(mat1[0][0]*mat2[0][0]+mat1[0][1]*mat2[1][0]+mat1[0][2]*mat2[2][0])>>_FIX_SHIFT_;
   result[0][1]=(mat1[0][0]*mat2[0][1]+mat1[0][1]*mat2[1][1]+mat1[0][2]*mat2[2][1])>>_FIX_SHIFT_;
   result[0][2]=(mat1[0][0]*mat2[0][2]+mat1[0][1]*mat2[1][2]+mat1[0][2]*mat2[2][2])>>_FIX_SHIFT_;

   result[1][0]=(mat1[1][0]*mat2[0][0]+mat1[1][1]*mat2[1][0]+mat1[1][2]*mat2[2][0])>>_FIX_SHIFT_;
   result[1][1]=(mat1[1][0]*mat2[0][1]+mat1[1][1]*mat2[1][1]+mat1[1][2]*mat2[2][1])>>_FIX_SHIFT_;
   result[1][2]=(mat1[1][0]*mat2[0][2]+mat1[1][1]*mat2[1][2]+mat1[1][2]*mat2[2][2])>>_FIX_SHIFT_;
   
   result[2][0]=(mat1[2][0]*mat2[0][0]+mat1[2][1]*mat2[1][0]+mat1[2][2]*mat2[2][0])>>_FIX_SHIFT_;
   result[2][1]=(mat1[2][0]*mat2[0][1]+mat1[2][1]*mat2[1][1]+mat1[2][2]*mat2[2][1])>>_FIX_SHIFT_;
   result[2][2]=(mat1[2][0]*mat2[0][2]+mat1[2][1]*mat2[1][2]+mat1[2][2]*mat2[2][2])>>_FIX_SHIFT_;

}


short Op01m;
short Op01Zr;
short Op01Xr;
short Op01Yr;
short Op11m;
short Op11Zr;
short Op11Xr;
short Op11Yr;
short Op21m;
short Op21Zr;
short Op21Xr;
short Op21Yr;
//double sc,sc2,sc3;
int32 sc,sc2,sc3;



#ifdef __OPT01__
void DSPOp01()
{
   unsigned short zr,yr,xr;

   zr = Angle(Op01Zr);
   xr = Angle(Op01Yr);
   yr = Angle(Op01Xr);

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=Cos(xr);  matrixB[1][2]=-Sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=Sin(xr);  matrixB[2][2]=Cos(xr);

   matrixB2[0][0]=Cos(yr);   matrixB2[0][1]=0;    matrixB2[0][2]=Sin(yr);
   matrixB2[1][0]=0;         matrixB2[1][1]=1;    matrixB2[1][2]=0;
   matrixB2[2][0]=-Sin(yr);  matrixB2[2][1]=0;    matrixB2[2][2]=Cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=Cos(zr); matrixB2[0][1]=-Sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=Sin(zr); matrixB2[1][1]=Cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc = ((double)Op01m)/32768.0;

   matrixA[0][0]=matrixB[0][0]; matrixA[0][1]=matrixB[0][1]; matrixA[0][2]=matrixB[0][2]; 
   matrixA[1][0]=matrixB[1][0]; matrixA[1][1]=matrixB[1][1]; matrixA[1][2]=matrixB[1][2]; 
   matrixA[2][0]=matrixB[2][0]; matrixA[2][1]=matrixB[2][1]; matrixA[2][2]=matrixB[2][2]; 

   #ifdef DebugDSP1
      Log_Message("OP01 ZR: %d XR: %d YR: %d",Op01Zr,Op01Xr,Op01Yr);
   #endif
}

#else

void DSPOp01()
{
   double zr,yr,xr;

   zr = ((double)Op01Zr)*6.2832/65536;
   xr = ((double)Op01Yr)*6.2832/65536;
   yr = ((double)Op01Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc = ((double)Op01m)/32768.0;

   matrixA[0][0]=matrixB[0][0]; matrixA[0][1]=matrixB[0][1]; matrixA[0][2]=matrixB[0][2]; 
   matrixA[1][0]=matrixB[1][0]; matrixA[1][1]=matrixB[1][1]; matrixA[1][2]=matrixB[1][2]; 
   matrixA[2][0]=matrixB[2][0]; matrixA[2][1]=matrixB[2][1]; matrixA[2][2]=matrixB[2][2]; 

   #ifdef DebugDSP1
      Log_Message("OP01 ZR: %d XR: %d YR: %d",Op01Zr,Op01Xr,Op01Yr);
   #endif
}
#endif


#ifdef __OPT11__
void DSPOp11()
{
   short zr,yr,xr;

   zr = Angle(Op11Zr);
   xr = Angle(Op11Yr);
   yr = Angle(Op11Xr);

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=Cos(xr);  matrixB[1][2]=-Sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=Sin(xr);  matrixB[2][2]=Cos(xr);

   matrixB2[0][0]=Cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=Sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-Sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=Cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=Cos(zr); matrixB2[0][1]=-Sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=Sin(zr); matrixB2[1][1]=Cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc2 = ((double)Op11m)/32768.0;

   matrixA2[0][0]=matrixB[0][0]; matrixA2[0][1]=matrixB[0][1]; matrixA2[0][2]=matrixB[0][2]; 
   matrixA2[1][0]=matrixB[1][0]; matrixA2[1][1]=matrixB[1][1]; matrixA2[1][2]=matrixB[1][2]; 
   matrixA2[2][0]=matrixB[2][0]; matrixA2[2][1]=matrixB[2][1]; matrixA2[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP11 ZR: %d XR: %d YR: %d SC: %d",Op11Zr,Op11Xr,Op11Yr,Op11m);
   #endif
}
#else

void DSPOp11()
{
   double zr,yr,xr;

   zr = ((double)Op11Zr)*6.2832/65536;
   xr = ((double)Op11Yr)*6.2832/65536;
   yr = ((double)Op11Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc2 = ((double)Op11m)/32768.0;

   matrixA2[0][0]=matrixB[0][0]; matrixA2[0][1]=matrixB[0][1]; matrixA2[0][2]=matrixB[0][2]; 
   matrixA2[1][0]=matrixB[1][0]; matrixA2[1][1]=matrixB[1][1]; matrixA2[1][2]=matrixB[1][2]; 
   matrixA2[2][0]=matrixB[2][0]; matrixA2[2][1]=matrixB[2][1]; matrixA2[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP11 ZR: %d XR: %d YR: %d SC: %d",Op11Zr,Op11Xr,Op11Yr,Op11m);
   #endif
}
#endif


#ifdef __OPT21__
void DSPOp21()
{
   short zr,yr,xr;

   zr = Angle(Op21Zr);
   xr = Angle(Op21Yr);
   yr = Angle(Op21Xr);


   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=Cos(xr);  matrixB[1][2]=-Sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=Sin(xr);  matrixB[2][2]=Cos(xr);

   matrixB2[0][0]=Cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=Sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-Sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=Cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=Cos(zr); matrixB2[0][1]=-Sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=Sin(zr); matrixB2[1][1]=Cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc3 = ((double)Op21m)/32768.0;

   matrixA3[0][0]=matrixB[0][0]; matrixA3[0][1]=matrixB[0][1]; matrixA3[0][2]=matrixB[0][2]; 
   matrixA3[1][0]=matrixB[1][0]; matrixA3[1][1]=matrixB[1][1]; matrixA3[1][2]=matrixB[1][2]; 
   matrixA3[2][0]=matrixB[2][0]; matrixA3[2][1]=matrixB[2][1]; matrixA3[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP21 ZR: %d XR: %d YR: %d",Op21Zr,Op21Xr,Op21Yr);
   #endif
}
#else

void DSPOp21()
{
   double zr,yr,xr;

   zr = ((double)Op21Zr)*6.2832/65536;
   xr = ((double)Op21Yr)*6.2832/65536;
   yr = ((double)Op21Xr)*6.2832/65536;

   matrixB[0][0]=1;       matrixB[0][1]=0;        matrixB[0][2]=0;       
   matrixB[1][0]=0;       matrixB[1][1]=cos(xr);  matrixB[1][2]=-sin(xr);
   matrixB[2][0]=0;       matrixB[2][1]=sin(xr);  matrixB[2][2]=cos(xr);

   matrixB2[0][0]=cos(yr);  matrixB2[0][1]=0; matrixB2[0][2]=sin(yr);
   matrixB2[1][0]=0;        matrixB2[1][1]=1; matrixB2[1][2]=0;
   matrixB2[2][0]=-sin(yr); matrixB2[2][1]=0; matrixB2[2][2]=cos(yr);

   MultMatrixB(matrixB3,matrixB,matrixB2);

   matrixB2[0][0]=cos(zr); matrixB2[0][1]=-sin(zr); matrixB2[0][2]=0;
   matrixB2[1][0]=sin(zr); matrixB2[1][1]=cos(zr);  matrixB2[1][2]=0;
   matrixB2[2][0]=0;       matrixB2[2][1]=0;        matrixB2[2][2]=1;

   MultMatrixB(matrixB,matrixB3,matrixB2);

   sc3 = ((double)Op21m)/32768.0;

   matrixA3[0][0]=matrixB[0][0]; matrixA3[0][1]=matrixB[0][1]; matrixA3[0][2]=matrixB[0][2]; 
   matrixA3[1][0]=matrixB[1][0]; matrixA3[1][1]=matrixB[1][1]; matrixA3[1][2]=matrixB[1][2]; 
   matrixA3[2][0]=matrixB[2][0]; matrixA3[2][1]=matrixB[2][1]; matrixA3[2][2]=matrixB[2][2]; 
   #ifdef DebugDSP1
      Log_Message("OP21 ZR: %d XR: %d YR: %d",Op21Zr,Op21Xr,Op21Yr);
   #endif
}
#endif

short Op0DX;
short Op0DY;
short Op0DZ;
short Op0DF;
short Op0DL;
short Op0DU;
short Op1DX;
short Op1DY;
short Op1DZ;
short Op1DF;
short Op1DL;
short Op1DU;
short Op2DX;
short Op2DY;
short Op2DZ;
short Op2DF;
short Op2DL;
short Op2DU;

#define swap(a,b) temp=a;a=b;b=temp;

void DSPOp0D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;

   a = matrixA[0][0]; b=matrixA[0][1]; c=matrixA[0][2];
   d = matrixA[1][0]; e=matrixA[1][1]; f=matrixA[1][2];
   g = matrixA[2][0]; h=matrixA[2][1]; i=matrixA[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op0DF=Op0DX;
     Op0DL=Op0DY;
     Op0DU=Op0DZ;
     #ifdef DebugDSP1
        Log_Message("OP0D Error!  Det == 0");
     #endif
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op0DX; y=Op0DY; z=Op0DZ;
   Op0DF=(short)((x*a2+y*d2+z*g2)/2*sc);
   Op0DL=(short)((x*b2+y*e2+z*h2)/2*sc);
   Op0DU=(short)((x*c2+y*f2+z*i2)/2*sc);

   #ifdef DebugDSP1
      Log_Message("OP0D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op0DX,Op0DY,Op0DZ,Op0DF,Op0DL,Op0DU);
   #endif
}

void DSPOp1D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;
   a = matrixA2[0][0]; b=matrixA2[0][1]; c=matrixA2[0][2];
   d = matrixA2[1][0]; e=matrixA2[1][1]; f=matrixA2[1][2];
   g = matrixA2[2][0]; h=matrixA2[2][1]; i=matrixA2[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op1DF=0; Op1DL=0; Op1DU=0;
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op1DX; y=Op1DY; z=Op1DZ;
   Op1DF=(short)((x*a2+y*d2+z*g2)/2*sc2);
   Op1DL=(short)((x*b2+y*e2+z*h2)/2*sc2);
   Op1DU=(short)((x*c2+y*f2+z*i2)/2*sc2);
   #ifdef DebugDSP1
      Log_Message("OP1D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op1DX,Op1DY,Op1DZ,Op1DF,Op1DL,Op1DU);
   #endif
}

void DSPOp2D()
{
   double a,b,c,d,e,f,g,h,i,det,temp;
   double a2,b2,c2,d2,e2,f2,g2,h2,i2,x,y,z;
   a = matrixA3[0][0]; b=matrixA3[0][1]; c=matrixA3[0][2];
   d = matrixA3[1][0]; e=matrixA3[1][1]; f=matrixA3[1][2];
   g = matrixA3[2][0]; h=matrixA3[2][1]; i=matrixA3[2][2];
   //abc
   //def
   //ghi
   det = a*e*i+b*f*g+c*d*h-g*e*c-h*f*a-i*d*b;
   if (det==0) {
     Op2DF=0; Op2DL=0; Op2DU=0;
     return;
   }
   swap(d,b); swap(g,c); swap(h,f);
   b=-b; d=-d; f=-f; h=-h;
   a2=(e*i-h*f)/det; b2=(d*i-g*f)/det; c2=(d*h-g*e)/det;
   d2=(b*i-h*c)/det; e2=(a*i-g*c)/det; f2=(a*h-g*b)/det;
   g2=(b*f-e*c)/det; h2=(a*f-d*c)/det; i2=(a*e-d*b)/det;
   x=Op2DX; y=Op2DY; z=Op2DZ;
   Op2DF=(short)((x*a2+y*d2+z*g2)/2*sc3);
   Op2DL=(short)((x*b2+y*e2+z*h2)/2*sc3);
   Op2DU=(short)((x*c2+y*f2+z*i2)/2*sc3);
   #ifdef DebugDSP1
      Log_Message("OP2D X: %d Y: %d Z: %d / F: %d L: %d U: %d",Op2DX,Op2DY,Op2DZ,Op2DF,Op2DL,Op2DU);
   #endif
}

short Op03F;
short Op03L;
short Op03U;
short Op03X;
short Op03Y;
short Op03Z;
short Op13F;
short Op13L;
short Op13U;
short Op13X;
short Op13Y;
short Op13Z;
short Op23F;
short Op23L;
short Op23U;
short Op23X;
short Op23Y;
short Op23Z;

void DSPOp03()
{
   double F,L,U;

   F=Op03F; L=Op03L; U=Op03U;
   Op03X=(short)((F*matrixA[0][0]+L*matrixA[1][0]+U*matrixA[2][0])/2*sc);
   Op03Y=(short)((F*matrixA[0][1]+L*matrixA[1][1]+U*matrixA[2][1])/2*sc);
   Op03Z=(short)((F*matrixA[0][2]+L*matrixA[1][2]+U*matrixA[2][2])/2*sc);

   #ifdef DebugDSP1
      Log_Message("OP03 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op03F,Op03L,Op03U,Op03X,Op03Y,Op03Z);
   #endif
}

void DSPOp13()
{
   double F,L,U;
   F=Op13F; L=Op13L; U=Op13U;
   Op13X=(short)((F*matrixA2[0][0]+L*matrixA2[1][0]+U*matrixA2[2][0])/2*sc2);
   Op13Y=(short)((F*matrixA2[0][1]+L*matrixA2[1][1]+U*matrixA2[2][1])/2*sc2);
   Op13Z=(short)((F*matrixA2[0][2]+L*matrixA2[1][2]+U*matrixA2[2][2])/2*sc2);
   #ifdef DebugDSP1
      Log_Message("OP13 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op13F,Op13L,Op13U,Op13X,Op13Y,Op13Z);
   #endif
}

void DSPOp23()
{
   double F,L,U;
   F=Op23F; L=Op23L; U=Op23U;
   Op23X=(short)((F*matrixA3[0][0]+L*matrixA3[1][0]+U*matrixA3[2][0])/2*sc3);
   Op23Y=(short)((F*matrixA3[0][1]+L*matrixA3[1][1]+U*matrixA3[2][1])/2*sc3);
   Op23Z=(short)((F*matrixA3[0][2]+L*matrixA3[1][2]+U*matrixA3[2][2])/2*sc3);
   #ifdef DebugDSP1
      Log_Message("OP23 F: %d L: %d U: %d / X: %d Y: %d Z: %d",Op23F,Op23L,Op23U,Op23X,Op23Y,Op23Z);
   #endif
}

short Op14Zr;
short Op14Xr;
short Op14Yr;
short Op14U;
short Op14F;
short Op14L;
short Op14Zrr;
short Op14Xrr;
short Op14Yrr;

//double Op14Temp;
int32 Op14Temp;
void DSPOp14()
{
//TODO
   Op14Temp=(Op14Zr*6.2832/65536.0)+(1/cos(Op14Xr*6.2832/65536.0))*((Op14U*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0)-(Op14F*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0));
   Op14Zrr=(short)(Op14Temp*65536.0/6.2832);
   Op14Temp=(Op14Xr*6.2832/65536.0)+((Op14U*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0));
   Op14Xrr=(short)(Op14Temp*65536.0/6.2832);
   Op14Temp=(Op14Yr*6.2832/65536.0)-tan(Op14Xr*6.2832/65536.0)*((Op14U*6.2832/65536.0)*cos(Op14Yr*6.2832/65536.0)+(Op14F*6.2832/65536.0)*sin(Op14Yr*6.2832/65536.0))+(Op14L*6.2832/65536.0);
   Op14Yrr=(short)(Op14Temp*65536.0/6.2832);
   
   
   #ifdef DebugDSP1   
      Log_Message("OP14 X:%d Y%d Z:%D U:%d F:%d L:%d",Op14Xr,Op14Yr,Op14Zr,Op14U,Op14F,Op14L);
      Log_Message("OP14 X:%d Y%d Z:%D",Op14Xrr,Op14Yrr,Op14Zrr);
   #endif
}

short Op0EH;
short Op0EV;
short Op0EX;
short Op0EY;

void DSPOp0E()
{

   // screen Directions UP
   //RVPos = Op0EV;
   RVPos = Op0EV<<_FIX_SHIFT_;
   //RHPos = Op0EH;
   RHPos = Op0EH<<_FIX_SHIFT_;
   GetRXYPos();
   //Op0EX = RXRes;
   Op0EX = RXRes>>_FIX_SHIFT_;
   //Op0EY = RYRes;
   Op0EY = RYRes>>_FIX_SHIFT_;

   #ifdef DebugDSP1
      Log_Message("OP0E COORDINATE H:%d V:%d   X:%d Y:%d",Op0EH,Op0EV,Op0EX,Op0EY);
   #endif
}

short Op0BX;
short Op0BY;
short Op0BZ;
short Op0BS;
short Op1BX;
short Op1BY;
short Op1BZ;
short Op1BS;
short Op2BX;
short Op2BY;
short Op2BZ;
short Op2BS;

void DSPOp0B()
{
    //Op0BS = (Op0BX*matrixA[0][0]+Op0BY*matrixA2[0][1]+Op0BZ*matrixA2[0][2]);
    Op0BS = (Op0BX*matrixA[0][0]+Op0BY*matrixA2[0][1]+Op0BZ*matrixA2[0][2])>>_FIX_SHIFT_;
#ifdef DebugDSP1
        Log_Message("OP0B");
#endif
}

void DSPOp1B()
{   
    //Op1BS = (Op1BX*matrixA2[0][0]+Op1BY*matrixA2[0][1]+Op1BZ*matrixA2[0][2]);
    Op1BS = (Op1BX*matrixA2[0][0]+Op1BY*matrixA2[0][1]+Op1BZ*matrixA2[0][2])>>_FIX_SHIFT_;
#ifdef DebugDSP1
      Log_Message("OP1B X: %d Y: %d Z: %d S: %d",Op1BX,Op1BY,Op1BZ,Op1BS);
      Log_Message("     MX: %d MY: %d MZ: %d Scale: %d",(short)(matrixA2[0][0]*100),(short)(matrixA2[0][1]*100),(short)(matrixA2[0][2]*100),(short)(sc2*100));
#endif

}

void DSPOp2B()
{
    //Op2BS = (Op2BX*matrixA3[0][0]+Op2BY*matrixA3[0][1]+Op2BZ*matrixA3[0][2]);
    Op2BS = (Op2BX*matrixA3[0][0]+Op2BY*matrixA3[0][1]+Op2BZ*matrixA3[0][2])>>_FIX_SHIFT_;
#ifdef DebugDSP1
      Log_Message("OP2B");
#endif
}

short Op08X,Op08Y,Op08Z,Op08Ll,Op08Lh;
long Op08Size;

void DSPOp08()
{
   Op08Size=(Op08X*Op08X+Op08Y*Op08Y+Op08Z*Op08Z)*2;
   Op08Ll = Op08Size&0xFFFF;
   Op08Lh = (Op08Size>>16) & 0xFFFF;
   #ifdef DebugDSP1
      Log_Message("OP08 %d,%d,%d",Op08X,Op08Y,Op08Z);
      Log_Message("OP08 ((Op08X^2)+(Op08Y^2)+(Op08X^2))=%x",Op08Size );
   #endif
}

short Op18X,Op18Y,Op18Z,Op18R,Op18D;

void DSPOp18()
{
   //double x,y,z,r;
   int32 x,y,z,r;
   x=Op18X; y=Op18Y; z=Op18Z; r=Op18R;
   r = (x*x+y*y+z*z-r*r);
   if (r>32767) r=32767;
   if (r<-32768) r=-32768;
   Op18D=(short)r;
   #ifdef DebugDSP1
      Log_Message("OP18 X: %d Y: %d Z: %d R: %D DIFF %d",Op18X,Op18Y,Op18Z,Op18D);
   #endif
}

short Op38X,Op38Y,Op38Z,Op38R,Op38D;

void DSPOp38()
{
   Op38D = (Op38X * Op38X + Op38Y * Op38Y + Op38Z * Op38Z - Op38R * Op38R) >> 15;
   Op38D++;

   #ifdef DebugDSP1
      Log_Message("OP38 X: %d Y: %d Z: %d R: %D DIFF %d",Op38X,Op38Y,Op38Z,Op38D);
   #endif
}


short Op28X;
short Op28Y;
short Op28Z;
short Op28R;

void DSPOp28()
{
	//to optimize... sqrt
   Op28R=(short)sqrt((double)(Op28X*Op28X+Op28Y*Op28Y+Op28Z*Op28Z));
   #ifdef DebugDSP1
      Log_Message("OP28 X:%d Y:%d Z:%d",Op28X,Op28Y,Op28Z);
      Log_Message("OP28 Vector Length %d",Op28R);
   #endif
}

short Op1CAZ;
unsigned short Op1CX,Op1CY,Op1CZ;
short Op1CXBR,Op1CYBR,Op1CZBR,Op1CXAR,Op1CYAR,Op1CZAR;
short Op1CX1;
short Op1CY1;
short Op1CZ1;
short Op1CX2;
short Op1CY2;
short Op1CZ2;

#ifdef __OPT1C__
void DSPOp1C()
{
   short ya,xa,za;
   ya = Angle(Op1CX);
   xa = Angle(Op1CY);
   za = Angle(Op1CZ);

   // rotate around Z
   //Op1CX1=(Op1CXBR*Cos(za)+Op1CYBR*Sin(za));
   Op1CX1=(Op1CXBR*Cos(za)+Op1CYBR*Sin(za))>>_FIX_SHIFT_;
   //Op1CY1=(Op1CXBR*-Sin(za)+Op1CYBR*Cos(za));
   Op1CY1=(Op1CXBR*-Sin(za)+Op1CYBR*Cos(za))>>_FIX_SHIFT_;
   Op1CZ1=Op1CZBR;
   // rotate around Y
   //Op1CX2=(Op1CX1*Cos(ya)+Op1CZ1*-Sin(ya));
   Op1CX2=(Op1CX1*Cos(ya)+Op1CZ1*-Sin(ya))>>_FIX_SHIFT_;
   Op1CY2=Op1CY1;
   //Op1CZ2=(Op1CX1*Sin(ya)+Op1CZ1*Cos(ya));
   Op1CZ2=(Op1CX1*Sin(ya)+Op1CZ1*Cos(ya))>>_FIX_SHIFT_;
   // rotate around X
   Op1CXAR=Op1CX2;
   //Op1CYAR=(Op1CY2*Cos(xa)+Op1CZ2*Sin(xa));
   Op1CYAR=(Op1CY2*Cos(xa)+Op1CZ2*Sin(xa))>>_FIX_SHIFT_;
   //Op1CZAR=(Op1CY2*-Sin(xa)+Op1CZ2*Cos(xa));
   Op1CZAR=(Op1CY2*-Sin(xa)+Op1CZ2*Cos(xa))>>_FIX_SHIFT_;

   #ifdef DebugDSP1
      Log_Message("OP1C Apply Matrix CX:%d CY:%d CZ",Op1CXAR,Op1CYAR,Op1CZAR);
   #endif
}
#else
void DSPOp1C()
{
   double ya,xa,za;
   ya = Op1CX/65536.0*PI*2;
   xa = Op1CY/65536.0*PI*2;
   za = Op1CZ/65536.0*PI*2;
   // rotate around Z
   Op1CX1=(Op1CXBR*cos(za)+Op1CYBR*sin(za));
   Op1CY1=(Op1CXBR*-sin(za)+Op1CYBR*cos(za));
   Op1CZ1=Op1CZBR;
   // rotate around Y
   Op1CX2=(Op1CX1*cos(ya)+Op1CZ1*-sin(ya));
   Op1CY2=Op1CY1;
   Op1CZ2=(Op1CX1*sin(ya)+Op1CZ1*cos(ya));
   // rotate around X
   Op1CXAR=Op1CX2;
   Op1CYAR=(Op1CY2*cos(xa)+Op1CZ2*sin(xa));
   Op1CZAR=(Op1CY2*-sin(xa)+Op1CZ2*cos(xa));

   #ifdef DebugDSP1
      Log_Message("OP1C Apply Matrix CX:%d CY:%d CZ",Op1CXAR,Op1CYAR,Op1CZAR);
   #endif
}

#endif

unsigned short Op0FRamsize;
unsigned short Op0FPass;

void DSPOp0F()
{
   Op0FPass = 0x0000;

   #ifdef DebugDSP1
      Log_Message("OP0F RAM Test Pass:%d", Op0FPass);
   #endif
}


short Op2FUnknown;
short Op2FSize;

void DSPOp2F()
{
	Op2FSize=0x100;
}
