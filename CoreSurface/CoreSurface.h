#ifndef CORESURFACE_H
#define CORESURFACE_H

#include <CoreFoundation/CoreFoundation.h>

#define kCoreSurfaceLockTypeGimmeVRAM   3

#ifdef __cplusplus
extern "C" {
#endif

typedef void * CoreSurfaceBufferRef;
typedef void * CoreSurfaceAcceleratorRef;

/* Keys for the CoreSurfaceBufferCreate dictionary. */
extern CFStringRef kCoreSurfaceBufferGlobal;        /* CFBoolean */
extern CFStringRef kCoreSurfaceBufferMemoryRegion;  /* CFStringRef */
extern CFStringRef kCoreSurfaceBufferPitch;         /* CFNumberRef */
extern CFStringRef kCoreSurfaceBufferWidth;         /* CFNumberRef */
extern CFStringRef kCoreSurfaceBufferHeight;        /* CFNumberRef */
extern CFStringRef kCoreSurfaceBufferPixelFormat;   /* CFNumberRef (fourCC) */
extern CFStringRef kCoreSurfaceBufferAllocSize;     /* CFNumberRef */
extern CFStringRef kCoreSurfaceBufferClientAddress; /* CFNumberRef */

CoreSurfaceBufferRef CoreSurfaceBufferLookup(long lookup);
CoreSurfaceBufferRef CoreSurfaceBufferCreate(CFDictionaryRef dict);
unsigned int CoreSurfaceBufferGetHeight(CoreSurfaceBufferRef surface);
unsigned int CoreSurfaceBufferGetWidth(CoreSurfaceBufferRef surface);
unsigned int CoreSurfaceBufferGetBytesPerRow(CoreSurfaceBufferRef surface);
unsigned int CoreSurfaceBufferGetPixelFormatType(CoreSurfaceBufferRef surface);
unsigned int CoreSurfaceBufferGetID(CoreSurfaceBufferRef surface);
unsigned int CoreSurfaceBufferGetPlaneCount(CoreSurfaceBufferRef surface);

/* lockType is one of kCoreSurfaceLockType*. */
int CoreSurfaceBufferLock(CoreSurfaceBufferRef surface, unsigned int lockType);
int CoreSurfaceBufferUnlock(CoreSurfaceBufferRef surface);
int CoreSurfaceBufferWrapClientMemory(CoreSurfaceBufferRef surface);
void *CoreSurfaceBufferGetBaseAddress(CoreSurfaceBufferRef surface);

/* Set type to 0. */
int CoreSurfaceAcceleratorCreate(CFAllocatorRef allocator, int type,
    CoreSurfaceAcceleratorRef *accel);
unsigned int CoreSurfaceAcceleratorTransferSurfaceWithSwap(
    CoreSurfaceAcceleratorRef accelerator, CoreSurfaceBufferRef dest,
    CoreSurfaceBufferRef src, CFDictionaryRef options);

#ifdef __cplusplus
}
#endif

#endif

