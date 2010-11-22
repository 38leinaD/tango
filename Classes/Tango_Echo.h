/*
 *  Tango_Echo.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_ECHO_H_
#define _TANGO_ECHO_H_

#include "TangoBase.h"

int _tango_ECHO(tango_connection_t *connection, unsigned short numberOfEchoes, char echoPayload);

#endif