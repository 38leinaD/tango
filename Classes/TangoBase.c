/*
 *  TangoBase.c
 *	tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "TangoBase.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TangoTypes.h"
#include "TangoSecurity.h"

#pragma mark -
#pragma mark Pointer Calculations

// All size-parameters in bytes!!! No double-bytes like in SMB spec

/**
 * Gets the pointer to the parameters-block for the given smb
 *
 * @param smb Message for which to get the parameter-pointer
 *
 * @return Pointer to pramaters-block
 */
unsigned char *_tango_smb_getParametersPointer(const tango_smb_t *smb) {
	return smb->data_ptr + SMB_HEADER_LENGTH + 1;
}

/**
 * Gets the size to the parameters-block for the given smb
 *
 * @param smb Message for which to get the parameter-size
 *
 * @return Size of parameters-block
 */
size_t _tango_smb_getParametersSize(const tango_smb_t *smb) {
	return *(smb->data_ptr + SMB_HEADER_LENGTH) * 2;
}

/**
 * Set the size of the paremters-block on the smb
 *
 * @param smb Message for which to set the parameter-size
 */
void _tango_smb_setParametersSize(tango_smb_t *smb, size_t size) {
	assert(size % 2 == 0);
	*(smb->data_ptr + SMB_HEADER_LENGTH) = size / 2;
}

// Parameters-size has to be set on SMB for the following functions to work!

/**
 * Gets the pointer to the data-block for the given smb
 *
 * @param smb Message for which to get the data-pointer
 *
 * @return Pointer to data-block
 */
unsigned char *_tango_smb_getDataPointer(const tango_smb_t *smb) {
	return _tango_smb_getParametersPointer(smb) + _tango_smb_getParametersSize(smb) + 2;
}

/**
 * Gets the size to the data-block for the given smb
 *
 * @param smb Message for which to get the data-size
 *
 * @return Size of data-block
 */
size_t _tango_smb_getDataSize(const tango_smb_t *smb) {
	return *((unsigned short *)(_tango_smb_getParametersPointer(smb) + _tango_smb_getParametersSize(smb)));
}

/**
 * Set the size of the data-block on the smb
 *
 * @param smb Message for which to set the data-size
 */
void _tango_smb_setDataSize(tango_smb_t *smb, size_t size) {
	*((unsigned short *)(_tango_smb_getParametersPointer(smb) + _tango_smb_getParametersSize(smb))) = size;
}

#pragma mark Logging/Debugging

#define debug(...)   fprintf(stderr, __VA_ARGS__)
#define error(...)   fprintf(stderr, __VA_ARGS__)

#define VERY_VERBOSE 1

void _tango_print_bytes(const unsigned char *buffer, size_t length) {	
	if (length == 0) {
		printf("Zero Bytes\n");
		return;
	}
	
	if (buffer == NULL) {
		printf("Pointer is NULL\n");
		return;
	}
	
	const unsigned char *ptr = buffer;
	while (ptr < buffer + length) {
		size_t charsToPrintInRow = (buffer + length) - ptr;
		if (charsToPrintInRow > 8) {
			charsToPrintInRow = 8;
		}
		
		printf("%08d |", ptr - buffer);
		
		for (unsigned int i=0; i<charsToPrintInRow; i++) {
			printf(" 0x%02x", *(ptr + i));
		}
		
		for (unsigned int i=0; i<8 - charsToPrintInRow; i++) {
			printf("     ");
		}
		
		printf(" | ");
		
		for (unsigned int i=0; i<charsToPrintInRow; i++) {
			char c = *(ptr + i); 
			if (c < 32 || c > 126) {
				c = '.';
			}
			printf("%c", c);
		}
		
		printf("\n");
		
		ptr += charsToPrintInRow;
	}
}

