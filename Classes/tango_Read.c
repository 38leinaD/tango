/*
 *  Tango_Read.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_Read.h"

#include "TangoBase.h"

/**
 *
 * @return -1 on error; number of read bytes on success
 */
int _tango_READ(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, unsigned char *buffer) {
	
	unsigned int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_ECHO);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// AndX Command
	*((unsigned char *)(parameters_ptr + parameters_offset)) = SMB_COM_NONE;
	parameters_offset+=2; // 1 byte reserved
	
	// AndX Offset
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=2; // 1 byte reserved
	
	
	
	_tango_smb_setParametersSize(smb, parameters_offset);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;

	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_READ(): Sending READ:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
		
	debug("_tango_READ(): Received response\n");
		
	/**
	 * 3. Evaluate Response
	 */
		
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
		
	if (_tango_smb_getParametersSize(smb) != 2) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_READ(): Parameters-block length %d (!= 2).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
		
		
#ifdef VERY_VERBOSE
	printf("_tango_READ(): Received response:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
		
	data_ptr = _tango_smb_getDataPointer(smb);
	data_offset = 0;
	
	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
