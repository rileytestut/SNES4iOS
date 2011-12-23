//
//  SaveStateSelectionViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface SaveStateSelectionViewController : UIViewController <UITableViewDelegate, UITableViewDataSource> {
	NSArray*         saveFiles;
	
	NSString *romFilter;
	NSString *selectedSavePath;
	NSString *selectedScreenshotPath;
	
	UIBarButtonItem *editButton;

	UITableView *saveTableView;
}

@property (nonatomic, retain) NSString *romFilter;
@property (nonatomic, readonly) NSString *selectedSavePath;
@property (nonatomic, readonly) NSString *selectedScreenshotPath;
@property (nonatomic, retain) IBOutlet UITableView *saveTableView;
@property (nonatomic, retain) IBOutlet UIBarButtonItem *editButton;

- (void) scanSaveDirectory;
- (IBAction) buttonPressed:(id)sender;
- (void) deleteSaveAtIndex:(NSUInteger)saveIndex;
@end
