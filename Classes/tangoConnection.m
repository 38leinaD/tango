//
//  tangoConnection.m
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "tangoConnection.h"

#import "tangoFileInfo.h"

@implementation tangoConnection

@synthesize username = _username;
@synthesize password = _password;
@synthesize share = _share;

@synthesize connection = _connection;

- (id)initWithUsername:(NSString *)username password:(NSString *)password share:(NSString *)share {
	self = [super init];
	if (self != nil) {
		_username = [username copy];
		_password = [password copy];
		_share = [share copy];
		
		_connection = tango_create([share cStringUsingEncoding:NSUnicodeStringEncoding],
                                   [username cStringUsingEncoding:NSUnicodeStringEncoding],
                                   [password cStringUsingEncoding:NSUnicodeStringEncoding]);
	}
	return self;
}

- (void) dealloc
{
	[_username release], _username = nil;
	[_password release], _password = nil;
	[_share release], _share = nil;
	
	tango_release(_connection);
	
	[super dealloc];
}

- (BOOL)connect {
	return tango_connect(_connection) > 0;
}

- (void)disconnect {
	tango_close(_connection);
}

- (NSArray *)listDirectory:(tangoFileInfo *)fileInfo {
	tango_file_info_t file_info_arr[256];
	tango_file_info_t folder_file_info = fileInfo.fileInfo;
	int file_count = tango_list_directory(_connection, &folder_file_info, file_info_arr, 256);
	
	NSMutableArray *fileArray = [NSMutableArray array];
	
	if (file_count > 0) {
		for (int i=0; i<file_count; i++) {
			tangoFileInfo *fileInfo = [[tangoFileInfo alloc] initWithFileInfo:&file_info_arr[i]];
			[fileArray addObject:fileInfo];
			[fileInfo release];
		}
	}
	
	return fileArray;
}

- (NSData *)readFile:(tangoFileInfo *)fileInfo {
    unsigned char *data = malloc(fileInfo.fileInfo.file_size);
    
    if (data == NULL) {
        return nil;
    }
    
	int bytes_read = 0;
    
    tango_file_info_t fi = fileInfo.fileInfo;
	if ((bytes_read = tango_read_file(self.connection, &fi, 0, fileInfo.fileInfo.file_size, data)) < 0) {
        return nil;
	}
                                                                                                           
    NSData *fileData = [NSData dataWithBytes:data length:bytes_read];
    free(data);
    
    return fileData;
}

- (BOOL)error {
	return tango_error(_connection) > 0;
}

- (NSString *)errorMessage {
	return [NSString stringWithCString:tango_error_message(_connection)
                              encoding:NSUnicodeStringEncoding];
}


@end
