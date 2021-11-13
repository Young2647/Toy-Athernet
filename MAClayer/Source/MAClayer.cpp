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
    //sender_window.resize(sender_SWS);
    
    // init receiver window
    receiver_LFR = 0;
    receiver_LFR = DEFAULT_RWS;
    receiver_LAF = receiver_LFR + receiver_RWS;
    max_frame_gen_idx = -1;
    
    last_receive_id = 0;
    for (int i = 0; i < 256; i++)
        id_controller_array.add(i);
}

void
MAClayer::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels, int numSamples)
{
    Mac_receiver.audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
    Mac_sender.audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
}

void 
MAClayer::receive() 
{
    while (!Mac_stop)
    {
        Array<int8_t> data = Mac_receiver.getData();
        MACframe receive_frame(data);
        if (receive_frame.getType() == TYPE_DATA)
        {
            int8_t receive_id = receive_frame.getFrame_id();
            cout << "Frame " << (int)receive_id << "received.\n";
            if (last_receive_id == receive_id - 1)
            {
                Write2File(receive_frame.getData());
                last_receive_id = receive_id;
                requestSend(receive_id);
            }
            else
            {
                ///wait to be implemented.
            }
        }
        else if (receive_frame.getType() == TYPE_ACK)
        {
            int ack_id = (int)receive_frame.getFrame_id();
            if (ack_id == sender_LAR + 1)
            {
                sender_LAR = ack_id;
                cout << "ACK" << ack_id << "received.\n";
            }
        }
    }
}

void
MAClayer::send() {
    //init parameters
    int id = 0;

    readFromFile(Mac_num_frame);
    while (!Mac_stop)
    {
        for (auto i : send_id_array)
        {
            id = i;
            if (frame_array[id].get()->getStatus() == Status_Waiting)
            {
                Mac_sender.sendOnePacket(frame_array[id].get()->getFrame_size() + 2, frame_array[id].get()->toBitStream());
                cout << "frame " << id << "sent.\n";
                frame_array[id].get()->setSendTime();
                if (frame_array[id].get()->getType() == TYPE_DATA)
                {
                    frame_array[id].get()->setStatus(Status_Sent);
                }
                else // ACK is defualt set as acked
                {
                    frame_array[id].get()->setStatus(Status_Acked);
                }
            }
            else if (frame_array[id].get()->getStatus() == Status_Sent && frame_array[id].get()->getTimeDuration() >= MAX_WAITING_TIME)
            {
                cout << "frame " << id << "ack not received. Try to resend package.\n";
                frame_array[id].get()->setStatus(Status_Waiting);
                frame_array[id].get()->addResendtimes();
            }
        }
        checkIdarray();
        this_thread::sleep_for(20ms);
    }
}

void
MAClayer::checkIdarray()
{
    bool frame_send_complete = true;
    while (frame_send_complete)
    {
        int head = send_id_array[0];
        if (frame_array[head].get()->getStatus() == Status_Sent)
        {
            if (frame_array[head].get()->ResendToomuch())
            {
                cerr << "Resend too many times. Link error.\n";
                StopMAClayer();//link error, mac layer stops
            }
            frame_send_complete = false;
        }
        else if (frame_array[head].get()->getStatus() == Status_Acked)
        {
            send_id_array.remove(0);
        }
    }
}

void
MAClayer::StartMAClayer()
{
    receive_thread = thread(&receive, this);
    cout << "receiving thread start.\n";
    Mac_receiver.startRecording();
    cout << "receiver start recording.\n";
    sending_thread = thread(&send, this);
    cout << "sending thread start.\n";
    Mac_stop = false;
    fout.open("OUTPUT.bin", ios::out || ios::binary);
}

void
MAClayer::StopMAClayer()
{
    Mac_stop = true;
    receive_thread.join();
    cout << "receiving thread stop.\n";
    Mac_receiver.stopRecording();
    cout << "receiver stop recording.\n";
    sending_thread.join();
    cout << "sending thread stop.\n";
    fout.close();
}


void 
MAClayer::Write2File(Array<int8_t>& byte_data)
{
    fout << (char*)byte_data.getRawDataPointer();
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
            data_frames[i].set(j, (int8_t)tmp - 48);
            //frame_bit[i][j] = 1;
        }
    }
    f.close();
}



// requestSend for ack packet
int
MAClayer::requestSend(int8_t data_frame_id) {
    //// No action if ack_id is not in sender's sliding window
    //if (ack_id <= sender_LAR || ack_id > sender_LFS)
    //    return;

    //sender_LAR = ack_id;
    //// slide window to sender_LAR + 1
    //while (sender_window.begin()->getFrame_id() <= sender_LAR) {
    //    sender_window.remove(sender_window.begin());
    //    sender_window.add()
    //}
    const ScopedLock sl(lock);
    int id = id_controller_array.take();
    unique_ptr<MACframe> ack_frame;
    ack_frame.reset(new MACframe(data_frame_id));
    ack_frame->setFrameId(id);
    send_id_array.insert(0, id);
    frame_array.set(id, ack_frame);
    return id;
}

// requestSend for data packet
int
MAClayer::requestSend(Array<int8_t> frame_data) {
    const ScopedLock s1(lock);
    int id = id_controller_array.take();
    unique_ptr<MACframe> data_frame;
    data_frame.reset(new MACframe(0, frame_data));
    data_frame->setFrameId(id);
    send_id_array.insert(0, id);
    frame_array.set(id, data_frame);
    return id;
}

