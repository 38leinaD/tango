/*
 *  TangoSecurity.h
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#ifndef _TANGO_SECURITY_H_
#define _TANGO_SECURITY_H_

/**
 * Generates the response in the LM challenge/response security.
 *  
 * @param secret Null-termined char-array that contains the secrete (the password) that is used for encrypting the challenge
 * @param challenge The 8-byte challenge
 * @param response The 24-byte response is provdided in at this memory-location to the caller. The caller is responsible for allocating the memory!
 *
 * @return 1 on success; 0 on failure
 */
int _tango_sec_lm_challenge_response_encrypt(char *secret, unsigned char *challenge, unsigned char *response);

#endif