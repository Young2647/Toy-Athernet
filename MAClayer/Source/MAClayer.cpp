/*
  ==============================================================================

    MAClayer.cpp
    Created: 31 Oct 2021 11:03:53am
    Author:  A

  ==============================================================================
*/

#include "MAClayer.h"

MAClayer::MAClayer()
{

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

    }

}

void 
MAClayer::StartReceiving()
{
    
}