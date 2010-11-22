//
//  tangoFSFolderViewController.m
//  tango
//
//  Created by Daniel Platz on 11/22/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "tangoFSFolderViewController.h"

#import "tangoFileInfo.h"

@implementation tangoFSFolderViewController


#pragma mark -
#pragma mark Initialization


- (id)initWithConnection:(tangoConnection *)connection andRootFileInfo:(tangoFileInfo *)fileInfo {
    self = [super initWithStyle:UITableViewStylePlain];
	if (self) {
		_connection = [connection retain];
		_fileImage = [fileInfo retain];
		
		_folderImage = [[UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"folder" ofType:@"png"]] retain];
		_fileImage = [[UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"file" ofType:@"png"]] retain];
		
		_rootFileInfo = [fileInfo retain];
	}
    return self;
}

#pragma mark -
#pragma mark View lifecycle


- (void)viewDidLoad {
    [super viewDidLoad];

	_fileArray = [[_connection listDirectory:_rootFileInfo] retain];
}

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}
*/
/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/


#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [_fileArray count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
    
	tangoFileInfo *fileInfo = [_fileArray objectAtIndex:indexPath.row];
   	cell.textLabel.text = [fileInfo name];

	if ([fileInfo isFolder]) {
		cell.detailTextLabel.text = @"folder";
		cell.imageView.image = _folderImage;
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	}
	else {
		cell.detailTextLabel.text = [NSString stringWithFormat:@"Size: %d Bytes", [fileInfo size]];
		cell.imageView.image = _fileImage;
		cell.accessoryType = UITableViewCellAccessoryNone;
	}
		
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
*/


/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/


#pragma mark -
#pragma mark Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	tangoFileInfo *fileInfo = [_fileArray objectAtIndex:indexPath.row];
	
	if ([fileInfo isFolder]) {
		
		tangoFSFolderViewController *folderController = [[tangoFSFolderViewController alloc] initWithConnection:_connection andRootFileInfo:fileInfo];
		[self.navigationController pushViewController:folderController animated:YES];
		[folderController release];
	}
	else {
		[tableView deselectRowAtIndexPath:indexPath animated:NO];
	}
}


#pragma mark -
#pragma mark Memory management

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Relinquish ownership any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
    // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
    // For example: self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end

