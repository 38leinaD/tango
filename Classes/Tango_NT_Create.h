/*
 *  Tango_NT_Create.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_NT_CREATE_H_
#define _TANGO_NT_CREATE_H_

#include "TangoBase.h"

int _tango_NT_Create(tango_connection_t *connection, tango_file_info_t *file_info, tango_open_t open_type, unsigned int create_disposition);

#endif