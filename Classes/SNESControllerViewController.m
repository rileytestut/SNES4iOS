//
//  SNESControllerViewController.m
//  SNESController
//
//  Created by Yusef Napora on 5/5/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "SNESControllerAppDelegate.h"
#import "SNESControllerViewController.h"
#import "SessionController.h"
#import "ControlPadManager.h"
#import "EmulationViewController.h"
#import "RomSelectionViewController.h"

#define	DefaultControllerImage @"landscape_controller"

#define RADIANS(degrees) ((degrees * M_PI) / 180.0)

unsigned long gp2x_pad_status;
static unsigned long newtouches[10];
static unsigned long oldtouches[10];

enum  { GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
	GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
	GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
	GP2X_VOL_UP=1<<23, GP2X_VOL_DOWN=1<<22, GP2X_PUSH=1<<27 };

void rt_dispatch_sync_on_main_thread(dispatch_block_t block) {
    if ([NSThread isMainThread]) {
        block();
    } else {
        dispatch_sync(dispatch_get_main_queue(), block);
    }
}

@implementation SNESControllerViewController
@synthesize imageView;
@synthesize infoButton;
@synthesize connectionButton;
@synthesize imageName;

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
    self.wantsFullScreenLayout = YES;
	self.infoButton.transform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(0.0));
    self.connectionButton.transform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(0.0));
	self.view.multipleTouchEnabled = YES;
	//self.imageView.image = [UIImage imageNamed:DefaultControllerImage];
}

- (void)viewWillAppear:(BOOL)animated {
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationFade];
    NSString *newImageName = @"landscape_controller";
    if (ControllerAppDelegate().controllerType == SNESControllerTypeLocal) {
        self.connectionButton.hidden = YES;
        self.infoButton.hidden = YES;
        newImageName = @"landscape_controller";
    }
    else {
        self.connectionButton.hidden = NO;
        self.connectionButton.hidden = NO;
        newImageName = @"snes-1";
    }
    [self changeBackgroundImage:newImageName];
    ControllerAppDelegate().viewController = self;
}

- (void) changeBackgroundImage:(NSString *)newImageName {
    self.imageName = newImageName;
    rt_dispatch_sync_on_main_thread(^{
        self.imageView.image = [UIImage imageNamed:self.imageName];
    });
    [self getControllerCoords];
}

- (void) viewDidAppear:(BOOL)animated
{
    if (ControllerAppDelegate().controllerType == SNESControllerTypeWireless) {
        if (!ControllerAppDelegate().sessionController) {
            ControllerAppDelegate().sessionController = [[SessionController alloc] initWithNibName:@"SessionController" bundle:[NSBundle mainBundle]];
        }
        if (! ControllerAppDelegate().sessionController.isConnected)
        {
            [ControllerAppDelegate().sessionController showModal];
        }
    }
    else {
        [ControllerAppDelegate().sessionController disconnect];
        [ControllerAppDelegate().sessionController stopSearching];
    }
}

- (void)viewWillDisappear:(BOOL)animated {
    [ControllerAppDelegate().sessionController dismissView];
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    if (ControllerAppDelegate().controllerType == SNESControllerTypeLocal) {
        return (interfaceOrientation == UIInterfaceOrientationPortrait);
    }
    else {
        return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || 
                interfaceOrientation == UIInterfaceOrientationLandscapeRight);
    }
}


- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}



- (IBAction) buttonPressed:(id)sender
{
	if (sender == connectionButton) {
		[ControllerAppDelegate().sessionController showModal];
	} 
}

- (IBAction)dismissController:(id)sender {
    [AppDelegate().romSelectionViewController dismissSNESController];
}

- (void) updateConnectionStatus
{
	if (ControllerAppDelegate().sessionController.isConnected) {
		[connectionButton setBackgroundImage:[UIImage imageNamed:@"ConnectedIcon.png"] forState:UIControlStateNormal];
	} else {
		[connectionButton setBackgroundImage:[UIImage imageNamed:@"NotConnectedIcon.png"] forState:UIControlStateNormal];
	}
}

- (void) showDisconnectionAlert
{
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Disconnected"
													message:@"Disconnected from server"
												   delegate:self 
										  cancelButtonTitle:@"Ignore"
										  otherButtonTitles:@"Reconnect",nil];
	[alert show];
}

