/*
  ==============================================================================

    MACframe.cpp
    Created: 31 Oct 2021 11:04:04am
    Author:  A

  ==============================================================================
*/

#include "MACframe.h"

// constructor for receive
MACframe::MACframe(Array<int8_t> all_data) : crc() {
    this->receive_time = std::chrono::system_clock::now();
    std::vector<int8_t> vec;
    for (int i = 0; i < all_data.size(); i++)
        vec.push_back(all_data[i]);
    type = all_data[0];
    frame_id = all_data[1];
    dst_address = all_data[2];
    src_address = all_data[3];
    frame_length = all_data[4];
    if (type == TYPE_DATA) {
        int8_t frame_crc = all_data[all_data.size() - 1];
        for (int i = 0; i < 5; i++)
            crc.updateCRC(all_data[i]);
        for (int i = 5; i < all_data.size() - 1; i++) {
            //if (i < 11)
            //    ip_port.add(all_data[i]);
            //else
                data.add(all_data[i]);
            crc.updateCRC(all_data[i]);
        }
        //translateAddrPort(false);
        if (frame_crc != crc.getCRC())
            bad_crc = 1;
    }
    else if (type == TYPE_ACK || type == TYPE_MACPING_REPLY) {
        for (int i = 5; i < all_data.size(); i++)
            data.add(all_data[i]);
    }
    else if (type == TYPE_ICMP_REQUEST || type == TYPE_ICMP_REPLY) {
        for (int i = 4; i < 9; i++)
            ip_port.add(all_data[i]);
        translateAddrPort(true);
        for (int i = 5; i < all_data.size(); i++)
            data.add(all_data[i]);
    }
    else if (type == TYPE_FTP_COMMAND)
    {
        for (int i = 5; i < 5 + frame_length; i++)
        {
            data.add(all_data[i]);
        }
    }
    resend_times = 0;
}

// constructor for ack frame
MACframe::MACframe(int8_t dst_address, int8_t src_address, int8_t ack_id) {
    type = (int8_t)TYPE_ACK;
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->frame_length = 1;//only one byte for ack_id
    this->ack_id = ack_id;
    for (int i = 0; i < 8; i++)
        data.insert(0, (int8_t)((ack_id >> i) & 1));
    frame_status = Status_Waiting;
    resend_times = 0;
}

// constructor for data frame
MACframe::MACframe(int8_t frame_id, int8_t dst_address, int8_t src_address, std::vector<int8_t> frame_data) : crc() {
    type = (int8_t)TYPE_DATA;
    this->frame_id = frame_id;
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->frame_length = frame_data.size();
    crc.updateCRC(type);
    crc.updateCRC(this->frame_id);
    crc.updateCRC(dst_address);
    crc.updateCRC(src_address);
    crc.updateCRC(this->frame_length);
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

//construct a MACPING frame
MACframe::MACframe(int8_t type, int8_t reply_id, int8_t dst_address, int8_t src_address)
{
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->type = (int8_t)type;
    this->ack_id = (int8_t)reply_id;
    for (int i = 0; i < 8; i++)
        data.insert(0, (int8_t)((reply_id >> i) & 1));
    frame_status = Status_Waiting;
    resend_times = 0;
}

// constructor for macperf frame
MACframe::MACframe(int8_t dst_address, int8_t src_address, int frame_bit_num) {
    type = (int8_t)TYPE_DATA;
    this->dst_address = dst_address;
    this->src_address = src_address;
    for (int i = 0; i < frame_bit_num; i++)
        data.add(rand() % 2);
    frame_status = Status_Waiting;
    resend_times = 0;
}

//constructor for icmp frame
MACframe::MACframe(int8_t type, int8_t icmp_id, int8_t dst_address, int8_t src_address, std::string ip_address)
{
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->type = type;
    this->ip_address = ip_address;
    this->icmp_id = icmp_id;
    std::vector<int8_t> address_array;
    split_address(ip_address, address_array);
    for (int i = 3; i >= 0; i--)
        for (int j = 0; j < 8; j++)
            data.insert(0, (int8_t)((address_array[i] >> j) & 1));
    for (int i = 0; i < 8; i++)
        data.insert(0, (int8_t)((icmp_id >> i) & 1));
    for (int i = 0; i < 42*8; i++)
        data.add(rand() % 2);
    frame_status = Status_Waiting;
    resend_times = 0;
}

//constructor for ftp frame
MACframe::MACframe(int8_t type, int8_t dst_address, int8_t src_address, int8_t cmd_type, std::vector<int8_t> data)
{
    this->dst_address = dst_address;
    this->src_address = src_address;
    this->type = type;
    this->frame_length = data.size() + 2; // cmd(1) + " "(1) + information(size)

    for (int k = 7; k >= 0; k--)
        this->data.add((int8_t)((cmd_type >> k) & 1));
    for (int k = 7; k >= 0; k--)
        this->data.add((int8_t)(((int8_t)0x20 >> k) & 1));
    for (int i = 0; i < data.size(); i++)
    {
        for (int k = 7; k >= 0; k--)
            this->data.add((int8_t)((data[i] >> k) & 1));
    }
    frame_status = Status_Waiting;
    resend_times = 0;
}

void
MACframe::setSendTime() {
    send_time = std::chrono::system_clock::now();
}

void
MACframe::setReceiveTime()
{
    receive_time = std::chrono::system_clock::now();
}

double
MACframe::getRTT()
{
    return (receive_time - send_time).count();
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
        ret_array.insert(0, (int8_t)((frame_length >> i) & 1));
    for (int i = 0; i < 8; i++)
        ret_array.insert(0, (int8_t)((src_address >> i) & 1));
    for (int i = 0; i < 8; i++)
        ret_array.insert(0, (int8_t)((dst_address >> i) & 1));
    for (int i = 0; i < 8; i++)
        ret_array.insert(0, (int8_t)((frame_id >> i) & 1));
    for (int i = 0; i < 8; i++)
        ret_array.insert(0, (int8_t)((type >> i) & 1));
    /*byteToBits(ret_array, frame_id);
    byteToBits(ret_array, type);*/
    return ret_array;
}

void
MACframe::translateAddrPort(bool icmp) {
    ip_address = "";
    if (icmp) {
        icmp_id = (int)(unsigned char)ip_port[0];
        for (int i = 1; i < 5; i++) {
            ip_address += std::to_string((int)(unsigned char)ip_port[i]);
            if (i != 4)
                ip_address += ".";
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            ip_address += std::to_string((int)(unsigned char)ip_port[i]);
            if (i != 3)
                ip_address += ".";
        }
        port = (int)(unsigned char)ip_port[4] * 16 * 16 + (int)(unsigned char)ip_port[5];
    }
}

void 
MACframe::split_address(const std::string& address_string, std::vector<int8_t>& address_array) {
    const std::string delimters = ".";
    std::string::size_type lastPost = address_string.find_first_not_of(delimters, 0);
    std::string::size_type pos = address_string.find_first_of(delimters, lastPost);
    while (std::string::npos != pos || std::string::npos != lastPost) {
        address_array.push_back((int8_t)atoi(address_string.substr(lastPost, pos - lastPost).c_str()));
        lastPost = address_string.find_first_not_of(delimters, pos);
        pos = address_string.find_first_of(delimters, lastPost);
    }
}


void MACframe::printFrame() {
    //std::cout << "address: " << ip_address << ", port: " << port << ", data: ";
    for (int i = 0; i < 40; i++)
        std::cout << (char)data[i];
}
