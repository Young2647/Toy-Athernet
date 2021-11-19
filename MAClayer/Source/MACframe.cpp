/*
  ==============================================================================

    MACframe.cpp
    Created: 31 Oct 2021 11:04:04am
    Author:  A

  ==============================================================================
*/

#include "MACframe.h"

MACframe::MACframe(Array<int8_t> all_data) : crc() {
    /*std::vector<int8_t> vec;
    for (int i = 0; i < all_data.size(); i++)
        vec.push_back(all_data[i]);*/
    type = all_data[0];
    frame_id = all_data[1];
    dst_address = all_data[2];
    src_address = all_data[3];
    if (type == TYPE_DATA) {
        int8_t frame_crc = all_data[all_data.size() - 1];
        for (int i = 2; i < all_data.size() - 1; i++) {
            data.add(all_data[i]);
            crc.updateCRC(all_data[i]);
        }
        if (frame_crc != crc.getCRC())
            bad_crc = 1;
    }
    else if (type == TYPE_ACK) {
        for (int i = 2; i < all_data.size(); i++) 
            data.add(all_data[i]);
    }
    resend_times = 0;
}

MACframe::MACframe(int8_t dst_address, int8_t src_address,int8_t ack_id) {
    type = (int8_t)TYPE_ACK;
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->ack_id = ack_id;
    for (int i = 0; i < 8; i++)
        data.insert(0, (int8_t)((ack_id >> i) & 1));
    frame_status = Status_Waiting;
    resend_times = 0;
}

MACframe::MACframe(int8_t dst_address, int8_t src_address, bool identifier, std::vector<int8_t> frame_data) : crc() {
    type = (int8_t)TYPE_DATA;
    this->dst_address = dst_address;
    this->src_address = src_address;
    for (int i = 0; i < frame_data.size(); i++) {
        crc.updateCRC(frame_data[i]);
        for (int k = 7; k >= 0; k--)
            data.add((int8_t)((frame_data[i] >> k) & 1));
    }
    auto tmp = crc.getCRC();
    for (int k = 7; k >= 0; k--)
       data.add((int8_t)(tmp >> k) & 1);
    frame_status = Status_Waiting;
    resend_times = 0;
}


void 
MACframe::setSendTime() { 
    send_time = std::chrono::system_clock::now();
}

double 
MACframe::getTimeDuration() { 
    std::chrono::duration<double, std::milli> diff = std::chrono::system_clock::now() - send_time;
    return diff.count();
}

Array<int8_t>
MACframe::toBitStream() {
    Array<int8_t> ret_array = Array<int8_t>(data);
    for (int i = 0; i < 8; i++) 
        ret_array.insert(0, (int8_t)((frame_id >> i) & 1));
    for (int i = 0; i < 8; i++) 
        ret_array.insert(0, (int8_t)((type >> i) & 1));
    /*byteToBits(ret_array, frame_id);
    byteToBits(ret_array, type);*/
    return ret_array;
}