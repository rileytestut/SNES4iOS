    //
//  EmulationViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/14/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SNES4iOSAppDelegate.h"
#import "EmulationViewController.h"
#import "ScreenView.h"
#import "MTStatusBarOverlay.h"
#import "SNESControllerViewController.h"
#import "ScreenLayer.h"

#import <pthread.h>
#import <QuartzCore/QuartzCore.h>

#define kSavedState @"savedState"

#define RADIANS(degrees) ((degrees * M_PI) / 180.0)


volatile int __emulation_run;
volatile int __emulation_saving;
volatile int __emulation_paused;

extern int iphone_main(char *filename);


pthread_t main_tid;


// C wrapper function for emulation core access
void refreshScreenSurface()
{
	[AppDelegate().emulationViewController performSelectorOnMainThread:@selector(refreshScreen) withObject:nil waitUntilDone:NO];
}

// entry point for emulator thread
void *threadedStart(void *arg)
{
	@autoreleasepool {
		char *filename = malloc(strlen((char *)arg) + 1);
    strcpy(filename, (char *)arg);
		printf("Starting emulator for %s\n", filename);
		__emulation_run = 1;
		iphone_main(filename);
		__emulation_run = 0;
		__emulation_saving = 0;
		
    free(filename);
	}
}



extern unsigned short *vrambuffer;  // this holds the 256x224 framebuffer in L565 format

void convertBufferToARGB(unsigned int *dest, unsigned short *source, int w, int h)
{
    int x, y;
    // convert to ARGB
    for (y=0; y < h; y++) {
        for (x=0; x < w; x++) {
            unsigned int index = (y*w)+x;
            unsigned short source_pixel = source[index];  
            unsigned char r = (source_pixel & 0xf800) >> 11;
            unsigned char g = (source_pixel & 0x07c0) >> 5;
            unsigned char b = (source_pixel & 0x003f);
            dest[index] = 0xff000000 | 
                                      (((r << 3) | (r >> 2)) << 16) | 
                                      (((g << 2) | (g >> 4)) << 8)  | 
                                      ((b << 3) | (b >> 2));
        }
    }
    
}

// helper function to save a snapshot of the current framebuffer contents
void saveScreenshotToFile(char *filepath)
{
    NSLog(@"writing screenshot to %s", filepath);
    int width = 256;
    int height = 224;
    
    unsigned int *argb_buffer = (unsigned int *)malloc(width * height * 4);
    convertBufferToARGB(argb_buffer, vrambuffer, width, height);
    
    // make data provider from buffer
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, argb_buffer, (width * height * 4), NULL);

    // set up for CGImage creation
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4 * width;
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little |  kCGImageAlphaNoneSkipFirst;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);

    UIImage *uiImage = [[UIImage alloc] initWithCGImage:imageRef];
	
	NSData *pngData = UIImagePNGRepresentation(uiImage);
	[pngData writeToFile:[NSString stringWithCString:filepath encoding:NSUTF8StringEncoding] atomically:YES];
	
	CGImageRelease(imageRef);
    free(argb_buffer);

}

@implementation EmulationViewController

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


- (void)loadView {
	self.view = (UIView *)[[ScreenView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    
    if (ControllerAppDelegate().controllerType == SNESControllerTypeLocal) {
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:@"UIDeviceOrientationDidChangeNotification" object:nil];
    }
    
    /*if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        UIButton *exitButton = [UIButton buttonWithType:UIButtonTypeCustom];
        exitButton.frame = CGRectMake(0, 0, 100, 100);
        [exitButton addTarget:self action:@selector(exit:) forControlEvents:UIControlEventTouchUpInside];
        [self.view addSubview:exitButton];
        
        UIButton *loadStateButton = [UIButton buttonWithType:UIButtonTypeCustom];
        loadStateButton.frame = CGRectMake(self.view.bounds.size.width - 100, 0, 100, 100);
        [loadStateButton addTarget:self action:@selector(loadState:) forControlEvents:UIControlEventTouchUpInside];
        [self.view addSubview:loadStateButton];
        
        UIButton *saveNewStateButton = [UIButton buttonWithType:UIButtonTypeCustom];
        saveNewStateButton.frame = CGRectMake(0, self.view.bounds.size.height - 100, 100, 100);
        saveNewStateButton.tag = 1;
        [saveNewStateButton addTarget:self action:@selector(saveState:) forControlEvents:UIControlEventTouchUpInside];
        [self.view addSubview:saveNewStateButton];
        
        UIButton *saveStateButton = [UIButton buttonWithType:UIButtonTypeCustom];
        saveStateButton.frame = CGRectMake(self.view.bounds.size.width - 100, self.view.bounds.size.height - 100, 100, 100);
        saveStateButton.tag = 2;
        [saveStateButton addTarget:self action:@selector(saveState:) forControlEvents:UIControlEventTouchUpInside];
        [self.view addSubview:saveStateButton];
    }*/
    //Above methods were a replacement to the double-tapping to access these options, which sometimes crashes the app. Convering over to GCD appears to have fixed this crash, but I've left these here just in case.
}

- (void)viewWillAppear:(BOOL)animated {
    [self didRotate:[NSNull null]];
}

#pragma mark - Save States

- (void)saveState:(id)sender {
    UIButton *button = (UIButton *)sender;
    __emulation_saving = button.tag;
    double delayInSeconds = 0.1;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
        NSString *message = @"Saved state!";
        if (button.tag == 1) {
            message = @"Saved new state!";
        }
        [[MTStatusBarOverlay threadSafeSharedOverlay] setStatusBarStyleManually:UIStatusBarStyleBlackOpaque];
        [[MTStatusBarOverlay threadSafeSharedOverlay] postMessage:message key:@"savedState" type:MTMessageTypePlain duration:1.0 animated:YES immediate:YES];
    });
}

