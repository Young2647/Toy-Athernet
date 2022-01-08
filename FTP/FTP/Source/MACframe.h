#ifndef _MACFRAME_H_
#define _MACFRAME_H_

#include <JuceHeader.h>
#include <chrono>
#include "defines.h"
#include "CRC8.h"
using namespace juce;
/// <summary>
/// A frame contains a header, a type, a frame_id, dst_address, src_address, length field, data
/// </summary>
class MACframe {
public:
    MACframe(Array<uint8_t> all_data);

    MACframe(uint8_t dst_address, uint8_t src_address, uint8_t ack_id); // constructor of ack frame
    MACframe(uint8_t frame_id, uint8_t dst_address, uint8_t src_address, std::vector<uint8_t> frame_data); // constructor of data frame
    MACframe(uint8_t type, uint8_t reply_id, uint8_t dst_address, uint8_t src_address); //constructor of MACPING frame
	MACframe(uint8_t dst_address, uint8_t src_address, int frame_bit_num); // constructor for macperf frame.
    MACframe(uint8_t type, uint8_t icmp_id, uint8_t dst_address, uint8_t src_address, std::string ip_address); // constructor for icmp frame.
    MACframe(uint8_t type, uint8_t dst_address, uint8_t src_address, uint8_t cmd_type, std::vector<uint8_t> data); //constructor for ftp frame.
    MACframe(uint8_t dst_address, uint8_t src_address, bool if_show_file);//constructor for file_end frame

    uint8_t getType() { return type; }
    uint8_t getFrame_id() { return frame_id; }
    uint8_t getAck_id() { return ack_id; }
    int getFrame_size() { return data.size(); }
    void setFrameId(uint8_t id) { frame_id = id; }
    Array<uint8_t> getData() { return data; }
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

    uint8_t getSrcAddr() { return src_address; }
    uint8_t getDstAddr() { return dst_address; }
    std::string getIPAddr() { return ip_address; }
    uint8_t getICMPID() { return icmp_id; }
    Array<uint8_t> toBitStream();
    void translateAddrPort(bool icmp);
    void split_address(const std::string& address_string, std::vector<uint8_t>& address_array);
    void printFrame();

private:
    uint8_t type;
    uint8_t frame_id;
    uint8_t dst_address;
    uint8_t src_address;
    uint8_t frame_length;
    uint8_t ack_id;
    uint8_t icmp_id;
    Array<uint8_t> data;
    Array<uint8_t> ip_port;
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
