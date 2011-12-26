//
//  RomSelectionViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"
#import "RomSelectionViewController.h"
#import "RomDetailViewController.h"
#import "EmulationViewController.h"
#import "SNESControllerViewController.h"
#import "ScreenLayer.h"
#import "SettingsViewController.h"

#define RADIANS(degrees) ((degrees * M_PI) / 180.0)

@implementation RomSelectionViewController

@synthesize romDetailViewController;


#pragma mark -
#pragma mark View lifecycle

- (void)viewDidLoad {
    [super viewDidLoad];
    
    arrayOfCharacters = [[NSMutableArray alloc] init];
	objectsForCharacters = [[NSMutableDictionary alloc] init];
	
	alphabetIndex = [NSArray arrayWithArray:
                     [@"A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z|#"
                      componentsSeparatedByString:@"|"]];
    
    self.clearsSelectionOnViewWillAppear = NO;
    self.contentSizeForViewInPopover = CGSizeMake(320.0, 600.0);
    
    UIBarButtonItem *refreshButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh target:self action:@selector(rescanRomDirectory)];
    self.navigationItem.rightBarButtonItem = refreshButton;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        UIBarButtonItem *searchButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemSearch target:self action:@selector(getMoreRoms)];
        self.navigationItem.leftBarButtonItem = searchButton;
    }
	
	[self scanRomDirectory:[AppDelegate() romDirectoryPath]];
}

- (void)rescanRomDirectory {
    [self scanRomDirectory:[AppDelegate() romDirectoryPath]];
}

- (void)getMoreRoms {
    [self.navigationController presentModalViewController:AppDelegate().webNavController animated:YES];
}

- (void)loadSNESController {
    if (AppDelegate().snesControllerViewController == nil) {
        AppDelegate().snesControllerViewController = [[SNESControllerViewController alloc] initWithNibName:@"SNESControllerViewController" bundle:[NSBundle mainBundle]];
    }
    AppDelegate().snesControllerViewController.imageName = @"snes-1";
    AppDelegate().snesControllerAppDelegate.controllerType = SNESControllerTypeWireless;
    AppDelegate().snesControllerViewController.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
    [self presentModalViewController:AppDelegate().snesControllerViewController animated:YES];
}

- (void)dismissSNESController {
    UIViewController *parentViewController = [AppDelegate().snesControllerViewController parentViewController];
    if ([AppDelegate().snesControllerViewController respondsToSelector:@selector(presentingViewController)]) {
        parentViewController = [AppDelegate().snesControllerViewController presentingViewController];//Fixes iOS 5 bug
    }
    [parentViewController dismissModalViewControllerAnimated:YES];
}

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/


- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self finishLoadingView];
    });
}

- (void)finishLoadingView {
    
    //iCloud stuff will go here.
}

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

// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}


#pragma mark -
#pragma mark ROM loading methods