void _tango_print_message_header(const tango_smb_t *smb) {
	if (smb == NULL) {
		printf("Header is NULL\n");
		return;
	}
	
	const unsigned char *buffer_ptr = smb->data_ptr;
	
	// Protocol Id
	printf("@PROTOCOL = 0x%02x%c%c%c\n", buffer_ptr[0], 
		   buffer_ptr[1],
		   buffer_ptr[2],
		   buffer_ptr[3]);
	buffer_ptr+=4;
	
	// Command
	printf("@COMMAND = 0x%02x\n", *buffer_ptr);
	buffer_ptr++;
	
	// Status
	printf("@STATUS = 0x%08x\n", *((unsigned int *)buffer_ptr));
	buffer_ptr+=4;
	
	// Flags
	printf("@FLAGS = 0x%02x\n", *buffer_ptr);
	buffer_ptr++;
	
	// Flags2
	printf("@FLAGS2 = 0x%04x\n", *((const unsigned short *)buffer_ptr));
	buffer_ptr+=2;
	
	// Extra
	printf("@EXTRA =\n");
	_tango_print_bytes(buffer_ptr, 12);
	buffer_ptr+=12;
	
	// TID
	printf("@TID = 0x%04x\n", *(unsigned short *)buffer_ptr);
	buffer_ptr+=2;
	
	// PID
	printf("@PID = 0x%04x\n", *(unsigned short *)buffer_ptr);
	buffer_ptr+=2;
	
	// UID
	printf("@UID = 0x%04x\n", *(unsigned short *)buffer_ptr);
	buffer_ptr+=2;
	
	// MID
	printf("@MID = 0x%04x\n", *(unsigned short *)buffer_ptr);
	buffer_ptr+=2;
}

void _tango_print_message(const tango_smb_t *smb) {
	_tango_print_message_header(smb);
	
	// Parameters
	printf("@PARAMETERS = \n");
	_tango_print_bytes(_tango_smb_getParametersPointer(smb), _tango_smb_getParametersSize(smb));
	
	// Data
	printf("@DATA = \n");
	_tango_print_bytes(_tango_smb_getDataPointer(smb), _tango_smb_getDataSize(smb));
}

void _tango_print_connection(const tango_connection_t *connection) {
	printf("@CONNECTION = \n");
	
	printf(" @Share = %s\n", connection->share);
	printf(" @Username = %s\n", connection->user_name);
	printf(" @Password = %s\n", connection->user_password);
}

#pragma mark Getters/Setters for SMB fields

// NT_STATUS

// Level

unsigned short _tango_smb_get_NT_STATUS_Level(tango_smb_t *smb) {
	// First two bits are Level; so, extract them.
	return (*(unsigned int *)(smb->data_ptr + SMB_HEADER_STATUS_INT) >> 30) & 0x03;
}

// Error-Code

unsigned int _tango_smb_get_NT_STATUS_Error(tango_smb_t *smb) {
	// First two bits are Level; so, throw them away.
	return ~(0x03 << 30) & *(unsigned int *)(smb->data_ptr + SMB_HEADER_STATUS_INT);
}

#pragma mark Error Handling

void _tango_set_error(tango_connection_t *connection, int error_number, const char *error_msg) {
	
	connection->error = error_number;
	
	if (error_msg != NULL) {
		strcpy(connection->error_message, error_msg);
	}
}

#pragma mark Message (De)allocation

tango_smb_t *_tango_create_smb() {
	tango_smb_t *smb = malloc(sizeof(tango_smb_t));
	
	smb->data_ptr = malloc(NBT_HEADER_LENGTH + MAX_SMB_SIZE);
	smb->data_ptr += NBT_HEADER_LENGTH;
	smb->size = MAX_SMB_SIZE;
	
	return smb;
}

void _tango_release_smb(tango_smb_t *smb) {
	if (smb == NULL) {
		debug("smb to be freed is NULL already");
		return;
	}
	
	if (smb->data_ptr == NULL) {
		debug("smb->data_ptr to be freed is NULL already");
		return;
	}
	
	free(smb->data_ptr - NBT_HEADER_LENGTH);
	free(smb);
}

#pragma mark Sending/Receiving

