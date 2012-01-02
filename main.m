//
//  main.m
//  SNES4iPad
//
//  Created by Yusef Napora on 5/10/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SNES4iOSAppDelegate.h"

int main(int argc, char *argv[]) {
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([SNES4iOSAppDelegate class]));
        return retVal;
    
}