- (void)scanRomDirectory:(NSString*)path 
{
	unsigned long numFiles = 0;
	[arrayOfCharacters removeAllObjects];
	[objectsForCharacters removeAllObjects];
	
	// make sure path has trailing '/' character
	NSString *romDir;
	if([[path substringWithRange:NSMakeRange([path length]-1,1)] compare:@"/"] == NSOrderedSame)
	{
		romDir = [[NSString alloc] initWithFormat:@"%@",path];
	}
	else
	{
		romDir = [[NSString alloc] initWithFormat:@"%@/",path];
	}
	
	int i;
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSArray* dirContents = [fileManager contentsOfDirectoryAtPath:romDir error:nil];
	NSInteger entries = [dirContents count];
	int characterLUT[27];
	NSMutableArray* arrayOfIndexedFiles[27];
	
	for(i = 0; i < 27; i++)
	{
		characterLUT[i] = 0;
		arrayOfIndexedFiles[i] = [[NSMutableArray alloc] init];
	}
	
	for ( i = 0; i < entries; i++ ) 
	{
		if(
		   ([[dirContents  objectAtIndex: i] length] < 4) ||
		   (
			[[[dirContents  objectAtIndex: i ] substringWithRange:NSMakeRange([[dirContents  objectAtIndex: i ] length]-4,4)] caseInsensitiveCompare:@".bin"] != NSOrderedSame &&
			[[[dirContents  objectAtIndex: i ] substringWithRange:NSMakeRange([[dirContents  objectAtIndex: i ] length]-4,4)] caseInsensitiveCompare:@".zip"] != NSOrderedSame &&
			[[[dirContents  objectAtIndex: i ] substringWithRange:NSMakeRange([[dirContents  objectAtIndex: i ] length]-4,4)] caseInsensitiveCompare:@".swc"] != NSOrderedSame &&
			[[[dirContents  objectAtIndex: i ] substringWithRange:NSMakeRange([[dirContents  objectAtIndex: i ] length]-4,4)] caseInsensitiveCompare:@".smc"] != NSOrderedSame 
			)
		   )
		{
			// Do nothing currently.
		}
		else
		{
			NSString* objectTitle = [dirContents  objectAtIndex: i ];
			NSString* objectIndexer = [[objectTitle substringWithRange:NSMakeRange(0,1)] uppercaseString];
			NSUInteger objectIndex = [objectIndexer rangeOfCharacterFromSet:[NSCharacterSet characterSetWithCharactersInString:@"ABCDEFGHIJKLMNOPQRSTUVWXYZ"]].location;
			
			if(objectIndex == NSNotFound)
			{
				objectIndex = 26;
			}
			else
			{
				char* objectLetter = (char *)[objectIndexer UTF8String];
				objectIndex = objectLetter[0] - 'A';
				if(objectIndex > 25)
				{
					objectIndex = 25;
				}
			}
			
			if(characterLUT[objectIndex] == 0)
			{
				characterLUT[objectIndex] = 1;
			}
			
			[arrayOfIndexedFiles[objectIndex] addObject:[dirContents objectAtIndex:i]];
			numFiles++;
		}
	}
	
	for(i = 0; i < 27; i++)
	{
		if(characterLUT[i] == 1)
		{
			NSString* characters = [NSString stringWithString:@"ABCDEFGHIJKLMNOPQRSTUVWXYZ#"];
			NSString* characterIndex = [characters substringWithRange:NSMakeRange(i,1)];
			[arrayOfCharacters addObject:characterIndex];
			[objectsForCharacters setObject:arrayOfIndexedFiles[i] forKey:characterIndex];
		}
	}
	
    
    [self checkForSram];
	
	[(UITableView*)self.view reloadData];
}

- (void) checkForSram {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *documentsDirectory = AppDelegate().romDirectoryPath;
    NSArray* dirContents = [fileManager contentsOfDirectoryAtPath:documentsDirectory error:nil];
    [dirContents enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSString *filename = obj;
        if ([filename hasSuffix:@"srm"]) {
            NSError *error = nil;
            NSString *originalFilePath = [documentsDirectory stringByAppendingPathComponent:filename];
            NSString *destinaionFilePath = [AppDelegate().sramDirectoryPath stringByAppendingPathComponent:filename];
            if ([fileManager copyItemAtPath:originalFilePath toPath:destinaionFilePath error:&error] && !error) {
                [fileManager removeItemAtPath:originalFilePath error:nil];
                NSLog(@"Successfully copied sram file to sram directory");
            }
            else {
                NSLog(@"%@. %@.", error, [error userInfo]);
            }
        }
    }];
}


