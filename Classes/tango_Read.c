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
int _tango_READ(tango_connection_t *connection, unsigned int offset, unsigned int bytes, tango_file_info_t *file_info, unsigned char *buffer) {
	
	unsigned int operation_successful = -1;
	
    /*
    if ((connection->server_capabilities & CAP_LARGE_READX) == 0 && (bytes & 0xffff0000) > 0) {
        error("_tango_READ(): Calling read for %04x bytes even though server does not support CAP_LARGE_READX\n", bytes);
    }
    */
    
	debug("_tango_READ(): Read FID(%04x) OFFSET(%04x) BYTES(%04x)\n", file_info->fid, offset, bytes);
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_READ_ANDX);
	
    unsigned char *buffer_ptr = buffer;
    unsigned int bytes_read = 0;
    unsigned int bytes_still_to_read = bytes;
    unsigned int current_file_offset = offset;

    while (bytes_still_to_read > 0) {
        debug("_tango_READ(): Doing a READ_AND_X for DataLength %04x and Offset %04x\n", bytes_still_to_read, current_file_offset);

        // Always read in 0xff00 chunks so smb_response buffer is not overflown; (size is constant HEADER + 0xffff)
        unsigned short bytes_now_to_read = bytes_still_to_read > 0xff00 ? 0xff00 : bytes_still_to_read;
        
        // Parameters
        unsigned char *parameters_ptr = _tango_smb_getParametersPointer(smb);
        unsigned int parameters_offset = 0;
        
        // AndX Command
        *((unsigned char *)(parameters_ptr + parameters_offset)) = SMB_COM_NONE;
        parameters_offset+=2; // 1 byte reserved
        
        // AndX Offset
        *((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=2; // 1 byte reserved
        
        // Fid
        *((unsigned short *)(parameters_ptr + parameters_offset)) = file_info->fid;
        parameters_offset+=2;
        
        // Offset
        *((unsigned int *)(parameters_ptr + parameters_offset)) = current_file_offset;
        parameters_offset+=4;
        
        // MaxCount
        *((unsigned short *)(parameters_ptr + parameters_offset)) = bytes_now_to_read;
        parameters_offset+=2;
        
        // MinCount
        *((unsigned short *)(parameters_ptr + parameters_offset)) = bytes_now_to_read;
        parameters_offset+=2;
        
        // MaxCountHigh
        /*
        unsigned short max_count_high = 0;
        if (connection->server_capabilities & CAP_LARGE_READX) {
            max_count_high = *(((unsigned short *)&bytes_still_to_read) + 1);
        }
        *((unsigned int *)(parameters_ptr + parameters_offset)) = max_count_high;
        parameters_offset+=4;
        */
        
        // Remaining
        *((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=2;
        
        // OffsetHigh
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
        debug("_tango_READ(): Sending READ:\n");
        debug("-----------------------------------------------------------------------------\n");
        _tango_print_message(smb);
        debug("-----------------------------------------------------------------------------\n");
    #endif
        
        if (!_tango_send_and_receive(connection, smb, NULL)) {
            goto bailout;
        }
            
        debug("_tango_READ(): Received response\n");
            
    #ifdef VERY_VERBOSE
        debug("_tango_READ(): Received response:\n");
        debug("-----------------------------------------------------------------------------\n");
        _tango_print_message_header(smb);
        debug("-----------------------------------------------------------------------------\n");
    #endif
        
        /**
         * 3. Evaluate Response
         */
            
        if (!_tango_evaluate_response_header(connection, smb)) {
            goto bailout;
        }
            
        if (_tango_smb_getParametersSize(smb) != 24) {
            _tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Read failed");
            error("_tango_READ(): Parameters-block length %d (!= 24).\n", (int)_tango_smb_getParametersSize(smb));
            goto bailout;
        }
        
        // Parameters
        parameters_ptr = _tango_smb_getParametersPointer(smb);
        parameters_offset = 0;
        
        // AndXCommand
        parameters_offset++;
        
        // AndXReserved
        parameters_offset++;
        
        // AndXOffset
        parameters_offset+=2;

        // Remaining
        parameters_offset+=2;
        
        // DataCompactionMode
        parameters_offset+=2;

        // Reserved
        parameters_offset+=2;

        // DataLength
        unsigned int data_length = *((unsigned short *)(parameters_ptr + parameters_offset));
        parameters_offset+=2;

        debug("_tango_READ(): Received data has length: %04x\n", data_length);
        
        // DataOffset
        unsigned short payload_data_offset = *((unsigned short *)(parameters_ptr + parameters_offset));
        parameters_offset+=2;

        // DataLengthHigh
        /*
        if (connection->server_capabilities & CAP_LARGE_READX) {
            unsigned short data_length_high = *((unsigned short *)(parameters_ptr + parameters_offset));
            data_length |= (unsigned int)(data_length_high) << 16;
        }
        */
        parameters_offset+=2;
        
        data_ptr = _tango_smb_getDataPointer(smb);
        data_offset = 0;
        
        //data_offset++; // Pad[]
        
        memcpy(buffer_ptr, ((unsigned char *)(smb->data_ptr + payload_data_offset)), data_length);
        buffer_ptr += data_length;
        
        bytes_still_to_read -= data_length;
        current_file_offset += data_length;
        bytes_read += data_length;
        
    }
        
    debug("_tango_READ(): Overall read: %04x bytes.\n", bytes_read);
	
	operation_successful = bytes_read;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
