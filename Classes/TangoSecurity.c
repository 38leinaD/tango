/*
 *  TangoSecurity.c
 *  tango
 *
 * Copyright (C) 2010 Daniel Platz
 * 
 * This software is available under a BSD license. Please see the LICENSE.TXT file for details.
 */

#include "TangoSecurity.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <CommonCrypto/CommonCryptor.h>

#pragma mark ---
#pragma mark - Private methods -

/**
 * Calculates the number of bits set (1) in a byte.
 *
 * @param c Byte that is counted
 *
 * @return Number of bits set to 1
 */
int _tango_sec_parity(unsigned char c) {
	int num_bits_set = 0;
	for (int i=0; i<8; i++) {
		num_bits_set += (c >> i & 0x01);
	}
	return num_bits_set % 2 == 1;
}

/**
 * Expands a 7-byte DES key to 8-byte as required by des_enc().
 *  Every 8th-bit is set to parity.
 *
 * @param in_key Pointer to 7-byte input-key
 * @param out_key Pointer to memory where to store 8-byte output-key. Has to be alloced by caller.
 *
 */
void _tango_sec_des_key_expand(unsigned char *in_key, unsigned char *out_key) {
	unsigned char window[2] = {
		0,0	
	};
	
	unsigned char c;
	for (int i=0; i<8; i++) {
		if (i==0) {
			window[0] = in_key[0];
		}
		else if (i==7) {
			window[0] = 0;
			window[1] = in_key[6];
		}
		else {
			window[0] = in_key[i];
			window[1] = in_key[i-1];
		}
		
		c = 0xFE & (*((unsigned short *)window) >> i);

		out_key[i] = (c & 0xFE) + (1 - _tango_sec_parity(c));
	}
}

/**
 * Generate 16-byte LM-hash
 *
 * @param password Input-string for which to generate the hash
 * @param lm_hash_result Pointer to a memory location of at least 16 bytes. After the operation, the LM-hash is provided at this memory-location to the caller.
 *
 * @return 1 on success; 0 on failure
 */
int _tango_sec_generate_lm_hash(char *str, unsigned char *lm_hash_result) {
	if (lm_hash_result == NULL) {
		return 0;
	}
	
	unsigned char keypair_compressed[14];
	unsigned char keypair_expanded[16];
	// Use internal buffer instead of lm_hash_result directly because CCCrypt seems to require an oversized buffer...
	unsigned char lm_hash_buffer[16];
	
	// Null it
	memset(keypair_compressed, 0, sizeof(keypair_compressed));
	
	// Fill with password-string and chop end if too long
	memcpy(keypair_compressed, str, fmin(strlen(str),13));
	
	// To upper-case
	for (int i=0; i<14; i++) {
		if (keypair_compressed[i] >= 0x61 && keypair_compressed[i] <= 0x7A) {
			keypair_compressed[i] -= 0x20;
		}
	}
	
	// TODO: Remove when working...
	/*
	{
		// test
		unsigned char key_7byte[7] = { 0x8B, 0x32, 0x93, 0xa1, 0xd3, 0xca, 0x87};
		unsigned char key_8byte[8];
		printf("binary in: %x %x %x %x %x %x %x\n", key_7byte[0],key_7byte[1],key_7byte[2],key_7byte[3],key_7byte[4],key_7byte[5],key_7byte[6]);
		_tango_sec_des_key_expand(key_7byte, key_8byte);
		
		printf("binary out: %x %x %x %x %x %x %x %x\n", key_8byte[0],key_8byte[1],key_8byte[2],key_8byte[3],key_8byte[4],key_8byte[5],key_8byte[6], key_8byte[7]);
	}*/
	
	// Expand keys to 8 byte as required by CCCrypt's DES
	_tango_sec_des_key_expand(keypair_compressed, keypair_expanded);
	_tango_sec_des_key_expand(keypair_compressed + 7, keypair_expanded + 8);
	
	// Encrypt KGS!@#$%...
	char *magic_string = "KGS!@#$%";
	CCCryptorStatus result;
	size_t data_out_moved = 0;
	// ... with first 7-byte key
	result = 
	CCCrypt(kCCEncrypt,
			kCCAlgorithmDES,
			kCCOptionPKCS7Padding,
			keypair_expanded,
			kCCKeySizeDES,
			NULL,
			magic_string,
			strlen(magic_string),
			lm_hash_buffer,
			16, 
			&data_out_moved);
	
	if (result != kCCSuccess) {
		return 0;
	}
	
	memcpy(lm_hash_result, lm_hash_buffer, 8);
	
	// ... with second 7-byte key
	result = 
	CCCrypt(kCCEncrypt,
			kCCAlgorithmDES,
			kCCOptionPKCS7Padding,
			keypair_expanded + 8,
			kCCKeySizeDES,
			NULL,
			magic_string,
			strlen(magic_string),
			lm_hash_buffer,
			16, 
			&data_out_moved);
	
	if (result != kCCSuccess) {
		return 0;
	}
	
	memcpy(lm_hash_result + 8, lm_hash_buffer, 8);
	
	return 1;
}