#pragma mark -
#pragma mark Table view data source

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath 
{
    static NSString *reuseIdentifier = @"labelCell";
	UITableViewCell*      cell;
	NSMutableDictionary*  objects;
	
	objects = objectsForCharacters;
	
	cell = [tableView dequeueReusableCellWithIdentifier:reuseIdentifier];
	if (cell == nil) 
	{
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:reuseIdentifier];
		cell.textLabel.adjustsFontSizeToFitWidth = YES;
		cell.textLabel.numberOfLines = 1;
		cell.textLabel.minimumFontSize = 9.0f;
		cell.textLabel.lineBreakMode = UILineBreakModeMiddleTruncation;
	}
	
	cell.accessoryType = UITableViewCellAccessoryNone;
	if([arrayOfCharacters count] <= 0)
	{
		cell.textLabel.text = @"";
		return cell;
	}
	
	cell.textLabel.text = [[objects objectForKey:[arrayOfCharacters objectAtIndex:indexPath.section]] objectAtIndex:indexPath.row];

	return cell;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)aTableView {
	if([arrayOfCharacters count] <= 0)
	{
		return 1;
	}
	return [arrayOfCharacters count];
}


- (NSInteger)tableView:(UITableView *)aTableView numberOfRowsInSection:(NSInteger)section {
	if([arrayOfCharacters count] <= 0)
	{
		return 0;
	}
	
	
	return [[objectsForCharacters objectForKey:[arrayOfCharacters objectAtIndex:section]] count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section 
{
	if([arrayOfCharacters count] <= 0)
		return @"";
	
	return [arrayOfCharacters objectAtIndex:section];
}


- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView 
{
	return alphabetIndex;
}

- (NSInteger)tableView:(UITableView *)tableView sectionForSectionIndexTitle:(NSString *)title atIndex:(NSInteger)index {	
	NSInteger count = 0;
	
	for(NSString *character in arrayOfCharacters)
	{
		if([character isEqualToString:title])
			return count;
		count++;
	}
	
	return 0; // in case of some eror donot crash d application
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/


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
	if([arrayOfCharacters count] <= 0)
	{
		return;
	}
	
	NSMutableDictionary*  objects;
	
	objects = objectsForCharacters;
	
	NSString *listingsPath = AppDelegate().romDirectoryPath;
	NSString *romPath;
	if([[listingsPath substringWithRange:NSMakeRange([listingsPath length]-1,1)] compare:@"/"] == NSOrderedSame)
	{
		romPath = [listingsPath stringByAppendingString:
				   [[objects objectForKey:[arrayOfCharacters objectAtIndex:indexPath.section]] objectAtIndex:indexPath.row]];
	}
	else
	{
		romPath = [listingsPath stringByAppendingPathComponent:
				   [[objects objectForKey:[arrayOfCharacters objectAtIndex:indexPath.section]] objectAtIndex:indexPath.row]];
	}
	
	self.romDetailViewController.detailItem = (id)[romPath copy];
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        if (AppDelegate().snesControllerViewController == nil) {
            AppDelegate().snesControllerViewController = [[SNESControllerViewController alloc] initWithNibName:@"SNESControllerViewController" bundle:[NSBundle mainBundle]];
        }
        SNESControllerViewController *controllerViewController = AppDelegate().snesControllerViewController;
        AppDelegate().snesControllerAppDelegate.controllerType = SNESControllerTypeLocal;
        AppDelegate().emulationViewController.view.userInteractionEnabled = NO;
        [controllerViewController.view insertSubview:AppDelegate().emulationViewController.view atIndex:0];
        controllerViewController.wantsFullScreenLayout = YES;
        CGFloat rotationAngle = 90.0f;
        AppDelegate().emulationViewController.view.bounds = CGRectMake(0, 0, 480, 320);
        ScreenLayer *screenLayer = (ScreenLayer *)AppDelegate().emulationViewController.view.layer;
        screenLayer.rotateTransform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(rotationAngle));
        
        [AppDelegate().emulationViewController didRotate:[NSNotification notificationWithName:@"Notification" object:nil]];
        controllerViewController.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
        [UIApplication sharedApplication].statusBarHidden = YES;
        [self presentViewController:controllerViewController animated:YES completion:^{
            [AppDelegate().emulationViewController startWithRom:romPath];
            [AppDelegate() showEmulator:YES];
        }];
    }
	
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}




@end

