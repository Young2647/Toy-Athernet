#ifndef _MACLAYER_H_
#define _MACLAYER_H_


#include "receiver.h"
#include "sender.h"
#include "MACframe.h"

class MAClayer : public AudioIODeviceCallback
{
public:
    MAClayer();

    void Write2File(Array<int8_t>& byte_data);//wait to be implemented.

    void receive(); //receiving datas

    void sendACK(int8_t frame_id); //wait to be implemented.

private:
    Receiver Mac_receiver;
    Sender Mac_sender;

    unsigned int frame_size;
    int last_receive_id = -1;
};

#endif // !_MACLAYER_H_