#pragma mark ---
#pragma mark - Public methods -

int _tango_sec_lm_challenge_response_encrypt(char *secret, unsigned char *challenge, unsigned char *response) {
	unsigned char lm_hash_compressed[21];
	unsigned char lm_hash_expanded[24];
	
	// Generate LM-Hash for secret
	_tango_sec_generate_lm_hash(secret, lm_hash_compressed);
	// Pad 5 null-bytes
	memset(lm_hash_compressed + 16, 0, 5);
	
	// Expand 7-byte DES keys to 8-byte keys
	_tango_sec_des_key_expand(lm_hash_compressed, lm_hash_expanded);
	_tango_sec_des_key_expand(lm_hash_compressed + 7, lm_hash_expanded + 8);
	_tango_sec_des_key_expand(lm_hash_compressed + 14, lm_hash_expanded + 16);
	
	CCCryptorStatus result;
	size_t data_out_moved = 0;
	
	// Encrypt challenge with first key
	
	// NOTE: lm_hash_compressed is reused as output-buffer for CCCrypt just because it is there anyway... no special meaning that this memory is used!
	
	result = 
	CCCrypt(kCCEncrypt,
			kCCAlgorithmDES,
			kCCOptionPKCS7Padding,
			lm_hash_expanded,
			kCCKeySizeDES,
			NULL,
			challenge,
			8,
			lm_hash_compressed,
			16, 
			&data_out_moved);
	
	if (result != kCCSuccess) {
		return 0;
	}
	
	// Copy encryped challenge to response-buffer
	memcpy(response, lm_hash_compressed, 8);
	
	// Encrypt challenge with second key
	result = 
	CCCrypt(kCCEncrypt,
			kCCAlgorithmDES,
			kCCOptionPKCS7Padding,
			lm_hash_expanded + 8,
			kCCKeySizeDES,
			NULL,
			challenge,
			8,
			lm_hash_compressed,
			16, 
			&data_out_moved);
	
	if (result != kCCSuccess) {
		return 0;
	}
	
	// Copy encryped challenge to response-buffer
	memcpy(response + 8, lm_hash_compressed, 8);
	
	// Encrypt challenge with third key
	result = 
	CCCrypt(kCCEncrypt,
			kCCAlgorithmDES,
			kCCOptionPKCS7Padding,
			lm_hash_expanded + 16,
			kCCKeySizeDES,
			NULL,
			challenge,
			8,
			lm_hash_compressed,
			16, 
			&data_out_moved);
	
	if (result != kCCSuccess) {
		return 0;
	}
	
	// Copy encryped challenge to response-buffer
	memcpy(response + 16, lm_hash_compressed, 8);
	
	return 1;
}