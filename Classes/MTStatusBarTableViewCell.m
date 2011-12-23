//
//  MTStatusBarTableViewCell.m
//
//  Created by Riley Testut on 3/20/11.
//  Copyright 2011 Riley Testut. All rights reserved.
//

#import "MTStatusBarTableViewCell.h"


@implementation MTStatusBarTableViewCell
@synthesize uuid;
@synthesize activityIndicator;
@synthesize progressLabel;
@synthesize messageType;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    if (self = [super initWithStyle:style reuseIdentifier:reuseIdentifier]) {
		self.textLabel.adjustsFontSizeToFitWidth = YES;
		self.selectionStyle = UITableViewCellSelectionStyleNone;
		self.textLabel.lineBreakMode = UILineBreakModeTailTruncation;
        self.progressLabel = [[UILabel alloc] initWithFrame:CGRectMake(226, 2, 44, 21)];
        [self.progressLabel setBackgroundColor:[UIColor clearColor]];
		self.progressLabel.lineBreakMode = UILineBreakModeTailTruncation;
		self.progressLabel.adjustsFontSizeToFitWidth = YES;
        [self.contentView addSubview:progressLabel];
        self.activityIndicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
		self.activityIndicator.hidesWhenStopped = YES;
        [self.contentView addSubview:self.activityIndicator];
    }
    return self;
}

-(void)layoutSubviews {
	[super layoutSubviews];
	self.textLabel.textAlignment = UITextAlignmentCenter;
    self.progressLabel.textAlignment = UITextAlignmentCenter;
    self.textLabel.frame = CGRectMake(35, self.textLabel.frame.origin.y, self.contentView.frame.size.width - 70, self.textLabel.frame.size.height);
    self.activityIndicator.frame = CGRectMake(6.0f, 3.0f, self.contentView.frame.size.height - 6, self.contentView.frame.size.height - 6);
	//self.titleLabel.adjustsFontSizeToFitWidth = YES;
	//self.titleLabel.font = self.textLabel.font;
	/*self.titleLabel.textAlignment = UITextAlignmentRight;
     self.titleLabel.frame = self.textLabel.frame;*/
	//NSLog(@"%@: %f (out of %f)", self.textLabel.text, self.detailTextLabel.frame.size.width, self.contentView.frame.size.width);
}

-(void)prepareForReuse {
    [super prepareForReuse];
    self.textLabel.text = @"";
    self.progressLabel.text = @"";
}
- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    [super setSelected:selected animated:animated];
    
    // Configure the view for the selected state
}

@end
