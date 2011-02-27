//
//  tangoLoginViewController.m
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "tangoLoginViewController.h"

#import "tangoFSFolderViewController.h"
#import "tangoFileInfo.h"

@implementation tangoLoginViewController

// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)releaseSubviews {
	[loginButton release], loginButton = nil;
	[usernameTextfield release], usernameTextfield = nil;
	[passwordTextfield release], passwordTextfield = nil;
	[shareTextfield release], shareTextfield = nil;
	[activityIndicator release], activityIndicator = nil;
}

- (void)viewDidUnload {
    [super viewDidUnload];
    
	[self releaseSubviews];
}


- (void)dealloc {
    [super dealloc];
	
	[self releaseSubviews];
}

- (IBAction)loginButtonPressed:(id)target {
	NSLog(@"pressed");
	
	if ([usernameTextfield.text isEqualToString:@""] || [passwordTextfield.text isEqualToString:@""]) {
		UIAlertView *myAlert = [[UIAlertView alloc]
								initWithTitle:@"Error" message:@"Username or password not set"
								delegate:nil 
								cancelButtonTitle:nil
								otherButtonTitles:@"OK", nil];
		[myAlert show]; 
        [myAlert autorelease];
		return;
	}
	
	if (![[shareTextfield.text substringToIndex:2] isEqualToString:@"\\\\"]) {
		UIAlertView *myAlert = [[UIAlertView alloc]
								initWithTitle:@"Error" message:@"Share is not valid\n"
																"Format: \\\\hostname\\share"
								delegate:nil 
								cancelButtonTitle:nil
								otherButtonTitles:@"OK", nil];
		[myAlert show]; 
        [myAlert autorelease];
		return;
	}
	
	[activityIndicator startAnimating];
	
	// Connect to share
	tangoConnection *connection = [[tangoConnection alloc] initWithUsername:usernameTextfield.text
																   password:passwordTextfield.text
																	  share:shareTextfield.text];
	
	if (![connection connect]) {
		UIAlertView *myAlert = [[UIAlertView alloc]
								initWithTitle:@"Error"
								message: [connection errorMessage]
								delegate:nil 
								cancelButtonTitle:nil
								otherButtonTitles:@"OK", nil];
		[myAlert show]; 
        [myAlert autorelease];
		[activityIndicator stopAnimating];
		return;
	}
	
	// Connected
	tangoFileInfo *rootFileInfo = [tangoFileInfo rootFileInfoForConnection:connection];
	tangoFSFolderViewController *folderController = [[tangoFSFolderViewController alloc] initWithConnection:connection andRootFileInfo:rootFileInfo];
	[connection release];
	
	[self.navigationController pushViewController:folderController animated:YES];
}

@end
