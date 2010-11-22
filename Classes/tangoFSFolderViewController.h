//
//  tangoFSFolderViewController.h
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

#include "tangoConnection.h"

@interface tangoFSFolderViewController : UITableViewController {
	tangoConnection *_connection;
	tangoFileInfo *_rootFileInfo;
	NSArray *_fileArray;
	
	UIImage *_fileImage;
	UIImage *_folderImage;
}

- (id)initWithConnection:(tangoConnection *)connection andRootFileInfo:(tangoFileInfo *)fileInfo;

@end
