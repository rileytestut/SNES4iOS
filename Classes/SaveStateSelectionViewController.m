//
//  SaveStateSelectionViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"
#import "SaveStateSelectionViewController.h"
#import <UIKit/UITableView.h>

@implementation SaveStateSelectionViewController

@synthesize romFilter, selectedSavePath, selectedScreenshotPath, saveTableView, editButton;

#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];

    saveFiles = [[NSMutableArray alloc] init];
	CGRect tableFrame = self.view.bounds;
	tableFrame.size.height -= 44;
	tableFrame.origin.y += 44;
	
    saveTableView = [[UITableView alloc] initWithFrame:tableFrame style:UITableViewStylePlain];
	saveTableView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
	saveTableView.dataSource = self;
	saveTableView.delegate = self;
	
	[self.view addSubview:saveTableView];
}


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


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Override to allow orientations other than the default portrait orientation.
    return YES;
}

 
- (void) scanSaveDirectory
{
	if (!self.romFilter) {
		return;
	}
	
	
	NSMutableArray *saveArray = [[NSMutableArray alloc] init];
	NSString *saveDir;
	NSString *path = AppDelegate().saveDirectoryPath;
    NSLog(@"Save Directory: %@", path);
	if([[path substringWithRange:NSMakeRange([path length]-1,1)] compare:@"/"] == NSOrderedSame)
	{
		saveDir = path;
	}
	else
	{
		saveDir = [path stringByAppendingString:@"/"];
	}
	
	int i;
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSArray* dirContents = [fileManager contentsOfDirectoryAtPath:saveDir error:nil];
	NSInteger entries = [dirContents count];

	
	for ( i = 0; i < entries; i++ ) 
	{
		if(([[dirContents  objectAtIndex: i] length] < 4) ||
		   [[[dirContents  objectAtIndex: i ] substringWithRange:NSMakeRange([[dirContents  objectAtIndex: i ] length]-3,3)] caseInsensitiveCompare:@".sv"] != NSOrderedSame)
		{
			// Do nothing currently.
		}
		else
		{
			NSString* objectTitle = [dirContents  objectAtIndex: i ];
			// only add saves for the rom we care about
			if (objectTitle.length > romFilter.length) {
			    NSString *romComparison = [objectTitle substringToIndex:[romFilter length]];
			    if (![romComparison isEqual:romFilter]) {
				    continue;
			    }
			
			    [saveArray addObject:objectTitle];
		    }
		}
	}
	
	// sort the array by decending filename (reverse chronological order, since the date is in the filename)
	NSSortDescriptor *sorter = [[NSSortDescriptor alloc] initWithKey:@"description" ascending:NO];
	saveFiles = [saveArray sortedArrayUsingDescriptors:[NSArray arrayWithObject:sorter]];
	
	[self.saveTableView reloadData];
    
}

- (IBAction) buttonPressed:(id)sender
{
	if (sender == editButton)
	{
		if (saveTableView.editing)
		{
			editButton.title = @"Edit";
			saveTableView.editing = NO;
		} else {
			editButton.title = @"Done";
			saveTableView.editing = YES;
		}
	}
}

- (void) deleteSaveAtIndex:(NSUInteger)saveIndex 
{
	NSString *savePath = [[AppDelegate() saveDirectoryPath] stringByAppendingPathComponent:
						  [saveFiles objectAtIndex:saveIndex]];
	NSString *screenshotPath = [savePath stringByAppendingPathExtension:@"png"];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSError *error = nil;
	if ([fileManager fileExistsAtPath:savePath]) {
		[fileManager removeItemAtPath:savePath error:&error];
	}
	if ([fileManager fileExistsAtPath:screenshotPath]) {
		[fileManager removeItemAtPath:screenshotPath error:&error];
	}
	
	if (!error) {
		NSMutableArray *mutableSaves = [saveFiles mutableCopy];

		[mutableSaves removeObjectAtIndex:saveIndex];
		saveFiles = [[NSArray alloc] initWithArray:mutableSaves];
		
		NSIndexPath *indexPath = [NSIndexPath indexPathForRow:saveIndex inSection:0];
		[saveTableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
	}
}

#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if([saveFiles count] <= 0)
	{
		return 0;
	}
	
	return [saveFiles count];
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{
    static NSString *reuseIdentifier = @"labelCell";
	UITableViewCell*      cell;	

	cell = [tableView dequeueReusableCellWithIdentifier:reuseIdentifier];
	if (cell == nil) 
	{
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:reuseIdentifier];
		cell.textLabel.numberOfLines = 1;
		cell.textLabel.adjustsFontSizeToFitWidth = YES;
		cell.textLabel.minimumFontSize = 9.0f;
		cell.textLabel.lineBreakMode = UILineBreakModeMiddleTruncation;
	}
	
	cell.accessoryType = UITableViewCellAccessoryNone;				
	
	if([saveFiles count] <= 0)
	{
		cell.textLabel.text = @"";
		return cell;
	}
	
	NSString *saveName = [saveFiles objectAtIndex:indexPath.row];
	NSString *dateString = [saveName substringFromIndex:[self.romFilter length]];
	NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	[dateFormatter setDateFormat:@"'-'yyMMdd-HHmmss'.sv'"];
	NSDate *saveDate = [dateFormatter dateFromString:dateString];
	
	[dateFormatter setDateFormat:@"EEE',' MMM d',' yyyy 'at' h:mm:ss a"];
	
	
	cell.textLabel.text = [dateFormatter stringFromDate:saveDate];

	// Set up the cell
	return cell;
}



// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}




// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
		[self deleteSaveAtIndex:indexPath.row];
	}   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}



/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{	
	if([saveFiles count] <= 0)
	{
		return;
	}
	
	NSString *listingsPath = AppDelegate().saveDirectoryPath;
	NSString *saveFile = [saveFiles objectAtIndex:indexPath.row];
	
	NSString *savePath = [listingsPath stringByAppendingPathComponent:saveFile];
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *screenshotFile = [saveFile stringByAppendingPathExtension:@"png"];
	NSString *screenshotPath = [AppDelegate().saveDirectoryPath stringByAppendingPathComponent:screenshotFile];
	
	[self willChangeValueForKey:@"selectedScreenshotPath"];
	if (selectedScreenshotPath)
	{
		selectedScreenshotPath = nil;
	}
	
	NSLog(@"Looking for screenshot at %@", screenshotPath);
	if ([fileManager fileExistsAtPath:screenshotPath])
	{
		NSLog(@"Found screenshot at %@", screenshotPath);
		selectedScreenshotPath = screenshotPath;
	} 
	[self didChangeValueForKey:@"selectedScreenshotPath"];
	
	
	[self willChangeValueForKey:@"selectedSavePath"];
	selectedSavePath = savePath;
	[self didChangeValueForKey:@"selectedSavePath"];

	

	
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}




@end

