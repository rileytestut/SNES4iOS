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
#ifndef _DSP1_H_
#define _DSP1_H_

// Simple vector and matrix types
typedef double MATRIX[3][3];
typedef double VECTOR[3];

enum AttitudeMatrix { MatrixA, MatrixB, MatrixC };

struct SDSP1 {
    bool8 waiting4command;
    bool8 first_parameter;
    uint8 command;
    uint32 in_count;
    uint32 in_index;
    uint32 out_count;
    uint32 out_index;
    uint16 parameters [10];
    uint16 output [10];

    // Attitude matrices
    MATRIX vMa;
    MATRIX vMb;
    MATRIX vMc;
    
    // Matrix and translaton vector for
    // transforming a 3D position into the global coordinate system,
    // from the view space coordinate system.
    MATRIX vM;
    VECTOR vT;

    // Focal distance
    double vFov;

    // A precalculated value for optimization
    double vPlaneD;
    
    // Raster position of horizon
    double vHorizon;

    // Convert a 2D screen coordinate to a 3D ground coordinate in global coordinate system.
    void ScreenToGround(VECTOR &v, double X2d, double Y2d);

    MATRIX &GetMatrix( AttitudeMatrix Matrix );
};

///////////////// DSP Commands ////////////////////

// DSP1 Command 02h
struct DSP1_Parameter
{
    DSP1_Parameter( int16 Fx, int16 Fy, int16 Fz,
		      uint16 Lfe, uint16 Les,
		      int8 Aas, int8 Azs );

    // Raster number of imaginary center
    int16 Vof;	// -32768 ~ +32767

    // Raster number representing
    // horizontal line.
    int16 Vva;	// -32768 ~ +32767

    // X,Y coordinate of the point
    // projected on the center of the screen
    // (ground coordinate)
    int16 Cx;	// -32768 ~ +32767
    int16 Cy;	// -32768 ~ +32767
};

// DSP1 Command 0Ah
struct DSP1_Raster
{
    DSP1_Raster( int16 Vs );

    // Linear transformation matrix elements
    // for each raster
    int16 An;
    int16 Bn;
    int16 Cn;
    int16 Dn;
};

// DSP1 Command 06h
struct DSP1_Project
{
    DSP1_Project( int16 x, int16 y, int16 z );

    int16 H;
    int16 V;
    int16 M;
};

// DSP1 Command 0Eh
struct DSP1_Target
{
    DSP1_Target( int16 h, int16 v );

    int16 X;
    int16 Y;
};

// DSP1 Command 04h
struct DSP1_Triangle
{
    DSP1_Triangle (int16 Theta, int16 r );
    int16 S;
    int16 C;
};

// DSP1 Command 08h
struct DSP1_Radius
{
    DSP1_Radius( int16 x, int16 y, int16 z );
    int16 Ll;
    int16 Lh;
};

// DSP1 Command 18h
int16 DSP1_Range( int16 x, int16 y, int16 z, int16 r );

// DSP1 Command 28h
int16 DSP1_Distance( int16 x, int16 y, int16 z );

// DSP1 Command 0Ch
struct DSP1_Rotate
{
    DSP1_Rotate (int16 A, int16 x1, int16 y1);

    int16 x2;
    int16 y2;
};

// DSP1 Command 1Ch
struct DSP1_Polar
{
    DSP1_Polar( int8 Za, int8 Xa, int8 Ya, int16 x, int16 y, int16 z );

    int16 X;
    int16 Y;
    int16 Z;
};

// DSP1 Command 01h, 11h and 21h
void DSP1_Attitude( int16 m, int8 Za, int8 Xa, int8 Ya, AttitudeMatrix Matrix );

// DSP1 Command 0Dh, 1Dh and 2Dh
struct DSP1_Objective
{
    DSP1_Objective( int16 x, int16 y, int16 z, AttitudeMatrix Matrix );

    int16 F;
    int16 L;
    int16 U;
};

// DSP1 Command 03h, 13h and 23h
struct DSP1_Subjective
{
    DSP1_Subjective( int16 F, int16 L, int16 U, AttitudeMatrix Matrix );

    int16 X;
    int16 Y;
    int16 Z;
};

// DSP1 Command 0Bh, 1Bh and 2Bh
int16 DSP1_Scalar( int16 x, int16 y, int16 z, AttitudeMatrix Matrix );

// DSP1 Command 14h
struct DSP1_Gyrate
{
    DSP1_Gyrate( int8 Zi, int8 Xi, int8 Yi,
		 int8 dU, int8 dF, int8 dL );

    int8 Z0;
    int8 X0;
    int8 Y0;
};

// DSP1 Command 00h
int16 DSP1_Multiply( int16 k, int16 I );

// DSP1 Command 10h
struct DSP1_Inverse
{
    DSP1_Inverse( int16 a, int16 b );

    int16 A;
    int16 B;
};

START_EXTERN_C
void S9xResetDSP1 ();
uint8 S9xGetDSP (uint16 Address);
void S9xSetDSP (uint8 Byte, uint16 Address);
END_EXTERN_C

#ifndef __GP32__ 
extern struct SDSP1 DSP1;
#else
extern "C" struct SDSP1 DSP1;
#endif

#endif
