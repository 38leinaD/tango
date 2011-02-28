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
	/*
	//TangoTestViewController *controller = [[TangoTestViewController alloc] initWithNibName:@"TangoTestApp" bundle:nil];
	UINavigationController *navigationController = [[UINavigationController alloc]
													initWithRootViewController:rootViewController];
	
	[window addSubview:navigationController.view];
	[window makeKeyAndVisible];
	*/
	
	
	tango_connection_t *connection = tango_create("\\\\172.16.116.129\\folder", "Administrator", "wn");
    //tango_connection_t *connection = tango_create("\\\\192.168.1.3\\folder", "Administrator", "wn");

	tango_connect(connection);
	
	tango_file_info_t root_file_info;
	tango_create_root_file_info(connection, &root_file_info);

    // Reading Test
    /*
	tango_file_info_t file_info_arr[256];
	
	int file_count = tango_list_directory(connection, &root_file_info, file_info_arr, 256);
	
	printf("File count: %d\n", file_count);
	printf("---------------------\n");
	if (file_count > 0) {
		for (int i=0; i<file_count; i++) {
			printf(" Name: %s; size: %d bytes; folder %d:\n", file_info_arr[i].filename, file_info_arr[i].file_size, file_info_arr[i].is_folder);
		}
	}

	unsigned char *file_buffer = malloc(file_info_arr[1].file_size);
	int bytes_read = 0;
	if ((bytes_read = tango_read_file(connection, &file_info_arr[1], 0, file_info_arr[1].file_size, file_buffer)) < 0) {
		printf("Error reading file\n");
	}
	
	_tango_print_bytes(file_buffer, bytes_read);
	*/
    
    
    // Write Test
    /*
    tango_file_info_t file_info;
    tango_create_file_info(connection, &root_file_info, &file_info, "helloworld.txt", 0);
    
    const char *hello_world = "HELLO WORLD123";
    
    int result = tango_write_file(connection, &file_info, 0, strlen(hello_world), hello_world);
    
    if (result < 0) {
		printf("Error writing file\n");
	}
     */
    
    // Echo Test
    /*
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
