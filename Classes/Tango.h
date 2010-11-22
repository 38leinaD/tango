/*
 *  Tango.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_H_
#define _TANGO_H_

#include "TangoTypes.h"

// tango Session-Handling
tango_connection_t *tango_create(const char *share, const char* username, const char *password);
int tango_connect(tango_connection_t *connection);
void tango_close(tango_connection_t *connection);
void tango_release(tango_connection_t *connection);

void tango_create_root_file_info(tango_connection_t *connection, tango_file_info_t *root_file_info);
int tango_list_directory(tango_connection_t *connection, tango_file_info_t *directory, tango_file_info_t file_info_arr[], unsigned int file_info_arr_size);

// tango_open_file
//int tango_read_file(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, unsigned char *buffer);
// tango_close_file

// tango Error-Handling
int tango_error(tango_connection_t *connection);
const char *tango_error_message(tango_connection_t *connection);

#endif