/*
 *  Tango_Write.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_WRITE_H_
#define _TANGO_WRITE_H_

#include "TangoBase.h"

int _tango_WRITE(tango_connection_t *connection, tango_file_info_t *file_info, const unsigned char *buffer, unsigned int bytes, unsigned int offset);

#endif