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
#ifndef _PORT_H_
#define _PORT_H_

/*
This port.h is really a right-of-passage for anyone trying to port the emulator 
	to another platform.  It must have started out as a set of defines for a
	single platform, and instead of using define blocks as new platforms were
	added, individual coders simply added exceptions and sprinkled #ifdef and #ifndef
	statements throughout the original list.

I can't take it anymore, it's too convoluted.  So I've commented out the entire
	section, and preemptively rewritten the first #define segment the way god intended,
	with a single define-block for each target platform.
*/

/*
**  _SNESPPC DEFINES
*/

//Title
#define TITLE "Snes9x"

//Required Includes
#include "pixform.h"
#include <zlib.h> //RC
//#include <windows.h>
#include <limits.h>
#include <string.h>
//Types Defined
typedef unsigned char	bool8;
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef signed char		int8;
typedef short			int16;
typedef int				int32;
typedef long long		int64;

//CSNES Types for conversion to 32 bit
/*typedef unsigned long	bool8_32;
typedef unsigned long	uint8_32;
typedef unsigned long	uint16_32;
typedef long			int8_32;
typedef long			int16_32;*/

//For Debugging Purposes:

typedef unsigned char	bool8_32;
typedef unsigned char	uint8_32;
typedef unsigned short	uint16_32;
typedef signed char		int8_32;
typedef short			int16_32;

//Defines for Extern C
#define EXTERN_C extern "C" 
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C }

#define VOID void

//Path Defines
#undef  _MAX_PATH
#define _MAX_DIR PATH_MAX
#define _MAX_DRIVE 1
#define _MAX_FNAME PATH_MAX
#define _MAX_EXT PATH_MAX
#define PATH_MAX 1024
#define _MAX_PATH (1024)

//True/False Defines
#define TRUE 1
#define FALSE 0

//Slash Char Definitions
#define SLASH_STR "/"
#define SLASH_CHAR '/'

//Misc Items
EXTERN_C void S9xGenerateSound ();

//Misc Items
//#define LSB_FIRST
#define STATIC static
#define FASTCALL
#define PIXEL_FORMAT RGB565
#define CHECK_SOUND()
#define ZeroMemory(a,b) memset((a),0,(b))
#define PACKING __attribute__ ((packed))
#define ALIGN_BY_ONE  __attribute__ ((aligned (1), packed))
#undef  FAST_LSB_WORD_ACCESS


//Additional Items for _SNESPPC port
void _makepath (char *path, const char *drive, const char *dir,
		const char *fname, const char *ext);
void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext);
#define strcasecmp strcmp
#define strncasecmp strncmp
//#define time(a) (0)

#ifdef INLINE
#undef INLINE
#define INLINE inline
#endif

static inline void __memcpy4(void *d, void *s, unsigned long c)
{
	unsigned long *dl=(unsigned long *)d;
	unsigned long *sl=(unsigned long *)s;
	for (; c; --c) *dl++=*sl++;
}

static inline void __memcpy4a(void *d, void *s, unsigned long c)
{
	unsigned long *dl=(unsigned long *)d;
	unsigned long *sl=(unsigned long *)s;
	for (; c; --c) *dl++=*sl++;
}

static inline void memcpy32(void *d, void *s, unsigned long c)
{
	unsigned long *dl=(unsigned long *)d;
	unsigned long *sl=(unsigned long *)s;
	for (; c; --c) *dl++=*sl++;
}

static inline void __memset4(void *d, unsigned long v, unsigned long c)
{	
	unsigned long *dl=(unsigned long *)d;		
	for (; c; --c) *dl++=v;
}

static inline void memset32(void *d, unsigned long v, unsigned long c)
{	
	unsigned long *dl=(unsigned long *)d;		
	for (; c; --c) *dl++=v;
}

static inline void memset16(void *d, unsigned short v, unsigned long c)
{	
	unsigned short *dl=(unsigned short *)d;		
	for (; c; --c) *dl++=v;
}

#endif //  _PORT_H_

