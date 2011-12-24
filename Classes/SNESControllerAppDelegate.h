//
//  SNESControllerAppDelegate.h
//  SNESController
//
//  Created by Yusef Napora on 5/5/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GameKit/GameKit.h>

#ifdef TARGET_OS_IPHONE

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <IOKit/hid/IOHIDEventSystem.h>

#endif

@class SNESControllerViewController;
@class SessionController;

@interface SNESControllerAppDelegate : NSObject <UIApplicationDelegate, GKSessionDelegate> {
    UIWindow *window;
    SNESControllerViewController *viewController;
	SessionController *sessionController;
	
#ifdef TARGET_OS_IPHONE
	MPVolumeView *volumeView;
	IOHIDEventSystemRef ioEventSystem;
#endif
}

@property (nonatomic, strong) IBOutlet UIWindow *window;
@property (nonatomic, strong) IBOutlet SNESControllerViewController *viewController;
@property (nonatomic, strong) SessionController *sessionController;

- (void) autosendStatus:(NSTimer *)timer;

@end

extern SNESControllerAppDelegate *ControllerAppDelegate();