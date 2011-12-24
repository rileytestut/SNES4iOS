//
//  ScreenLayer.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/15/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import "IOSurface/IOSurface.h"

@interface ScreenLayer : CALayer {
	IOSurfaceRef _surface;
    uint32_t _seed;
    CGAffineTransform rotateTransform;
}
@property (nonatomic) CGAffineTransform rotateTransform;

- (void) orientationChanged:(NSNotification *)notification;

@end
