/*
 *  Tango_Find2First.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_TreeConnect.h"

#include "TangoBase.h"

void _tango_print_TRANS2_FIND_FIRST2_REQUEST(const tango_smb_t *smb) {
	_tango_print_message_header(smb);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned char parameters_offset = 0;
	printf("@PARAMETERS = \n");
	
	printf("@TotalParameterCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@TotalDataCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@MaxParameterCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@MaxDataCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@MaxSetupCount = 0x%02x\n", *(unsigned char *)(parameters_ptr + parameters_offset++));

	// Reserved
	parameters_offset++;
	
	printf("@Flags = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@Timeout = 0x%08x\n", *(unsigned int *)(parameters_ptr + parameters_offset));
	parameters_offset+=4;
	
	// Reserved2
	parameters_offset+=2;
	
	unsigned char parameter_count = *(unsigned char *)(parameters_ptr + parameters_offset);
	printf("@ParameterCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@ParameterOffset = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	unsigned char data_count = *(unsigned char *)(parameters_ptr + parameters_offset);
	printf("@DataCount = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	printf("@DataOffset = 0x%04x\n", *(unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	unsigned char setup_count = *(unsigned char *)(parameters_ptr + parameters_offset);
	printf("@SetupCount = 0x%02x\n", *(unsigned char *)(parameters_ptr + parameters_offset++));

	// Reserved3
	if (parameters_offset % 2 == 1) {
		parameters_offset++;
	}
	
	printf("@Setup = \n");
	_tango_print_bytes(parameters_ptr + parameters_offset, 2*setup_count);

	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned char data_offset = 0;
	printf("@DATA = \n");
	
	printf("@Name = 0x%02x\n", *(unsigned char *)(data_ptr + data_offset++));
	
	// Pad
	if (data_offset % 2 == 1) {
		data_offset++;
	}
	
	printf("@Parameter = \n");
	printf("@SearchAttributes = 0x%04x\n", *(unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	printf("@SearchCount = 0x%04x\n", *(unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	printf("@Flags = 0x%04x\n", *(unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	printf("@InformationLevel = 0x%04x\n", *(unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	printf("@SearchStorageType = 0x%08x\n", *(unsigned int *)(data_ptr + data_offset));
	data_offset+=4;
	printf("@Filename = %s\n", data_ptr + data_offset);
	data_offset = strlen(data_ptr + data_offset) + 1;
	
	printf("@Data = \n");
	_tango_print_bytes(data_ptr + data_offset, data_count);
	data_offset += data_count;
}

int _tango_TRANS2_FIND_FIRST2(tango_connection_t *connection, const char *search_pattern, tango_file_info_t file_info_arr[], unsigned int file_info_arr_size) {

	int operation_successful = -1;
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	// Header
	_tango_populate_request_header(connection, smb, SMB_COM_TRANSACTION2);
	
	// Parameters
	unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
	unsigned int parameters_offset = 0;
	unsigned int parameters_offset_params2 = 0;
	unsigned int parameters_offset_data2 = 0;
	
	// TotalParameterCount (Total parameter bytes being sent)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 13 + strlen(search_pattern) + 1;
	parameters_offset+=2;	
	
	// TotalDataCount (Total data bytes being sent)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=2;
	
	// MaxParameterCount (Max parameter bytes to return)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x000A; // just a guess...
	parameters_offset+=2;
	
	// MaxDataCount (Max data bytes to return)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0xFF00;
	parameters_offset+=2;
	
	// MaxSetupCount (Max setup words to return)
	*((unsigned char *)(parameters_ptr + parameters_offset)) = 255;
	parameters_offset+=1;
	
	// Reserved
	*((unsigned char *)(parameters_ptr + parameters_offset)) = 0x00;
	parameters_offset+=1;
	
	// Flags
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x0000;
	parameters_offset+=2;
	
	// Timeout
	*((unsigned int *)(parameters_ptr + parameters_offset)) = 0x1234;
	parameters_offset+=4;
	
	// Reserved2
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0x0000;
	parameters_offset+=2;
	
	// ParameterCount (Parameter bytes sent this buffer)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 13 + strlen(search_pattern) + 1;
	parameters_offset+=2;
	
	// ParameterOffset (Offset (from header start) to Parameters)
	// not known yet...
	parameters_offset_params2 = parameters_offset;
	parameters_offset+=2;
	
	// DataCount (Data bytes sent this buffer)
	*((unsigned short *)(parameters_ptr + parameters_offset)) = 0;
	parameters_offset+=2;
	
	// DataOffset (Offset (from header start) to data)
	// not known yet
	parameters_offset_data2 = parameters_offset;
	parameters_offset+=2;
	
	// SetupCount (Count of setup words)
	*((unsigned char *)(parameters_ptr + parameters_offset)) = 1;
	parameters_offset+=1;	
	
	// Reserved3 (Reserved (pad above to word boundary))
	if (parameters_offset % 2 == 1) {
		// Pad to word boundary
		parameters_offset+=1;	
	}
	
	// Setup[SetupCount] (Setup words (# = SetupWordCount))
	*((unsigned short *)(parameters_ptr + parameters_offset)) = TRANS2_FIND_FIRST2;
	parameters_offset+=2;

	_tango_smb_setParametersSize(smb, parameters_offset);
	
	// Data
	unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
	unsigned int data_offset = 0;
	
	// Name[] (Must be NULL)
	*((unsigned char *)(data_ptr + data_offset)) = '\0';
	data_offset+=1;
	
	// Pad[] (Pad to SHORT or LONG)
	if (data_offset % 2 == 1) {
		data_offset+=1;
	}
	
	// Parameters[ParameterCount] (Parameter bytes (# = ParameterCount))
	*((unsigned short *)(parameters_ptr + parameters_offset_params2)) = 
		SMB_HEADER_LENGTH +
		1 + _tango_smb_getParametersSize(smb) + 2 + data_offset;
	
	// SearchAtributes ???
	*((unsigned short *)(data_ptr + data_offset)) = 0xFF;
	data_offset+=2;
	
	// SearchCount (Maximum number of entries to return)
	*((unsigned short *)(data_ptr + data_offset)) = 200; // Max for now...
	data_offset+=2;
	
	// Flags (when to close the search)
	*((unsigned short *)(data_ptr + data_offset)) = 0x00; // = close search
	data_offset+=2;
	
	// Information-Level
	*((unsigned short *)(data_ptr + data_offset)) = SMB_INFO_STANDARD;
	data_offset+=2;
	
	// SearchStorageType ???
	*((unsigned int *)(data_ptr + data_offset)) = 0x00;
	data_offset+=4;
	
	// SearchString
	strcpy((char *)(data_ptr + data_offset), search_pattern);
	data_offset+=strlen(search_pattern) + 1;
	
	// Pad1[] (Pad to SHORT or LONG)
	if (data_offset % 2 == 1) {
		data_offset+=1;
	}
	
	// Data[DataCount] (Data bytes (# = DataCount))
	*((unsigned short *)(parameters_ptr + parameters_offset_data2)) = 
	SMB_HEADER_LENGTH +
	1 + _tango_smb_getParametersSize(smb) + 2 + data_offset;
	
	//*((unsigned char *)buffer_ptr) = 0x00;
	//buffer_ptr+=4;

	_tango_smb_setDataSize(smb, data_offset);
	
	/**
	 * 2. Send Request and receive Response
	 */
	
