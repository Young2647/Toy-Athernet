/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <fstream>
#include <conio.h>
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

    int num_bits_per_frame = 440;

    std::unique_ptr<MAClayer> mac_layer;
    if (mac_layer.get() == nullptr)
    {
        mac_layer.reset(new MAClayer(3, num_bits_per_frame, 63, YHD, ZYB, 20));
    }

    std::cout << "Press any ENTER to start MAClayer.\n";
    getchar();
    dev_manager.addAudioCallback(mac_layer.get());
    mac_layer.get()->StartMAClayer();
    auto start_time = std::chrono::system_clock::now();
    std::cout << "Press any ENTER to stop MAClayer.\n";
    while (!mac_layer.get()->getStop())
    {
        int init_sent_num = mac_layer.get()->getSentframeNum();
        this_thread::sleep_for(1000ms);
        int curr_sent_num = mac_layer.get()->getSentframeNum();
        cout << "kbps = " << (float) (curr_sent_num - init_sent_num) * num_bits_per_frame / (float) 1000 << "kb/s.\n";
        if (kbhit()) mac_layer.get()->callStop();
    }
    mac_layer.get()->StopMAClayer();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
    std::cout << "Transmit time : " << duration << "ms.\n";
    dev_manager.removeAudioCallback(mac_layer.get());
    DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();

    return 0;
}
