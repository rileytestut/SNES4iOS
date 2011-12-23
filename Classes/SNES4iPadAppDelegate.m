//
//  SNES4iPadAppDelegate.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNES4iPadAppDelegate.h"

#import "EmulationViewController.h"
#import "RomSelectionViewController.h"
#import "RomDetailViewController.h"
#import "SettingsViewController.h"
#import "ControlPadConnectViewController.h"
#import "ControlPadManager.h"
#import "WebBrowserViewController.h"

SNES4iPadAppDelegate *AppDelegate()
{
	return (SNES4iPadAppDelegate *)[[UIApplication sharedApplication] delegate];
}

@implementation SNES4iPadAppDelegate

@synthesize window, splitViewController, romSelectionViewController, romDetailViewController, settingsViewController;
@synthesize controlPadConnectViewController, controlPadManager;
@synthesize romDirectoryPath, saveDirectoryPath, snapshotDirectoryPath;
@synthesize emulationViewController, webViewController, webNavController;


#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
      
      [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    
	settingsViewController = [[SettingsViewController alloc] init];
	// access the view property to force it to load
	settingsViewController.view = settingsViewController.view;
	
	controlPadConnectViewController = [[ControlPadConnectViewController alloc] init];
	controlPadManager = [[ControlPadManager alloc] init];
	

	NSString *documentsPath = [SNES4iPadAppDelegate applicationDocumentsDirectory];
//	romDirectoryPath = [[documentsPath stringByAppendingPathComponent:@"ROMs/SNES/"] retain];
	romDirectoryPath = [documentsPath copy];
	saveDirectoryPath = [[romDirectoryPath stringByAppendingPathComponent:@"saves"] retain];
	snapshotDirectoryPath = [[saveDirectoryPath stringByAppendingPathComponent:@"snapshots"] retain];
    
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    [fileManager createDirectoryAtPath:saveDirectoryPath withIntermediateDirectories:YES attributes:nil error:nil];
    [fileManager createDirectoryAtPath:snapshotDirectoryPath withIntermediateDirectories:YES attributes:nil error:nil];
    //Apple says its better to attempt to create the directories and accept an error than to manually check if they exist.
    [fileManager release];
		
	// Make the main emulator view controller
	emulationViewController = [[EmulationViewController alloc] init];
	emulationViewController.view.hidden = YES;
	
	// Make the web browser view controller
	webViewController = [[WebBrowserViewController alloc] initWithNibName:@"WebBrowserViewController" bundle:nil];
	
	// And put it in a navigation controller with back/forward buttons
	webNavController = [[UINavigationController alloc] initWithRootViewController:webViewController];
	webNavController.navigationBar.barStyle = UIBarStyleBlack;
	
    
	// Add the split view controller's view to the window and display.
    [window addSubview:splitViewController.view];
	
	// Add the emulation view in its hidden state.
	[window addSubview:emulationViewController.view];
	
    [window makeKeyAndVisible];
    
    return YES;
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Save data if appropriate
}

- (void) showEmulator:(BOOL)showOrHide
{
	if (showOrHide) {
        self.splitViewController.view.hidden = YES;
        self.emulationViewController.view.hidden = NO;
		[[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
	} else {
        self.emulationViewController.view.hidden = YES;
        self.splitViewController.view.hidden = NO;
		[[UIApplication sharedApplication] setStatusBarHidden:NO withAnimation:UIStatusBarAnimationFade];
	}
}

+ (NSString *) applicationDocumentsDirectory 
{    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
    return basePath;
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc {
	[romDirectoryPath release];
	[settingsViewController release];
	[romDetailViewController release];
	[romSelectionViewController release];
	[controlPadManager release];
	[controlPadConnectViewController release];
    [splitViewController release];
    [window release];
    [super dealloc];
}


@end

