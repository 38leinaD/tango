/*
 *  Tango_NegotiateProtocol.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_NegotiateProtocol.h"

#include "TangoBase.h"

#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_DIALECT_INDEX_SHORT			(0)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SECURITY_MODE_BYTE			(2)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_MPX_COUNT_SHORT			(3)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_NUMBER_VCS_SHORT			(5)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_BUFFER_SIZE_INT			(7)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_RAW_SIZE_INT				(11)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SESSION_KEY_INT				(15)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_CAPABILITIES_INT				(19)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SYSTEM_TIME_LOW_INT			(23)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SYSTEM_TIME_HIGH_INT			(27)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SERVER_TIME_ZONE_SHORT		(31)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_ENCRYPTION_KEY_LENGTH_BYTE	(33)
#define SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_LENGTH						(34)

#define SMB_NEGOTIATE_PROTOCOL_DIALECT_LM012							"NT LM 0.12"

void _tango_print_NEGOTIATE_PROTOCOL_RESPONSE(const tango_smb_t *smb) {
	_tango_print_message_header(smb);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	
	printf("@PARAMETERS = \n");
	
	printf("@DialectIndex = 0x%04x\n", *(unsigned short *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_DIALECT_INDEX_SHORT));
	printf("@SecurityMode = 0x%02x\n", *(unsigned char *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SECURITY_MODE_BYTE));
	printf("@MaxMpxCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_MPX_COUNT_SHORT));
	printf("@MaxNumberVCs = 0x%04x\n", *(unsigned short *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_NUMBER_VCS_SHORT));
	printf("@MaxBufferSize = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_BUFFER_SIZE_INT));
	printf("@MaxRawSize = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_MAX_RAW_SIZE_INT));
	printf("@SessionKey = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SESSION_KEY_INT));
	printf("@Capabilities = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_CAPABILITIES_INT));
	printf("@SystemTimeLow = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SYSTEM_TIME_LOW_INT));
	printf("@SystemTimeHigh = 0x%08x\n", *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SYSTEM_TIME_HIGH_INT));
	printf("@ServerTimeZone = 0x%04x\n", *(unsigned short *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SERVER_TIME_ZONE_SHORT));
	printf("@EncryptionKeyLength = 0x%02x\n", *(unsigned char *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_ENCRYPTION_KEY_LENGTH_BYTE));
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	
	printf("@DATA = \n");
	
	_tango_print_bytes(data_ptr, _tango_smb_getDataSize(smb));
}

int _tango_NEGOTIATE_PROTOCOL(tango_connection_t *connection) {
	
	int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	// Header
	_tango_populate_request_header(connection, smb, SMB_COM_NEGOTIATE);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	_tango_smb_setParametersSize(smb, 0);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;
	// Set to LanManager 0.12 for NT
	
	data_ptr[0] = 0x02;
	data_offset++;
	
	strcpy((char *)(data_ptr + data_offset), SMB_NEGOTIATE_PROTOCOL_DIALECT_LM012);
	data_offset += strlen(SMB_NEGOTIATE_PROTOCOL_DIALECT_LM012) + 1;
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_NEGOTIATE_PROTOCOL(): Sending NEGOTIATE_PROTOCOL request:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}

	debug("_tango_NEGOTIATE_PROTOCOL(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	// Read Parameters
	if (_tango_smb_getParametersSize(smb) != 34) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response.\n");
		debug("_tango_NEGOTIATE_PROTOCOL(): Parameters-block length %d (!= 34).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
	/** TODO if no index is supported!
	 Server Response ================
	 UCHAR WordCount; USHORT DialectIndex; USHORT ByteCount;
	 Description ============
	 Count of parameter words = 1 Index of selected dialect Count of data bytes = 0
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_NEGOTIATE_PROTOCOL(): Received response for NEGOTIATE_PROTOCOL_REQUEST:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_NEGOTIATE_PROTOCOL_RESPONSE(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	parameters_ptr = _tango_smb_getParametersPointer(smb);
	
	// params->DialectIndex 
	// We assume he is Ok with our dialect as the NT_STATUS Level was set to SUCCESS
	
	// params->SecurityMode
	// TODO: Handle Sec mode
	
	// params->MaxMpxCount
	// Don't care for now
	
	// params->MaxNumberVCs
	// Don't care for now
	
	//connection->max_buffer_size = params->MaxBufferSize;
	
	// params->MaxRawSize
	// We don't support raw-mode to keep it easy
	
	// params->SessionKey
	connection->session_key = *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_SESSION_KEY_INT);
	
	// params->Capabilities
	unsigned int capabilities = *(unsigned int *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_CAPABILITIES_INT);

	connection->extended_security = capabilities & CAP_EXTENDED_SECURITY;
	if (connection->extended_security) {
		error("_tango_NEGOTIATE_PROTOCOL(): Extended Security unsupported.\n");
		_tango_set_error(connection, kTangoErrorUnsupported, "Unsupported security mode");
		goto bailout;
	}
    
    debug("_tango_NEGOTIATE_PROTOCOL(): Server Capabilities 0x%x\n", capabilities);
    connection->server_capabilities = capabilities;
    if (capabilities & CAP_LARGE_READX) {
        debug("_tango_NEGOTIATE_PROTOCOL(): - CAP_LARGE_READX\n");
    }
	
	//time_t time = (time_t)((((((unsigned long)params->SystemTimeLow) << 32) | params->SystemTimeHigh)/10000000) - 11644473600);
	
	// params->ServerTimeZone
	// Too late... don't care for it now
	
	// params->EncryptionKeyLength
	unsigned char encryption_key_length = *(unsigned char *)(parameters_ptr + SMB_NEGOTIATE_PROTOCOL_RSP_PARAMETERS_ENCRYPTION_KEY_LENGTH_BYTE);
	

	
	// Store challange (no extended security)
	if (encryption_key_length == 0) {
		debug("_tango_NEGOTIATE_PROTOCOL(): No encryption (key-length == 0)\n");
	}
	else if (encryption_key_length == 8) {
		// Ok
	}
	else {
		error("_tango_NEGOTIATE_PROTOCOL(): Unsupported key-length (key-length = %u)\n", encryption_key_length);
		_tango_set_error(connection, kTangoErrorUnsupported, "Unsupported encryption.\n");
	}
	
	// Read Data
	
	data_ptr = _tango_smb_getDataPointer(smb);
	
	if (encryption_key_length == 8) {
		memcpy(&connection->sec_challenge, data_ptr, 8);

	}
	
	operation_successful = 1;
	connection->session_status = kTangoSessionStatusProtocolNegotiated;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}