//
//  SessionController.m
//  SNESController
//
//  Created by Yusef Napora on 5/17/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SessionController.h"
#import "SNESControllerAppDelegate.h"
#import "SNESControllerViewController.h"

#define SESSION_ID @"com.snes-hd.controller"
#define CONNECT_TIMEOUT 30

@implementation SessionController

@synthesize gkSession, statusLabel, serverListView, spinner, cancelButton, disconnectButton;

- (id)initWithNibName:(NSString *)nibName bundle:(NSBundle *)nibBundle
{
	if (self = [super initWithNibName:nibName bundle:nibBundle])
	{
		gkSession = [[GKSession alloc] initWithSessionID:SESSION_ID displayName:nil sessionMode:GKSessionModeClient];
		gkSession.delegate = self;
		
		availableServers = [[NSMutableArray alloc] init];
	}
	return self;
}



- (void) viewDidLoad
{
	[cancelButton setBackgroundImage:[[UIImage imageNamed:@"grayButton.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]
							forState:UIControlStateNormal];
	[disconnectButton setBackgroundImage:[[UIImage imageNamed:@"grayButton.png"] stretchableImageWithLeftCapWidth:10 topCapHeight:0]
							forState:UIControlStateNormal];
}

- (void) searchForEmulators
{
	gkSession.available = YES;
}

- (void) stopSearching
{
	gkSession.available = NO;
}

- (BOOL) isConnected
{
	return (serverPeerID != nil);
}

- (BOOL) serversAvailable
{
	return ([availableServers count] > 0 && ![self isConnected]);
}


- (void) viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];
	[self updateView];
}

- (void) updateView
{
	if (self.isConnected) {
		statusLabel.text = [NSString stringWithFormat:@"Connected to %@", serverName];
		serverListView.hidden = YES;
		cancelButton.hidden = NO;
		disconnectButton.hidden = NO;
		[spinner stopAnimating];
	} else {
		[self searchForEmulators];
		if (self.serversAvailable) {
			statusLabel.text = @"Tap to connect:";
			serverListView.hidden = NO;
			cancelButton.hidden = NO;
			disconnectButton.hidden = YES;
			[serverListView reloadData];
			[spinner stopAnimating];
		} else {
			statusLabel.text = @"Searching for SNES (HD)";
			serverListView.hidden = YES;
			[spinner startAnimating];
			cancelButton.hidden = NO;
			disconnectButton.hidden = YES;
		}
	}
}



- (IBAction) buttonPressed:(id)sender
{
	if (sender == cancelButton)
	{
		[self stopSearching];
		[self dismissView];
	} else if (sender == disconnectButton) {
		[self disconnect];
		[self dismissView];
	}
}

- (void) connectToServerAtIndex:(NSUInteger)serverIndex
{
	NSString *peerID = [availableServers objectAtIndex:serverIndex];
	[gkSession connectToPeer:peerID withTimeout:30];
}
	
- (void) disconnect
{
	[gkSession disconnectFromAllPeers];
	serverPeerID = nil;
	serverName = nil;
	[availableServers removeAllObjects];
	[self updateView];
}
			

- (void) sendPadStatus:(unsigned long)status
{
	if (!gkSession || !serverPeerID)
		return;
	
	NSData *message = [NSData dataWithBytes:&status length:sizeof(status)];
	NSError *error = nil;
	[gkSession sendDataToAllPeers:message withDataMode:GKSendDataUnreliable error:&error];
	if (error)
	{
		NSLog(@"Error sending pad status: %@", error);
	}
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft ||
			interfaceOrientation == UIInterfaceOrientationLandscapeRight);
}


// Use this to show the modal view (pops-up from the bottom)`
- (void) showModal
{
	UIView* padView = ControllerAppDelegate().viewController.view;
	CGPoint middleCenter = CGPointMake(240, 160);
	CGSize screenSize = [UIScreen mainScreen].bounds.size;
	CGSize offSize = CGSizeMake(screenSize.height, screenSize.width);
	CGPoint offScreenCenter = CGPointMake(offSize.width / 2.0, offSize.height * 1.5);
	self.view.center = offScreenCenter; // we start off-screen
	[self updateView];
	[padView addSubview:self.view];
		
	// Show it with a transition effect
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.7]; // animation duration in seconds
	self.view.center = middleCenter;
	[UIView commitAnimations];
}

- (void) dismissView
{
	CGSize screenSize = [UIScreen mainScreen].bounds.size;
	CGSize offSize = CGSizeMake(screenSize.height, screenSize.width);
	CGPoint offScreenCenter = CGPointMake(offSize.width / 2.0, offSize.height * 1.5);
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:0.7];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(dismissViewEnded:finished:context:)];
	self.view.center = offScreenCenter;
	[UIView commitAnimations];
}

- (void) dismissViewEnded:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
	[self.view removeFromSuperview];
}


#pragma mark -
#pragma mark GKSession Delegate methods
- (void)session:(GKSession *)session peer:(NSString *)peerID didChangeState:(GKPeerConnectionState)state {
	switch (state) {
		case GKPeerStateAvailable:
			if (![availableServers containsObject:peerID])
			{
				[availableServers addObject:peerID];
				[self updateView];
			}
			break;
		case GKPeerStateConnected:
			serverPeerID = peerID;
			serverName = [session displayNameForPeer:peerID];
			[self stopSearching];
			[self dismissView];
			[ControllerAppDelegate().viewController updateConnectionStatus];
			break;
		case GKPeerStateDisconnected:
			[self disconnect];
			[self dismissView];
			[ControllerAppDelegate().viewController updateConnectionStatus];
			[ControllerAppDelegate().viewController showDisconnectionAlert];
			break;

		default:
			break;
	}
}

#pragma mark -
#pragma mark UITableView Data Source methods
- (NSInteger) numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	return [availableServers count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{
	UITableViewCell*      cell;	
	
	cell = [tableView dequeueReusableCellWithIdentifier:@"labelCell"];
	if (cell == nil) 
	{
		cell = [[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"labelCell"];
		cell.textLabel.numberOfLines = 1;
		cell.textLabel.adjustsFontSizeToFitWidth = YES;
		cell.textLabel.minimumFontSize = 9.0f;
		cell.textLabel.lineBreakMode = UILineBreakModeMiddleTruncation;
	}
	
	cell.accessoryType = UITableViewCellAccessoryNone;
	if ([availableServers count] <= 0 || indexPath.row >= [availableServers count])
	{
		cell.textLabel.text = @"";
		return cell;
	}
	
	cell.textLabel.text = [gkSession displayNameForPeer:[availableServers objectAtIndex:indexPath.row]];
	return cell;
}

#pragma mark -
#pragma mark Table View delegate methods
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{	
	if([availableServers count] <= 0)
	{
		return;
	}
	
	[self connectToServerAtIndex:indexPath.row];
}
	


@end
