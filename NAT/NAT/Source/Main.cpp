/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <fstream>
#include <conio.h>
#include <stdlib.h>
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
    int mode = MODE_ICMP_NODE1;
    if (mode == MODE_UDP_NODE2_SEND)
    {
        std::fstream notify_file;
        notify_file.open("WRITE_DOWN.txt", ios::in);
        cout << "Waiting for python to notify...\n";
        while (!notify_file)
        {
            notify_file.open("WRITE_DOWN.txt", ios::in);
        }
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        std::unique_ptr<MAClayer> mac_layer;
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, YHD, ZYB, 20));
        }
        mac_layer.get()->setSender();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_UDP_NODE1_RECEIVE) {
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, ZYB, YHD, 20));
        }
        mac_layer.get()->setReceiver();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_UDP_NODE2_RECEIVE)
    {
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, YHD, ZYB, 20));
        }
        mac_layer.get()->setReceiver();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        fstream notify_file("NOTIFY_DONE.txt"); //notify python node 
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_UDP_NODE1_SEND)
    {
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, ZYB, YHD, 20));
        }
        mac_layer.get()->setSendIP();
        mac_layer.get()->setSender();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_ICMP_NODE1)
    {
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, ZYB, YHD, 20));
        }
        //mac_layer.get()->setSendIP();
        //mac_layer.get()->setSender();
        mac_layer.get()->setICMPsender();
        string dst_addr = "10.20.198.211";
        mac_layer.get()->setDstIP(dst_addr);
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_ICMP_NODE2)
    {
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, YHD, ZYB, 20));
        }
        //mac_layer.get()->setSendIP();
        //mac_layer.get()->setSender();
        mac_layer.get()->setReceiver();
        mac_layer.get()->setICMPreceiver();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            std::fstream notify_file;
            notify_file.open("WRITE_DOWN.txt", ios::in);
            cout << "Waiting for python to notify...\n";
            while (!notify_file)
            {
                notify_file.open("WRITE_DOWN.txt", ios::in);
                if (kbhit()) break;
            }
            //c++ get notified, send reply to node1
            if(mac_layer.get()->readFromFile(1, "reply.txt"))
                mac_layer.get()->requestSend(0);
            if (kbhit()) mac_layer.get()->callStop(1);
        }
        mac_layer.get()->StopMAClayer();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
        
    }

    int num_bits_per_frame = 840;

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
        if (mac_layer.get()->getIfPerfing()) {
            int init_sent_num = mac_layer.get()->getSentframeNum();
            this_thread::sleep_for(1000ms);
            int curr_sent_num = mac_layer.get()->getSentframeNum();
            cout << "kbps = " << (float)(curr_sent_num - init_sent_num) * num_bits_per_frame / (float)1000 << "kb/s.\n";
        }
        if (kbhit()) mac_layer.get()->callStop(1);
    }
    mac_layer.get()->StopMAClayer();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
    std::cout << "Transmit time : " << duration << "ms.\n";
    dev_manager.removeAudioCallback(mac_layer.get());
    DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();

    return 0;
}
