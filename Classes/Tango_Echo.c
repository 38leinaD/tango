/*
 *  Tango_Echo.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_Echo.h"

#include "TangoBase.h"

int _tango_ECHO(tango_connection_t *connection, unsigned short number_of_echoes, char echoPayload) {
	
	unsigned int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_ECHO);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// Echo count
	*((unsigned short *)(parameters_ptr + parameters_offset)) = number_of_echoes;
	parameters_offset+=2;
		
	_tango_smb_setParametersSize(smb, parameters_offset);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;
	
	*((unsigned char *)(data_ptr + data_offset)) = echoPayload;
	data_offset+=1;
	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_ECHO(): Sending ECHO:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send(connection, smb)) {
		goto bailout;
	}
	
	for (int i=1; i<=number_of_echoes; i++) {
		if (!_tango_receive(connection, smb)) {
			goto bailout;
		}
		
		debug("_tango_ECHO(): Received response %d of %d\n", i, number_of_echoes);
		
		/**
		 * 3. Evaluate Response
		 */
		
		if (!_tango_evaluate_response_header(connection, smb)) {
			goto bailout;
		}
		
		if (_tango_smb_getParametersSize(smb) != 2) {
			_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Operation failed");
			error("_tango_ECHO(): Parameters-block length %d (!= 2).\n", (int)_tango_smb_getParametersSize(smb));
			goto bailout;
		}
		
		
#ifdef VERY_VERBOSE
		printf("_tango_ECHO(): Received response:\n");
		printf("-----------------------------------------------------------------------------\n");
		_tango_print_message(smb);
		printf("-----------------------------------------------------------------------------\n");
#endif
		
		data_ptr = _tango_smb_getDataPointer(smb);
		data_offset = 0;
		
		char echo = *((unsigned char *)(data_ptr + data_offset));
		
		if (echoPayload != echo) {
			_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
			error("_tango_ECHO(): Echo does not echo (%02x) the payload of request (%02x).\n", echo, echoPayload);
			goto bailout;
		}
	}

	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