int _tango_send(tango_connection_t *connection, tango_smb_t *smb_request) {
	unsigned int nbt_length  = 		SMB_HEADER_LENGTH + \
	1 + _tango_smb_getParametersSize(smb_request) +\
	2 + _tango_smb_getDataSize(smb_request);
	
	unsigned int bytes_to_send = NBT_HEADER_LENGTH + nbt_length;
	
	unsigned char *buffer_ptr = smb_request->data_ptr - NBT_HEADER_LENGTH;
	int result;
	
	// Set NBT Header (message-length)
	if (nbt_length > (0x01 << 24)) {
		debug("_tango_send(): Message-size to big for NBT: %d bytes\n", (int)smb_request->size);
		return 0;
	}
	
	debug("_tango_send(): Bytes to send (including nbt-header!): %d\n", bytes_to_send);
	
	*((unsigned int*)buffer_ptr) = htonl(nbt_length);
	
	// Send in loop to make sure everything gets sent
	while ((result = send(connection->socket, buffer_ptr, bytes_to_send, 0)) < bytes_to_send && result > 0) {
		bytes_to_send -= result;
		buffer_ptr += result;
		debug("_tango_send(): Sent %d bytes.\n", result);
	}
	
	if (result < 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
		error( "_tango_send(): Error send()-ing: %s\n", strerror(errno));
		return 0;
	}	
	else {
		debug("_tango_send(): Sent %d bytes.\n", result);
	}
	
	return 1;
}

int _tango_receive(tango_connection_t *connection, tango_smb_t *smb_response) {
	// Prepare for response-handling
	
	struct pollfd pollfd[1];
	
	pollfd->fd = connection->socket;
	pollfd->events = POLLIN;
	pollfd->revents = 0;
	
	int result = poll(pollfd, 1, 3000); // Wait for most 3 seconds
	
	// Error during poll()
	if(result < 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
		error( "_tango_send_and_receive(): Error during poll()-ing: %s\n", strerror(errno));
		return 0;
	}
	
	// Timeout...
	if(result == 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
		error( "_tango_send_and_receive(): Timeout during poll(): %s\n", strerror(errno));
		return 0;
	}
	
	// Message received
	
	unsigned char *buffer_ptr = smb_response->data_ptr - NBT_HEADER_LENGTH;
	
	// Read NBT-header first
	result = recv(connection->socket, buffer_ptr, 4, 0);
	if(result < 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
		error( "_tango_send_and_receive(): Error during recv()-ing: %s\n", strerror(errno));
		return 0;
	}
	
	// Timeout...
	if(result == 0) {
		_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
		error( "_tango_send_and_receive(): Timeout during recv(): %s\n", strerror(errno));
		return 0;
	}
	
	debug("_tango_send_and_receive(): Received %d bytes.\n", result);
	
	unsigned int bytes_to_receive = ntohl(*((unsigned int *)buffer_ptr));
	smb_response->size = bytes_to_receive;
	
	buffer_ptr+=4;
	while (bytes_to_receive > 0) {
		result = recv(connection->socket, buffer_ptr, bytes_to_receive, 0);
		if(result < 0) {
			_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
			error( "_tango_send_and_receive(): Error during recv()-ing: %s\n", strerror(errno));
			return 0;
		}
		
		debug("_tango_send_and_receive(): Received %d bytes.\n", result);
		
		// Timeout...
		if(result == 0) {
			_tango_set_error(connection, kTangoErrorConnectionProblem, "Connection problem");
			error( "_tango_send_and_receive(): Timeout during recv(): %s\n", strerror(errno));
			return 0;
		}
		
		bytes_to_receive -= result;
		buffer_ptr += result;
	}
	
	return 1;
	
}

/**
 * Sends the smb_request and receives the smb_response
 *
 * @param *connection Connection with attached socket
 * @param *smb_request Storage with SMB to send
 * @param *smb_response Storage for response-SMB; if smb_response is set to NULL, response overrides is written to smb_request
 *
 * @return 1 on success; 0 on failure; Details are provided in connection->error and connection->error_message and can be read via tango_error and smb_error_message respectively.
 */
