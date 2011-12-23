
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the GXDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// GXDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef GXDLL_EXPORTS
#define GXDLL_API __declspec(dllexport)
#else
#define GXDLL_API __declspec(dllimport)
#endif

struct GXDisplayProperties {
	DWORD cxWidth;
	DWORD cyHeight;			// notice lack of 'th' in the word height.
	long cbxPitch;			// number of bytes to move right one x pixel - can be negative.
	long cbyPitch;			// number of bytes to move down one y pixel - can be negative.
	long cBPP;				// # of bits in each pixel
	DWORD ffFormat;			// format flags.
};

struct GXKeyList {
	short vkUp;				// key for up
	POINT ptUp;				// x,y position of key/button.  Not on screen but in screen coordinates.
	short vkDown;
	POINT ptDown;
	short vkLeft;
	POINT ptLeft;
	short vkRight;
	POINT ptRight;
	short vkA;
	POINT ptA;
	short vkB;
	POINT ptB;
	short vkC;
	POINT ptC;
	short vkStart;
	POINT ptStart;
};

struct GXScreenRect {
	DWORD dwTop;
	DWORD dwLeft;
	DWORD dwWidth;
	DWORD dwHeight;
};

GXDLL_API int GXOpenDisplay(HWND hWnd, DWORD dwFlags);
GXDLL_API int GXCloseDisplay();
GXDLL_API void * GXBeginDraw();
GXDLL_API int GXEndDraw();
GXDLL_API int GXOpenInput();
GXDLL_API int GXCloseInput();
//The following two lines modified by Dan East to make this header C compatible:
//Added "struct" to the following two prototypes:
GXDLL_API struct GXDisplayProperties GXGetDisplayProperties();
GXDLL_API struct GXKeyList GXGetDefaultKeys(int iOptions);
GXDLL_API int GXSuspend();
GXDLL_API int GXResume();
GXDLL_API int GXSetViewport( DWORD dwTop, DWORD dwHeight, DWORD dwReserved1, DWORD dwReserved2 );
GXDLL_API BOOL GXIsDisplayDRAMBuffer();


// Although these flags can be unrelated they still
// have unique values.

#define GX_FULLSCREEN	0x01		// for OpenDisplay()
#define GX_NORMALKEYS	0x02
#define GX_LANDSCAPEKEYS	0x03

#ifndef kfLandscape
	#define kfLandscape	0x8			// Screen is rotated 270 degrees
	#define kfPalette	0x10		// Pixel values are indexes into a palette
	#define kfDirect	0x20		// Pixel values contain actual level information
	#define kfDirect555	0x40		// 5 bits each for red, green and blue values in a pixel.
	#define kfDirect565	0x80		// 5 red bits, 6 green bits and 5 blue bits per pixel
	#define kfDirect888	0x100		// 8 bits each for red, green and blue values in a pixel.
	#define kfDirect444	0x200		// 4 red, 4 green, 4 blue
	#define kfDirectInverted 0x400
#endif

