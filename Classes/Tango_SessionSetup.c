/*
 *  Tango_SessionSetup.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_SessionSetup.h"

#include "TangoBase.h"
#include "TangoSecurity.h"

void _tango_print_SESSION_SETUP_RESPONSE(const tango_smb_t *smb) {
	_tango_print_message_header(smb);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	
	printf("@PARAMETERS = \n");
	
	/*
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
	*/
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	
	printf("@DATA = \n");
	
	_tango_print_bytes(data_ptr, _tango_smb_getDataSize(smb));
}

int _tango_SESSION_SETUP(tango_connection_t *connection) {
	
	int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	// Header
	_tango_populate_request_header(connection, smb, SMB_COM_SESSION_SETUP_ANDX);
	
	/*
	 * Calculate response for LM-challengde/response security
	 */
	unsigned char sec_response[24];
	if (!_tango_sec_lm_challenge_response_encrypt(connection->user_password, connection->sec_challenge, sec_response)) {
		_tango_set_error(connection, kTangoErrorCryptoError, "Encryption error");
		debug("_tango_SESSION_SETUP(): Error while executing cryptographic functions.\n");
		goto bailout;
	};
	
#ifdef VERY_VERBOSE	
	printf("_tango_SESSION_SETUP(): LM-Challenge is:\n");
	_tango_print_bytes(connection->sec_challenge, 8);
	
	printf("_tango_SESSION_SETUP(): LM-Response is:\n");
	_tango_print_bytes(sec_response, 24);
#endif
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	
	// AndX Command
	*((unsigned char *)(parameters_ptr + parameters_offset)) = SMB_COM_NONE;
	parameters_offset+=2; // 1 byte reserved
	
	// AndX Offset
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=2;
	
	// MaxBufferSize
	*((unsigned short *)(parameters_ptr + parameters_offset)) = MAX_SMB_SIZE;
	parameters_offset+=2;
	
	// MaxMpxCount
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 2;
	parameters_offset+=2;
	
	// VcNumber
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 1300;
	parameters_offset+=2;
	
	// SessionKey
	*((unsigned int *)(parameters_ptr + parameters_offset)) = connection->session_key;
	parameters_offset+=4;
	
	// CaseInsensitivePasswordLength (used for response of challenge/response security)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 24;
	parameters_offset+=2;
	
	// CaseSensitivePasswordLength
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=2;
	
	// Reserved
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=4;
	
	// Capabilities
	*((unsigned int *)(parameters_ptr + parameters_offset)) = CAP_EXTENDED_SECURITY | CAP_STATUS32 | CAP_RAW_MODE;
	parameters_offset+=4;	
	
	_tango_smb_setParametersSize(smb, parameters_offset);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;

	// LM-Security response
	memcpy(data_ptr + data_offset, sec_response, 24);
	data_offset += 24;
	
	// AccountName
	strcpy((char *)(data_ptr + data_offset), connection->user_name);
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	// PrimaryDomain
	strcpy((char *)(data_ptr + data_offset), "?");
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	// NativeOS
	strcpy((char *)((data_ptr + data_offset)), "iOS");
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	// NativeLanMan
	strcpy((char *)((data_ptr + data_offset)), "tango");
	data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	
	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_SESSION_SETUP(): Sending SESSION SETUP request:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
	
	debug("_tango_SESSION_SETUP(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to login");
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) != 6) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Unable to login");
		error("_tango_SESSION_SETUP(): Parameters-block length %d (!= 6).\n", (int)_tango_smb_getParametersSize(smb));
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	printf("_tango_SESSION_SETUP(): Received response for SESSION_SETUP_REQUEST:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	parameters_ptr = _tango_smb_getParametersPointer(smb);
	
	// Skip AndX struct
	parameters_ptr += 4;
	
	// Action (bit 0 tells if logged in with requested User)
	unsigned short action = *((unsigned short *)parameters_ptr);
	
	if (action & 0x01) {
		debug("_tango_SESSION_SETUP(): Only logged in anonymously.\n");
		connection->session_flags |= kTangoSessionFlagLoggedInAnonymously;
	}
	else {
		debug("_tango_SESSION_SETUP(): Logged in successfully.\n");
		connection->session_flags ^= kTangoSessionFlagLoggedInAnonymously;
	}
	
	connection->session_status = kTangoSessionStatusLoggedIn;
	operation_successful = 1;

bailout:
	_tango_release_smb(smb);
	return operation_successful;
}

