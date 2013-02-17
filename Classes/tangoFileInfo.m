//
//  tangoFileInfo.m
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "tangoFileInfo.h"

#import "tangoConnection.h"

@implementation tangoFileInfo

@synthesize fileInfo = _fileInfo;

+ (id)rootFileInfoForConnection:(tangoConnection *)connection {
	tango_file_info_t file_info;
	tango_create_root_file_info(connection.connection, &file_info);
	return [[[tangoFileInfo alloc] initWithFileInfo:&file_info] autorelease];
}

- (id)initWithFileInfo:(tango_file_info_t *)fileInfo {
	self = [super init];
	if (self != nil) {
		_fileInfo = *fileInfo;
	}
	return self;
}

- (NSString *)name {
	return [NSString stringWithUTF8String:_fileInfo.filename];	
}

- (NSUInteger)size {
	return _fileInfo.file_size;	
}

- (BOOL)isFolder {
	return _fileInfo.is_folder > 0;
}

@end
