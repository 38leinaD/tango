//
//  tangoTestAppDelegate.h
//  tango
//
//  Created by Daniel Platz on 6/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface tangoTestAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	
	IBOutlet UIViewController *rootViewController;
	
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end

