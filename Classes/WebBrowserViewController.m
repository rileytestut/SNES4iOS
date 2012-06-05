#import "SNES4iOSAppDelegate.h"
#import "WebBrowserViewController.h"
#import "RomSelectionViewController.h"

@class WebPolicyDecisionListenerPrivate;

@interface WebPolicyDecisionListener : NSObject
{
//    WebPolicyDecisionListenerPrivate *_private;
}
- (id)_initWithTarget:(id)fp8 action:(SEL)fp12;
- (void)dealloc;
- (void)_usePolicy:(int)fp8;
- (void)_invalidate;
- (void)use;
- (void)ignore;
- (void)download;
@end

@interface NSURLDownload : NSObject
{
  
}
- (id)initWithRequest:(NSMutableURLRequest *)request delegate:(id)delegate;
- (NSURLRequest *)request;
- (void)setDestination:(NSString *)path allowOverwrite:(BOOL)allowOverwrite;
@end

@implementation NSObject (MyNSObject)

#if 0
- (id)NSURLDownload_initWithRequest:(NSMutableURLRequest *)request delegate:(id)delegate
{
  NSLog(@"FTW!!!!!!!!!!!!!!!!!!!");
  NSString* filename = [[[request URL] path] lastPathComponent];
  //if([filename hasSuffix:@".zip"] || [filename hasSuffix:@".smc"] || [filename hasSuffix:@".swc"] || [filename hasSuffix:@".zip"] || [filename hasSuffix:@".bin"])
  if(filename != nil)
  {
    [request setURL:[NSString stringWithFormat:@"%@/%@", AppDelegate().romDirectoryPath, filename]];
  }
  
}
- (void)WebDownloadInternal_downloadDidBegin:(NSURLDownload *)download
{
  NSString* filename = [[[[download request] URL] path] lastPathComponent];
  //if([filename hasSuffix:@".zip"] || [filename hasSuffix:@".smc"] || [filename hasSuffix:@".swc"] || [filename hasSuffix:@".zip"] || [filename hasSuffix:@".bin"])
  if(filename != nil)
  {
    [download setDestination:[NSString stringWithFormat:@"%@/%@", AppDelegate().romDirectoryPath, filename] allowOverwrite:YES];
  }
}
#endif


-(void)UIWebView_webView:(id)sender decidePolicyForMIMEType:(NSString*)type request:(NSURLRequest*)request frame:(id)frame decisionListener:(WebPolicyDecisionListener*)listener
{
  //NSMutableURLRequest *urlRequest = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:apiUrl]  cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:30];
  //[urlRequest setValue:[Settings sharedInstance].preferredLanguage forHTTPHeaderField:@"Language"];
  //[urlRequest setValue:@"gzip" forHTTPHeaderField:@"Accept-Encoding"];

  if([type isEqualToString:@"application/zip"] || [type isEqualToString:@"application/x-zip"] || [type isEqualToString:@"application/octet-stream"] || [type isEqualToString:@"multipart/x-zip"])
  {
    [listener ignore];
  	
    [AppDelegate().webViewController startingDownload:request withType:type];
  }
/*
  else
  {
    [self UIWebView_webView:sender decidePolicyForMIMEType:type request:request frame:frame decisionListener:listener];
  }
*/
}

@end

#import <objc/runtime.h>
#import <objc/message.h>

