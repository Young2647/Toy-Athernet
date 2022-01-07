/*
  ==============================================================================

    MAClayer.cpp
    Created: 31 Oct 2021 11:03:53am
    Author:  A

  ==============================================================================
*/

#include "MAClayer.h"
MAClayer::MAClayer(int num_samples_per_bit, int num_bits_per_frame, int num_frame, int8_t src_addr, int8_t dst_addr, int window_size) : Mac_receiver(num_samples_per_bit), Mac_sender(num_samples_per_bit), crc() {
    this->Mac_num_frame = num_frame;
    this->num_bits_per_frame = num_bits_per_frame;
    this->num_samples_per_bit = num_samples_per_bit;
    this->Mac_num_receive_frame = DEFAULT_RECEIVE_NUM;
    //this->all_byte_num = MAX_BYTE_NUM;
    this->src_addr = src_addr;
    this->dst_addr = dst_addr;
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

    //init ack_array
    ack_array.resize(Mac_num_frame);
    ack_array.fill(false);

    for (int i = 0; i < num_frame * 3; i++)
        frame_to_receive_list.push_back(1);

    //init file output
    //file_output.resize(Mac_num_frame);
    this->window_size = window_size;
}

MAClayer::~MAClayer()
{
    if (receive_thread.joinable()) receive_thread.join();
    if (sending_thread.joinable()) sending_thread.join();
    if (macping_thread.joinable()) macping_thread.join();
    //if (ack_sending_thread.joinable()) ack_sending_thread.join();
    //if (!timers.empty())
    //{
    //    for (int i = 0; i < timers.size(); i++)
    //    {
    //        if (timers[i].joinable())
    //            timers[i].join();
    //    }
    //}
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
    bool if_receive_done = false;
    while (!Mac_stop)
    {
        lock.enter();
        if (frame_receive_num == Mac_num_receive_frame) if_receive_done = true;
        lock.exit();
        if (if_receive_done && !getStop())
        {
            cerr << "All frames received.\n";
            receive_end = true;
            callStop(0);
        }
        Array<int8_t> data = Mac_receiver.getData();
        if (data.isEmpty())
        {
            if (debug_on)
                cout << "nothing received.\n";
            continue;
        }
        MACframe receive_frame(data);
        vector<int8_t> vec;
        for (int i = 0; i < 50; i++) {
            vec.push_back(receive_frame.getData()[i]);
        }
        if (receive_frame.getSrcAddr() != this->dst_addr || receive_frame.getDstAddr() != this->src_addr)
        {
            if (debug_on)
                cerr << "address not match.\n";
            continue;
        }
        //if (debug_on)
        //    cout << Mac_receiver.getMaxPower() << endl;
        if (receive_frame.getType() == TYPE_DATA)
        {
            int8_t receive_id = receive_frame.getFrame_id();
            if (debug_on)
                cout << "Frame " << (int)receive_id << "received.";
            if (receive_frame.isBadCRC() && !macperf_on) {
                if (debug_on)
                    cout << " however CRC is wrong.\n";
            }
            else {
                if (!macperf_on && debug_on)
                    cout << " CRC check pass!\n";
                if (!macperf_on)
                    if (frame_to_receive_list[receive_id]) {
                        if (!is_icmp_receiver)
                            receive_frame.printFrame();
                        file_output.push_back(receive_frame.getData());
                        frame_to_receive_list[receive_id] = 0;
                        frame_receive_increase = 1;
                        Write2File(receive_frame.getData(), "output.txt");
                        ofstream notify_file = std::ofstream("NOTIFY_DONE.txt"); //notify python node 
                        notify_file.close();
                    }
                requestSend(receive_id);
            }
        }
        else if (receive_frame.getType() == TYPE_ACK)
        {
            int ack_id = (int)receive_frame.getData()[0];
            if (frame_array[ack_id].get())
                frame_array[ack_id].get()->setStatus(Status_Acked);// let the frame in frame array to be marked as acked.
            //cv.notify_one();
            if (debug_on)
                cout << "ACK " << ack_id << " received.\n";
            if (!ack_array[ack_id])
            {
                ack_array.set(ack_id, true);
                frame_sent_num++;
            }
            if (frame_sent_num >= Mac_num_frame)
            {
                frame_sent_num = 0;
                Mac_num_frame = 0;
                //send_end = true;
                //cout << "All data sent.\n";
                //callStop(0);
            }
            else
            {
                if (macperf_on)
                {
                    requestSend();
                }
                else
                {
                    //while (cur_frame <= frame_sent_num)
                    //{

                    //}
                    requestSend(data_frames[frame_sent_num]);
                }
            }
        }
        else if (receive_frame.getType() == TYPE_MACPING_REQUEST)
        {
            cout << "MAC request " << (int)receive_frame.getFrame_id() << " received.\n";
            requestSend((int)receive_frame.getFrame_id(), TYPE_MACPING_REPLY);
        }
        else if (receive_frame.getType() == TYPE_MACPING_REPLY)
        {
            int reply_id = (int)receive_frame.getData()[0];
            cout << "MAC " << reply_id << "get replied. ";
            std::chrono::duration<double, std::milli> diff = receive_frame.getReceiveTime() - frame_array[reply_id].get()->getSendTime();
            cout << "RTT is " << diff.count() << "ms.\n";
            mac_ping_array.removeFirstMatchingValue(reply_id);
            //requestSend(reply_id, TYPE_MACPING_REQUEST);
        }
        else if (receive_frame.getType() == TYPE_ICMP_REQUEST)
        {
            cout << "ICMP request " << (int)receive_frame.getFrame_id() << " received.\n";
            Array<int8_t> all_data = receive_frame.getData();
            all_data.insert(0, receive_frame.getFrame_id());
            std::vector<int8_t> vec;
            for (int i = 0; i < all_data.size(); i++) vec.push_back(all_data[i]);
            Write2File(all_data, "request.bin");
            ofstream notify_file = std::ofstream("ICMP_NOTIFY.txt"); //notify python to work
            notify_file.close();
        }
        else if (receive_frame.getType() == TYPE_ICMP_REPLY)
        {
            int reply_id = (int)receive_frame.getICMPID();
            cout << "ICMP " << reply_id << "get replied from " << receive_frame.getIPAddr();
            std::chrono::duration<double, std::milli> diff = receive_frame.getReceiveTime() - frame_array[reply_id].get()->getSendTime();
            cout << "RTT is " << diff.count() << "ms.\n";
            icmp_array.removeFirstMatchingValue(reply_id);
        }
        else if (receive_frame.getType() == TYPE_FTP_COMMAND)
        {
            cout << "FTP command " << (int)receive_frame.getFrame_id() << " received.\n";
            Array<int8_t> all_data = receive_frame.getData();
            std::vector<int8_t> vec;
            for (int i = 0; i < all_data.size(); i++) vec.push_back(all_data[i]);
            Write2File(all_data, "command.bin");
            ofstream notify_file = std::ofstream("NOTIFY_DONE.txt"); //notify python to work
            notify_file.close();
        }
        else
        {
            cerr << "what is this ?\n";
        }
        Mac_receiver.clearFrameData();
    }
}

