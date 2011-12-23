//
//  ControlPadManager.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

#define MAX_CONTROL_PADS 4

extern unsigned long padStatusForPadNumber(int which);

@interface ControlPadManager : NSObject <GKSessionDelegate> {
	GKSession *gkSession;
	
	NSMutableArray *controlPadPeerIDs;
	
	NSUInteger padAwaitingConnection;
	NSString *pendingConnectionPeerID;
	
	// these longs hold the current status of all the control pads.
	// status == a mask of which buttons are currently pressed
	// this is the value returned by statusForPadNumber:
	unsigned long padStatus[MAX_CONTROL_PADS];
}

@property (nonatomic, assign) NSUInteger padAwaitingConnection;
@property (nonatomic, retain) NSString *pendingConnectionPeerID;

- (void) searchForConnectionToPadNumber:(NSUInteger)padNumber;
- (void) stopSearchingForConnection;

- (void) acceptPendingConnection;
- (void) denyPendingConnection;
- (void) disconnectPadNumber:(NSUInteger)padNumber;
- (void) disconnectPeer:(NSString *)peerID;
- (unsigned long) statusForPadNumber:(NSUInteger)padNumber;
- (NSString *) deviceNameForPadNumber:(NSUInteger)padNumber;
- (NSString *) deviceNameForPendingConnection;

@end
