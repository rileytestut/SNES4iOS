//
//  SNES4iPadAppDelegate.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"

#import "EmulationViewController.h"
#import "RomSelectionViewController.h"
#import "RomDetailViewController.h"
#import "SettingsViewController.h"
#import "ControlPadConnectViewController.h"
#import "ControlPadManager.h"
#import "WebBrowserViewController.h"

SNES4iOSAppDelegate *AppDelegate()
{
	return (SNES4iOSAppDelegate *)[[UIApplication sharedApplication] delegate];
}

@implementation SNES4iOSAppDelegate

@synthesize window, splitViewController, romSelectionViewController, romDetailViewController, settingsViewController;
@synthesize controlPadConnectViewController, controlPadManager;
@synthesize romDirectoryPath, saveDirectoryPath, snapshotDirectoryPath;
@synthesize emulationViewController, webViewController, webNavController;
@synthesize tabBarController;
@synthesize snesControllerAppDelegate, snesControllerViewController;


#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
      
      [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    
	settingsViewController = [[SettingsViewController alloc] init];
	// access the view property to force it to load
	settingsViewController.view = settingsViewController.view;
	
	controlPadConnectViewController = [[ControlPadConnectViewController alloc] init];
	controlPadManager = [[ControlPadManager alloc] init];
	

	NSString *documentsPath = [SNES4iOSAppDelegate applicationDocumentsDirectory];
//	romDirectoryPath = [[documentsPath stringByAppendingPathComponent:@"ROMs/SNES/"] retain];
	self.romDirectoryPath = [documentsPath copy];
	self.saveDirectoryPath = [romDirectoryPath stringByAppendingPathComponent:@"saves"];
	self.snapshotDirectoryPath = [saveDirectoryPath stringByAppendingPathComponent:@"snapshots"];
    
    NSFileManager *fileManager = [[NSFileManager alloc] init];
    [fileManager createDirectoryAtPath:saveDirectoryPath withIntermediateDirectories:YES attributes:nil error:nil];
    [fileManager createDirectoryAtPath:snapshotDirectoryPath withIntermediateDirectories:YES attributes:nil error:nil];
    //Apple says its better to attempt to create the directories and accept an error than to manually check if they exist.
		
	// Make the main emulator view controller
	emulationViewController = [[EmulationViewController alloc] init];
	emulationViewController.view.hidden = YES;
	
	// Make the web browser view controller
	webViewController = [[WebBrowserViewController alloc] initWithNibName:@"WebBrowserViewController" bundle:nil];
	
	// And put it in a navigation controller with back/forward buttons
	webNavController = [[UINavigationController alloc] initWithRootViewController:webViewController];
	webNavController.navigationBar.barStyle = UIBarStyleBlack;
    
    
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.romSelectionViewController = [[RomSelectionViewController alloc] initWithNibName:@"RomSelectionViewController" bundle:[NSBundle mainBundle]];
    UINavigationController *masterNavigationController = [[UINavigationController alloc] initWithRootViewController:self.romSelectionViewController];
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        
        self.snesControllerAppDelegate = [[SNESControllerAppDelegate alloc] init];
        self.snesControllerViewController = [[SNESControllerViewController alloc] initWithNibName:@"SNESControllerViewController" bundle:[NSBundle mainBundle]];
        self.snesControllerAppDelegate.viewController = self.snesControllerViewController;
        self.tabBarController = [[UITabBarController alloc] init];
        [self.tabBarController setViewControllers:[NSArray arrayWithObjects:masterNavigationController, nil]];
        self.window.rootViewController = self.tabBarController;
    } else {
    	self.romDetailViewController = [[RomDetailViewController alloc] initWithNibName:@"DetailView" bundle:[NSBundle mainBundle]];
        self.romSelectionViewController.romDetailViewController = self.romDetailViewController;
        self.splitViewController = [[UISplitViewController alloc] init];
        self.splitViewController.delegate = self.romDetailViewController;
        self.splitViewController.viewControllers = [NSArray arrayWithObjects:masterNavigationController, self.romDetailViewController, nil];
        
        self.window.rootViewController = self.splitViewController;
    }
    [self.window makeKeyAndVisible];
	
    
	// Add the split view controller's view to the window and display.
    //[window addSubview:splitViewController.view];
	
	// Add the emulation view in its hidden state.
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        [window addSubview:emulationViewController.view];
    }
    else {
        emulationViewController.view.hidden = NO;
    }
	
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
        if (self.emulationViewController.view.superview != nil) {
            [self.emulationViewController.view removeFromSuperview];
        }
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



@end

