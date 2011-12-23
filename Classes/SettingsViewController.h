//
//  SettingsViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>



@interface SettingsViewController : UIViewController {
	UISwitch*				autosave;
	UISwitch*				smoothScaling;
	UISwitch*				fpsDisplay;
	UISwitch*				transparency;
	UISwitch*				speedHack;
	UISwitch*				multiTap;
	UISwitch*				autoconnect;
}

@property (nonatomic, retain) IBOutlet UISwitch* autosave;
@property (nonatomic, retain) IBOutlet UISwitch* smoothScaling;
@property (nonatomic, retain) IBOutlet UISwitch* fpsDisplay;
@property (nonatomic, retain) IBOutlet UISwitch* transparency;
@property (nonatomic, retain) IBOutlet UISwitch* speedHack;
@property (nonatomic, retain) IBOutlet UISwitch* multiTap;
@property (nonatomic, retain) IBOutlet UISwitch* autoconnect;

- (IBAction) settingChanged:(id)sender;


@end

// Global accessor
extern SettingsViewController *SettingsController();