#ifdef VERY_VERBOSE
	printf("_tango_TRANS2_FIND_FIRST2(): Sending TRANS2_FIND_FIRST2:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_TRANS2_FIND_FIRST2_REQUEST(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
	
	if (!_tango_send_and_receive(connection, smb, NULL)) {
		goto bailout;
	}
	
	debug("_tango_TRANS2_FIND_FIRST2(): Received response\n");
	
	/**
	 * 3. Evaluate Response
	 */
	
	if (!_tango_evaluate_response_header(connection, smb)) {
		goto bailout;
	}
	
	if (_tango_smb_getParametersSize(smb) == 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_TRANS2_FIND_FIRST2(): Parameters-block length ==0.\n");
		goto bailout;
	}
	
	if (_tango_smb_getDataSize(smb) == 0) {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Invalid response");
		error("_tango_TRANS2_FIND_FIRST2(): Data-block length ==0.\n");
		goto bailout;
	}
	
#ifdef VERY_VERBOSE
	printf("_tango_TRANS2_FIND_FIRST2(): Received response:\n");
	printf("-----------------------------------------------------------------------------\n");
	_tango_print_message(smb);
	printf("-----------------------------------------------------------------------------\n");
#endif
		
	// Parameters
	parameters_ptr = _tango_smb_getParametersPointer(smb);
	parameters_offset = 0;
	
	// TotalParameterCount
	unsigned short resp_total_parameter_count = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// TotalDataCount
	unsigned short resp_total_data_count = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// Reserved
	parameters_offset+=2;
	
	// ParameterCount
	unsigned short resp_parameter_count = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// ParameterOffset
	unsigned short resp_parameter_offset = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// ParameterDisplacement
	unsigned short resp_parameter_displacement = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// DataCount
	unsigned short resp_data_count = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// DataOffset
	unsigned short resp_data_offset = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// DataDisplacement
	unsigned short resp_data_displacement = *((unsigned short *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;
	
	// SetupCount
	unsigned char resp_setup_count = *((unsigned char *)(parameters_ptr + parameters_offset));
	parameters_offset+=2;

	if (resp_total_parameter_count != resp_parameter_count) {
		_tango_set_error(connection, kTangoErrorUnsupported, "Unsupported feature");
		error("_tango_TRANS2_FIND_FIRST2(): Not full parameter-block sent in inital response\n");
		goto bailout;
	}
	
	if (resp_total_data_count != resp_data_count) {
		_tango_set_error(connection, kTangoErrorUnsupported, "Unsupported feature");
		error("_tango_TRANS2_FIND_FIRST2(): Not full data-block sent in inital response\n");
		goto bailout;
	}
	
	// Data
	data_ptr = _tango_smb_getDataPointer(smb);
	data_offset = 0;
	
	// Pad
	data_offset++;
	
	// Parameters2
	// Sid
	unsigned short resp_data_sid = *((unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	
	// SearchCount
	unsigned short resp_data_search_count = *((unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	
	// EndOfSearch
	unsigned short resp_data_end_of_search = *((unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	
	// EaErrorOffset
	unsigned short resp_data_ea_error_offset = *((unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	
	// LastNameOffset
	unsigned short resp_data_last_name_offset = *((unsigned short *)(data_ptr + data_offset));
	data_offset+=2;
	
	if (resp_data_end_of_search == 0) {
		// TODO Handle multiple server responses
		error("_tango_TRANS2_FIND_FIRST2(): Not all search-results have been returned. Swallow it for now.\n");
	}
	
	// Pad1
	if (data_offset % 2 == 1) {
		data_offset++;
	}
	
	// Data2
	
	/*
	 * Read file-infos
	 */
	
	int idx = -1;
	for (int i=0; i<(resp_data_search_count < file_info_arr_size ? resp_data_search_count : file_info_arr_size); i++) {
		
		data_offset+=12; // dont care for dates for now
		
		// DataSize
		unsigned int file_size = *((unsigned int *)(data_ptr + data_offset));
		data_offset+=4;
		
		// AllocationSize
		data_offset+=4;
		
		// Attributes
		unsigned short file_attributes = *((unsigned short *)(data_ptr + data_offset));
		data_offset+=2;
		
		// TODO: No idea why a padding is needed here....
		if (i==0) data_offset++;
		
		//FileNameLength
		unsigned char file_name_length = *((unsigned char *)(data_ptr + data_offset));
		data_offset+=1;

		if (strcmp((char *)(data_ptr + data_offset), ".") != 0 && strcmp((char *)(data_ptr + data_offset), "..") != 0) {
			idx++;
			
			//file_info_arr[idx].connection = connection;
			
			// FileName
			strcpy(file_info_arr[idx].filename, (char *)(data_ptr + data_offset));

			// Filesize
			file_info_arr[idx].file_size = file_size;
			
			// Attributes
			file_info_arr[idx].is_folder = file_attributes & 0x0010;
			
			// folder
			unsigned int pos_of_last_slash = strrchr(search_pattern, '\\') - search_pattern;
			strncpy(file_info_arr[idx].path, search_pattern, pos_of_last_slash + 1);
			file_info_arr[idx].path[pos_of_last_slash + 1] = '\0';
			
		}
		
		data_offset += strlen((char *)(data_ptr + data_offset)) + 1;
	}
	
	operation_successful = idx + 1;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}