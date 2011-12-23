//
//  MTStatusBarTableViewCell.h
//
//  Created by Riley Testut on 3/20/11.
//  Copyright 2011 Riley Testut. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MTStatusBarOverlay.h"

@interface MTStatusBarTableViewCell : UITableViewCell {
    id __unsafe_unretained uuid;
    NSInteger messageType;
    UIActivityIndicatorView *activityIndicator;
    UILabel *progressLabel;
}
@property (nonatomic, unsafe_unretained) id uuid;
@property (nonatomic, strong) UIActivityIndicatorView *activityIndicator;
@property (nonatomic, strong) UILabel *progressLabel;
@property (nonatomic) NSInteger messageType;

@end
