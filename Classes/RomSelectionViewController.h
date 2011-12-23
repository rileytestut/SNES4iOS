//
//  RomSelectionViewController.h
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RomDetailViewController;

@interface RomSelectionViewController : UITableViewController {
    RomDetailViewController *romDetailViewController;
	
	NSMutableArray      *arrayOfCharacters;
	NSMutableDictionary *objectsForCharacters;
	NSArray             *alphabetIndex;
}

@property (nonatomic, strong) IBOutlet RomDetailViewController *romDetailViewController;

- (void) scanRomDirectory:(NSString *)path;

@end
