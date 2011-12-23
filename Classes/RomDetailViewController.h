//
//  RomDetailViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class SaveStateSelectionViewController;

@interface RomDetailViewController : UIViewController <UIPopoverControllerDelegate, UISplitViewControllerDelegate> {
    
    UIPopoverController *splitViewPopoverController;
	UIPopoverController *buttonPopoverController;
    UIToolbar *toolbar;
    
    id detailItem;
    UILabel *romTitleLabel;
	UIImageView *romImageView;
	UIBarButtonItem *romTitleButton;
	UIBarButtonItem *settingsButton;
	UIBarButtonItem *searchButton;

	UIImageView *snapshotImageView;
	UIButton *powerButton;
	UIButton *loadButton;
	UIButton *ejectButton;
	
	
	UIView *multiTapView;
	UIButton *playerOneButton;
	UIButton *playerTwoButton;
	UIButton *playerThreeButton;
	UIButton *playerFourButton;
	
	SaveStateSelectionViewController *saveStateSelectionViewController;
}

@property (nonatomic, retain) IBOutlet UIToolbar *toolbar;

@property (nonatomic, retain) id detailItem;
@property (nonatomic, retain) IBOutlet UILabel *romTitleLabel;
@property (nonatomic, retain) IBOutlet UIImageView *romImageView;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *romTitleButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *settingsButton;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *searchButton;

@property (nonatomic, retain) IBOutlet UIButton *powerButton;
@property (nonatomic, retain) IBOutlet UIButton *loadButton;
@property (nonatomic, retain) IBOutlet UIButton *ejectButton;
@property (nonatomic, retain) IBOutlet UIImageView *snapshotImageView;

@property (nonatomic, retain) IBOutlet UIView *multiTapView;
@property (nonatomic, retain) IBOutlet UIButton *playerOneButton;
@property (nonatomic, retain) IBOutlet UIButton *playerTwoButton;
@property (nonatomic, retain) IBOutlet UIButton *playerThreeButton;
@property (nonatomic, retain) IBOutlet UIButton *playerFourButton;

@property (nonatomic, retain) IBOutlet SaveStateSelectionViewController *saveStateSelectionViewController;

- (IBAction) buttonPressed:(id)sender;

- (void) updateConnectionButtons;
- (void) timerTriggeredConnectionDisplay:(NSTimer *)timer;

@end
