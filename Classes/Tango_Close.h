/*
 *  Tango_Close.h
 *  tango
 *
 * Copyright (C) 2011 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_CLOSE_H_
#define _TANGO_CLOSE_H_

#include "TangoBase.h"

int _tango_Close(tango_connection_t *connection, tango_file_info_t *file_info);

#endif