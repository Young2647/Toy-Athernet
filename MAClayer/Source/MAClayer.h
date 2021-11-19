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
    MAClayer(int num_samples_per_bit, int num_bits_per_frame, int num_frame);
    
    ~MAClayer();
    
    void Write2File();

    void receive(); //receiving datas

    void send(); //sending data

    void checkIdarray();

    void StartMAClayer();

    void StopMAClayer();

    void readFromFile(int num_frame);

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples);

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}

    void audioDeviceStopped() {}

    int requestSend(int8_t ack_id);
    int requestSend(std::vector<int8_t> frame_data);

    void startTimer(int8_t frame_data_id);
    void wait(int8_t data_frame_id);

    bool getStop() { return all_stop; }
    void callStop();
private:
    Receiver Mac_receiver;
    Sender Mac_sender;
    
    thread receive_thread;
    thread sending_thread;

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
    vector<int> frame_to_receive_list;
    int frame_receive_num = 0;
    
    int Mac_num_frame; //total frame num
    int num_bits_per_frame; // every frame bits num
    int num_samples_per_bit; // every bit sample num
    bool Mac_stop;
    bool keep_timer;
    std::ofstream fout;
    std::condition_variable cv;
    std::vector<unique_ptr<MACframe>> frame_array;
    std::vector<std::vector<int8_t>> data_frames;
    
    Array<bool> ack_array;//array that record if ack is received
    std::vector<Array<int8_t>> file_output;
    Array<int> send_id_array;
    Array<int> id_controller_array;
    std::vector<thread> timers;
    STATE state;
    CriticalSection lock;
    mutex cv_m;
    
    bool all_stop = false;
    CRC8 crc;

    int all_byte_num;
};


#endif // !_MACLAYER_H_
