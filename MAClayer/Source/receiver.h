#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <JuceHeader.h>
#include <vector>
#include<fstream>
#include "defines.h"
using namespace juce;



class Receiver : public AudioIODeviceCallback
{
public:
    Receiver();
    
    Receiver(int bitlen, int packlen);

    void GenerateCarrierWave();

    void GenerateHeader();

    void audioDeviceAboutToStart(AudioIODevice* device) override;

    void audioDeviceStopped() override {}

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples) override;

    void startRecording();

    void stopRecording();

    Array<int8_t> Int2Byte(Array<int>& int_data);

    int Demodulate(float sample);

    Array<int8_t> getData();

    void clearFrameData() { frame_data.clear(); }

    bool isRecording;
private:

    Array<float> processingHeader;
    Array<float> processingData;
    Array<float> syncHeader;
    Array<float> carrierWave;
    Array<int8_t> frame_data;
    std::vector<float> recordedSound;
    CriticalSection lock;
    
    int recordedSampleNum = -1;
    int bitLen = 48; //the length of one bit
    int packLen = 100; // how many bits per frame
    int headerLength = 480;
    int sampleRate;
    float syncPower_localMax;
    float power_;
    int state; // processing state
    int data_state;
    std::ofstream fout;
    std::ofstream of;
    bool _ifheadercheck = false;
    Array<float> tempBuffer;
    Array<int> int_data;
};

#endif