#define SetNSError(ERROR_VAR, FORMAT,...)  \
  if (ERROR_VAR) {  \
    NSString *errStr = [@"error:]: " stringByAppendingFormat:FORMAT,##__VA_ARGS__];  \
    *ERROR_VAR = [NSError errorWithDomain:@"NSCocoaErrorDomain" \
                     code:-1  \
                   userInfo:[NSDictionary dictionaryWithObject:errStr forKey:NSLocalizedDescriptionKey]]; \
  }

@implementation NSObject (doWork)

+ (BOOL)changeMethod:(SEL)origSel_ withMethod:(SEL)altSel_ error:(NSError**)error_ {

#if OBJC_API_VERSION >= 2
  Method origMethod = class_getInstanceMethod(self, origSel_);
  if (!origMethod) {
    NSLog(@"Couldn't find orig method %@ %@", NSStringFromSelector(origSel_), [self class]);
    SetNSError(error_, @"method %@ not found for class %@", NSStringFromSelector(origSel_), [self class]);
    return NO;
  }

  Method altMethod = class_getInstanceMethod(self, altSel_);
  if (!altMethod) {
    NSLog(@"Couldn't find alt method %@ %@", NSStringFromSelector(altSel_), [self class]);
    SetNSError(error_, @"alt method %@ not found for class %@", NSStringFromSelector(altSel_), [self class]);
    return NO;
  }

  class_addMethod(self,
          origSel_,
          class_getMethodImplementation(self, origSel_),
          method_getTypeEncoding(origMethod));
  class_addMethod(self,
          altSel_,
          class_getMethodImplementation(self, altSel_),
          method_getTypeEncoding(altMethod));

  method_exchangeImplementations(class_getInstanceMethod(self, origSel_), class_getInstanceMethod(self, altSel_));
  return YES;
#else
  //  Scan for non-inherited methods.
  Method directOriginalMethod = NULL, directAlternateMethod = NULL;

  void *iterator = NULL;
  struct objc_method_list *mlist = class_copyMethodList(self, &iterator);
  while (mlist) {
    int method_index = 0;
    for (; method_index < mlist->method_count; method_index++) {
      if (mlist->method_list[method_index].method_name == origSel_) {
        assert(!directOriginalMethod);
        directOriginalMethod = &mlist->method_list[method_index];
      }
      if (mlist->method_list[method_index].method_name == altSel_) {
        assert(!directAlternateMethod);
        directAlternateMethod = &mlist->method_list[method_index];
      }
    }
    free(mlist);
    mlist = class_copyMethodList(self, &iterator);
  }

  //  If either method is inherited, copy it up to the target class to make it non-inherited.
  if (!directOriginalMethod || !directAlternateMethod) {
    Method inheritedOriginalMethod = NULL, inheritedAlternateMethod = NULL;
    if (!directOriginalMethod) {
      inheritedOriginalMethod = class_getInstanceMethod(self, origSel_);
      if (!inheritedOriginalMethod) {
        SetNSError(error_, @"method %@ not found for class %@", NSStringFromSelector(origSel_), [self className]);
        return NO;
      }
    }
    if (!directAlternateMethod) {
      inheritedAlternateMethod = class_getInstanceMethod(self, altSel_);
      if (!inheritedAlternateMethod) {
        SetNSError(error_, @"alt method %@ not found for class %@", NSStringFromSelector(altSel_), [self className]);
        return NO;
      }
    }

    int hoisted_method_count = !directOriginalMethod && !directAlternateMethod ? 2 : 1;
    struct objc_method_list *hoisted_method_list = malloc(sizeof(struct objc_method_list) + (sizeof(struct objc_method)*(hoisted_method_count-1)));
    hoisted_method_list->method_count = hoisted_method_count;
    Method hoisted_method = hoisted_method_list->method_list;

    if (!directOriginalMethod) {
      bcopy(inheritedOriginalMethod, hoisted_method, sizeof(struct objc_method));
      directOriginalMethod = hoisted_method++;
    }
    if (!directAlternateMethod) {
      bcopy(inheritedAlternateMethod, hoisted_method, sizeof(struct objc_method));
      directAlternateMethod = hoisted_method;
    }
    class_addMethod(self, hoisted_method_list);
  }

  //  zle.
  IMP temp = directOriginalMethod->method_imp;
  directOriginalMethod->method_imp = directAlternateMethod->method_imp;
  directAlternateMethod->method_imp = temp;

  return YES;
#endif
}



+ (BOOL)changeMethodStatic:(SEL)origSel_ withMethod:(SEL)altSel_ error:(NSError**)error_ {


  Method origMethod = class_getClassMethod(self, origSel_);
  if (!origMethod) {
  //NSLog(@"Couldn't find orig method");
    SetNSError(error_, @"method %@ not found for class %@", NSStringFromSelector(origSel_), [self class]);
    return NO;
  }

  Method altMethod = class_getClassMethod(self, altSel_);
  if (!altMethod) {
  //NSLog(@"Couldn't find alt method");
    SetNSError(error_, @"alt method %@ not found for class %@", NSStringFromSelector(altSel_), [self class]);
    return NO;
  }

  class_addMethod(self,
          origSel_,
          class_getMethodImplementation(self, origSel_),
          method_getTypeEncoding(origMethod));
  class_addMethod(self,
          altSel_,
          class_getMethodImplementation(self, altSel_),
          method_getTypeEncoding(altMethod));

  method_exchangeImplementations(class_getClassMethod(self, origSel_), class_getClassMethod(self, altSel_));
  return YES;

}
@end

@implementation WebBrowserViewController


- (void) viewDidLoad
{  isDownloading = 0;
	self.navigationItem.hidesBackButton = YES;
	
	/*UIBarButtonItem *moreButton = [[UIBarButtonItem alloc] initWithTitle:@"Reload" style:UIBarButtonItemStylePlain target:self action:@selector(reloadButtonClicked)];
	 self.navigationItem.rightBarButtonItem = moreButton;
	 [moreButton release];*/
	NSLog(@"Swizzling policy delegate");
	[NSClassFromString(@"WebDefaultPolicyDelegate") changeMethod: @selector(webView:decidePolicyForMIMEType:request:frame:decisionListener:) withMethod: @selector(UIWebView_webView:decidePolicyForMIMEType:request:frame:decisionListener:) error:nil];
	
	[webView setScalesPageToFit: YES];
    
	webView.delegate = self;
	
	UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"ArrowBack.png"]
																   style:UIBarButtonItemStylePlain
																  target:webView action:@selector(goBack)];
    self.navigationItem.leftBarButtonItem = backButton;
    self.navigationItem.hidesBackButton = YES;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        UIBarButtonItem *forwardButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"ArrowForward.png"]
                                                                          style:UIBarButtonItemStylePlain
                                                                         target:webView action:@selector(goForward)];
        self.navigationItem.rightBarButtonItem = forwardButton;
    }
    else {
        UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(dismissWebController)];
        self.navigationItem.rightBarButtonItem = doneButton;
    }
	
	
	self.contentSizeForViewInPopover = CGSizeMake(600, 700);
	[self loadBaseURL];
}

