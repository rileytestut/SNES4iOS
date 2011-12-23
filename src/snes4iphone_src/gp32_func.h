#ifndef __gp32_func_h__
#define __gp32_func_h__

extern "C" int funcSADDMULT1616(int a,int b,int c,int d);

#define SADDMULT1616(res,a,b,c,d) {\
	res=funcSADDMULT1616(a,b,c,d);\
	}
#define SMULT1616(res,a,b) {\
	res=funcSADDMULT1616(a,b,0,0);\
	} 

#endif
