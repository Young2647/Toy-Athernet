#ifndef _MACFRAME_H_
#define _MACFRAME_H_

#include <JuceHeader.h>
#include <chrono>
#include "defines.h"
using namespace juce;
/// <summary>
/// A frame contains a header, a type, a frame_id, data
/// </summary>
class MACframe {
public:
    MACframe(Array<int8_t> all_data);

    MACframe(int8_t frame_id); // constructor of ack frame
    MACframe(int8_t frame_id, Array<int8_t> frame_data); // constructor of data frame

    int8_t getType() { return type; }
    int8_t getFrame_id() { return frame_id; }
    Array<int8_t> getData() { return data; }
    void setSendTime();
    double getTimeDuration();
private:
    Array<float> header;
    int8_t type;
    int8_t frame_id;
    Array<int8_t> data;
    auto send_time;
};
#pragma once

#endif