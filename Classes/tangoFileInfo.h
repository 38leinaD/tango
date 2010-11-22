//
//  tangoFileInfo.h
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "Tango.h"

@class tangoConnection;

@interface tangoFileInfo : NSObject {
	tango_file_info_t _fileInfo;	
}

@property (readonly, nonatomic) tango_file_info_t fileInfo;

+ (tangoFileInfo *)rootFileInfoForConnection:(tangoConnection *)connection;

- (id)initWithFileInfo:(tango_file_info_t *)fileInfo;

- (NSString *)name;
- (NSUInteger)size;
- (BOOL)isFolder;

@end
