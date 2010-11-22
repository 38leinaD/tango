//
//  tangoTestAppDelegate.m
//  tango
//
//  Created by Daniel Platz on 6/12/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "tangoTestAppDelegate.h"

#import "tangoLoginViewController.h"
#import "Tango.h"

@implementation tangoTestAppDelegate

@synthesize window;


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
	
    // Override point for customization after application launch
	
	//TangoTestViewController *controller = [[TangoTestViewController alloc] initWithNibName:@"TangoTestApp" bundle:nil];
	UINavigationController *navigationController = [[UINavigationController alloc]
													initWithRootViewController:rootViewController];
	
	[window addSubview:navigationController.view];
	[window makeKeyAndVisible];
	
	
	/*
	tango_connection_t *connection = tango_create("\\\\172.16.116.129\\folder\\afolder", "Administrator", "wn");
	tango_connect(connection);
	
	tango_file_info_t file_info_arr[256];
	
	int file_count = tango_list_current_directory(connection, file_info_arr, 256);
	
	printf("File count: %d\n", file_count);
	printf("---------------------\n", file_count);
	if (file_count > 0) {
		for (int i=0; i<file_count; i++) {
			printf(" Name: %s; size: %d bytes; folder %d:\n", file_info_arr[i].filename, file_info_arr[i].file_size, file_info_arr[i].is_folder);
		}
	}

	unsigned char *file_buffer = malloc(file_info_arr[2].file_size);
	
	if (tango_read_file(connection, &file_info_arr[2], 0, file_info_arr[2].file_size, file_buffer) < 0) {
		printf("Error reading file\n");
	}
	
	_tango_print_bytes(file_buffer, file_info_arr[2].file_size);
	
	if (tango_test_connection(connection) <= 0) printf("Echo failed\n");
	
	tango_close(connection);
	tango_release(connection);
	*/
	
	return YES;
}


- (void)dealloc {
    [window release];
    [super dealloc];
}


@end
