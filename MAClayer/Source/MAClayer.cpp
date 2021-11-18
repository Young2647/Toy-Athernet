/*
  ==============================================================================

    MAClayer.cpp
    Created: 31 Oct 2021 11:03:53am
    Author:  A

  ==============================================================================
*/

#include "MAClayer.h"
MAClayer::MAClayer(int num_samples_per_bit, int num_bits_per_frame, int num_frame) : Mac_receiver(num_samples_per_bit, num_bits_per_frame), Mac_sender(num_bits_per_frame, num_samples_per_bit) {
    this->Mac_num_frame = num_frame;
    this->num_bits_per_frame = num_bits_per_frame;
    this->num_samples_per_bit = num_samples_per_bit;

    sender_LFS = 0;
    //sender_window.resize(sender_SWS);
    
    // init receiver window
    receiver_LFR = 0;
    receiver_LFR = DEFAULT_RWS;
    receiver_LAF = receiver_LFR + receiver_RWS;
    max_frame_gen_idx = -1;
    keep_timer = 0;
    for (int i = 0; i < 256; i++)
        id_controller_array.add(i);
    frame_array.resize(256);
    trans_timeout = 500ms;
    //init sender and receiver
    
}

MAClayer::~MAClayer()
{
    if (receive_thread.joinable()) receive_thread.join();
    if (sending_thread.joinable()) sending_thread.join();
    if (!timers.empty())
    {
        for (int i = 0; i < timers.size(); i++)
        {
            if (timers[i].joinable())
                timers[i].join();
        }
    }
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
    cout << "...receiving...\n";
    while (!Mac_stop)
    {
        Array<int8_t> data = Mac_receiver.getData();
        if (data.isEmpty())
        {
            cout << "nothing received.\n";
            continue;
        }
        MACframe receive_frame(data);
        vector<int8_t> vec;
        for (int i = 0; i < 50; i++) {
            vec.push_back(receive_frame.getData()[i]);
        }
        if (receive_frame.getType() == TYPE_DATA)
        {
            int8_t receive_id = receive_frame.getFrame_id();
            cout << "Frame " << (int)receive_id << "received.\n";
            Write2File(receive_frame.getData());
            requestSend(receive_id);
        }
        else if (receive_frame.getType() == TYPE_ACK)
        {
            int ack_id = (int)receive_frame.getData()[0];
            if (frame_array[ack_id].get())
                frame_array[ack_id].get()->setStatus(Status_Acked);// let the frame in frame array to be marked as acked.
            cv.notify_one();
            cout << "ACK " << ack_id << " received.\n";
            requestSend(data_frames[ack_id + 1]);//wait to be implemented
        }
        Mac_receiver.clearFrameData();
    }
}

void
MAClayer::send() {
    //init parameters
    int id = 0;

    readFromFile(Mac_num_frame);
    requestSend(data_frames[0]);
    while (!Mac_stop)
    {
        for (auto i : send_id_array)
        {
            id = i;
            if (frame_array[id].get()->getStatus() == Status_Waiting)
            {
                auto tmp = frame_array[id].get()->getFrame_size();
                Mac_sender.sendOnePacket(frame_array[id].get()->getFrame_size() + 16, frame_array[id].get()->toBitStream());
                
                frame_array[id].get()->setSendTime();
                if (frame_array[id].get()->getType() == TYPE_DATA)
                {
                    frame_array[id].get()->setStatus(Status_Sent);
                    cout << "frame " << id << " sent.\n";
                    //if (!keep_timer)
                    //    startTimer(id);
                }
                else // ACK is defualt set as acked
                {
                    frame_array[id].get()->setStatus(Status_Acked);
                    cout << "ack " << (int)frame_array[id].get()->getData()[0] << " sent.\n";
                }
            }
            /*else if (frame_array[id].get()->getStatus() == Status_Sent && frame_array[id].get()->getTimeDuration() >= MAX_WAITING_TIME)
            {
                cout << "frame " << id << " ack not received. Try to resend package.\n";
                frame_array[id].get()->setStatus(Status_Waiting);
                frame_array[id].get()->addResendtimes();
            }*/
        }
        checkIdarray();
        this_thread::sleep_for(20ms);
    }
}

