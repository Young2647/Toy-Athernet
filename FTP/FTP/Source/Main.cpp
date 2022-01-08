/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#include "sender.h"
#include "receiver.h"
#include "defines.h"
#include "MACframe.h"
#include "MAClayer.h"
#include "FTP.h"
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
    int mode = MODE_FTP_NODE1;
    if (mode == MODE_UDP_NODE2_SEND)
    {
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
        int num_bits_per_frame = 408; // 51 bytes
        int num_frame = 30; //30 frames
        std::unique_ptr<MAClayer> mac_layer;
        if (mac_layer.get() == nullptr)
        {
            mac_layer.reset(new MAClayer(3, num_bits_per_frame, num_frame, YHD, ZYB, 20));
        }
        //mac_layer.get()->setSender();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        int notify_num = 0;
        std::cout << "Press any ENTER to stop MAClayer.\n";
        while (!mac_layer.get()->getStop())
        {
            std::fstream notify_file;
            notify_file.open("WRITE_DOWN.txt", ios::in);
            while (!notify_file && !mac_layer.get()->getStop())
            {
                notify_file.open("WRITE_DOWN.txt", ios::in);
                if (kbhit()) break;
            }
            if (notify_file)
            {
                notify_file.close();
                system("del WRITE_DOWN.txt");
                notify_num++;
                cout << "Get notified by python\n";
            }
            mac_layer.get()->readFromFile("input.bin");
            if (notify_num == 1)
            {
                mac_layer.get()->requestSend(0);
                mac_layer.get()->setSender();
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
    else if (mode == MODE_UDP_NODE1_RECEIVE) {
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
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
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
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
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        std::cout << "Transmit time : " << duration << "ms.\n";
        dev_manager.removeAudioCallback(mac_layer.get());
        DeletedAtShutdown::deleteAll();
        juce::MessageManager::deleteInstance();

        return 0;
    }
    else if (mode == MODE_UDP_NODE1_SEND)
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
        mac_layer.get()->setSendIP();
        mac_layer.get()->setSender();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->readFromFile(num_frame, "input.txt");
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
            bool imm_stop = false;
            while (!notify_file)
            {
                notify_file.open("WRITE_DOWN.txt", ios::in);
                if (kbhit())
                {
                    imm_stop = true;
                    break;
                }
            }
            if (imm_stop)
            {
                mac_layer.get()->callStop(1);
                break;
            }
            if (notify_file)
            {
                notify_file.close();
                system("del WRITE_DOWN.txt");
            }
            //c++ get notified, send reply to node1
            if (mac_layer.get()->readFromFile(1, "reply.txt"))
            {
                mac_layer.get()->sendICMPreply();
            }
            this_thread::sleep_for(200ms);
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
    else if (mode == MODE_FTP_NODE1)
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
        mac_layer.get()->setSendIP();
        dev_manager.addAudioCallback(mac_layer.get());
        mac_layer.get()->StartMAClayer();
        auto start_time = std::chrono::system_clock::now();
        std::cout << "Press any ENTER to stop MAClayer.\n";
        string dst_addr = "ftp.ncnu.edu.tw";

        while (!mac_layer.get()->getStop())
        {
            //input command
            std::string cmd;
            getline(cin, cmd);
            if (cmd == "") mac_layer.get()->callStop(1);
            std::vector<uint8_t> cmd_data;
            COMMMAND instruct_cmd = ParseCmd(cmd, cmd_data);
            if (instruct_cmd != WRNG) // valid command
            {
                if (instruct_cmd == RETR)
                {
                    istringstream in(cmd);
                    vector<string> temp;
                    string t;
                    while (getline(in, t, ' '))
                    {
                        temp.push_back(t);
                    }
                    if (temp.size() > 2)
                    {
                        mac_layer.get()->setRETRfilename(temp[2]);
                    }
                    else
                    {
                        mac_layer.get()->setRETRfilename(temp[1]);
                    }
                }
                mac_layer.get()->requestSend(TYPE_FTP_COMMAND, instruct_cmd, cmd_data); // send command to NAT node
            }

            
            this_thread::sleep_for(200ms);
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
    else if (mode == MODE_FTP_NODE2)
    {
        std::cout << "Press any ENTER to start MAClayer.\n";
        getchar();
        std::unique_ptr<MAClayer> mac_layer;
        int num_bits_per_frame = 448; // 56 bytes
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
            std::fstream notify_file;
            notify_file.open("WRITE_DOWN.txt", ios::in);
            cout << "Waiting for python to notify...\n";
            bool imm_stop = false;
            while (!notify_file)
            {
                notify_file.open("WRITE_DOWN.txt", ios::in);
                if (kbhit())
                {
                    imm_stop = true;
                    break;
                }
            }
            if (imm_stop)
            {
                mac_layer.get()->callStop(1);
                break;
            }
            if (notify_file)
            {
                notify_file.close();
                system("del WRITE_DOWN.txt");
            }
            //c++ get notified, send reply to node1
            if (mac_layer.get()->readFromFile("response.bin", true))
            {
                mac_layer.get()->requestSend(0);
                //mac_layer.get()->setSender();
                system("del response.bin");
            }
            this_thread::sleep_for(200ms);
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