void
MAClayer::send() {
    //init parameters
    int id = 0;
    if (macperf_on)
        requestSend();
    else if (macping_on)
        requestSend(0, TYPE_MACPING_REQUEST);
    /* else if (icmp_on)
         requestSend(0, TYPE_ICMP_REQUEST);*/
    else if (is_sender) {
        if (if_send_ip)
            readFromFile(Mac_num_frame, "input.txt");
        else
            readFromFile("input.in");
        requestSend(data_frames[0]);
    }
    while (!Mac_stop)
    {
        for (int i = 0; i < min(send_id_array.size(), window_size); i++)
        {
            id = send_id_array[i];
            if (id > 255 || id < 0) continue;
            if (frame_array[id].get()->getStatus() == Status_Waiting)
            {
                auto tmp = frame_array[id].get()->getFrame_size();
                if (csma_on)
                {
                    this_thread::sleep_for(100ms);
                    while (Mac_receiver.getChannelPower() > 0.006f)// the channel is blocked
                    {
                        this_thread::sleep_for(back_off_time);
                    }
                }
                Mac_sender.sendOnePacket(frame_array[id].get()->getFrame_size() + FRAME_OFFSET, frame_array[id].get()->toBitStream());

                frame_array[id].get()->setSendTime();
                if (frame_array[id].get()->getType() == TYPE_DATA)
                {
                    frame_array[id].get()->setStatus(Status_Sent);
                    if (debug_on)
                        cout << "frame " << id << " sent.\n";
                    //if (!keep_timer)
                    //    startTimer(id);
                }
                else // ACK is defualt set as acked
                {
                    frame_array[id].get()->setStatus(Status_Acked);
                    if (frame_array[id].get()->getType() == TYPE_ICMP_REQUEST)
                    {
                        cout << "ICMP request " << id << " sent.\n";
                    }
                    else if (frame_array[id].get()->getType() == TYPE_ICMP_REPLY)
                    {
                        cout << "ICMP reply " << (int)frame_array[id].get()->getICMPID() << " sent.\n";
                    }
                    if (frame_array[id].get()->getType() == TYPE_MACPING_REQUEST)
                    {
                        cout << "macping request " << id << " sent.\n";
                    }
                    else if (frame_array[id].get()->getType() == TYPE_MACPING_REPLY)
                    {
                        cout << "macping reply " << (int)frame_array[id].get()->getAck_id() << " sent.\n";
                    }
                    else if (frame_array[id].get()->getType() == TYPE_FTP_COMMAND)
                    {
                        cout << "ftp command " << id << " sent.\n";
                    }
                    else if (frame_array[id].get()->getType() == TYPE_FTP_RESPONSE)
                    {
                        cout << "ftp response " << (int)frame_array[id].get()->getAck_id() << " sent.\n";
                    }
                    else if (frame_array[id].get()->getType() == TYPE_ACK) {
                        if (frame_receive_increase) {
                            frame_receive_num++;
                            frame_receive_increase = 0;
                        }
                        cout << "ack " << (int)frame_array[id].get()->getAck_id() << " sent.\n";
                    }
                }
            }
            else if (frame_array[id].get()->getStatus() == Status_Sent && frame_array[id].get()->getTimeDuration() >= MAX_WAITING_TIME)
            {
                if (debug_on)
                    cout << "frame " << id << " ack not received. Try to resend package.\n";
                frame_array[id].get()->setStatus(Status_Waiting);
                if (frame_array[id].get()->ResendToomuch())
                {
                    cerr << "Resend too many times. Link error.\n";
                    callStop(1);//link error, mac layer stops
                }
                else
                {
                    frame_array[id].get()->addResendtimes();
                }
            }
        }
        checkIdarray();
        this_thread::sleep_for(20ms);
    }
}