/*
#ifndef _SNESPPC
#define _SNESPPC
#endif 

#ifndef RC_OPTIMIZED
#define RC_OPTIMIZED
#endif

#ifdef inline
#undef inline
#endif

#ifdef INLINE
#undef INLINE
#endif

#define inline __inline
#define INLINE __inline

#ifdef DEBUG
#ifndef _PROFILE_
#define _PROFILE_
#endif
#endif

#ifndef _SNESPPC
#ifndef STORM
#include <memory.h>
#include <string.h>
#else
//#include <strings.h>
//#include <clib/powerpc_protos.h>
#endif

#include <sys/types.h>
#else
#include <windows.h>
#endif

#define PIXEL_FORMAT RGB565
//#define GFX_MULTI_FORMAT

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC

#ifdef _SNESPPC
#include "zlib/zlib.h" //RC
#else
#include "zlib.h"
#endif

#define ZLIB
#define EXECUTE_SUPERFX_PER_LINE
#define SOUND
#define VAR_CYCLES
#define CPU_SHUTDOWN
#define SPC700_SHUTDOWN
#define PIXEL_FORMAT RGB555
#define CHECK_SOUND()
#define M_PI 3.14159265359
#undef  _MAX_PATH

#undef DEBUGGER // Apple Universal Headers sometimes #define DEBUGGER
#undef GFX_MULTI_FORMAT

int    strncasecmp(const char *s1, const char *s2, unsigned n);
int    strcasecmp(const char *s1, const char *s2 );

#endif

#ifndef snes9x_types_defined
#define snes9x_types_defined

//CSNES
#ifdef _SNESPPC
typedef unsigned long bool8;
#else
typedef unsigned char bool8;
#endif

#ifndef __WIN32__
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;
//typedef long int32;
//typedef unsigned long uint32;
typedef int int32;
typedef unsigned int uint32;
#ifdef _SNESPPC
typedef __int64 int64;
//CSNES
typedef unsigned long	uint8_32;
typedef unsigned long	uint16_32;
typedef long			int8_32;
typedef long			int16_32;

#else
typedef long long int64;
#endif
#else // __WIN32__

#ifdef __BORLANDC__
//#include <systypes.h>
#else

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;
typedef short int16;

#ifndef WSAAPI
// winsock2.h typedefs int32 as well.
typedef long int32;
#endif

typedef unsigned int uint32;

#endif // __BORLANDC__

typedef __int64 int64;

#endif // __WIN32__
#endif // snes9x_types_defined
#include "pixform.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef STORM
#define EXTERN_C
#define START_EXTERN_C
#define END_EXTERN_C
#else
#if defined(__cplusplus) || defined(c_plusplus)
#define EXTERN_C extern "C"
#define START_EXTERN_C extern "C" {
#define END_EXTERN_C }
#else
#define EXTERN_C extern
#define START_EXTERN_C
#define END_EXTERN_C
#endif
#endif

#ifndef __WIN32__

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define _MAX_DIR PATH_MAX
#define _MAX_DRIVE 1
#define _MAX_FNAME PATH_MAX
#define _MAX_EXT PATH_MAX
#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif

#ifdef _SNESPPC
#define strcasecmp strcmp
#define strncasecmp strncmp
#define time(a) (0)
#ifdef _MAX_PATH
#undef _MAX_PATH
#define _MAX_PATH (1024)
#endif
#endif

#define ZeroMemory(a,b) memset((a),0,(b))

void _makepath (char *path, const char *drive, const char *dir,
		const char *fname, const char *ext);
void _splitpath (const char *path, char *drive, char *dir, char *fname,
		 char *ext);
#else // __WIN32__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

EXTERN_C void S9xGenerateSound ();

#ifdef STORM
EXTERN_C int soundsignal;
EXTERN_C void MixSound(void);
//Yes, CHECK_SOUND is getting defined correctly!
#define CHECK_SOUND if (Settings.APUEnabled) if(SetSignalPPC(0L, soundsignal) & soundsignal) MixSound
#else
#define CHECK_SOUND()
#endif

#ifdef __DJGPP
#define SLASH_STR "\\"
#define SLASH_CHAR '\\'
#else
#define SLASH_STR "/"
#define SLASH_CHAR '/'
#endif

#ifdef __linux
typedef void (*SignalHandler)(int);
#define SIG_PF SignalHandler
#endif

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || \
    defined(__WIN32__) || defined(__alpha__)
#define LSB_FIRST
#define FAST_LSB_WORD_ACCESS
#else
#ifdef _SNESPPC
#define LSB_FIRST
//NOPE! #define FAST_LSB_WORD_ACCESS //RC
#else 
#define MSB_FIRST
#endif
#endif

#ifdef __sun
#define TITLE "Snes9X: Solaris"
#endif

#ifdef __linux
#define TITLE "Snes9X: Linux"
#endif

#ifndef TITLE
#define TITLE "Snes9x"
#endif

#ifdef STORM
#define STATIC
#define strncasecmp strnicmp
#else
#define STATIC static
#endif

#endif
*/
