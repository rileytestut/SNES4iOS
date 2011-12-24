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

@property (nonatomic, strong) IBOutlet UIToolbar *toolbar;

@property (nonatomic, copy) id detailItem;
@property (nonatomic, strong) IBOutlet UILabel *romTitleLabel;
@property (nonatomic, strong) IBOutlet UIImageView *romImageView;
@property (nonatomic, strong) IBOutlet UIBarButtonItem *romTitleButton;
@property (nonatomic, strong) IBOutlet UIBarButtonItem *settingsButton;
@property (nonatomic, strong) IBOutlet UIBarButtonItem *searchButton;

@property (nonatomic, strong) IBOutlet UIButton *powerButton;
@property (nonatomic, strong) IBOutlet UIButton *loadButton;
@property (nonatomic, strong) IBOutlet UIButton *ejectButton;
@property (nonatomic, strong) IBOutlet UIImageView *snapshotImageView;

@property (nonatomic, strong) IBOutlet UIView *multiTapView;
@property (nonatomic, strong) IBOutlet UIButton *playerOneButton;
@property (nonatomic, strong) IBOutlet UIButton *playerTwoButton;
@property (nonatomic, strong) IBOutlet UIButton *playerThreeButton;
@property (nonatomic, strong) IBOutlet UIButton *playerFourButton;

@property (nonatomic, copy) NSString *saveStatePath;

@property (nonatomic, strong) IBOutlet SaveStateSelectionViewController *saveStateSelectionViewController;

- (IBAction) buttonPressed:(id)sender;

- (void) updateConnectionButtons;
- (void) timerTriggeredConnectionDisplay:(NSTimer *)timer;

@end
