/*
  ==============================================================================

    CRC8.h
    Created: 19 Nov 2021 12:55:14pm
    Author:  zhaoyb

  ==============================================================================
*/

#ifndef _CRC8_H_
#define _CRC8_H_
#include <JuceHeader.h>
using namespace juce;
class CRC8
{
public:
	CRC8();
	int8_t getCRC() { return crc; }
	void resetCRC() { crc = init; }
	void updateCRC(int8_t data);
	~CRC8() {};

private:
	int8_t poly = 0x8c; // reverse of 0x07
	int8_t init = 0x00;
	Array<int8_t> crcTable;
	int8_t crc;
};


#pragma once
#endif
