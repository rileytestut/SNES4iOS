//
//  DetailViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNES4iPadAppDelegate.h"
#import "RomDetailViewController.h"
#import "RomSelectionViewController.h"
#import "SaveStateSelectionViewController.h"
#import "SettingsViewController.h"
#import "ControlPadConnectViewController.h"
#import "ControlPadManager.h"
#import "EmulationViewController.h"

@interface RomDetailViewController ()
@property (nonatomic, retain) UIPopoverController *splitViewPopoverController;
@property (nonatomic, retain) UIPopoverController *buttonPopoverController;
- (void)configureView;
@end



@implementation RomDetailViewController

@synthesize toolbar, splitViewPopoverController, buttonPopoverController, detailItem;
@synthesize romTitleLabel, romTitleButton, romImageView, powerButton, loadButton, ejectButton;
@synthesize playerOneButton, playerTwoButton, playerThreeButton, playerFourButton, multiTapView;
@synthesize settingsButton, searchButton, snapshotImageView;
@synthesize saveStateSelectionViewController;

- (void) viewDidLoad
{
	[self.saveStateSelectionViewController addObserver:self
											forKeyPath:@"selectedSavePath" 
											   options:NSKeyValueObservingOptionNew
											   context:nil];
	CGRect newFrame = self.saveStateSelectionViewController.view.frame;
	newFrame.origin = CGPointMake(0, 560);
	self.saveStateSelectionViewController.view.frame = newFrame;
	[self.view addSubview: self.saveStateSelectionViewController.view];
	[self configureView];
}

- (void) viewDidAppear:(BOOL)animated
{
	if ([SettingsController().autoconnect isOn]) {
		if (! [AppDelegate().controlPadManager deviceNameForPadNumber:0]) {
			[NSTimer scheduledTimerWithTimeInterval:2.0f 
											 target:self 
										   selector:@selector(timerTriggeredConnectionDisplay:) 
										   userInfo:playerOneButton 
											repeats:NO];
		}
	}
}
			 
- (void) timerTriggeredConnectionDisplay:(NSTimer *)timer
{
	UIButton *b = (UIButton *)[timer userInfo];
	if (b == nil) {
		b = playerOneButton;
	}
	[self buttonPressed:b];
}
			

#pragma mark -
#pragma mark KVO observation
- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	NSLog(@"selected save changed");
	if (object == self.saveStateSelectionViewController)
	{
		if (self.saveStateSelectionViewController.selectedSavePath != nil)
		{
			self.loadButton.hidden = NO;
		}
		if (self.saveStateSelectionViewController.selectedScreenshotPath != nil)
		{
			UIImage *screenshot = [[UIImage alloc] initWithContentsOfFile:
								   self.saveStateSelectionViewController.selectedScreenshotPath];
			[self.snapshotImageView setImage:screenshot];
			[screenshot release];
			self.snapshotImageView.hidden = NO;
		} else {
			[self.snapshotImageView setImage:nil];
			self.snapshotImageView.hidden = YES;
		}

	}
}

#pragma mark -
#pragma mark Button Presses
- (IBAction) buttonPressed:(id)sender
{
	if (sender == settingsButton)
	{
		if (!buttonPopoverController) {
			buttonPopoverController = [[UIPopoverController alloc] initWithContentViewController:AppDelegate().settingsViewController];
			buttonPopoverController.delegate = self;
			[buttonPopoverController presentPopoverFromBarButtonItem:settingsButton 
											permittedArrowDirections:UIPopoverArrowDirectionAny 
															animated:YES];
		}
	} else if (sender == searchButton) {
		if (!buttonPopoverController) {
			buttonPopoverController = [[UIPopoverController alloc] initWithContentViewController:AppDelegate().webNavController];
			buttonPopoverController.delegate = self;
			[buttonPopoverController presentPopoverFromBarButtonItem:searchButton 
											permittedArrowDirections:UIPopoverArrowDirectionAny 
															animated:YES];
		}
	}
	else if (sender == powerButton) {
		NSLog(@"power button pressed");
		[AppDelegate().emulationViewController startWithRom:(NSString *)detailItem];
		[AppDelegate() showEmulator:YES];
	} else if (sender == loadButton) {
		NSLog(@"load button pressed");
		[AppDelegate().emulationViewController startWithRom:
            self.saveStateSelectionViewController.selectedSavePath ];
		[AppDelegate() showEmulator:YES];
	} else if (sender == ejectButton) {
		self.detailItem = nil;
	} else if (sender == playerOneButton || sender == playerTwoButton ||
			   sender == playerThreeButton || sender == playerFourButton)
	{
		if (!buttonPopoverController) {
			UIButton *b = (UIButton *)sender;
			AppDelegate().controlPadConnectViewController.currentPadNumber = b.tag;
			buttonPopoverController = [[UIPopoverController alloc] initWithContentViewController:AppDelegate().controlPadConnectViewController];
			buttonPopoverController.delegate = self;
			[buttonPopoverController presentPopoverFromRect:b.frame 
													 inView:[b superview]
								   permittedArrowDirections:UIPopoverArrowDirectionAny
												   animated:YES];
		}
	}
}

