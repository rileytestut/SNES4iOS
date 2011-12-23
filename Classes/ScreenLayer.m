//
//  ScreenLayer.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/15/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ScreenLayer.h"
#import "SettingsViewController.h"

#import <math.h>
#define RADIANS( degrees ) ( degrees * M_PI / 180 )

unsigned int *screenPixels;
@implementation ScreenLayer

+ (id) defaultActionForKey:(NSString *)key
{
    return nil;
}

- (id)init {
	if (self = [super init])
	{
		NSLog(@"creating IOSurface buffer");
		CFMutableDictionaryRef dict;
		int w = 256; 
		int h = 224; 
		int pitch = w * 2, allocSize = 2 * w * h;
        int bytesPerElement = 2;
	    char pixelFormat[4] = {'5', '6', '5', 'L'};
		
		dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
										 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(dict, kIOSurfaceIsGlobal, kCFBooleanTrue);
		CFDictionarySetValue(dict, kIOSurfaceBytesPerRow,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pitch));
		CFDictionarySetValue(dict, kIOSurfaceBytesPerElement,
                 			 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bytesPerElement));
		CFDictionarySetValue(dict, kIOSurfaceWidth,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &w));
		CFDictionarySetValue(dict, kIOSurfaceHeight,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &h));
		CFDictionarySetValue(dict, kIOSurfacePixelFormat,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, pixelFormat));
		CFDictionarySetValue(dict, kIOSurfaceAllocSize,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &allocSize));
		
		_surface = IOSurfaceCreate(dict);
        
        NSLog(@"created IOSurface at %p", _surface);

		screenPixels = IOSurfaceGetBaseAddress(_surface);
        NSLog(@"Base address: %p", screenPixels);
		
        rotateTransform = CGAffineTransformMakeRotation(RADIANS(90));
        self.affineTransform = rotateTransform;
		if ([SettingsController().smoothScaling isOn])
		{
			[self setMagnificationFilter: kCAFilterLinear];
		} else {
			[self setMagnificationFilter: kCAFilterNearest];
		}
		
		if (1) {
		    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
            [[NSNotificationCenter defaultCenter] addObserver:self 
                                                     selector:@selector(orientationChanged:) 
                                                         name:@"UIDeviceOrientationDidChangeNotification" 
                                                       object:nil];
        }
	}
	return self;
}
		

- (void)display {
    //NSLog(@"ScreenLayer display");
    IOSurfaceLock(_surface, 1, &_seed);
    self.affineTransform = CGAffineTransformIdentity;
    self.contents = nil;
    self.affineTransform = rotateTransform;
    self.contents = (id) _surface;
    IOSurfaceUnlock(_surface, 1, &_seed);
}

- (void) orientationChanged:(NSNotification *)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    if (orientation == UIDeviceOrientationLandscapeLeft) {
        rotateTransform = CGAffineTransformMakeRotation(RADIANS(90));
    } else if (orientation == UIDeviceOrientationLandscapeRight) {
        rotateTransform = CGAffineTransformMakeRotation(RADIANS(270));
    }
    
}


- (void)dealloc {
    [super dealloc];
}


@end
