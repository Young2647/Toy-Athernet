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

MACframe::MACframe(int8_t frame_id) {
    data.add((int8_t)TYPE_ACK);
    data.add(frame_id);
}

MACframe::MACframe(int8_t frame_id, Array<int8_t> frame_data) {
    data.add((int8_t)TYPE_DATA);
    data.add(frame_id);
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