- (void)loadState:(id)sender {
    
}

#pragma mark -

- (void)exit:(id)sender {
    __emulation_run = 0;
    [AppDelegate() showEmulator:NO];
}

- (void) refreshScreen
{
    //[(ScreenView *) self.view update];
    [self.view setNeedsDisplay];
	//[self.view.layer setNeedsDisplay];
}

- (void) startWithRom:(NSString *)romFile
{
    
    /*
    pthread_create(&main_tid, NULL, threadedStart, (void *) [[romFile lastPathComponent] UTF8String]);
	
	struct sched_param    param;
    param.sched_priority = 46;
    if(pthread_setschedparam(main_tid, SCHED_OTHER, &param) != 0)
    {
		fprintf(stderr, "Error setting pthread priority\n");
    }*/
    
    dispatch_queue_t dispatchQueue = dispatch_queue_create("EmulationThread", DISPATCH_QUEUE_CONCURRENT);
    dispatch_async(dispatchQueue, ^{
        threadedStart((void *) [[romFile lastPathComponent] UTF8String]);
    });
    dispatch_release(dispatchQueue);
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft ||
                interfaceOrientation == UIInterfaceOrientationLandscapeRight);
    }
    else {
        return (interfaceOrientation == UIInterfaceOrientationPortrait);
    }
}

- (void) didRotate:(NSNotification *)notification {
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    if (orientation == UIDeviceOrientationPortrait || orientation == UIDeviceOrientationLandscapeLeft || 
        orientation == UIDeviceOrientationLandscapeRight) {
        CGFloat rotationAngle = 0.0f;
        //These coordinates take into considerationt the fact that the UIWindow is in portrait mode
        if (orientation == UIDeviceOrientationPortrait) {
            if (![AppDelegate().snesControllerViewController.imageName isEqualToString:@"portrait_controller"]) {
                [AppDelegate().snesControllerViewController changeBackgroundImage:@"portrait_controller"];
            }
            rotationAngle = 0.0f;
            self.view.superview.transform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(rotationAngle));
            ((ScreenLayer *)self.view.layer).rotateTransform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(0.0));
            self.view.bounds = CGRectMake(0, 0, 320, 240);
            self.view.superview.bounds = CGRectMake(0, 0, 320, 480);
            self.view.frame = CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height);
            self.view.superview.frame = CGRectMake(0, 0, self.view.superview.frame.size.width, self.view.superview.frame.size.height);
        }
        else if (orientation == UIDeviceOrientationLandscapeLeft) {
            if (![AppDelegate().snesControllerViewController.imageName isEqualToString:@"landscape_controller"]) {
                [AppDelegate().snesControllerViewController changeBackgroundImage:@"landscape_controller"];
            }
            rotationAngle = 0.0f;
            self.view.superview.transform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(rotationAngle));
            ((ScreenLayer *)self.view.layer).rotateTransform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(90.0));
            self.view.bounds = CGRectMake(0, 0, 480, 320);
            self.view.superview.bounds = CGRectMake(0, 0, 320, 480);
            self.view.frame = CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height);
            self.view.superview.frame = CGRectMake(0, 0, self.view.superview.frame.size.width, self.view.superview.frame.size.height);
        }
        else if (orientation == UIDeviceOrientationLandscapeRight) {
            if (![AppDelegate().snesControllerViewController.imageName isEqualToString:@"landscape_controller"]) {
                [AppDelegate().snesControllerViewController changeBackgroundImage:@"landscape_controller"];
            }
            rotationAngle = 180.0f;
            self.view.superview.transform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(rotationAngle));
            ((ScreenLayer *)self.view.layer).rotateTransform = CGAffineTransformRotate(CGAffineTransformIdentity, RADIANS(90.0));
            self.view.bounds = CGRectMake(0, 0, 480, 320);
            self.view.superview.bounds = CGRectMake(0, 0, 320, 480);
            self.view.frame = CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height);
            self.view.superview.frame = CGRectMake(0, 0, self.view.superview.frame.size.width, self.view.superview.frame.size.height);
        }
    }
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}




@end