void
MAClayer::macperf_send() {
    const int perf_frame_len = 1024;
    MACframe perf_frame(dst_addr, src_addr, perf_frame_len);
    int8_t id = 0;
    while (!Mac_stop) {
        perf_frame.setFrameId(id++);
        Mac_sender.sendOnePacket(perf_frame_len, perf_frame.toBitStream());
        id = (id == 256) ? 0 : id;
        this_thread::sleep_for(100ms);
    }
}

//void
//MAClayer::send()
//{
//    readFromFile(Mac_num_frame);
//    for (int i = 0; i < Mac_num_frame; i++)
//    {
//        requestSend(data_frames[i]);
//    }
//    while (!Mac_stop)
//    {
//        bool if_done = false;
//        lock.enter();
//        if (frame_sent_num + 1 == Mac_num_frame) if_done = true;
//        lock.exit();
//
//        if (if_done && !getStop())
//        {
//            cout << "All frames sent.\n";
//            callStop();
//        }
//        if (!temp_ack_array.isEmpty())
//        {
//            send_id_array.insertArray(0,temp_ack_array.getRawDataPointer(), temp_ack_array.size());
//        }
//        for (auto i : send_id_array)
//        {
//            if (Mac_stop) break;
//            auto id = i;
//            if (frame_array[id].get()->getStatus() == Status_Waiting)
//            {
//                auto tmp = frame_array[id].get()->getFrame_size();
//                Mac_sender.sendOnePacket(frame_array[id].get()->getFrame_size() + FRAME_OFFSET, frame_array[id].get()->toBitStream());
//                
//                frame_array[id].get()->setSendTime();
//                if (frame_array[id].get()->getType() == TYPE_DATA)
//                {
//                    frame_array[id].get()->setStatus(Status_Sent);
//                    cout << "frame " << id << " sent.\n";
//                    this_thread::sleep_for(70ms);
//                    if (!keep_timer)
//                        startTimer(id);
//
//                }
//                else // ACK is defualt set as acked
//                {
//                    frame_array[id].get()->setStatus(Status_Acked);
//                    cout << "ack " << (int)frame_array[id].get()->getAck_id() << " sent.\n";
//                    this_thread::sleep_for(70ms);
//                }
//            }
//            else if (frame_array[id].get()->getStatus() == Status_Sent && frame_array[id].get()->getTimeDuration() >= MAX_WAITING_TIME)
//            {
//                cout << "frame " << id << " ack not received. Try to resend package.\n";
//                frame_array[id].get()->setStatus(Status_Waiting);
//                if (frame_array[id].get()->ResendToomuch())
//                {
//                    cerr << "Resend too many times. Link error.\n";
//                    callStop();//link error, mac layer stops
//                }
//                else
//                {
//                    frame_array[id].get()->addResendtimes();
//                }
//            }
//        }
//        checkIdarray();
//        this_thread::sleep_for(70ms);
//    }
//}

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
        else if (frame_array[head].get()->getStatus() == Status_Waiting)
        {
            frame_send_complete = false;
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
    cout << "sender start sending.\n";
    sending_thread = thread(&MAClayer::send, this);
    cout << "sending thread start.\n";
    //ack_sending_thread = thread(&MAClayer::sendAck, this);
    //cout << "ack sending thread start.\n";
    if (macping_on) {
        macping_thread = thread(&MAClayer::mac_ping, this);
        cout << "macping thread start.\n";
    }
    else if (is_icmp_sender) {
        icmp_thread = thread(&MAClayer::sendIcmpReq, this);
        cout << "icmp thread start.\n";
    }
    fout.open("OUTPUT.bin", ios::out | ios::binary);
}

