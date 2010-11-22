//
//  tangoConnection.h
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "Tango.h"

@class tangoFileInfo;

@interface tangoConnection : NSObject {
	NSString *_username;
	NSString *_password;
	NSString *_share;
	
	tango_connection_t *_connection;
}

@property (readonly, copy) NSString *username;
@property (readonly, copy) NSString *password;
@property (readonly, copy) NSString *share;

@property (readonly, assign) tango_connection_t *connection;


- (id)initWithUsername:(NSString *)username password:(NSString *)password share:(NSString *)share;

- (BOOL)connect;
- (void)disconnect;

- (NSArray *)listDirectory:(tangoFileInfo *)fileInfo;

- (BOOL)error;
- (NSString *)errorMessage;

@end
