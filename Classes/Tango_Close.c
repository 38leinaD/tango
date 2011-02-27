/*
 *  Tango_Close.h
 *  tango
 *
 * Copyright (C) 2011 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_Close.h"

#include "TangoBase.h"

int _tango_Close(tango_connection_t *connection, tango_file_info_t *file_info) {
	
	int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_CLOSE);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// FID
	*((unsigned short *)(parameters_ptr + parameters_offset)) = file_info->fid;
	parameters_offset+=2;
    
    // Modify-Time
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=4;
	
	_tango_smb_setParametersSize(smb, parameters_offset);
		
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;
	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	debug("_tango_Close(): Sending Close:\n");
	debug("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	debug("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
	
	debug("_tango_Close(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) != 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to close");
		error("_tango_Close(): Parameters-block length %d (!= 0).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
	if (_tango_smb_getDataSize(smb) != 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to close");
		error("_tango_Close(): Data-block length %d (!= 0).\n", (int)_tango_smb_getDataSize(smb));
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	debug("_tango_Close(): Received response:\n");
	debug("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	debug("-----------------------------------------------------------------------------\n");
#endif
	
	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
