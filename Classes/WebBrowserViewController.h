// This class is lifted verbatim from snes4iphone v.6.1.3 by ZodTTD
// Pretty neat stuff, really.  I like how he swizzles the web kit policy decision method

#import <UIKit/UIKit.h>


@interface WebBrowserViewController : UIViewController < UIAlertViewDelegate, UIWebViewDelegate>
{
	IBOutlet UIWebView*	      webView;
  UIAlertView*              downloadWaitAlertView;
  int                       isDownloading;
  NSURLRequest*             downloadRequest;
  NSString*                 downloadType;  
}


-(void)loadBaseURL;
-(void)startingDownload:(NSURLRequest*)request withType:(NSString*)type;
-(void)startDownload;
- (void)dismissWebController;

@end
