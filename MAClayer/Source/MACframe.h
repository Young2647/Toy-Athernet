#ifndef _MACFRAME_H_
#define _MACFRAME_H_

#include <JuceHeader.h>
using namespace juce;
/// <summary>
/// A frame contains a header, a type, a frame_id, data
/// </summary>
class MACframe {
public:
    MACframe(Array<int8_t> all_data);

    MACframe(int8_t frame_id);

    int8_t getType() { return type; }
    int8_t getFrame_id() { return frame_id; }
    Array<int8_t> getData() { return data; }
private:
    Array<float> header;
    int8_t type;
    int8_t frame_id;
    Array<int8_t> data;
};
#pragma once

#endif