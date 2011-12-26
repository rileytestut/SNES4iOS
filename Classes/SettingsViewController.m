    //
//  SettingsViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"
#import "SettingsViewController.h"
#import "RomDetailViewController.h"

#define USER_DEFAULT_KEY_AUTOSAVE @"Autosave"
#define USER_DEFAULT_KEY_SMOOTH_SCALING @"SmoothScaling"
#define USER_DEFAULT_KEY_FPS_DISPLAY @"FPSDisplay"
#define USER_DEFAULT_KEY_TRANSPARENCY @"Transparency"
#define USER_DEFAULT_KEY_SPEED_HACK @"SpeedHack"
#define USER_DEFAULT_KEY_MULTI_TAP @"MultiTap"
#define USER_DEFAULT_KEY_AUTOCONNECT @"Autoconnect"
#define USER_DEFAULT_KEY_ICLOUD @"iCloud"

unsigned long __fps_debug = 0;
int iphone_soundon = 1;
int __autosave = 0;
int __transparency = 0;
int __speedhack = 0;
int __smooth_scaling = 0;

SettingsViewController *SettingsController()
{
	return AppDelegate().settingsViewController;
}

@implementation SettingsViewController

@synthesize autosave, smoothScaling, fpsDisplay, transparency, speedHack, multiTap, autoconnect, iCloud, useiCloud;


- (void) viewDidLoad
{
    BOOL iCloudEnabled = [NSUbiquitousKeyValueStore self] != nil && [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:nil] != nil;
	NSDictionary *firstRunValues = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSNumber numberWithBool:NO], USER_DEFAULT_KEY_AUTOSAVE,
									[NSNumber numberWithBool:YES], USER_DEFAULT_KEY_SMOOTH_SCALING,
									[NSNumber numberWithBool:NO], USER_DEFAULT_KEY_FPS_DISPLAY,
									[NSNumber numberWithBool:YES], USER_DEFAULT_KEY_TRANSPARENCY,
									[NSNumber numberWithBool:NO], USER_DEFAULT_KEY_SPEED_HACK,
									[NSNumber numberWithBool:NO], USER_DEFAULT_KEY_MULTI_TAP,
									[NSNumber numberWithBool:YES], USER_DEFAULT_KEY_AUTOCONNECT,
                                    [NSNumber numberWithBool:iCloudEnabled], USER_DEFAULT_KEY_ICLOUD,
									nil];
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	for (NSString *defaultKey in [firstRunValues allKeys])
	{
		NSNumber *value = [defaults objectForKey:defaultKey];
		if (!value)
		{
			value = [firstRunValues objectForKey:defaultKey];
			[defaults setObject:value forKey:defaultKey];
		} 			
	}
	
	[autosave setOn:[defaults boolForKey:USER_DEFAULT_KEY_AUTOSAVE]];
	[smoothScaling setOn:[defaults boolForKey:USER_DEFAULT_KEY_SMOOTH_SCALING]];
	[fpsDisplay setOn:[defaults boolForKey:USER_DEFAULT_KEY_FPS_DISPLAY]];
	[transparency setOn:[defaults boolForKey:USER_DEFAULT_KEY_TRANSPARENCY]];
	[speedHack setOn:[defaults boolForKey:USER_DEFAULT_KEY_SPEED_HACK]];
	[multiTap setOn:[defaults boolForKey:USER_DEFAULT_KEY_MULTI_TAP]];
	[autoconnect setOn:[defaults boolForKey:USER_DEFAULT_KEY_AUTOCONNECT]];
    [iCloud setOn:[defaults boolForKey:USER_DEFAULT_KEY_ICLOUD]];
    [iCloud setEnabled:iCloudEnabled];
	
	[self settingChanged:nil];
}	



- (void) viewDidAppear:(BOOL)animated
{
	self.contentSizeForViewInPopover = CGSizeMake(320, 335);
}

- (IBAction) settingChanged:(id)sender
{
	NSString *defaultKey = nil;
	if (sender == autosave) defaultKey = USER_DEFAULT_KEY_AUTOSAVE;
	else if (sender == smoothScaling) defaultKey = USER_DEFAULT_KEY_SMOOTH_SCALING;
	else if (sender == fpsDisplay) defaultKey = USER_DEFAULT_KEY_FPS_DISPLAY;
	else if (sender == transparency) defaultKey = USER_DEFAULT_KEY_TRANSPARENCY;
	else if (sender == speedHack) defaultKey = USER_DEFAULT_KEY_SPEED_HACK;
	else if (sender == multiTap) defaultKey = USER_DEFAULT_KEY_MULTI_TAP;
	else if (sender == autoconnect) defaultKey = USER_DEFAULT_KEY_AUTOCONNECT;
    else if (sender == iCloud) defaultKey = USER_DEFAULT_KEY_ICLOUD;
	
	if (defaultKey)
	{
		UISwitch *toggle = (UISwitch *)sender;
		[[NSUserDefaults standardUserDefaults] setBool:[toggle isOn] forKey:defaultKey];
	}
	
	__fps_debug = [fpsDisplay isOn];
 	__autosave = [autosave isOn];
 	__transparency = [transparency isOn];
 	__speedhack = [speedHack isOn];
	__smooth_scaling = [smoothScaling isOn];
    self.useiCloud = [iCloud isOn];
	
	if (sender == multiTap)
	{
		AppDelegate().romDetailViewController.multiTapView.hidden = ![multiTap isOn];
	}
}
	

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Overriden to allow any orientation.
    return YES;
}


- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}




@end
