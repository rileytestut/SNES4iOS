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

@property (nonatomic, strong) IBOutlet UISwitch* autosave;
@property (nonatomic, strong) IBOutlet UISwitch* smoothScaling;
@property (nonatomic, strong) IBOutlet UISwitch* fpsDisplay;
@property (nonatomic, strong) IBOutlet UISwitch* transparency;
@property (nonatomic, strong) IBOutlet UISwitch* speedHack;
@property (nonatomic, strong) IBOutlet UISwitch* multiTap;
@property (nonatomic, strong) IBOutlet UISwitch* autoconnect;

- (IBAction) settingChanged:(id)sender;


@end

// Global accessor
extern SettingsViewController *SettingsController();