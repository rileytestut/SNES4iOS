//
//  SessionController.h
//  SNESController
//
//  Created by Yusef Napora on 5/17/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GameKit/GameKit.h>


@interface SessionController : UIViewController <GKSessionDelegate, UITableViewDataSource, UITableViewDelegate> {
	GKSession *gkSession;
	NSString *serverPeerID;
	NSString *serverName;
	NSMutableArray *availableServers;
	
	UILabel *statusLabel;
	UIActivityIndicatorView *spinner;
	UITableView *serverListView;
	UIButton *cancelButton;
	UIButton *disconnectButton;
}

@property (nonatomic, retain) GKSession *gkSession;
@property (nonatomic, readonly) BOOL isConnected;
@property (nonatomic, readonly) BOOL serversAvailable;

@property (nonatomic, retain) IBOutlet UILabel *statusLabel;
@property (nonatomic, retain) IBOutlet UIActivityIndicatorView *spinner;
@property (nonatomic, retain) IBOutlet UITableView *serverListView;
@property (nonatomic, retain) IBOutlet UIButton *cancelButton;
@property (nonatomic, retain) IBOutlet UIButton *disconnectButton;

- (void) searchForEmulators;
- (void) stopSearching;
- (void) connectToServerAtIndex:(NSUInteger)serverIndex;
- (void) disconnect;

- (void) updateView;

- (void) showModal;
- (void) dismissView;

- (IBAction) buttonPressed:(id)sender;

- (void) sendPadStatus:(unsigned long)status;
@end
