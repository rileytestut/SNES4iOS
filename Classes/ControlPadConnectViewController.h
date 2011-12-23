//
//  ControlPadConnectViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef enum {
	ControlPadConnectionStateConnected,
	ControlPadConnectionStateAvailable,
	ControlPadConnectionStateSearching
} ControlPadConnectionState;

@interface ControlPadConnectViewController : UIViewController {

	UILabel *controllerLabel;
	UILabel *statusLabel;
	
	UIActivityIndicatorView *spinner;
	UIButton *acceptButton;
	UIButton *denyButton;
	UIButton *disconnectButton;

	NSUInteger currentPadNumber;
	NSArray *numberStrings;
	
	ControlPadConnectionState state;
}

@property (nonatomic, strong) IBOutlet UILabel *controllerLabel;
@property (nonatomic, strong) IBOutlet UILabel *statusLabel;
@property (nonatomic, strong) IBOutlet UIActivityIndicatorView *spinner;
@property (nonatomic, strong) IBOutlet UIButton *acceptButton;
@property (nonatomic, strong) IBOutlet UIButton *denyButton;
@property (nonatomic, strong) IBOutlet UIButton *disconnectButton;
@property (nonatomic, assign) NSUInteger currentPadNumber;
@property (nonatomic, assign) ControlPadConnectionState state;

- (IBAction) buttonPressed:(id)sender;


@end
