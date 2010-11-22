/*
 *  Tango_Find2First.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_TREE_CONNECT_H_
#define _TANGO_TREE_CONNECT_H_

#include "TangoTypes.h"

int _tango_TRANS2_FIND_FIRST2(tango_connection_t *connection, const char *search_pattern, tango_file_info_t file_info_arr[], unsigned int file_info_arr_size);

#endif