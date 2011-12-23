    //
//  EmulationViewController.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/14/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "SNES4iPadAppDelegate.h"
#import "EmulationViewController.h"
#import "ScreenView.h"

#import <pthread.h>
#import <QuartzCore/QuartzCore.h>


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
	[pngData writeToFile:[NSString stringWithCString:filepath] atomically:YES];
	
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
	
}


- (void) refreshScreen
{
    //[(ScreenView *) self.view update];
    [self.view setNeedsDisplay];
	//[self.view.layer setNeedsDisplay];
}

- (void) startWithRom:(NSString *)romFile
{
    pthread_create(&main_tid, NULL, threadedStart, (void *) [[romFile lastPathComponent] UTF8String]);
	
	struct sched_param    param;
    param.sched_priority = 46;
    if(pthread_setschedparam(main_tid, SCHED_OTHER, &param) != 0)
    {
		fprintf(stderr, "Error setting pthread priority\n");
    }
}



- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft ||
			interfaceOrientation == UIInterfaceOrientationLandscapeRight);
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


