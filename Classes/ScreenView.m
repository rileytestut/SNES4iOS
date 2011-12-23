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
#import "SNES4iPadAppDelegate.h"

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



- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch *touch = [touches anyObject];
	if (touch.tapCount == 2) {
		
		UIActionSheet *sheet = [[UIActionSheet alloc] initWithTitle:@"Select an option"
														   delegate:self
												  cancelButtonTitle:nil destructiveButtonTitle:@"Quit Game"
												  otherButtonTitles:@"Save State", @"Save State to New File", nil];
		
		CGPoint touchPoint = [touch locationInView:self];
		CGRect rect = CGRectMake(touchPoint.x, touchPoint.y, 60, 60);
		
		__emulation_paused = 1;
        //clearFramebuffer();
		[sheet showFromRect:rect inView:self animated:YES];
	}
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{	
	NSInteger quitIndex = [actionSheet destructiveButtonIndex];
    NSInteger saveCurrentIndex = [actionSheet firstOtherButtonIndex];
	NSInteger saveNewIndex = saveCurrentIndex + 1;
	
	if (buttonIndex == quitIndex) {
        NSLog(@"Quit button clicked");
		__emulation_run = 0;
		[AppDelegate() showEmulator:NO];
	} else if (buttonIndex == saveCurrentIndex) {
		NSLog(@"save to current file button clicked");
		__emulation_saving = 2;
	} else if (buttonIndex == saveNewIndex) {
		NSLog(@"save to new file button clicked");
		__emulation_saving = 1;
	}
	
    [actionSheet dismissWithClickedButtonIndex:buttonIndex animated:YES];
    [actionSheet release];
	__emulation_paused = 0;
	
}
	

- (void)dealloc {
    [super dealloc];
}


@end
