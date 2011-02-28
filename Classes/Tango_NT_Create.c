/*
 *  Tango_NT_Create.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_NT_Create.h"

#include "TangoBase.h"

int _tango_NT_Create(tango_connection_t *connection, tango_file_info_t *file_info, tango_open_t open_type, unsigned int create_disposition) {
	
	int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_NT_CREATE_ANDX);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// AndX Command
	*((unsigned char *)(parameters_ptr + parameters_offset)) = SMB_COM_NONE;
	parameters_offset+=2;
	
	// AndX Offset
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=2; // 1 byte reserved
	
	// Reserved
	*((unsigned char *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=1;
	
	// NameLength
	*((unsigned short *)(parameters_ptr + parameters_offset)) = strlen(file_info->path) + strlen(file_info->filename) + 1;
	parameters_offset+=2;
	
	// Flags
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x02; // OpLock
	parameters_offset+=4;
	
	// RootDirectoryFid
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00; // share relative to root
	parameters_offset+=4;
	
	// DesiredAccess
    // See http://msdn.microsoft.com/en-us/library/cc230294(v=prot.10).aspx
    unsigned int access = 0x20000;
    if (open_type & kTangoOpenFileForRead) access |= 0x01;
    if (open_type & kTangoOpenFileForWrite) access |= 0x02;

	*((unsigned int *)(parameters_ptr + parameters_offset)) = access;
	parameters_offset+=4;
	
	// AllocationSize
	LARGE_INTEGER allocationSize = {
		0,0
	};
	*((LARGE_INTEGER *)(parameters_ptr + parameters_offset)) = allocationSize;
	parameters_offset+=sizeof(LARGE_INTEGER);
	
	// ExtFileAttributes
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=4;
	
	// ShareAccess
    unsigned int share_access = 0x00;
    if (open_type & kTangoOpenFileForRead) share_access |= FILE_SHARE_READ;
    if (open_type & kTangoOpenFileForWrite) share_access |= FILE_NO_SHARE;
    
	*((unsigned int *)(parameters_ptr + parameters_offset)) = share_access;
	parameters_offset+=4;
	
	// CreateDisposition
	*((unsigned int *)(parameters_ptr + parameters_offset)) = create_disposition;
	parameters_offset+=4;

	// CreateOptions
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=4;
	
	// ImpersonationLevel
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x02; // SECURITY_IMPERSONATION
	parameters_offset+=4;

	// SecurityFlags
	*((unsigned char *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=1;

	_tango_smb_setParametersSize(smb, parameters_offset);
		
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;
	
	// Share
	strcpy((char *)data_ptr, file_info->path);
	strcpy((char *)data_ptr + strlen(file_info->path), file_info->filename);
	data_offset+=strlen((char *)data_ptr) + 1;
	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	debug("_tango_NT_Create(): Sending NT_Create:\n");
	debug("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	debug("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
	
	debug("_tango_NT_Create(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) == 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to open");
		error("_tango_NT_Create(): Parameters-block length %d (== 0).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
	if (_tango_smb_getDataSize(smb) != 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to open");
		error("_tango_NT_Create(): Data-block length %d (!= 0).\n", (int)_tango_smb_getDataSize(smb));
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	debug("_tango_NT_Create(): Received response:\n");
	debug("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	debug("-----------------------------------------------------------------------------\n");
#endif
	
	// Parameters
	parameters_ptr = _tango_smb_getParametersPointer(smb);
	parameters_offset = 0;
	
	// AndXCommand
	parameters_offset++;
	
	// AndXReserved
	parameters_offset++;
	
	// AndXOffset
	parameters_offset+=2;
	
	// OpLockLevel
	unsigned char oplock_level = *((unsigned char *)(parameters_ptr + parameters_offset));
	parameters_offset++;
	
	if (oplock_level == 0) {
		debug("_tango_NT_Create: No oplock granted\n");
	}
	else {
		debug("_tango_NT_Create: Oplock: %02x\n", oplock_level);
	}

	// FID
	file_info->fid = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	debug("_tango_NT_Create: FID is %04x\n", file_info->fid);
	
	// CreateAction
	parameters_offset+=4;
	
	// CreationTime
	parameters_offset+=8;
	
	// LastAccessTime
	parameters_offset+=8;
	
	// LastWriteTime
	parameters_offset+=8;
	
	// ChangeTime
	parameters_offset+=8;
	
	// ExtFileAttributes
	parameters_offset+=4;
	
	// AllocationSize
	parameters_offset+=sizeof(LARGE_INTEGER);
	
	// EndOfFile
	parameters_offset+=sizeof(LARGE_INTEGER);
	
	// FileType
	parameters_offset+=2;
	
	// DeviceType
	parameters_offset+=2;
	
	// Directory?
	unsigned char directory = *((unsigned char *)(parameters_ptr + parameters_offset));
	parameters_offset++;
	
	if (directory > 0) {
		debug("_tango_NT_Create: This is a directory\n");
	}
	
	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
