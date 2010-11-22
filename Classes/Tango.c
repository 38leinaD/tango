/*
 *  Tango.c
 *	tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango.h"

#include "TangoBase.h"
#include "Tango_NegotiateProtocol.h"
#include "Tango_SessionSetup.h"
#include "Tango_TreeConnect.h"
#include "Tango_Find2First.h"
#include "Tango_TreeDisconnect.h"
#include "Tango_LogOff.h"
#include "Tango_Echo.h"
#include "tango_Read.h"

#pragma mark -
#pragma mark - Public API -


#pragma mark Session-Handling

tango_connection_t *tango_create(const char *share, const char* username, const char *password) {
	static unsigned short next_mid = 0;
	
	tango_connection_t *tango_connection_ptr = malloc(sizeof(tango_connection_t));
	if (tango_connection_ptr == NULL) {
		_tango_set_error(tango_connection_ptr, kTangoErrorGeneralSystemError, "tango_create(): Unable to malloc mem.\n");
		error("tango_create(): Unable to malloc mem.\n");
		return NULL;
	}
	
	// Null it
	memset(tango_connection_ptr, 0, sizeof(tango_connection_t));
	
	// Create a socket
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		_tango_set_error(tango_connection_ptr, kTangoErrorGeneralSystemError, "tango_create(): Failed to create socket().\n");
		debug("tango_create(): Failed to create socket().\n");
		goto bailout;
	}
		
	tango_connection_ptr->socket = sock;
	struct sockaddr_in sock_addr;
	
	// Parse share-string
	if (strncmp(share, "\\\\", 2) != 0 || strlen(share) < 3) {
		_tango_set_error(tango_connection_ptr, kTangoErrorParameterInvalid, "tango_create(): Passed parameter not a valid share.\n");
		debug("tango_create(): Passed parameter not a valid share.\n");
		goto bailout;
	}
	
	const char *begin_ptr = share + 2;
	char *end_ptr = strchr(begin_ptr, '\\');
	

	// Form: \\hostname\share
	char hostname[64];
	assert(strlen(begin_ptr) < 64 && "Hostname longer than 64 bytes");
	strncpy(hostname, begin_ptr, end_ptr - begin_ptr);
	hostname[end_ptr - begin_ptr] = '\0';
	int sin_addr_set = inet_pton(AF_INET, hostname, &(sock_addr.sin_addr));
	
	if (!sin_addr_set) {
		_tango_set_error(tango_connection_ptr, kTangoErrorParameterInvalid, "tango_create(): Invalid share.\n");
		error("tango_create(): Passed parameter not a valid share or contains no valid hostname/IP.\n");
		goto bailout;
	}

	
	char *slash_ptr = strchr(share + 2, '\\');
	slash_ptr = strchr(slash_ptr + 1, '\\');
	if (slash_ptr != NULL) {
		// Format: \\hostname\share\subfolder
		unsigned int slash_idx = slash_ptr - share;
		strncpy(tango_connection_ptr->share, share, slash_idx);
		tango_connection_ptr->share[slash_idx + 1] = '\0';	
	}
	else {
		// Format: \\hostname\share
		slash_ptr = strchr(share + 2, '\\');
		strcpy(tango_connection_ptr->share, share);
	}

	// Configure port and connection-type
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(445); // Default-port for SMB over TCP/IP without NetBios

	tango_connection_ptr->sock_addr = sock_addr;
	
	// Set our Ids
	tango_connection_ptr->pid = 0x1234;
	tango_connection_ptr->mid = next_mid++;
	
	// Store username and password
	strcpy(tango_connection_ptr->user_name, username);
	strcpy(tango_connection_ptr->user_password, password);

	return tango_connection_ptr;
	
bailout:
	free(tango_connection_ptr);
	return NULL;
}

void tango_release(tango_connection_t *connection) {
	free(connection);
}

int tango_connect(tango_connection_t *connection) {
	// Connect
	int result = connect(connection->socket, (struct sockaddr *)&connection->sock_addr, sizeof(struct sockaddr_in));
	if(result < 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Failed to connect");
		error("tango_connect(): Failed to connect(); %s.\n", strerror( errno ));	
		return -1;
	}
	
	if (_tango_NEGOTIATE_PROTOCOL(connection) < 0) return 0;	
	if (_tango_SESSION_SETUP(connection) < 0) return 0;
	if (_tango_TREE_CONNECT(connection) < 0) return 0;
	
	return 1;
}

void tango_close(tango_connection_t *connection) {
	
	if (connection->session_status >= kTangoSessionStatusConnectedToShare) {
		_tango_TREE_DISCONNECT(connection);
		connection->session_status = kTangoSessionStatusLoggedIn;
		connection->tid = 0x00;
	}
	
	if (connection->session_status >= kTangoSessionStatusLoggedIn) {
		_tango_LOGOFF(connection);
		connection->session_status = kTangoSessionStatusDisconnected;
		connection->uid = 0x00;
		connection->session_flags = 0x00;
	}
	
	close(connection->socket);
	connection->socket = 0;
}

int tango_test_connection(tango_connection_t *connection) {
	if(connection->session_status <= kTangoSessionStatusProtocolNegotiated) {
		_tango_set_error(connection, kTangoErrorGeneralSystemError, "Not connected yet.");
		error("tango_test_connection(): No tango_connect() has been successfully called yet.");	
		return -1;
	}
	
	return _tango_ECHO(connection, 2, 'F');
}

void tango_create_root_file_info(tango_connection_t *connection, tango_file_info_t *root_file_info) {
	//root_file_info->connection = connection;
	root_file_info->file_size = 0;
	root_file_info->is_folder = 1;
	
	//unsigned int pos_of_last_slash = strrchr(connection->share, '\\') - connection->share;
	strcpy(root_file_info->filename, "");  
	strcpy(root_file_info->path, "");
}


int tango_list_directory(tango_connection_t *connection, tango_file_info_t *directory, tango_file_info_t file_info_arr[], unsigned int file_info_arr_size) {
	char search_pattern[256];

	strcpy(search_pattern, directory->path);
	strcpy(search_pattern + strlen(search_pattern), directory->filename);

	strcpy(search_pattern + strlen(search_pattern), "\\*");
	return _tango_TRANS2_FIND_FIRST2(connection, search_pattern, file_info_arr, file_info_arr_size);
}
 
int tango_read_file(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, unsigned char *buffer) {
	return _tango_READ(connection, file_info, offset, bytes, buffer);
}

#pragma mark Error-Handling

/**
 * Returns the most recent error (type tango_error) that has occured in one of the operations.
 *  Note that the error is cleared after this call; i.e. it is reset to kTangoErrorNone
 *
 * @param *connection Connection to query for the current error
 *
 * @return The most recent error
 */
int tango_error(tango_connection_t *connection) {
	int error = connection->error;
	connection->error = kTangoErrorNone;
	
	return error;
}

/**
 * Returns the textual error-message for the most recent erroneous operation.
 *
 * @param *connection Connection to query for the error-message
 *
 * @return The error-message or NULL
 */
const char *tango_error_message(tango_connection_t *connection) {
	return connection->error_message;
}