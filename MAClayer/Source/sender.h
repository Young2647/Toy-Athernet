#ifndef _SENDER_H_
#define _SENDER_H_

#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <JuceHeader.h>

#ifndef PI
#define PI acos(-1)
#endif // !PI

using namespace std;
using namespace juce;

class Sender : public AudioIODeviceCallback, private HighResolutionTimer {
public:
    Sender();
    void setHeaderLen(int len);
    void setCarrierFreq(int freq);
    int** getBitStream();
    float* generateHeader();
    void Modulation(int* frame_bit);
    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples);
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override {}
    void audioDeviceStopped() {}
    void GenerateCarrierWave();
    void send();
    void BeginSend();
    void hiResTimerCallback() override
    {
        if (isPlaying && playingSampleNum >= output_buffer.getNumSamples())
        {
            isPlaying = false;
            stopTimer();

        }
    }
    bool isPlaying;

private:
    int header_len;
    int sample_rate;
    int carrier_freq;
    int carrier_amp;
    int carrier_phase;
    int num_bits_per_frame;
    int num_samples_per_bit;
    int playingSampleNum;
    int num_frame;
    int len_zeros;
    int len_warm_up;
    CriticalSection lock;
    vector<float> frame_wave;
    AudioBuffer<float> output_buffer;
    Array<float> carrier_wave;
    //    AudioDeviceManager audioDeviceManager;
    //    unique_ptr<audioDevice> device;
};


#endif