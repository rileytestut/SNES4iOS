//
//  SNESControllerAppDelegate.m
//  SNESController
//
//  Created by Yusef Napora on 5/5/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNESControllerAppDelegate.h"
#import "SNESControllerViewController.h"
#import "SessionController.h"

#define SNES_CONTROLLER_SESSION_ID @"com.snes4iphone.controller"

SNESControllerAppDelegate *ControllerAppDelegate()
{
	return ((SNES4iOSAppDelegate *)[[UIApplication sharedApplication] delegate]).snesControllerAppDelegate;
}

#ifdef TARGET_OS_IPHONE

extern unsigned long gp2x_pad_status;

#define VOL_BUTTON_UP 0xe9
#define VOL_BUTTON_DOWN 0xea

#define L_BUTTON (1<<10)
#define R_BUTTON (1<<11)

void handle_event (void* target, void* refcon, IOHIDServiceRef service, IOHIDEventRef event) {
	// handle the events here.
	//NSLog(@"Received event of type %2d from service %p.", IOHIDEventGetType(event), service);
	IOHIDEventType type = IOHIDEventGetType(event);
	if (type == kIOHIDEventTypeKeyboard)
	{
		NSLog(@"button event");
		
		int usagePage = IOHIDEventGetIntegerValue(event, kIOHIDEventFieldKeyboardUsagePage);
		if (usagePage == 12) {
			int usage = IOHIDEventGetIntegerValue(event, kIOHIDEventFieldKeyboardUsage);
			int down = IOHIDEventGetIntegerValue(event, kIOHIDEventFieldKeyboardDown);
			
			unsigned long buttonMask = 0;
			if (usage == VOL_BUTTON_UP) {
				buttonMask = R_BUTTON;
			} else if (usage == VOL_BUTTON_DOWN) {
				buttonMask = L_BUTTON;
			}

			if (buttonMask != 0) {
				if (down) {
					gp2x_pad_status |= buttonMask;
				} else {
					gp2x_pad_status &= ~buttonMask;
				}
				[ControllerAppDelegate().sessionController sendPadStatus:gp2x_pad_status];
			}
		}
	}
}

#endif // TARGET_IPHONE_SIMULATOR


@implementation SNESControllerAppDelegate

@synthesize window;
@synthesize viewController;
@synthesize sessionController;
@synthesize controllerType;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
#ifdef TARGET_OS_IPHONE
	NSLog(@"Setting up event handler");
	// register our event handler callback
	ioEventSystem = IOHIDEventSystemCreate(NULL);
	IOHIDEventSystemOpen(ioEventSystem, handle_event, NULL, NULL, NULL);
	
	// set the audio session category and activate it 
	AVAudioSession *session = [AVAudioSession sharedInstance];
	[session setCategory:AVAudioSessionCategoryAmbient error:nil];
	[session setActive:YES error:nil];
	
	// create a hidden MPVolumeView and add it to our window.
	// this disables the standard volume overlay display
	volumeView = [[MPVolumeView alloc] initWithFrame:window.bounds];
	volumeView.hidden = YES;
	[window addSubview:volumeView];
	
	// Add a timer to send the pad status every 25ms, whether there's any input or not
	//[NSTimer scheduledTimerWithTimeInterval:0.025 target:self selector:@selector(autosendStatus:) userInfo:nil repeats:YES];
	
#endif
	
	sessionController = [[SessionController alloc] initWithNibName:@"SessionController" bundle:nil];
    
	
	[window addSubview:viewController.view];
    [window makeKeyAndVisible];

	
	return YES;
}

- (void) autosendStatus:(NSTimer *)timer
{
	//[self.sessionController sendPadStatus:gp2x_pad_status];
}

#ifdef TARGET_OS_IPHONE
- (void) applicationWillTerminate:(UIApplication *)application
{
	// clean up our event handler
	IOHIDEventSystemClose(ioEventSystem, NULL);
	CFRelease(ioEventSystem);
}
#endif



	
@end
