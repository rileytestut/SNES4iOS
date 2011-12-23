//
//  SNES4iPadAppDelegate.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>


@class EmulationViewController;
@class RomSelectionViewController;
@class RomDetailViewController;
@class SettingsViewController;
@class ControlPadConnectViewController;
@class ControlPadManager;
@class WebBrowserViewController;

@interface SNES4iPadAppDelegate : NSObject <UIApplicationDelegate> {
    
    UIWindow *window;
    
    UISplitViewController *splitViewController;
    
	EmulationViewController *emulationViewController;
    RomSelectionViewController *romSelectionViewController;
    RomDetailViewController *romDetailViewController;
	SettingsViewController *settingsViewController;
	ControlPadConnectViewController *controlPadConnectViewController;
	ControlPadManager *controlPadManager;
	WebBrowserViewController *webViewController;
	UINavigationController *webNavController;
	
	NSString *romDirectoryPath, *__weak saveDirectoryPath, *__weak snapshotDirectoryPath;
}

@property (nonatomic, strong) IBOutlet UIWindow *window;

@property (nonatomic, strong) IBOutlet UISplitViewController *splitViewController;
@property (nonatomic, strong) IBOutlet RomSelectionViewController *romSelectionViewController;
@property (nonatomic, strong) IBOutlet RomDetailViewController *romDetailViewController;
@property (nonatomic, strong) SettingsViewController *settingsViewController;
@property (nonatomic, strong) ControlPadConnectViewController *controlPadConnectViewController;
@property (nonatomic, strong) ControlPadManager *controlPadManager;

@property (nonatomic, strong) EmulationViewController *emulationViewController;
@property (nonatomic, strong) WebBrowserViewController *webViewController;
@property (nonatomic, strong) UINavigationController *webNavController;

@property (nonatomic, readonly) NSString *romDirectoryPath;
@property (nonatomic, readonly) NSString *saveDirectoryPath;
@property (nonatomic, readonly) NSString *snapshotDirectoryPath;

- (void) showEmulator:(BOOL)showOrHide;
//- (void) showEmulationMenu:(BOOL)showOrHide;
+ (NSString *) applicationDocumentsDirectory;
@end

extern SNES4iPadAppDelegate *AppDelegate();