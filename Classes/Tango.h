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
void tango_create_file_info(tango_connection_t *connection, tango_file_info_t *parent_file_info, tango_file_info_t *file_info, const char *file_name, int is_folder);

int tango_list_directory(tango_connection_t *connection, tango_file_info_t *directory, tango_file_info_t file_info_arr[], unsigned int file_info_arr_size);

int tango_read_file(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, unsigned char *buffer);
int tango_write_file(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, const unsigned char *buffer);

// tango Error-Handling
int tango_error(tango_connection_t *connection);
const char *tango_error_message(tango_connection_t *connection);

#endif