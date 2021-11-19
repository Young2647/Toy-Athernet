#ifndef _MACFRAME_H_
#define _MACFRAME_H_

#include <JuceHeader.h>
#include <chrono>
#include "defines.h"
#include "CRC8.h"
using namespace juce;
/// <summary>
/// A frame contains a header, a type, a frame_id, dst_address, src_address, data
/// </summary>
class MACframe {
public:
    MACframe(Array<int8_t> all_data);

    MACframe(int8_t dst_address, int8_t src_address, int8_t ack_id); // constructor of ack frame
    MACframe(int8_t dst_address, int8_t src_address, bool identifier, std::vector<int8_t> frame_data); // constructor of data frame
    
    int8_t getType() { return type; }
    int8_t getFrame_id() { return frame_id; }
    int8_t getAck_id() { return ack_id; }
    int getFrame_size() { return data.size(); }
    void setFrameId(int8_t id) { frame_id = id; }
    Array<int8_t> getData() { return data; }
    Status getStatus() { return frame_status; }
    void setStatus(Status status) { frame_status = status; }
    bool isBadCRC() { return bad_crc; }
    void setSendTime();
    double getTimeDuration();
    void addResendtimes() { resend_times++; }
    bool ResendToomuch() { return !(resend_times < MAX_RESEND_TIME); }

    int8_t getSrcAddr() { return src_address; }
    int8_t getDstAddr() { return dst_address; }
    Array<int8_t> toBitStream();

private:
    int8_t type;
    int8_t frame_id;
    int8_t dst_address;
    int8_t src_address;
    int8_t ack_id;
    Array<int8_t> data;
    std::chrono::system_clock::time_point send_time;
    Status frame_status;
    int resend_times;
    CRC8 crc;
    bool bad_crc = 0;
};
#pragma once

#endif