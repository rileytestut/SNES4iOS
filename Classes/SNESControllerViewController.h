//
//  SNESControllerViewController.h
//  SNESController
//
//  Created by Yusef Napora on 5/5/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface SNESControllerViewController : UIViewController <UIAlertViewDelegate> {
	UIImageView *imageView;
	UIButton *infoButton;
	UIButton *connectionButton;
	
	CGRect A;
	CGRect B;
	CGRect AB;
	CGRect Up;
	CGRect Left;
	CGRect Down;
	CGRect Right;
	CGRect UpLeft;
	CGRect DownLeft;
	CGRect UpRight;
	CGRect DownRight;
	CGRect Select;
	CGRect Start;
	CGRect LPad;
	CGRect RPad;
	CGRect LPad2;
	CGRect RPad2;
	CGRect Menu;
	CGRect ButtonUp;
	CGRect ButtonLeft;
	CGRect ButtonDown;
	CGRect ButtonRight;
	CGRect ButtonUpLeft;
	CGRect ButtonDownLeft;
	CGRect ButtonUpRight;
	CGRect ButtonDownRight;
	
	
}

@property (nonatomic, retain) IBOutlet UIImageView *imageView;
@property (nonatomic, retain) IBOutlet UIButton *infoButton;
@property (nonatomic, retain) IBOutlet UIButton *connectionButton;

- (IBAction) buttonPressed:(id)sender;
- (void) getControllerCoords;
- (void) updateConnectionStatus;
- (void) showDisconnectionAlert;


@end

