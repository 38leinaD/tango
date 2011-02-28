/*
 *  Tango_Write.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "Tango_WRITE.h"

#include "TangoBase.h"

/**
 *
 * @return -1 on error; number of written bytes on success
 */
int _tango_WRITE(tango_connection_t *connection, tango_file_info_t *file_info, const unsigned char *buffer, unsigned int bytes, unsigned int offset) {
	
	unsigned int operation_successful = -1;

	debug("_tango_WRITE(): Write to FID(%04x) BYTES(%04x) at OFFSET(%04x) \n", file_info->fid, bytes, offset);
	
	/**
	 * 1. Define Request
	 */
	
	tango_smb_t *smb = _tango_create_smb();
	
	_tango_populate_request_header(connection, smb, SMB_COM_WRITE_ANDX);
	
    unsigned char *buffer_ptr = buffer;
    unsigned int bytes_written = 0;
    unsigned int bytes_still_to_write = bytes;
    unsigned int current_file_offset = offset;

    while (bytes_still_to_write > 0) {
        debug("_tango_WRITE(): Doing a WRITE_AND_X for DataLength %04x and Offset %04x\n", bytes_still_to_write, current_file_offset);

        // Always read in 0xff00 chunks so smb_response buffer is not overflown; (size is constant HEADER + 0xffff)
        unsigned short bytes_now_to_write = bytes_still_to_write > 0xff00 ? 0xff00 : bytes_still_to_write;
        
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
        
        // Reserved
        *((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=4;
        
        // WriteMode
        *((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=2;
        
        // Remaining
        *((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=2;
        
        // DataLengthHigh
        *((unsigned short *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=2;
        
        // DataLength
        *((unsigned short *)(parameters_ptr + parameters_offset)) = bytes_now_to_write;
        parameters_offset+=2;
        
        // DataOffset
        unsigned short *data_offset_ptr = (unsigned short *)(parameters_ptr + parameters_offset); // needs to be set later
        parameters_offset+=2;
        
        // OffsetHigh
        *((unsigned int *)(parameters_ptr + parameters_offset)) = 0x00;
        parameters_offset+=4;
        
        _tango_smb_setParametersSize(smb, parameters_offset);
        
        // Data
        unsigned char *data_ptr = _tango_smb_getDataPointer(smb);
        unsigned int data_offset = 0;
        
        if ((data_ptr - smb->data_ptr) % 2 == 1) {
            data_offset++; // Pad to short
        }
        
        memcpy(data_ptr + data_offset, buffer, bytes_now_to_write);
        
        // Set DataOffset in Parameter-List
        *data_offset_ptr = (data_ptr + data_offset) - smb->data_ptr;
        
        data_offset+=bytes_now_to_write;
        
        _tango_smb_setDataSize(smb, data_offset);
        
        /**
         * 2. Send Request and receive Response
         */
        
    #ifdef VERY_VERBOSE
        debug("_tango_WRITE(): Sending WRITE:\n");
        debug("-----------------------------------------------------------------------------\n");
        _tango_print_message(smb);
        debug("-----------------------------------------------------------------------------\n");
    #endif
        
        if (!_tango_send_and_receive(connection, smb, NULL)) {
            goto bailout;
        }
            
        debug("_tango_WRITE(): Received response\n");
            
    #ifdef VERY_VERBOSE
        debug("_tango_WRITE(): Received response:\n");
        debug("-----------------------------------------------------------------------------\n");
        _tango_print_message(smb);
        debug("-----------------------------------------------------------------------------\n");
    #endif
        
        /**
         * 3. Evaluate Response
         */
            
        if (!_tango_evaluate_response_header(connection, smb)) {
            goto bailout;
        }
            
        if (_tango_smb_getParametersSize(smb) != 12) {
            _tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Write failed");
            error("_tango_WRITE(): Parameters-block length %d (!= 12).\n", (int)_tango_smb_getParametersSize(smb));
            goto bailout;
        }
        
        if (_tango_smb_getDataSize(smb) != 0) {
            _tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Write failed");
            error("_tango_WRITE(): Data-block length %d (!= 0).\n", (int)_tango_smb_getDataSize(smb));
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

        // Count
        unsigned short count_of_written_bytes = *((unsigned short *)(parameters_ptr + parameters_offset));
        parameters_offset+=2;
        
        if (count_of_written_bytes != bytes_now_to_write) {
            _tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Write failed");
            error("_tango_WRITE(): Bytes written (%04x) unequal number of bytes requested to write (%04x).\n", count_of_written_bytes, bytes_now_to_write);
            goto bailout;
        }
        
        // Remaining
        parameters_offset+=2;
        
        // Reserved
        parameters_offset+=4;
        
        
        
        bytes_still_to_write -= count_of_written_bytes;
        current_file_offset += count_of_written_bytes;
        bytes_written += count_of_written_bytes;
        
    }
        
    debug("_tango_WRITE(): Overall written: %04x bytes.\n", bytes_written);
	
	operation_successful = bytes_written;
	
bailout:
	_tango_release_smb(smb);
	return operation_successful;
}
