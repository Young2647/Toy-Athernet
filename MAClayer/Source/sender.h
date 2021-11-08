#ifndef _SENDER_H_
#define _SENDER_H_

#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <JuceHeader.h>
#include "defines.h"

using namespace std;
using namespace juce;

class Sender : public AudioIODeviceCallback, private HighResolutionTimer {
public:
    Sender();
    void setHeaderLen(int len);

    void setCarrierFreq(int freq);

    int** getBitStream();

    Array<float> generateHeader();

    void Modulation(int* frame_bit);

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples);

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}

    void audioDeviceStopped() {}

    void GenerateCarrierWave();

    void send();

    int beginSend();

    void hiResTimerCallback() override
    {
        if (isPlaying && playingSampleNum >= output_buffer.getNumSamples())
        {
            isPlaying = false;
            stopTimer();

        }
    }

    

private:
    int header_len;
    int sample_rate;
    int carrier_freq;
    int carrier_amp;
    int carrier_phase;
    int playingSampleNum;
    int num_frame;
    int len_zeros;
    int len_warm_up;
    int len_frame;
    int output_buffer_idx;
    CriticalSection lock;
    Array<float> frame_wave;
    AudioBuffer<float> output_buffer;
    Array<float> carrier_wave;
    Array<float> header_wave;
    bool isPlaying;
    //    AudioDeviceManager audioDeviceManager;
    //    unique_ptr<audioDevice> device;
};


#endif