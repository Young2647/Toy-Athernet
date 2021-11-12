/*
  ==============================================================================

    MAClayer.cpp
    Created: 31 Oct 2021 11:03:53am
    Author:  A

  ==============================================================================
*/

#include "MAClayer.h"

MAClayer::MAClayer() {
    sender_LFS = 0;
    sending_thread = thread(sendData(1));
    receive_thread = thread(&receive, this);
    sender_window.resize(SWS);
    max_frame_gen_idx = -1;
}


void 
MAClayer::receive() {
    Array<int8_t> data = Mac_receiver.getData();
    MACframe receive_frame(data);
    if (receive_frame.getType() == TYPE_DATA)
    {
        int8_t receive_id = receive_frame.getFrame_id();
        cout << "Frame " << receive_id << "received.\n";
        sendACK(receive_id);
        if (last_receive_id == receive_id - 1)
        {
            Write2File(receive_frame.getData());
            last_receive_id = receive_id;
        }
        else
        {
            ///wait to be implemented.
        }
    }
    else if (receive_frame.getType() == TYPE_ACK)
    {
        int8_t ack_id = receive_frame.getFrame_id();

        requestSend();
    }

}

void
MAClayer::StartReceiving()
{

}


void
MAClayer::readFromFile(int num_frame) {
    data_frames.resize(num_frame);
    ifstream f;
    char tmp;
    f.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021-main\\input.in");
    for (int i = 0; i < num_frame; i++) {
        data_frames[i].resize(num_bits_per_frame);
        for (int j = 0; j < num_bits_per_frame; j++) {
            f >> tmp;
            frame_bit[i][j] = (int8_t)tmp - 48;
            //frame_bit[i][j] = 1;
        }
    }
    f.close();
}

unique_ptr<MACframe>
MACframe::sendACK(int8_t frame_id) {
    unique_ptr<MACframe> ack_frame;
    ack_frame.reset(new MACframe(frame_id));
    Mac_sender.sendOnePacket(2, ack_frame->getData());
    STATE = FrameDetection;
    return ack_frame;
}

unique_ptr<MACframe>
MACframe::sendData(int8_t frame_id) {
    
    Mac_sender.sendOnePacket(num_bits_per_frame, data_frame->getData());
    STATE = FrameDetection;
    return data_frame;
}

unique_ptr<Macframe>
MACframe::generateNextFrame() {
    unique_ptr<MACframe> data_frame;
    data_frame.reset(new MACframe(++max_frame_gen_idx, data_frames[max_frame_gen_idx]));
    data_frame->setSendTime();
}

void
MACframe::requestSend(int8_t ack_id) {
    // No action if ack_id is not in sender's sliding window
    if (ack_id <= sender_LAR || ack_id > sender_LFS)
        return;

    sender_LAR = ack_id;
    // slide window to sender_LAR + 1
    while (sender_window.begin()->getFrame_id() <= sender_LAR) {
        sender_window.remove(sender_window.begin());
        sender_window.add()
    }
}

