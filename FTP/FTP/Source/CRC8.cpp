/*
  ==============================================================================

    CRC8.cpp
    Created: 19 Nov 2021 12:55:38pm
    Author:  zhaoyb

  ==============================================================================
*/

#include "CRC8.h"

CRC8::CRC8() {
	// create crc table
	crc = init;
	for (int i = 0; i <= 0xff; i++) {
		int remainder = i & 0xff;
		for (int bit = 0; bit < 8; bit++) {
			if (remainder & 0x01)
				remainder = (remainder >> 1) ^ poly;
			else
				remainder >>= 1;
		}
		crcTable.add(remainder);
	}
}

void CRC8::updateCRC(uint8_t data) {
	data ^= crc;
	crc = (uint8_t)(crcTable[data & 0xff] ^ (crc << 8));
}
