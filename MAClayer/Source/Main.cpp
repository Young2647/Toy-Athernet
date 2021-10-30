/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include<fstream>
#include "sender.h"
#include "receiver.h"
#define PI acos(-1)
using namespace juce;
using namespace std;



#define TYPE_ACK 0
#define TYPE_DATA 1

class MAClayer : public AudioIODeviceCallback  
{
public :
    MAClayer() {

    };

    void Write2File(Array<int8_t> & byte_data) {
        ///wait to be implemented.
    }

    void receive() {
        AudioBuffer<float> tempbuffer;
        Array<int8_t> data = Mac_receiver.Demodulate(tempbuffer);
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

    void sendACK(int8_t frame_id) {

    }

private :
    Receiver Mac_receiver;
    Sender Mac_sender;

    unsigned int frame_size;
    int last_receive_id = -1;
};



/// <summary>
/// A frame contains a header, a type, a frame_id, data
/// </summary>
class MACframe {
public :
    MACframe(Array<int8_t> all_data) {
        type = all_data[0];
        frame_id = all_data[1];
        for (int i = 2; i < all_data.size(); i++)
        {
            data.add(all_data[i]);
        }
    }

    MACframe(int8_t frame_id) {

    }

    int8_t getType() { return type; }
    int8_t getFrame_id() { return frame_id; }
    Array<int8_t> getData() { return data; }
private :
    Array<float> header;
    int8_t type;
    int8_t frame_id;
    Array<int8_t> data;
};





//====================================main func==========================================
int main(int argc, char* argv[])
{

    MessageManager::getInstance();
    //Initialize Player 
    AudioDeviceManager dev_manager;
    dev_manager.initialiseWithDefaultDevices(1, 1);
    AudioDeviceManager::AudioDeviceSetup dev_info;
    dev_info = dev_manager.getAudioDeviceSetup();
    dev_info.sampleRate = 48000; // Setup sample rate to 48000 Hz
    dev_manager.setAudioDeviceSetup(dev_info, false);


    std::unique_ptr<Receiver> receiver;
    if (receiver.get() == nullptr)
    {
        receiver.reset(new Receiver());
    }

    std::cout << "Press any ENTER to start recording.\n";
    getchar();
    getchar();
    if (!(*receiver).isRecording)
    {
        dev_manager.addAudioCallback(receiver.get());
        (*receiver).startRecording();
    }

    std::cout << "Press any ENTER to stop recording.\n";
    getchar();

    (*receiver).stopRecording();


    //sender->send();
    /*
    std::ifstream temp("C:\\CS120\\CS120-Shanghaitech-Fall2021-main\\out.out");
    AudioBuffer<float> tempBuffer;
    tempBuffer.setSize(1, 100000);
    auto* s = tempBuffer.getWritePointer(0);
    char tmp;
    for (int i = 0; i < 58560; i++)
    {
        temp >> s[i];
    }
    */
    //receiver->Demodulate(sender->output_buffer);
    //(*receiver).WritetoFile();
    dev_manager.removeAudioCallback(receiver.get());
    DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();
    // ..your code goes here!


    return 0;
}