void
MAClayer::StopMAClayer()
{

    if (!Mac_stop)
    {
        //Mac_sender.printOutput_buffer();
        Mac_stop = true;
        if (receive_thread.joinable()) receive_thread.join();
        cout << "receiving thread stop.\n";
        Mac_receiver.stopRecording();
        cout << "receiver stop recording.\n";
        if (sending_thread.joinable()) sending_thread.join();
        cout << "sending thread stop.\n";
        //if (ack_sending_thread.joinable()) ack_sending_thread.join();
        //cout << "ack sending thread stop.\n";
        if (macping_thread.joinable()) macping_thread.join();
        if (icmp_thread.joinable()) icmp_thread.join();
        cout << "macping thread stop,\n";
        fout.close();
    }
}

void
MAClayer::mac_ping()
{
    while (!Mac_stop)
    {
        requestSend(0, TYPE_MACPING_REQUEST);

        this_thread::sleep_for(1000ms);
        auto temp_ping_array = mac_ping_array;
        for (int i = 0; i < temp_ping_array.size(); i++)
        {
            int id = temp_ping_array[i];
            if (frame_array[id])
            {
                std::chrono::duration<double, std::milli> diff = std::chrono::system_clock::now() - frame_array[id].get()->getSendTime();
                if (diff.count() > 2000)
                {
                    mac_ping_array.removeFirstMatchingValue(id);
                    cout << "time out for mac ping " << id << "\n";
                }
            }
        }
    }
}

void
MAClayer::sendIcmpReq()
{
    int cnt = 0;
    while (cnt < 10 || frame_array.size() > 0) {
        if (cnt < 10) {
            this_thread::sleep_for(1000ms);
            requestSend(TYPE_ICMP_REQUEST, 0, dst_ip);
        }
        auto temp_icmp_array = icmp_array;
        for (int i = 0; i < temp_icmp_array.size(); i++)
        {
            int id = temp_icmp_array[i];
            if (frame_array[id])
            {
                std::chrono::duration<double, std::milli> diff = std::chrono::system_clock::now() - frame_array[id].get()->getSendTime();
                if (diff.count() > 2000)
                {
                    icmp_array.removeFirstMatchingValue(id);
                    cout << "time out for icmp " << id << "\n";
                }
            }
        }
        cnt++;
    }
}

