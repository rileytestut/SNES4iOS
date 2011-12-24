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
	NSString *__weak selectedSavePath;
	NSString *__weak selectedScreenshotPath;
	
	UIBarButtonItem *editButton;

	UITableView *saveTableView;
}

@property (nonatomic, copy) NSString *romFilter;
@property (nonatomic, copy) NSString *selectedSavePath;
@property (nonatomic, copy) NSString *selectedScreenshotPath;
@property (nonatomic, strong) IBOutlet UITableView *saveTableView;
@property (nonatomic, strong) IBOutlet UIBarButtonItem *editButton;

- (void) scanSaveDirectory;
- (IBAction) buttonPressed:(id)sender;
- (void) deleteSaveAtIndex:(NSUInteger)saveIndex;
@end
