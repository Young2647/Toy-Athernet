#ifndef _MACLAYER_H_
#define _MACLAYER_H_


#include "receiver.h"
#include "sender.h"
#include "MACframe.h"
#include <fstream>
#include <thread>
#include <vector>

class MAClayer : public AudioIODeviceCallback
{
public:
    MAClayer(int num_samples_per_bit, int num_bits_per_frame, int num_frame, int8_t src_addr, int8_t dst_addr, int window_size = DEAFULT_WINDOW_SIZE);
    
    ~MAClayer();
    
    void Write2File();
    void Write2File(int8_t id, Array<int8_t> data, const string file_name);
    void receive(); //receiving datas

    void send(); //sending data

    void macperf_send(); // sending macperf frame

    void sendAck();//sending ack

    void mac_ping(); // send macping request

    void sendIcmpReq(); // send icmp request

    void checkIdarray();

    void StartMAClayer();

    void StopMAClayer();

    bool readFromFile(int num_frame, const string file_name);

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples);

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}

    void audioDeviceStopped() {}

    int requestSend(int8_t ack_id);
    int requestSend(std::vector<int8_t> frame_data);
    int requestSend(int8_t request_id, int8_t type);
    int requestSend();
    int requestSend(int i); //send the ith element in data_frame
    int requestSend(int8_t type, int8_t icmp_id, std::string ip_address);
    void startTimer(int8_t frame_data_id);
    void wait(int8_t data_frame_id);
    bool getIfPerfing() { return macperf_on; }

    bool getStop() { return all_stop; }
    void callStop(bool identifier);

    int getSentframeNum() { return frame_sent_num; }
    void setReceiver() { is_receiver = true; is_sender = false; }
    void setSender() { is_receiver = false; is_sender = true; }
    void setSendIP() { if_send_ip = true; }
    void setICMPsender() { is_icmp_sender = true; }
    void setICMPreceiver() { is_icmp_receiver = true; }
    void setDstIP(string dst_addr) { dst_ip = dst_addr; }
private:
    Receiver Mac_receiver;
    Sender Mac_sender;
    
    thread receive_thread;
    thread sending_thread;
    //thread ack_sending_thread;
    thread macping_thread;
    thread icmp_thread;

    int max_rescend;
    std::chrono::milliseconds trans_timeout;

    unsigned int frame_size;

    int max_frame_gen_idx;

    // variables maintained for sender's sliding window
    int sender_LFS;
    int sender_LAR;
    int sender_SWS;

    // receiver's sliding window
    int receiver_LFR;
    int receiver_LAF;
    int receiver_RWS;

    //the num of frames that have been successfully sent.
    int frame_sent_num = 0;
    int frame_receive_num = 0;
    vector<int> frame_to_receive_list;
    bool frame_receive_increase = 0;
    
    int Mac_num_frame; //total frame num to send
    int Mac_num_receive_frame; // total frame num to receive
    int num_bits_per_frame; // every frame bits num
    int num_samples_per_bit; // every bit sample num
    bool Mac_stop;
    bool keep_timer;
    std::ofstream fout;
    std::condition_variable cv;
    std::vector<unique_ptr<MACframe>> frame_array;
    std::vector<std::unique_ptr<MACframe>> ack_queue; //queue to send ack
    std::vector<std::vector<int8_t>> data_frames;
    
    Array<bool> ack_array;//array that record if ack is received
    std::vector<Array<int8_t>> file_output;
    Array<int> send_id_array;
    Array<int> id_controller_array;
    Array<int> mac_ping_array; // array that record the buffer for mac ping
    Array<int> icmp_array; // array that record the buffer of icmp
    std::vector<thread> timers;
    STATE state;
    CriticalSection lock;
    mutex cv_m;
    
    bool all_stop = false;
    CRC8 crc;

    int all_byte_num;
    Array<int> temp_ack_array;

    int8_t dst_addr;
    int8_t src_addr;

    bool csma_on = false;//if we have csma
    std::chrono::milliseconds back_off_time = 10ms;

    bool macperf_on = false;
    bool macping_on = false;
    bool icmp_on = false;
    bool debug_on = true;
    bool is_receiver = false;
    bool is_sender = false;
    bool is_icmp_sender = false;
    bool is_icmp_receiver = false;
    int throughput;
    int acked_list;
    int window_size;
    bool receive_end = false;
    bool send_end = false;

    //ip and port for atherNode
    bool if_send_ip = false;
    string dst_ip;
    string ip;
    int port;
    int8_t node1_addr[4] = { (int8_t)192, (int8_t)168, (int8_t)1, (int8_t)2 };
    int8_t node1_port[2] = { (int8_t)91, (int8_t)38 };

    std::chrono::system_clock::time_point test_time;
};


#endif // !_MACLAYER_H_
