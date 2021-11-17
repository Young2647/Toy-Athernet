/*
  ==============================================================================

    MACframe.cpp
    Created: 31 Oct 2021 11:04:04am
    Author:  A

  ==============================================================================
*/

#include "MACframe.h"

MACframe::MACframe(Array<int8_t> all_data) {
    type = all_data[0];
    frame_id = all_data[1];
    for (int i = 2; i < all_data.size(); i++) 
        data.add(all_data[i]);
    resend_times = 0;
}

MACframe::MACframe(int8_t ack_id) {
    type = (int8_t)TYPE_ACK;
    data.add(ack_id);
    frame_status = Status_Waiting;
    resend_times = 0;
}

MACframe::MACframe(bool identifier, std::vector<int8_t> frame_data) {
    type = (int8_t)TYPE_DATA;
    for (auto i : frame_data)
        data.add(i);
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

int8_t*
MACframe::byteToBits(int8_t number) {
    int8_t* ret_array = new int8_t[8];
    for (int i = 7; i >= 0; i--) 
        ret_array[i] = (0, (int8_t)((number >> i) & 1));
    return ret_array;
}

Array<int8_t>
MACframe::toBitStream() {
    Array<int8_t> ret_array = Array<int8_t>(data);
    ret_array.insertArray(0, byteToBits(frame_id), 8);
    ret_array.insertArray(0, byteToBits(type), 8);
    return ret_array;
}