- (void)dismissWebController {
    UIViewController *parentViewController = [self parentViewController];
    if ([self respondsToSelector:@selector(presentingViewController)]) {
        parentViewController = [self presentingViewController];//Fixes iOS 5 bug
    }
    [parentViewController dismissModalViewControllerAnimated:YES];
}

- (void)webViewDidFinishLoad:(UIWebView *)aWebView
{
    NSString* title = [aWebView stringByEvaluatingJavaScriptFromString: @"document.title"];
    self.navigationItem.title = title;
	
	self.navigationItem.leftBarButtonItem.enabled = aWebView.canGoBack;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
        self.navigationItem.rightBarButtonItem.enabled = aWebView.canGoForward;
    }
}


- (void)loadBaseURL
{
  NSString *urlAddress = @"http://www.coolrom.com/roms/snes/";

  //[NSClassFromString(@"WebDownload") changeMethod: @selector(initWithRequest:delegate:) withMethod: @selector(NSURLDownload_initWithRequest:delegate:) error:nil];
  
  //Create a URL object.
  NSURL *url = [NSURL URLWithString:urlAddress];

  //URL Requst Object
  NSURLRequest *requestObj = [NSURLRequest requestWithURL:url];
    


  //Load the request in the UIWebView.
	NSLog(@"loading URL: %@", url);
  [webView loadRequest:requestObj];
}

- (void)didReceiveMemoryWarning 
{
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

-(void)startingDownload:(NSURLRequest*)request withType:(NSString*)type
{
	NSLog(@"Starting Download with request: %@", request);
  downloadRequest = [request copy];
  downloadType = [type copy];
  NSString* alertMessage = [NSString stringWithFormat:@"%@%@", @"Please confirm that you own and\nare downloading this file legally:\n", [[[request URL] path] lastPathComponent]];
	UIAlertView* downloadAlertView=[[UIAlertView alloc] initWithTitle:nil
                    message:alertMessage
										delegate:self cancelButtonTitle:nil
                    otherButtonTitles:@"DENY",@"CONFIRM",nil];
	[downloadAlertView show];
}

-(void)startDownload
{
  @autoreleasepool {
    [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
    
	NSLog(@"Downloading...");
    //[NSThread detachNewThreadSelector:@selector(updatingDownload) toTarget:SOApp.webBrowserView withObject:nil];
    	
    NSURLResponse *urlResponse = [[NSURLResponse alloc] init];

    NSError *error;
    //NSLog(@" *********** REQUESTING CATALOG CHECK FROM: %@", apiUrl);
    NSData *returnData = [NSURLConnection sendSynchronousRequest:downloadRequest returningResponse:&urlResponse error:&error];

    if(returnData != nil)
    {
      NSString* fileName = [urlResponse suggestedFilename];
      if(![fileName hasSuffix:@".zip"] && ![fileName hasSuffix:@".smc"] && ![fileName hasSuffix:@".swc"] && ![fileName hasSuffix:@".zip"] && ![fileName hasSuffix:@".bin"])
      {
        if([downloadType isEqualToString:@"application/zip"] || [downloadType isEqualToString:@"application/x-zip"])
        {
          [fileName stringByAppendingPathExtension:@"zip"];
        }
        else
        {
          [fileName stringByAppendingPathExtension:@"bin"];
        }
      }
      [returnData writeToFile:[NSString stringWithFormat:@"%@/%@", AppDelegate().romDirectoryPath, fileName] atomically:NO];
    
      [AppDelegate().romSelectionViewController scanRomDirectory:AppDelegate().romDirectoryPath];
    }

    isDownloading = 0;
    
    [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
    [downloadWaitAlertView dismissWithClickedButtonIndex:0 animated:YES];
      [self dismissWebController];
  }
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
  if(buttonIndex == 1)
  {
    // Confirmed
  	downloadWaitAlertView=[[UIAlertView alloc] initWithTitle:nil
                      message:@"Downloading now.\nThis prompt will close when the download is done.\n\n"
  										delegate:self cancelButtonTitle:nil
                      otherButtonTitles:nil];
  	UIActivityIndicatorView* indiView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
  	indiView.center=CGPointMake(145, 115);
  	[downloadWaitAlertView addSubview:indiView];
  	[indiView startAnimating];
  	[downloadWaitAlertView show];
    [NSThread detachNewThreadSelector:@selector(startDownload) toTarget:AppDelegate().webViewController withObject:nil];
  }
}


@end

