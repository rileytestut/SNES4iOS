//
//  EmulationViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/14/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface EmulationViewController : UIViewController <UIActionSheetDelegate, UIAlertViewDelegate> {
}

@property (strong, nonatomic) id pauseAlert;

- (void) startWithRom:(NSString *)romFile;

/*
- (void) stop;
- (void) pause;
- (void) resume;
- (void) saveCurrentState;
- (void) saveNewState;
*/

- (void) refreshScreen;
- (void) didRotate:(NSNotification *)notification;
- (void) showPauseDialogFromRect:(CGRect)rect;
- (void)object:(id)object clickedButtonAtIndex:(NSInteger)buttonIndex;

@end
