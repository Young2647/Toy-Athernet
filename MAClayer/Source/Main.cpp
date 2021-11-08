/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include<fstream>
#include "sender.h"
#include "receiver.h"
#include "defines.h"
#include "MACframe.h"
#include "MAClayer.h"
using namespace juce;
using namespace std;




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
