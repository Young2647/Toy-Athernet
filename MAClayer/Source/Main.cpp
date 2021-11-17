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


    std::unique_ptr<MAClayer> mac_layer;
    if (mac_layer.get() == nullptr)
    {
        mac_layer.reset(new MAClayer(48, 416, 2));
    }

    std::cout << "Press any ENTER to start MAClayer.\n";
    getchar();
    dev_manager.addAudioCallback(mac_layer.get());
    mac_layer.get()->StartMAClayer();

    std::cout << "Press any ENTER to stop MAClayer.\n";
    getchar();
    mac_layer.get()->StopMAClayer();

    dev_manager.removeAudioCallback(mac_layer.get());
    DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();

    return 0;
}
