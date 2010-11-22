/*
 *  Tango_TreeConnect.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_TreeConnect.h"

#include "TangoBase.h"

int _tango_TREE_CONNECT(tango_connection_t *connection) {
	
	unsigned int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	// Header
	_tango_populate_request_header(connection, smb, SMB_COM_TREE_CONNECT_ANDX);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// AndX Command
	*((unsigned char *)(parameters_ptr + parameters_offset)) = SMB_COM_NONE;
	parameters_offset+=2; // 1 byte reserved
	
	// AndX Offset
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=2;
	
	// Flags
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=2;
	
	// PasswordLength;
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 1;
	parameters_offset+=2;
	
	_tango_smb_setParametersSize(smb, parameters_offset);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;

	//msg_data.byte_count = 1 + strlen(connection->share) + 1 + 6;
	
	// Password
	strcpy((char *)(data_ptr + data_offset), "");
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	// Path
	strcpy((char *)(data_ptr + data_offset), connection->share);
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	// Service
	strcpy((char *)(data_ptr + data_offset), "?????");
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_TREE_CONNECT_ANDX(): Sending TREE_CONNECT_ANDX:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}

	debug("_tango_TREE_CONNECT_ANDX(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) != 6) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_TREE_CONNECT_ANDX(): Parameters-block length %d (!= 6).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	printf("_tango_TREE_CONNECT_ANDX(): Received response:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	connection->session_status = kTangoSessionStatusConnectedToShare;
	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
