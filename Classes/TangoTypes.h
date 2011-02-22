/*
 *  TangoTypes.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGOTYPES_H_
#define _TANGOTYPES_H_

#include <netinet/in.h>
#include <arpa/inet.h>

#pragma mark System Message Block Type

typedef struct _tango_smb_t {
	unsigned char *data_ptr;
	size_t size;
} tango_smb_t;


#pragma mark Commands

#define SMB_COM_NEGOTIATE				(0x72)
#define SMB_COM_SESSION_SETUP_ANDX		(0x73)
#define SMB_COM_LOGOFF_ANDX				(0x74)
#define SMB_COM_TREE_DISCONNECT			(0x71)
#define SMB_COM_TREE_CONNECT_ANDX		(0x75)
#define SMB_COM_TRANSACTION2			(0x32)
#define SMB_COM_TRANSACTION2_SECONDARY	(0x33)
#define SMB_COM_NT_CREATE_ANDX			(0xA2)
#define SMB_COM_OPEN_ANDX				(0x2D)
#define SMB_COM_READ_ANDX				(0x2E)
#define SMB_COM_ECHO					(0x2B)
#define SMB_COM_NONE					(0xFF)

#pragma mark SubCommands Transcation 2
#define TRANS2_FIND_FIRST2	(0x01)

#pragma mark NT_STATUS Levels

#define SUCCESS	0x00
#define INFORMATION 0x01
#define WARNING 0x02
#define ERROR	0x03

#pragma mark Flags

#define SMB_FLAGS_CASELESS_PATHNAMES	0x08	// Pathes are treated as caseless (like on windows system)
#define SMB_FLAGS_SERVER_TO_REDIR		0x80	// Is this a reply from the server?
// All others are obsolte...

#pragma mark Flags2

#define SMB_FLAGS2_UNICODE_STRINGS		0x8000	// Strings encoded as unicode?
#define SMB_FLAGS2_32BIT_STATUS			0x4000	// Use NT_STATUS
#define SMB_FLAGS2_READ_IF_EXECUTE		0x2000	// Does execute-permission also grant read?
#define SMB_FLAGS2_DFS_PATHNAME			0x1000	// Client knows about distributed file systems?
#define SMB_FLAGS2_EXTENDED_SECURITY	0x0800	// Client understand extened security features?

#define SMB_FLAGS2_IS_LONG_NAME			0x0040	// Long filenames are supported (otherwise, 8.3 names are only supported)

#define SMB_FLAGS2_SECURITY_SIGNATURE	0x0004	// Is MAC included in signature
#define	SMB_FLAGS2_EAS					0x0002	// Extened attributes are supported?
#define SMB_FLAGS2_KNOWS_LONG_NAMES		0x0001	// Long filenames in response are acceptable (why distinction?)

#pragma mark NEGOTIATE_PROTOCOL_RESPONSE Flags

#pragma mark SecurityMode

#define NEGOTIATE_SECURITY_SIGNATURES_REQUIRED	0x08
#define NEGOTIATE_SECURITY_SIGNATURES_ENABLED	0x04
#define NEGOTIATE_SECURITY_CHALLENGE_RESPONSE	0x02
#define NEGOTIATE_SECURITY_USER_LEVEL			0x01

#pragma mark Capabilities

#define CAP_EXTENDED_SECURITY		0x80000000
#define CAP_COMPRESSED_DATA			0x40000000
#define CAP_BULK_TRANSFER			0x20000000
#define CAP_UNIX					0x00800000
#define CAP_LARGE_WRITEX			0x00008000
#define CAP_LARGE_READX				0x00004000
#define CAP_INFOLEVEL_PASSTHROUGH	0x00002000
#define CAP_DFS						0x00001000
#define CAP_NT_FIND					0x00000200
#define CAP_LOCK_AND_READ			0x00000100
#define CAP_LEVEL_II_OPLOCKS		0x00000080
#define CAP_STATUS32				0x00000040
#define CAP_RPC_REMOTE_APIS			0x00000020
#define CAP_NT_SMBS					0x00000010
#define CAP_LARGE_FILES				0x00000008
#define CAP_UNICODE					0x00000004
#define CAP_MPX_MODE				0x00000002
#define CAP_RAW_MODE				0x00000001

#pragma mark Search Information Levels

#define	SMB_INFO_STANDARD			0x01

#pragma mark File Access

#define FILE_NO_SHARE				0x00000000
#define FILE_SHARE_READ				0x00000001
#define FILE_SHARE_WRITE			0x00000002
#define FILE_SHARE_DELETE			0x00000004

#define FILE_SUPERSEDE				0x00000000
#define FILE_OPEN					0x00000001
#define FILE_CREATE					0x00000002
#define FILE_OPEN_IF				0x00000003
#define FILE_OVERWRITE				0x00000004
#define FILE_OVERWRITE_IF			0x00000005

typedef struct { 
	unsigned int LowPart;
	int HighPart;
} LARGE_INTEGER;

#pragma mark --- Tango ---

#pragma mark Types

typedef enum {
	FLAG_UNSET = 0,
	FLAG_SET = 1,
} tango_flag_t;

#pragma mark Errors

enum {
	kTangoErrorNone,
	kTangoErrorParameterInvalid,
	kTangoErrorGeneralSystemError,
	kTangoErrorConnectionProblem,
	kTangoErrorCryptoError,
	kTangoErrorInvalidResponseMessage,
	kTangoErrorUnsupported,
	kTangoErrorSMBError,
};

#pragma mark ---
#pragma mark Connection types

enum {
	kTangoSessionStatusDisconnected,
	kTangoSessionStatusProtocolNegotiated,
	kTangoSessionStatusLoggedIn,
	kTangoSessionStatusConnectedToShare
};

enum {
	kTangoSessionFlagLoggedInAnonymously = 1
};

typedef struct _tango_connection {
	// Network-Connection
	struct sockaddr_in sock_addr;
	int socket;
	
	// Client Properties
	unsigned short pid; // Process Id (assigned by client)
	unsigned short mid; // Multiplex Id (assigned by client; unique per connection)
	
	unsigned char session_status;
	unsigned char session_flags;
	
	char share[128]; // Path of the share
	char user_name[32]; // Username
	char user_password[32]; // Password
	
	// Server Properties
	unsigned short tid; // Current Tree Id (assigned by server)
	unsigned short uid; // User Id (assigned by server)
	unsigned int max_buffer_size; // Negotiated maxium message-size (min(server, client))
	unsigned char sec_challenge[8]; // Challenge in the challenge/response security
	unsigned int session_key;
	unsigned char extended_security;
	
	// Error-Handling
	int error;
	char error_message[128];	
} tango_connection_t;

typedef struct _tango_file_info {
	//tango_connection_t *connection;
	
	char path[128];
	char filename[128];

	unsigned char is_folder;
	unsigned int file_size;
	
	unsigned short fid;
} tango_file_info_t;

#endif