- (void) alertViewCancel:(UIAlertView *)alertView
{
	[self updateConnectionStatus];
	[alertView dismissWithClickedButtonIndex:-1 animated:YES];
}

- (void) alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (buttonIndex == [alertView firstOtherButtonIndex])
	{
		[alertView dismissWithClickedButtonIndex:buttonIndex animated:YES];
		[ControllerAppDelegate().sessionController showModal];
	}
}

#define MyCGRectContainsPoint(rect, point)						  \
(((point.x >= rect.origin.x) &&								        \
(point.y >= rect.origin.y) &&							          \
(point.x <= rect.origin.x + rect.size.width) &&			\
(point.y <= rect.origin.y + rect.size.height)) ? 1 : 0)


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{	    
	int touchstate[10];
	//Get all the touches.
	int i;
	NSSet *allTouches = [event allTouches];
	int touchcount = [allTouches count];
	
	for (i = 0; i < 10; i++) 
	{
		touchstate[i] = 0;
		oldtouches[i] = newtouches[i];
	}
	
	for (i = 0; i < touchcount; i++) 
	{
		UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
		
		if( touch != nil && 
		   ( touch.phase == UITouchPhaseBegan ||
			touch.phase == UITouchPhaseMoved ||
			touch.phase == UITouchPhaseStationary) )
		{
			struct CGPoint point;
			point = [touch locationInView:self.view];
			
			touchstate[i] = 1;
			
			if (MyCGRectContainsPoint(Left, point)) 
			{
				gp2x_pad_status |= GP2X_LEFT;
				newtouches[i] = GP2X_LEFT;
			}
			else if (MyCGRectContainsPoint(Right, point)) 
			{
				gp2x_pad_status |= GP2X_RIGHT;
				newtouches[i] = GP2X_RIGHT;
			}
			else if (MyCGRectContainsPoint(Up, point)) 
			{
				gp2x_pad_status |= GP2X_UP;
				newtouches[i] = GP2X_UP;
			}
			else if (MyCGRectContainsPoint(Down, point))
			{
				gp2x_pad_status |= GP2X_DOWN;
				newtouches[i] = GP2X_DOWN;
			}
			else if (MyCGRectContainsPoint(ButtonLeft, point)) 
			{
				gp2x_pad_status |= GP2X_A;
				newtouches[i] = GP2X_A;
			}
			else if (MyCGRectContainsPoint(ButtonRight, point)) 
			{
				gp2x_pad_status |= GP2X_B;
				newtouches[i] = GP2X_B;
			}
			else if (MyCGRectContainsPoint(ButtonUp, point)) 
			{
				gp2x_pad_status |= GP2X_Y;
				newtouches[i] = GP2X_Y;
			}
			else if (MyCGRectContainsPoint(ButtonDown, point)) 
			{
				gp2x_pad_status |= GP2X_X;
				newtouches[i] = GP2X_X;
			}
			else if (MyCGRectContainsPoint(ButtonUpLeft, point)) 
			{
				gp2x_pad_status |= GP2X_A | GP2X_Y;
				newtouches[i] = GP2X_A | GP2X_Y;
			}
			else if (MyCGRectContainsPoint(ButtonDownLeft, point)) 
			{
				gp2x_pad_status |= GP2X_X | GP2X_A;
				newtouches[i] = GP2X_X | GP2X_A;
			}
			else if (MyCGRectContainsPoint(ButtonUpRight, point)) 
			{
				gp2x_pad_status |= GP2X_B | GP2X_Y;
				newtouches[i] = GP2X_B | GP2X_Y;
			}
			else if (MyCGRectContainsPoint(ButtonDownRight, point)) 
			{
				gp2x_pad_status |= GP2X_X | GP2X_B;
				newtouches[i] = GP2X_X | GP2X_B;
			}
			else if (MyCGRectContainsPoint(UpLeft, point)) 
			{
				gp2x_pad_status |= GP2X_UP | GP2X_LEFT;
				newtouches[i] = GP2X_UP | GP2X_LEFT;
			} 
			else if (MyCGRectContainsPoint(DownLeft, point)) 
			{
				gp2x_pad_status |= GP2X_DOWN | GP2X_LEFT;
				newtouches[i] = GP2X_DOWN | GP2X_LEFT;
			}
			else if (MyCGRectContainsPoint(UpRight, point)) 
			{
				gp2x_pad_status |= GP2X_UP | GP2X_RIGHT;
				newtouches[i] = GP2X_UP | GP2X_RIGHT;
			}
			else if (MyCGRectContainsPoint(DownRight, point)) 
			{
				gp2x_pad_status |= GP2X_DOWN | GP2X_RIGHT;
				newtouches[i] = GP2X_DOWN | GP2X_RIGHT;
			}			
			else if (MyCGRectContainsPoint(LPad, point)) 
			{
				gp2x_pad_status |= GP2X_L;
				newtouches[i] = GP2X_L;
			}
			else if (MyCGRectContainsPoint(RPad, point)) 
			{
				gp2x_pad_status |= GP2X_R;
				newtouches[i] = GP2X_R;
			}			
			else if (MyCGRectContainsPoint(LPad2, point)) 
			{
				gp2x_pad_status |= GP2X_VOL_DOWN;
				newtouches[i] = GP2X_VOL_DOWN;
			}
			else if (MyCGRectContainsPoint(RPad2, point)) 
			{
				gp2x_pad_status |= GP2X_VOL_UP;
				newtouches[i] = GP2X_VOL_UP;
			}
			else if (MyCGRectContainsPoint(Select, point)) 
			{
				gp2x_pad_status |= GP2X_SELECT;
				newtouches[i] = GP2X_SELECT;
			}
			else if (MyCGRectContainsPoint(Start, point)) 
			{
				gp2x_pad_status |= GP2X_START;
				newtouches[i] = GP2X_START;
			}
			else if (MyCGRectContainsPoint(Menu, point)) 
			{
                CGRect rect = CGRectMake(Menu.origin.x + Menu.size.width/2, Menu.size.height, 60, 60);
				[AppDelegate().emulationViewController showPauseDialogFromRect:rect];
			}
			
			if(oldtouches[i] != newtouches[i])
			{
				gp2x_pad_status &= ~(oldtouches[i]);
			}
		}	
	} 
	
	for (i = 0; i < 10; i++) 
	{
		if(touchstate[i] == 0)
		{
			gp2x_pad_status &= ~(newtouches[i]);
			newtouches[i] = 0;
			oldtouches[i] = 0;
		}
	}
	
    if (ControllerAppDelegate().controllerType == SNESControllerTypeWireless) {
        [ControllerAppDelegate().sessionController sendPadStatus:gp2x_pad_status];
    }
    else {
        NSData *data = [NSData dataWithBytes:&gp2x_pad_status length:sizeof(gp2x_pad_status)];
        [AppDelegate().controlPadManager convertData:data padNumber:0];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
}


- (void)getControllerCoords {
    char string[256];
    FILE *fp;
	
	NSString *filepath = [[NSBundle mainBundle] pathForResource:self.imageName ofType:@"txt"];
	fp = fopen([filepath UTF8String], "r");
	
	if (fp) 
	{
		int i = 0;
        while(fgets(string, 256, fp) != NULL && i < 24) {
			char* result = strtok(string, ",");
			int coords[4];
			int i2 = 1;
			while( result != NULL && i2 < 5 )
			{
				coords[i2 - 1] = atoi(result);
				result = strtok(NULL, ",");
				i2++;
			}
			
			switch(i)
			{
				case 0:    DownLeft   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 1:    Down   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 2:    DownRight    = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 3:    Left  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 4:    Right  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 5:    UpLeft     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 6:    Up     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 7:    UpRight  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 8:    Select = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 9:    Start  = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 10:   LPad   = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 11:   RPad   = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 12:   Menu   = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 13:   ButtonDownLeft   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 14:   ButtonDown   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 15:   ButtonDownRight    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 16:   ButtonLeft  		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 17:   ButtonRight  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 18:   ButtonUpLeft     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 19:   ButtonUp     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 20:   ButtonUpRight  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 21:   LPad2   = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
				case 22:   RPad2   = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			}
           	i++;
        }
        fclose(fp);
    }
}

@end
