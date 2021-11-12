#ifndef _MACLAYER_H_
#define _MACLAYER_H_


#include "receiver.h"
#include "sender.h"
#include "MACframe.h"
#include <thread>


class MAClayer : public AudioIODeviceCallback
{
public:
    MAClayer();

    void Write2File(Array<int8_t>& byte_data);//wait to be implemented.

    void receive(); //receiving datas

    void StartReceiving();

    void readFromFile(int num_frame);

    unique_ptr<MACframe> sendACK(int8_t frame_id); 

    unique_ptr<MACframe> sendData(int8_t frame_id);

    void requestSend(int8_t ack_id);

private:
    Receiver Mac_receiver;
    Sender Mac_sender;
    
    thread receive_thread;
    thread sending_thread;

    int max_rescend;
    std::chrono::milliseconds trans_timeout;

    unsigned int frame_size;
    int last_receive_id = -1;

    Array<Array<int8_t>> data_frames;
    int max_frame_gen_idx;

    // variables maintained for sender's sliding window
    int sender_LFS;
    int sender_LAR;
    int sender_SWS;
    Array<unique_ptr<MACframe>> sender_window;
};


#endif // !_MACLAYER_H_