- (void) updateConnectionButtons
{
	NSArray *buttonArray = [NSArray arrayWithObjects:
							playerOneButton, playerTwoButton, playerThreeButton, playerFourButton, nil];
	
	int i = 0;
	for (UIButton *b in buttonArray)
	{
		NSString *deviceName = [AppDelegate().controlPadManager deviceNameForPadNumber:i];
		if (deviceName && ! [deviceName isEqual:@""]) {
			b.titleLabel.text = deviceName;
		} else {
			b.titleLabel.text = @"Not Connected";
		}
		i++;
	}
}

#pragma mark -
#pragma mark PopoverController delegate methods
- (void) popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{
	if (popoverController == buttonPopoverController)
	{
		[buttonPopoverController release];
		buttonPopoverController = nil;
	}
    if (AppDelegate().controlPadConnectViewController.state == ControlPadConnectionStateSearching) {
        [AppDelegate().controlPadManager stopSearchingForConnection];
    }
}


#pragma mark -
#pragma mark Managing the detail item

/*
 When setting the detail item, update the view and dismiss the popover controller if it's showing.
 */
- (void)setDetailItem:(id)newDetailItem {
    if (detailItem != newDetailItem) {
        [detailItem release];
        detailItem = [newDetailItem retain];
        
        // Update the view.

        [self configureView];
    }

    if (splitViewPopoverController != nil) {
        [splitViewPopoverController dismissPopoverAnimated:YES];
    }        
}


- (void)configureView {
	self.multiTapView.hidden = ! [SettingsController().multiTap isOn];
    // Update the user interface for the detail item.
	if (!detailItem) {
		self.romTitleButton.title = @"No ROM Selected";
		self.saveStateSelectionViewController.view.hidden = YES;
		self.snapshotImageView.hidden = YES;
		self.romTitleLabel.hidden = YES;
		self.powerButton.hidden = YES;
		self.loadButton.hidden = YES;
		self.romImageView.hidden = YES;
		self.ejectButton.hidden = YES;
		return;
	}
	NSString *romFile = [[detailItem description] lastPathComponent];
    self.romTitleLabel.text = [romFile stringByDeletingPathExtension];   
	self.romTitleButton.title = romTitleLabel.text;
	self.romTitleLabel.hidden = NO;
	
	self.saveStateSelectionViewController.romFilter = romFile;
	[self.saveStateSelectionViewController scanSaveDirectory];
	self.saveStateSelectionViewController.view.hidden = NO;
	self.snapshotImageView.hidden = YES;
	self.loadButton.hidden = YES;
	self.powerButton.hidden = NO;
	self.romImageView.hidden = NO;
	self.ejectButton.hidden = NO;
}


#pragma mark -
#pragma mark Split view support

- (void)splitViewController: (UISplitViewController*)svc willHideViewController:(UIViewController *)aViewController withBarButtonItem:(UIBarButtonItem*)barButtonItem forPopoverController: (UIPopoverController*)pc {
    
    barButtonItem.title = @"ROMs";
    NSMutableArray *items = [[toolbar items] mutableCopy];
    [items insertObject:barButtonItem atIndex:0];
    [toolbar setItems:items animated:YES];
    [items release];
    self.splitViewPopoverController = pc;
}


// Called when the view is shown again in the split view, invalidating the button and popover controller.
- (void)splitViewController: (UISplitViewController*)svc willShowViewController:(UIViewController *)aViewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem {
    
    NSMutableArray *items = [[toolbar items] mutableCopy];
    [items removeObjectAtIndex:0];
    [toolbar setItems:items animated:YES];
    [items release];
    self.splitViewPopoverController = nil;
}


#pragma mark -
#pragma mark Rotation support

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}


#pragma mark -
#pragma mark View lifecycle

/*
 // Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
 */

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/

- (void)viewDidUnload {
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
    self.splitViewPopoverController = nil;
}


#pragma mark -
#pragma mark Memory management

/*
- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}
*/

- (void)dealloc {
	[saveStateSelectionViewController removeObserver:self forKeyPath:@"selectedSavePath"];
	[saveStateSelectionViewController release];
    [splitViewPopoverController release];
    [toolbar release];
    
    [detailItem release];
    [romTitleLabel release];
	[romImageView release];
    [super dealloc];
}

@end
