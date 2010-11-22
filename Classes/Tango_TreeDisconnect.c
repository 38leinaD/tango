/*
 *  Tango_TreeDisconnect.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_TreeDisconnect.h"

#include "TangoBase.h"

int _tango_TREE_DISCONNECT(tango_connection_t *connection) {
	
	unsigned int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_TREE_DISCONNECT);
	
	_tango_smb_setParametersSize(smb, 0);
	_tango_smb_setDataSize(smb, 0);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_TREE_DISCONNECT_ANDX(): Sending TREE_DISCONNECT:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
	
	debug("_tango_TREE_DISCONNECT_ANDX(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) != 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_TREE_DISCONNECT_ANDX(): Parameters-block length %d (!= 0).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
	if (_tango_smb_getDataSize(smb) != 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_TREE_DISCONNECT_ANDX(): Data-block length %d (!= 0).\n", (int)_tango_smb_getDataSize(smb));
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	printf("_tango_TREE_DISCONNECT_ANDX(): Received response:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	operation_successful = 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
