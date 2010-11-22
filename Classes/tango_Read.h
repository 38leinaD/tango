/*
 *  Tango_Read.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_READ_H_
#define _TANGO_READ_H_

#include "TangoBase.h"

int _tango_READ(tango_connection_t *connection, tango_file_info_t *file_info, unsigned int offset, unsigned int bytes, unsigned char *buffer);

#endif