/*
 *  Stamina.LIB
 *  
 *  Please READ /License.txt FIRST! 
 * 
 *  Copyright (C)2003,2004,2005 Rafa� Lindemann, Stamina
 *
 *  $Id: $
 *
 *
 *  Based on code from http://www.createwindow.com/programming/crc32/
 */

#include "stdafx.h"

#include "CRC32.h"


namespace Stamina {

	CRC32::CRC32() {
		buildLookup();
		reset();
	}
	void CRC32::buildLookup() {
		// This is the official polynomial used by CRC-32 
		// in PKZip, WinZip and Ethernet. 
		unsigned long ulPolynomial = 0x04c11db7; 

		// 256 values representing ASCII character codes. 
		for(int i = 0; i <= 0xFF; i++) 
		{ 
			_lookup[i]=reflect(i, 8) << 24; 
			for (int j = 0; j < 8; j++) 
					_lookup[i] = (_lookup[i] << 1) ^ (_lookup[i] & (1 << 31) ? ulPolynomial : 0); 
			_lookup[i] = reflect(_lookup[i], 32); 
		} 
	}

	unsigned long CRC32::reflect(unsigned long ref, char ch) {
		unsigned long value(0); 

		// Swap bit 0 for bit 7 
		// bit 1 for bit 6, etc. 
		for(int i = 1; i < (ch + 1); i++) 
		{ 
			if(ref & 1) 
					value |= 1 << (ch - i); 
			ref >>= 1; 
		} 
		return value; 
	}

	unsigned long CRC32::calculate(const char * text) {
		reset();
		return add(text , strlen(text));
	}

	unsigned long CRC32::add(const char * buffer , unsigned int size) {
		while(size--) 
			_state = (_state >> 8) ^ _lookup[(_state & 0xFF) ^ *buffer++]; 
		// Exclusive OR the result with the beginning value. 
		return getState();
	}
	void CRC32::reset() {
		_state = 0xffffffff;
	}
	unsigned long CRC32::getState() {
		return _state ^ 0xffffffff; 
	}


}