void
MAClayer::sendAck()
{
    while (!Mac_stop)
    {
        if (!ack_queue.empty())
        {
            Mac_sender.sendOnePacket(ack_queue[0].get()->getFrame_size() + FRAME_OFFSET, ack_queue[0].get()->toBitStream());
            cout << "ACK " << (int)ack_queue[0].get()->getAck_id() << " sent.\n";
            ack_queue.erase(ack_queue.begin());
            this_thread::sleep_for(100ms);
        }
    }
}

void
MAClayer::Write2File(Array<int8_t> data, const string file_name)
{
    ofstream tmp_writer = std::ofstream(file_name, ios::out | ios::binary);
    tmp_writer.write((char*)data.getRawDataPointer(), data.size());
}

void
MAClayer::Write2File()
{
    for (auto data : file_output)
        fout.write((char*)data.getRawDataPointer(), data.size());
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


string
MAClayer::translateAddrPort(std::vector<int8_t> ip_port) {
    string ip_address = "";
    for (int i = 0; i < 4; i++) {
        ip_address += std::to_string((int)(unsigned char)ip_port[i]);
        if (i != 3)
            ip_address += ".";
    }
    return ip_address;
}


void
MAClayer::sendICMPreply()
{
    int8_t id = data_frames[0][0];
    std::vector<int8_t> data;
    data.assign(data_frames[0].begin() + 1, data_frames[0].end());
    string ip_address = translateAddrPort(data);
    requestSend(TYPE_ICMP_REPLY, id, ip_address);
}

void
MAClayer::sendFTPresponse()
{
    requestSend(TYPE_DATA, RESP, data_frames[0]);
}

bool
MAClayer::readFromFile(int num_frame, const string file_name) {
    data_frames.clear();
    data_frames.resize(num_frame);
    fstream test;
    test.open(getPath(file_name), ios::in | ios::binary);
    if (!test) return false; //read file failed!
    ifstream f(getPath(file_name), ios::in | ios::binary);
    //ofstream f1("test.out");
    char tmp;
    for (int i = 0; i < num_frame; i++) {
        if (if_send_ip) {
            for (int j = 0; j < 4; j++)
                data_frames[i].push_back(node1_addr[j]);
            for (int j = 0; j < 2; j++)
                data_frames[i].push_back(node1_port[j]);
        }
        while (f.get(tmp))
        {
            data_frames[i].push_back(tmp);
        }

    }
    /*for (int i = 0; i < data_frames[0].size(); i++) {
        f1 << (int)data_frames[0][i];
        if ((i + 1) % 8 == 0) {
            f1 << endl;
        }
    }
    f1.close();*/
    f.close();
    return true;
}

bool
MAClayer::readFromFile(const string file_name, bool if_read_all) {
    fstream test;
    test.open(getPath(file_name), ios::in | ios::binary);
    if (!test) return false; //read file failed!    
    if (if_read_all)
    {
        ifstream f(getPath(file_name), ios::in | ios::binary);
        f.seekg(0, ios::end);
        int file_length = f.tellg();
        f.seekg(0, ios::beg);
        int byte_num = (if_send_ip) ? (num_bits_per_frame - FRAME_OFFSET - IP_PORT_LEN - CRC_LEN) / 8 : (num_bits_per_frame - FRAME_OFFSET - CRC_LEN) / 8;
        Mac_num_frame = (int)(file_length / byte_num);
        data_frames.resize(Mac_num_frame + 1);
        for (int i = 0; i < Mac_num_frame; i++)
        {
            char tmp;
            for (int j = 0; j < byte_num; j++) {
                f.get(tmp);
                data_frames[i].push_back(tmp);
            }
        }
        if (file_length - Mac_num_frame * byte_num != 0)
        {
            char tmp;
            while (f.get(tmp))
            {
                data_frames[Mac_num_frame].push_back(tmp);
            }
            Mac_num_frame++;
        }
        return true;
    }
    else
    {
        data_frames.resize(cur_frame + 1);
        ifstream f(getPath(file_name), ios::in | ios::binary);
        //ofstream f1("test.out");
        char tmp;
        if (if_send_ip) {
            for (int j = 0; j < 4; j++)
                data_frames[cur_frame].push_back(node1_addr[j]);
            for (int j = 0; j < 2; j++)
                data_frames[cur_frame].push_back(node1_port[j]);
        }
        while (f.get(tmp))
        {
            data_frames[cur_frame].push_back(tmp);
        }
        cur_frame++;
        f.close();
        return true;
    }

}



// requestSend for ack packet
int
MAClayer::requestSend(int8_t data_frame_id) {
    const ScopedLock sl(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> ack_frame;
    ack_frame.reset(new MACframe(dst_addr, src_addr, data_frame_id));
    ack_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    //temp_ack_array.insert(-1, id);
    //ack_queue.push_back(std::move(ack_frame));
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
    data_frame.reset(new MACframe(id, dst_addr, src_addr, frame_data));
    //data_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    frame_array[id] = std::move(data_frame);
    return id;
}

// requestSend for macping packet
int
MAClayer::requestSend(int8_t request_id, int8_t type)
{
    const ScopedLock sl(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> macping_frame;
    macping_frame.reset(new MACframe(type, request_id, dst_addr, src_addr));
    macping_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    mac_ping_array.add(id);
    //temp_ack_array.insert(-1, id);
    //ack_queue.push_back(std::move(ack_frame));
    frame_array[id] = std::move(macping_frame);
    return id;
}

//requestSend for macperf packet
int
MAClayer::requestSend() {
    const ScopedLock sl(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> macperf_frame;
    macperf_frame.reset(new MACframe(dst_addr, src_addr, num_bits_per_frame - FRAME_OFFSET));
    macperf_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    frame_array[id] = std::move(macperf_frame);
    return id;
}

// requestSend for icmp packet
int
MAClayer::requestSend(int8_t type, int8_t icmp_id, std::string ip_address)
{
    const ScopedLock sl(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> icmp_frame;
    icmp_frame.reset(new MACframe(type, icmp_id, dst_addr, src_addr, ip_address));
    icmp_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    icmp_array.add(id);
    //temp_ack_array.insert(-1, id);
    //ack_queue.push_back(std::move(ack_frame));
    frame_array[id] = std::move(icmp_frame);
    return id;
}

//requestSend for ftp data
int
MAClayer::requestSend(int8_t type, int8_t cmd_type, std::vector<int8_t> data)
{
    const ScopedLock sl(lock);
    int id = id_controller_array.getFirst();
    id_controller_array.remove(0);
    unique_ptr<MACframe> ftp_frame;
    ftp_frame.reset(new MACframe(type, dst_addr, src_addr, cmd_type, data));
    ftp_frame->setFrameId(id);
    send_id_array.insert(-1, id);
    icmp_array.add(id);
    frame_array[id] = std::move(ftp_frame);
    return id;
}


int
MAClayer::requestSend(int data_id) {
    return requestSend(data_frames[data_id]);
}

void
MAClayer::callStop(bool identifier)
{
    if (!is_receiver) receive_end = true;
    if (!is_sender) send_end = true;
    if ((receive_end && send_end) || identifier == true) {
        if (!all_stop)
        {
            all_stop = true;
            if (!macperf_on)
                Write2File();
        }
    }
}


//void
//MAClayer::startTimer(int8_t data_frame_id) {
//    keep_timer = 1;
//    if (!timers.empty()) {
//        timers[0].join();
//        timers.erase(timers.begin());
//    }
//    thread timer(&MAClayer::wait, this, data_frame_id);
//    timers.push_back(std::move(timer));
//}
//
//void 
//MAClayer::wait(int8_t data_frame_id) {
//    unique_lock<mutex> lk(cv_m);
//    while (keep_timer)
//    {
//        auto now = std::chrono::system_clock::now();
//        if (cv.wait_until(lk, now + trans_timeout, [&]() {return frame_array[data_frame_id].get()->getStatus() == Status_Acked; })) {
//            keep_timer = 0;
//        }
//        else if (frame_array[data_frame_id].get()->ResendToomuch())
//        {
//            keep_timer = 0;
//            cerr << "Resend too many times. Link error.\n";
//            callStop(1);//link error, mac layer stops
//        }
//        else
//        {
//            if (debug_on)
//                cerr << "frame " << data_frame_id << "timeout. Try to resend package.\n";
//            frame_array[data_frame_id].get()->setStatus(Status_Waiting);
//            frame_array[data_frame_id].get()->addResendtimes();
//        }
//    }
//}