int _tango_send_and_receive(tango_connection_t *connection, tango_smb_t *smb_request, tango_smb_t *smb_response) {

	if (!_tango_send(connection, smb_request)) return 0;
	
	// Prepare storage for response
	if (smb_response == NULL) {
		// Use smb_request to store response
		smb_response = smb_request;
	}
	
	if (!_tango_receive(connection, smb_response)) return 0;
	
	return 1;
}

#pragma mark Message-Construction and Evaluation

void _tango_populate_request_header(tango_connection_t *connection, tango_smb_t *smb, unsigned char command) {
	unsigned char *buffer_ptr = smb->data_ptr;
	
	// Protocol
	*(buffer_ptr) = 0xff;
	*(buffer_ptr + 1) = 'S';
	*(buffer_ptr + 2) = 'M';
	*(buffer_ptr + 3) = 'B';
	
	// Command
	*(unsigned char *)(buffer_ptr + SMB_HEADER_COMMAND_BYTE) = command;
	
	// Status
	*(unsigned int *)(buffer_ptr + SMB_HEADER_STATUS_INT) = 0x00;
	
	// Flags
	*(unsigned char *)(buffer_ptr + SMB_HEADER_FLAGS_BYTE) = 0x00;
	*(unsigned char *)(buffer_ptr + SMB_HEADER_FLAGS_BYTE) |= SMB_FLAGS_CASELESS_PATHNAMES;

	// Flags2
	*(unsigned short *)(buffer_ptr + SMB_HEADER_FLAGS2_SHORT) = 0x0000;
	*(unsigned short *)(buffer_ptr + SMB_HEADER_FLAGS2_SHORT) |= SMB_FLAGS2_32BIT_STATUS;
	*(unsigned short *)(buffer_ptr + SMB_HEADER_FLAGS2_SHORT) |= SMB_FLAGS2_IS_LONG_NAME;
	*(unsigned short *)(buffer_ptr + SMB_HEADER_FLAGS2_SHORT) |= SMB_FLAGS2_KNOWS_LONG_NAMES;
	
	// Extra
	
	// TID
	*(unsigned short *)(buffer_ptr + SMB_HEADER_TID_SHORT) = connection->tid;
	// PID
	*(unsigned short *)(buffer_ptr + SMB_HEADER_PID_SHORT) = connection->pid;
	// UID
	*(unsigned short *)(buffer_ptr + SMB_HEADER_UID_SHORT) = connection->uid;
	// MID
	*(unsigned short *)(buffer_ptr + SMB_HEADER_MID_SHORT) = connection->mid;
}

int _tango_evaluate_response_header(tango_connection_t *connection, tango_smb_t *smb) {
	unsigned char *buffer_ptr = smb->data_ptr;
	if (*(buffer_ptr) != 0xff || 
		*(buffer_ptr + 1) != 'S' ||
		*(buffer_ptr + 2) != 'M' ||
		*(buffer_ptr + 3) != 'B') {
		_tango_set_error(connection, kTangoErrorInvalidResponseMessage, "Response invalid");
		error("_tango_evaluate_response_header(): Response has invalid SMB header.\n");
		return 0;
	}
	
	// TODO: CHECK LENGTH OF MESSAGE WITH LENGTH SPECIFIED IN DATA-LENGTH AND PARAMETERS-LENGTH
	
	// TODO: Check if SERVER_TO_REDIR is set
	
	if (_tango_smb_get_NT_STATUS_Level(smb) != SUCCESS) {
		_tango_set_error(connection, kTangoErrorSMBError, "Response invalid");
		error("_tango_evaluate_response_header(): NT_STATUS Level is not SUCCESS.\n");
		return 0;
	}
	
	// Read UID
	connection->uid = *((unsigned short *)(buffer_ptr + SMB_HEADER_UID_SHORT));
	// Read TID
	connection->tid = *((unsigned short *)(buffer_ptr + SMB_HEADER_TID_SHORT));
	
	return 1;
}


