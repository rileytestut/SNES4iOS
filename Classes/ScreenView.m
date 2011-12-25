//
//  ScreenView.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/15/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ScreenView.h"
#import "ScreenLayer.h"
#import "SettingsViewController.h"
#import "SNES4iOSAppDelegate.h"
#import "EmulationViewController.h"

extern volatile int __emulation_run;
extern volatile int __emulation_paused;
extern volatile int __emulation_saving;
extern void clearFramebuffer(void);

@implementation ScreenView

+ (Class) layerClass
{
    return [ScreenLayer class];
}

- (id) initWithFrame:(CGRect)f
{
    if (self = [super initWithFrame:f])
    {
        NSLog(@"ScreenView init");
        self.clearsContextBeforeDrawing = NO;
    }
    return self;
}


// Wierd things happen without this empty drawRect
- (void)drawRect:(CGRect)rect {
   // [self update];
    //[self.layer display];
}


@end
