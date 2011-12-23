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

#ifdef __GP32__
#include "gp32_func.h"
#endif
//#define DebugDSP1

// uncomment some lines to test
//#define printinfo
//#define debug02
//#define debug0A
//#define debug06

#define __OPT__
#define __OPT01__
#define __OPT02__
#define __OPT04__ //was commented on original source (yoyo)
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


/***************************************************************************\
*  Math tables                                                              *
\***************************************************************************/

double *CosTable2;
double *SinTable2;

int32  *CosTable2Fix;
int32  *SinTable2Fix;

#define INCR 2048
#define Angle(x) (((x)/(65536/INCR)) & (INCR-1))

#define AngleFix(x) (((x)>>5) & (INCR-1))

#define Cos(x) ((double) CosTable2[x])
#define Sin(x) ((double) SinTable2[x])

#define CosFix(x) (CosTable2Fix[x])
#define SinFix(x) (SinTable2Fix[x])

#define PI 3.14159265358979323846264338327

double Atan(double x)
{
	if ((x>=1) || (x<=1)) 
		return (x/(1+0.28*x*x));
	else
		return (PI/2 - Atan(1/x));
}


/***************************************************************************\
*  DSP1 code                                                                *
\***************************************************************************/


void InitDSP(void)
{
#ifdef __OPT__
        unsigned int i;
	CosTable2 = (double *) malloc(INCR*sizeof(double));
	SinTable2 = (double *) malloc(INCR*sizeof(double));
	
	CosTable2Fix = (int32 *) malloc(INCR*sizeof(int32));
	SinTable2Fix = (int32 *) malloc(INCR*sizeof(int32));
	for (i=0; i<INCR; i++){
		CosTable2[i] = (cos((double)(2*PI*i/INCR)));
		SinTable2[i] = (sin((double)(2*PI*i/INCR)));
		CosTable2Fix[i] = (65536*cos((double)(2*PI*i/INCR)));
		SinTable2Fix[i] = (65536*sin((double)(2*PI*i/INCR)));
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
dsp_opcode[0]++;

   Op00Result=Op00Multiplicand*Op00Multiplier/32768;
   #ifdef DebugDSP1
      Log_Message("OP00 MULT %d*%d/32768=%d",Op00Multiplicand,Op00Multiplier,Op00Result);
   #endif
}

signed short Op10Coefficient;
signed short Op10Exponent;
signed short Op10CoefficientR;
signed short Op10ExponentR;
float Op10Temp;

void DSPOp10()
{
dsp_opcode[0x10]++;

        Op10ExponentR=-Op10Exponent;
        Op10Temp = Op10Coefficient / 32768.0;
	if (Op10Temp == 0) {
		Op10CoefficientR = 0;
	} else
		Op10Temp = 1/Op10Temp;	
        if (Op10Temp > 0) 
                while (Op10Temp>=1.0) {
                        Op10Temp=Op10Temp/2.0;
                        Op10ExponentR++;
                }
        else
                while (Op10Temp<-1.0) {
                        Op10Temp=Op10Temp/2.0;
                        Op10ExponentR++;
                }
        Op10CoefficientR = Op10Temp*32768;
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
   dsp_opcode[0x04]++;

   int32 angle;   
   //angle = Angle(Op04Angle);
   angle = AngleFix(Op04Angle);

   //Op04Sin = Sin(angle) * Op04Radius;
   SMULT1616(Op04Sin,SinFix(angle),(int32)Op04Radius)
   //Op04Cos = Cos(angle) * Op04Radius;
   SMULT1616(Op04Cos,CosFix(angle),(int32)Op04Radius);

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
dsp_opcode[0x0C]++;

   Op0CX2=(Op0CX1*Cos(Angle(Op0CA))+Op0CY1*Sin(Angle(Op0CA)));
   Op0CY2=(Op0CX1*-Sin(Angle(Op0CA))+Op0CY1*Cos(Angle(Op0CA)));
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
double Op02CXF;
double Op02CYF;
double ViewerX0;
double ViewerY0;
double ViewerZ0;
double ViewerX1;
double ViewerY1;
double ViewerZ1;
double ViewerX;
double ViewerY;
double ViewerZ;
int ViewerAX;
int ViewerAY;
int ViewerAZ;
double NumberOfSlope;
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
double ScreenLZ1;
int    ReversedLES;
short Op02LESb;
double NAzsB,NAasB;
double ViewerXc;
double ViewerYc;
double ViewerZc;
double CenterX,CenterY;
short Op02CYSup,Op02CXSup;
double CXdistance;

#define VofAngle 0x3880

short TValDebug,TValDebug2;
short ScrDispl;


#ifdef __OPT02__
void DSPOp02()
{
dsp_opcode[0x02]++;

	ViewerZ1=-Cos(Angle(Op02AZS));
	ViewerX1=Sin(Angle(Op02AZS))*Sin(Angle(Op02AAS));
	ViewerY1=Sin(Angle(Op02AZS))*Cos(Angle(Op02AAS));

	
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

   ViewerXc=Op02FX;
   ViewerYc=Op02FY;
   ViewerZc=Op02FZ;

   CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   Op02CX = (short)CenterX;
   Op02CY = (short)CenterY;

   ViewerXc=ViewerX;//-Op02FX);
   ViewerYc=ViewerY;//-Op02FY);
   ViewerZc=ViewerZ;//-Op02FZ);

   CenterX = (-sin(NAasB)*ViewerZc*CXdistance)+ViewerXc;
   if (CenterX<-32768) CenterX = -32768; if (CenterX>32767) CenterX=32767;
   CenterY = (cos(NAasB)*ViewerZc*CXdistance)+ViewerYc;
   if (CenterY<-32768) CenterY = -32768; if (CenterY>32767) CenterY=32767;

   TValDebug = (NAzsB*65536/6.28);
   TValDebug2 = ScrDispl;

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

   TValDebug = (NAzsB*65536/6.28);
   TValDebug2 = ScrDispl;

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

double RasterRX;
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
double RVPos,RHPos,RXRes,RYRes;


void GetRXYPos(){
   double scalar;

   if (Op02LES==0) return;


   NAzs = NAzsB - Atan((RVPos) / (double)Op02LES);
   NAas = NAasB;// + Atan(RHPos) / (double)Op02LES);

   if (cos(NAzs)==0) NAzs+=0.001;
   if (tan(NAzs)==0) NAzs+=0.001;

   RXRes = (-sin(NAas)*ViewerZc/(tan(NAzs))+ViewerXc);
   RYRes = (cos(NAas)*ViewerZc/(tan(NAzs))+ViewerYc);
   scalar = ((ViewerZc/sin(NAzs))/(double)Op02LES);
   RXRes += scalar*-sin(NAas+PI/2)*RHPos;
   RYRes += scalar*cos(NAas+PI/2)*RHPos;
}

void DSPOp0A()
{
dsp_opcode[0x0A]++;

  double x2,y2,x3,y3,x4,y4,m,ypos;


   if(Op0AVS==0) {Op0AVS++; return;}
   ypos=Op0AVS-ScrDispl;
   // CenterX,CenterX = Center (x1,y1)
   // Get (0,Vs) coords (x2,y2)
   RVPos = ypos; RHPos = 0;
   GetRXYPos(); x2 = RXRes; y2 = RYRes;
   // Get (-128,Vs) coords (x3,y3)
   RVPos = ypos; RHPos = -128;
   GetRXYPos(); x3 = RXRes; y3 = RYRes;
   // Get (127,Vs) coords (x4,y4)
   RVPos = ypos; RHPos = 127;
   GetRXYPos(); x4 = RXRes; y4 = RYRes;

   // A = (x4-x3)/256
   m = (x4-x3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
   Op0AA = (short)(m);
   // C = (y4-y3)/256
   m = (y4-y3)/256*256; if (m>32767) m=32767; if (m<-32768) m=-32768;
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
double ObjPZ2;*/
int32 ObjPX;
int32 ObjPY;
int32 ObjPZ;
int32 ObjPX1;
int32 ObjPY1;
int32 ObjPZ1;
int32 ObjPX2;
int32 ObjPY2;
int32 ObjPZ2;
double DivideOp06;
int Temp;
int tanval2;

#ifdef __OPT06__
void DSPOp06()
{

   dsp_opcode[0x06]++;

   ObjPX=Op06X-Op02FX;
   ObjPY=Op06Y-Op02FY;
   ObjPZ=Op06Z-Op02FZ;



   // rotate around Z
   //tanval2 = Angle(-Op02AAS+32768);
//   tanval2 = (-Op02AAS+32768)/(65536/INCR);
   //ObjPX1=(ObjPX*Cos(tanval2)+ObjPY*-Sin(tanval2));
   //ObjPY1=(ObjPX*Sin(tanval2)+ObjPY*Cos(tanval2));
   //ObjPZ1=ObjPZ;
   tanval2 = AngleFix(-Op02AAS+32768);
   SADDMULT1616(ObjPX1,ObjPX,CosFix(tanval2),ObjPY,-SinFix(tanval2))
   SADDMULT1616(ObjPY1,ObjPX,SinFix(tanval2),ObjPY,CosFix(tanval2))
   ObjPZ1=ObjPZ;


   // rotate around X
//   tanval2 = (-Op02AZS/(65536/INCR)) & 1023;
   //tanval2 = Angle(-Op02AZS);
//   tanval2 = (-Op02AZS)/256;
   /*ObjPX2=ObjPX1;
   ObjPY2=(ObjPY1*Cos(tanval2)+ObjPZ1*-Sin(tanval2));
   ObjPZ2=(ObjPY1*Sin(tanval2)+ObjPZ1*Cos(tanval2));*/
   tanval2 = AngleFix(-Op02AZS);
   ObjPX2=ObjPX1;
   SADDMULT1616(ObjPY2,ObjPY1,CosFix(tanval2),ObjPZ1,-SinFix(tanval2))
   SADDMULT1616(ObjPZ2,ObjPY1,SinFix(tanval2),ObjPZ1,CosFix(tanval2))

   #ifdef debug06
   Log_Message("ObjPX2: %f ObjPY2: %f ObjPZ2: %f\n",ObjPX2,ObjPY2,ObjPZ2);
   #endif

   ObjPZ2=ObjPZ2-Op02LFE;

   if (ObjPZ2<0)
   {
      Op06H=(short)(-ObjPX2*Op02LES/-(ObjPZ2)); //-ObjPX2*256/-ObjPZ2;
      Op06V=(short)(-ObjPY2*Op02LES/-(ObjPZ2)); //-ObjPY2*256/-ObjPZ2;
      //Op06S=(unsigned short)(256*(double)Op02LES/-ObjPZ2);
      Op06S=(unsigned short)(256*(int32)Op02LES/-ObjPZ2);
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



double matrixB[3][3];
double matrixB2[3][3];
double matrixB3[3][3];

double matrixA[3][3];
double matrixA2[3][3];
double matrixA3[3][3];

void MultMatrixB(double result[3][3],double mat1[3][3],double mat2[3][3])
{
   result[0][0]=(mat1[0][0]*mat2[0][0]+mat1[0][1]*mat2[1][0]+mat1[0][2]*mat2[2][0]);
   result[0][1]=(mat1[0][0]*mat2[0][1]+mat1[0][1]*mat2[1][1]+mat1[0][2]*mat2[2][1]);
   result[0][2]=(mat1[0][0]*mat2[0][2]+mat1[0][1]*mat2[1][2]+mat1[0][2]*mat2[2][2]);

   result[1][0]=(mat1[1][0]*mat2[0][0]+mat1[1][1]*mat2[1][0]+mat1[1][2]*mat2[2][0]);
   result[1][1]=(mat1[1][0]*mat2[0][1]+mat1[1][1]*mat2[1][1]+mat1[1][2]*mat2[2][1]);
   result[1][2]=(mat1[1][0]*mat2[0][2]+mat1[1][1]*mat2[1][2]+mat1[1][2]*mat2[2][2]);
   
   result[2][0]=(mat1[2][0]*mat2[0][0]+mat1[2][1]*mat2[1][0]+mat1[2][2]*mat2[2][0]);
   result[2][1]=(mat1[2][0]*mat2[0][1]+mat1[2][1]*mat2[1][1]+mat1[2][2]*mat2[2][1]);
   result[2][2]=(mat1[2][0]*mat2[0][2]+mat1[2][1]*mat2[1][2]+mat1[2][2]*mat2[2][2]);

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
double sc,sc2,sc3;



#ifdef __OPT01__
void DSPOp01()
{
dsp_opcode[0x01]++;

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
dsp_opcode[0x11]++;

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
dsp_opcode[0x21]++;

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
dsp_opcode[0x0D]++;

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
dsp_opcode[0x1D]++;

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
dsp_opcode[0x2D]++;

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
dsp_opcode[0x03]++;

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
dsp_opcode[0x13]++;

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
dsp_opcode[0x23]++;

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

double Op14Temp;
void DSPOp14()
{
dsp_opcode[0x14]++;

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
dsp_opcode[0x0E]++;

   // screen Directions UP
   RVPos = Op0EV;
   RHPos = Op0EH;
   GetRXYPos();
   Op0EX = RXRes;
   Op0EY = RYRes;

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
dsp_opcode[0x0B]++;

    Op0BS = (Op0BX*matrixA[0][0]+Op0BY*matrixA2[0][1]+Op0BZ*matrixA2[0][2]);
#ifdef DebugDSP1
        Log_Message("OP0B");
#endif
}

void DSPOp1B()
{   
dsp_opcode[0x1B]++;

    Op1BS = (Op1BX*matrixA2[0][0]+Op1BY*matrixA2[0][1]+Op1BZ*matrixA2[0][2]);
#ifdef DebugDSP1
      Log_Message("OP1B X: %d Y: %d Z: %d S: %d",Op1BX,Op1BY,Op1BZ,Op1BS);
      Log_Message("     MX: %d MY: %d MZ: %d Scale: %d",(short)(matrixA2[0][0]*100),(short)(matrixA2[0][1]*100),(short)(matrixA2[0][2]*100),(short)(sc2*100));
#endif

}

void DSPOp2B()
{
dsp_opcode[0x2B]++;

    Op2BS = (Op2BX*matrixA3[0][0]+Op2BY*matrixA3[0][1]+Op2BZ*matrixA3[0][2]);
#ifdef DebugDSP1
      Log_Message("OP2B");
#endif
}

short Op08X,Op08Y,Op08Z,Op08Ll,Op08Lh;
long Op08Size;

void DSPOp08()
{
dsp_opcode[0x08]++;

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
dsp_opcode[0x18]++;

   double x,y,z,r;
   x=Op18X; y=Op18Y; z=Op18Z; r=Op18R;
   r = (x*x+y*y+z*z-r*r);
   if (r>32767) r=32767;
   if (r<-32768) r=-32768;
   Op18D=(short)r;
   #ifdef DebugDSP1
      Log_Message("OP18 X: %d Y: %d Z: %d R: %D DIFF %d",Op18X,Op18Y,Op18Z,Op18D);
   #endif
}

short Op28X;
short Op28Y;
short Op28Z;
short Op28R;

int32 fixed_sqrt(int32 r)
{
   int32 t,b,c=0;

   for (b=0x10000000;b!=0;b>>=2) {
      t = c + b;
      c >>= 1;
      if (t <= r) {
         r -= t;
         c += b;
      }
   }
   return(c);
}


void DSPOp28()
{
dsp_opcode[0x28]++;

//   Op28R=(short)sqrt((double)(Op28X*Op28X+Op28Y*Op28Y+Op28Z*Op28Z));
	Op28R=(short)fixed_sqrt((int32)(Op28X*Op28X+Op28Y*Op28Y+Op28Z*Op28Z));
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
dsp_opcode[0x1C]++;

   short ya,xa,za;
   ya = Angle(Op1CX);
   xa = Angle(Op1CY);
   za = Angle(Op1CZ);

   // rotate around Z
   Op1CX1=(Op1CXBR*Cos(za)+Op1CYBR*Sin(za));
   Op1CY1=(Op1CXBR*-Sin(za)+Op1CYBR*Cos(za));
   Op1CZ1=Op1CZBR;
   // rotate around Y
   Op1CX2=(Op1CX1*Cos(ya)+Op1CZ1*-Sin(ya));
   Op1CY2=Op1CY1;
   Op1CZ2=(Op1CX1*Sin(ya)+Op1CZ1*Cos(ya));
   // rotate around X
   Op1CXAR=Op1CX2;
   Op1CYAR=(Op1CY2*Cos(xa)+Op1CZ2*Sin(xa));
   Op1CZAR=(Op1CY2*-Sin(xa)+Op1CZ2*Cos(xa));

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
