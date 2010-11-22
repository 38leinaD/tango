//
//  tangoLoginViewController.h
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface tangoLoginViewController : UIViewController {
	IBOutlet UIButton *loginButton;
	IBOutlet UITextField *usernameTextfield;
	IBOutlet UITextField *passwordTextfield;
	IBOutlet UITextField *shareTextfield;
	IBOutlet UIActivityIndicatorView *activityIndicator;
}

- (IBAction)loginButtonPressed:(id)target;

@end
