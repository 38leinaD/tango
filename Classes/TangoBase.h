/*
 *  TangoBase.h
 *	tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGOBASE_H_
#define _TANGOBASE_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#import <CoreFoundation/CoreFoundation.h>

#include "TangoTypes.h"

#pragma mark -
#pragma mark Defines

#define NBT_HEADER_LENGTH	(4)
#define MAX_SMB_SIZE		(1024*16) // 16 Kilobyte

#define SMB_HEADER_PROTOCOL_INT			(0)
#define SMB_HEADER_COMMAND_BYTE			(4)
#define SMB_HEADER_STATUS_INT			(5)
#define SMB_HEADER_FLAGS_BYTE			(9)
#define SMB_HEADER_FLAGS2_SHORT			(10)
#define SMB_HEADER_EXTRA_12BYTE			(12)
#define SMB_HEADER_TID_SHORT			(24)
#define SMB_HEADER_PID_SHORT			(26)
#define SMB_HEADER_UID_SHORT			(28)
#define SMB_HEADER_MID_SHORT			(30)
#define SMB_HEADER_LENGTH				(32)

#pragma mark -
#pragma mark Functions

/**
 * Gets the pointer to the parameters-block for the given smb
 *
 * @param smb Message for which to get the parameter-pointer
 *
 * @return Pointer to pramaters-block
 */
unsigned char *_tango_smb_getParametersPointer(const tango_smb_t *smb);

/**
 * Gets the size to the parameters-block for the given smb
 *
 * @param smb Message for which to get the parameter-size
 *
 * @return Size of parameters-block
 */
size_t _tango_smb_getParametersSize(const tango_smb_t *smb);

/**
 * Set the size of the paremters-block on the smb
 *
 * @param smb Message for which to set the parameter-size
 */
void _tango_smb_setParametersSize(tango_smb_t *smb, size_t size);

// Parameters-size has to be set on SMB for the following functions to work!

/**
 * Gets the pointer to the data-block for the given smb
 *
 * @param smb Message for which to get the data-pointer
 *
 * @return Pointer to data-block
 */
unsigned char *_tango_smb_getDataPointer(const tango_smb_t *smb);

/**
 * Gets the size to the data-block for the given smb
 *
 * @param smb Message for which to get the data-size
 *
 * @return Size of data-block
 */
size_t _tango_smb_getDataSize(const tango_smb_t *smb);

/**
 * Set the size of the data-block on the smb
 *
 * @param smb Message for which to set the data-size
 */
void _tango_smb_setDataSize(tango_smb_t *smb, size_t size);

#pragma mark -
#pragma mark Logging/Debugging

#define debug(...)   fprintf(stderr, __VA_ARGS__)
#define error(...)   fprintf(stderr, __VA_ARGS__)

void _tango_print_bytes(const unsigned char *buffer, size_t length);

void _tango_print_message_header(const tango_smb_t *smb);

void _tango_print_message(const tango_smb_t *smb);

void _tango_print_connection(const tango_connection_t *connection);

#pragma mark -
#pragma mark Getters/Setters for SMB fields

// NT_STATUS

// Level

unsigned short _tango_smb_get_NT_STATUS_Level(tango_smb_t *smb);

// Error-Code

unsigned int _tango_smb_get_NT_STATUS_Error(tango_smb_t *smb);

#pragma mark -
#pragma mark Error Handling

void _tango_set_error(tango_connection_t *connection, int error_number, const char *error_msg);

#pragma mark -
#pragma mark Message (De)allocation

tango_smb_t *_tango_create_smb();

void _tango_release_smb(tango_smb_t *smb);

#pragma mark -
#pragma mark Sending/Receiving

/**
 * Send the current content of connection->buffer (first int is read-out for length) and the respons is again stored in the buffer.
 *
 * @param *connection Connection with attached socket
 * @param *smb_request Storage with SMB to send
 * @param *smb_response Storage for response-SMB; if smb_response is set to NULL, response overrides is written to smb_request
 *
 * @return 1 on success; 0 on failure; Details are provided in connection->error and connection->error_message and can be read via tango_error and smb_error_message respectively.
 */
int _tango_send_and_receive(tango_connection_t *connection, tango_smb_t *smb_request, tango_smb_t *smb_response);

#pragma mark -
#pragma mark Message-Construction and Evaluation

void _tango_populate_request_header(tango_connection_t *connection, tango_smb_t *smb, unsigned char command);

int _tango_evaluate_response_header(tango_connection_t *connection, tango_smb_t *smb);

#endif
