    //
//  ControlPadConnectViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"
#import "ControlPadConnectViewController.h"
#import "ControlPadManager.h"


@implementation ControlPadConnectViewController

@synthesize state, controllerLabel, statusLabel, spinner, acceptButton, denyButton, disconnectButton, currentPadNumber;



- (void)viewDidLoad {
    [super viewDidLoad];
	self.contentSizeForViewInPopover = CGSizeMake(424, 128);
	
	numberStrings = [[NSArray alloc] initWithObjects:@"One", @"Two", @"Three", @"Four", nil];
	
	[self.acceptButton setBackgroundImage:[[UIImage imageNamed:@"greenButton.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]
								 forState:UIControlStateNormal];
	[self.denyButton setBackgroundImage:[[UIImage imageNamed:@"redButton.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]
							   forState:UIControlStateNormal];
	[self.disconnectButton setBackgroundImage:[[UIImage imageNamed:@"redButton.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]
							   forState:UIControlStateNormal];
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Overriden to allow any orientation.
    return YES;
}

- (void) viewWillAppear:(BOOL)animated
{
	controllerLabel.text = [NSString stringWithFormat:@"Player %@", [numberStrings objectAtIndex:currentPadNumber]];
	
	NSString *deviceName = [AppDelegate().controlPadManager deviceNameForPadNumber:currentPadNumber];
	if (deviceName == nil)
	{
		self.state = ControlPadConnectionStateSearching;
	}
}

- (void) viewWillDisappear:(BOOL)animated
{
	[AppDelegate().controlPadManager stopSearchingForConnection];
}

- (void) setState:(ControlPadConnectionState)s
{
	if (state != s)
	{
		state = s;
		switch (state)
		{
			case ControlPadConnectionStateSearching:
				acceptButton.hidden = YES;
				denyButton.hidden = YES;
				disconnectButton.hidden = YES;
				[spinner startAnimating];
				statusLabel.text = @"Waiting For Connection...";
				[AppDelegate().controlPadManager searchForConnectionToPadNumber:currentPadNumber];
				break;
			case ControlPadConnectionStateAvailable:
				acceptButton.hidden = NO;
				denyButton.hidden = NO;
				disconnectButton.hidden = YES;
				[spinner stopAnimating];
				statusLabel.text = [NSString stringWithFormat:@"Found: %@", 
									[AppDelegate().controlPadManager deviceNameForPendingConnection]];
				break;
			case ControlPadConnectionStateConnected:
				acceptButton.hidden = YES;
				denyButton.hidden = YES;
				disconnectButton.hidden = NO;
				[spinner stopAnimating];
				statusLabel.text = [NSString stringWithFormat:@"Connected to %@", 
									[AppDelegate().controlPadManager deviceNameForPadNumber:currentPadNumber]];
				break;
		}
	}
}
				
				

- (IBAction) buttonPressed:(id)sender
{
	NSLog(@"button pressed: %@", [sender description]);
	if (sender == acceptButton)
	{
		[AppDelegate().controlPadManager acceptPendingConnection];
	} else if (sender == denyButton)
	{
		[AppDelegate().controlPadManager denyPendingConnection];
		self.state = ControlPadConnectionStateSearching;
	} else if (sender == disconnectButton) {
		[AppDelegate().controlPadManager disconnectPadNumber:currentPadNumber];
		self.state = ControlPadConnectionStateSearching;
	}
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
