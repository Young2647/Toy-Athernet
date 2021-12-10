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
    MACframe(int8_t dst_address, int8_t src_address, std::vector<int8_t> frame_data); // constructor of data frame
    MACframe(int8_t type, int8_t reply_id, int8_t dst_address, int8_t src_address); //constructor of MACPING frame
	MACframe(int8_t dst_address, int8_t src_address, int frame_bit_num); // constructor for macperf frame.
    MACframe(int8_t dst_address, int8_t src_address); // constructor for icmp frame.
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
    void setReceiveTime();
    double getRTT();
    std::chrono::system_clock::time_point getSendTime() { return send_time; }
    std::chrono::system_clock::time_point getReceiveTime() { return receive_time; }
    double getTimeDuration();
    void addResendtimes() { resend_times++; }
    bool ResendToomuch() { return !(resend_times < MAX_RESEND_TIME); }

    int8_t getSrcAddr() { return src_address; }
    int8_t getDstAddr() { return dst_address; }
    Array<int8_t> toBitStream();
    void translateAddrPort();
    void printFrame();

private:
    int8_t type;
    int8_t frame_id;
    int8_t dst_address;
    int8_t src_address;
    int8_t ack_id;
    Array<int8_t> data;
    Array<int8_t> ip_port;
    std::string ip_address;
    int port;
    std::chrono::system_clock::time_point send_time;
    std::chrono::system_clock::time_point receive_time;
    Status frame_status;
    int resend_times;
    CRC8 crc;
    bool bad_crc = 0;
};
#pragma once

#endif