void
MAClayer::checkIdarray()
{
    bool frame_send_complete = true;
    while (frame_send_complete && send_id_array.size() != 0)
    {
        int head = send_id_array[0];
        if (frame_array[head].get()->getStatus() == Status_Sent)
        {
            //if (frame_array[head].get()->ResendToomuch())
            //{
            //    cerr << "Resend too many times. Link error.\n";
            //    StopMAClayer();//link error, mac layer stops
            //}
            frame_send_complete = false;
        }
        else if (frame_array[head].get()->getStatus() == Status_Acked)
        {
            send_id_array.remove(0);
            id_controller_array.add(frame_array[head].get()->getFrame_id());
            frame_send_complete = true;
        }
    }
}

void
MAClayer::StartMAClayer()
{
    Mac_stop = false;
    receive_thread = thread(&MAClayer::receive, this);
    cout << "receiving thread start.\n";
    Mac_receiver.startRecording();
    cout << "receiver start recording.\n";
    Mac_sender.startSend();
    cout << "sender start sending";
    sending_thread = thread(&MAClayer::send, this);
    cout << "sending thread start.\n";
    fout.open("OUTPUT.bin", ios::out | ios::binary);
}

void
MAClayer::StopMAClayer()
{
    if (!Mac_stop)
    {
        Mac_sender.printOutput_buffer();
        Mac_stop = true;
        receive_thread.join();
        cout << "receiving thread stop.\n";
        Mac_receiver.stopRecording();
        cout << "receiver stop recording.\n";
        sending_thread.join();
        cout << "sending thread stop.\n";
        fout.close();
    }
}


void 
MAClayer::Write2File(Array<int8_t>& byte_data)
{
    fout.write((char*)byte_data.getRawDataPointer(), byte_data.size());
}

std::string getPath(const std::string& target, int depth = 5) {
    std::string path = target;
    for (int i = 0; i < depth; ++i) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
        path = "../" + path;
    }
    return target;
}

void
MAClayer::readFromFile(int num_frame) {
    data_frames.resize(num_frame);
    ifstream f(getPath("test.in"), ios::in | ios::binary);
    ofstream f1("test.out");
    char tmp;
    for (int i = 0; i < num_frame; i++) {
        data_frames[i].resize(num_bits_per_frame-16);
        for (int j = 0; j < (num_bits_per_frame-16)/8; j++) {
            while (f.get(tmp)) {
                for (int k = 7; k >= 0; k--) {
                    data_frames[i][j * 8 + (7 - k)] = (int8_t)((tmp >> k) & 1);
                }
            }
        }
    }
    for (int i = 0; i < (num_bits_per_frame - 16); i++) {
        f1 << (int)data_frames[0][i];
        if ((i + 1) % 8 == 0) {
            f1 << endl;
        }
    }
    f1.close();
    f.close();

    ofstream of;
    of.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021\\MAClayer\\sender.out", ios::trunc);
    for (int i = 0; i < (num_bits_per_frame - 16); i++) {
        of << (int)data_frames[0][i];
        if ((i+1) % 8 == 0) {
            of << endl;
        }
    }
    of.close();
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
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> ack_frame;
    ack_frame.reset(new MACframe(data_frame_id));
    ack_frame->setFrameId(id);
    send_id_array.insert(0, id);
    frame_array[id] = std::move(ack_frame);
    return id;
}

// requestSend for data packet
int
MAClayer::requestSend(std::vector<int8_t> frame_data) {
    const ScopedLock s1(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> data_frame;
    data_frame.reset(new MACframe(0, frame_data));
    data_frame->setFrameId(id);
    send_id_array.insert(0, id);
    frame_array[id] = std::move(data_frame);
    return id;
}

void
MAClayer::startTimer(int8_t data_frame_id) {
    keep_timer = 1;
    if (!timers.empty()) {
        timers[0].join();
        timers.erase(timers.begin());
    }
    thread timer(&MAClayer::wait, this, data_frame_id);
    timers.push_back(std::move(timer));
}

void 
MAClayer::wait(int8_t data_frame_id) {
    unique_lock<mutex> lk(cv_m);
    while (keep_timer)
    {
        auto now = std::chrono::system_clock::now();
        if (cv.wait_until(lk, now + trans_timeout, [&]() {return frame_array[data_frame_id].get()->getStatus() == Status_Acked; })) {
            keep_timer = 0;
        }
        else if (frame_array[data_frame_id].get()->ResendToomuch())
        {
            keep_timer = 0;
            cerr << "Resend too many times. Link error.\n";
            StopMAClayer();//link error, mac layer stops
        }
        else
        {
            cerr << "frame " << data_frame_id << "timeout. Try to resend package.\n";
            frame_array[data_frame_id].get()->setStatus(Status_Waiting);
            frame_array[data_frame_id].get()->addResendtimes();
        }
    }
}


