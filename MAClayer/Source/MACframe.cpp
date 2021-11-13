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
}

MACframe::MACframe(int8_t ack_id) {
    type = (int8_t)TYPE_ACK;
    data.add(ack_id);
}

MACframe::MACframe(bool identifier, Array<int8_t> frame_data) {
    type = (int8_t)TYPE_DATA;
    data.addArray(frame_data);
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
    ret_array.insert(0, type);
    ret_array.insert(1, frame_id);
    return ret_array;
}