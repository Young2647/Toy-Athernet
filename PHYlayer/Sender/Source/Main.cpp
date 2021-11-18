//
//  main.cpp
//  cs120_pj1_source
//
//  Created by ’‘ﬁ»≤© on 2021/10/14.
//

#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <JuceHeader.h>
#define PI acos(-1)
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


Sender::Sender() {
    len_warm_up = 480;
    header_len = 480;
    sample_rate = 48000;
    carrier_freq = 5000;
    carrier_phase = 0;
    carrier_amp = 1;
    num_bits_per_frame = 100;
    num_samples_per_bit = 48;
    len_zeros = 20;
    num_frame = 100;
    for (size_t i = 0; i < num_bits_per_frame * num_samples_per_bit; i++)
        frame_wave.push_back(0);
    GenerateCarrierWave();
    isPlaying = false;
}

void Sender::setHeaderLen(int len) {
    header_len = len;
}

void Sender::setCarrierFreq(int freq) {
    carrier_freq = freq;
}

int** Sender::getBitStream() {
    int** frame_bit = new int* [num_frame];
    ifstream f;
    char tmp;
    f.open("C:\\CS120\\CS120-Shanghaitech-Fall2021\\input.in");
    for (int i = 0; i < num_frame; i++) {
        frame_bit[i] = new int[num_bits_per_frame];
        for (int j = 0; j < num_bits_per_frame; j++) {
            f >> tmp;
            frame_bit[i][j] = (int)tmp - 48;
            //frame_bit[i][j] = 1;
        }
    }
    f.close();
    return frame_bit;
}


void Sender::GenerateCarrierWave() {

    for (int j = 0; j < num_samples_per_bit; j++)
    {
        carrier_wave.add(carrier_amp * cos(j * 2 * PI * ((float)carrier_freq / (float)sample_rate) + carrier_phase));
    }
}

float* Sender::generateHeader() {
    int start_freq = 2000;
    int end_freq = 10000;
    float freq_step = (end_freq - start_freq) / (header_len / 2);
    float time_gap = (float)1 / (float)sample_rate;
    vector<float> fp;
    vector<float> omega;
    for (int i = 0; i < header_len; i++)
    {
        fp.push_back(0);
        omega.push_back(0);
    }
    float* header_stack = new float[header_len];
    fp[0] = start_freq;
    fp[header_len / 2] = end_freq;
    for (int i = 1; i < header_len / 2; i++)
        fp[i] = fp[i - 1] + freq_step;
    for (int i = header_len / 2 + 1; i < header_len; i++)
        fp[i] = fp[i - 1] - freq_step;
    for (int i = 1; i < header_len; i++)
        omega[i] = omega[i - 1] + (fp[i] + fp[i - 1]) / 2.0 * time_gap;
    for (int i = 0; i < header_len; i++)
        header_stack[i] = sin(2 * PI * omega[i]);
    return header_stack;
}


void Sender::Modulation(int* frame_bit) {
    for (int i = 0; i < num_bits_per_frame * num_samples_per_bit; i++)
        frame_wave[i] = 0;
    for (int i = 0; i < num_bits_per_frame; i++) {
        for (int j = 0; j < num_samples_per_bit; j++)
            frame_wave[i * num_samples_per_bit + j] = (frame_bit[i] * 2 - 1) * carrier_wave[j];
    }
}


void Sender::send() {
    float* header = generateHeader();
    int** frame_bit = getBitStream();
    int len_frame = header_len + num_samples_per_bit * num_bits_per_frame + len_zeros;
    int len_buffer = num_frame*len_frame;
    output_buffer.setSize(1, len_buffer + 480 + len_warm_up);
    output_buffer.clear();
    for (int j = 0; j < 480; j++)
        output_buffer.setSample(0, j, carrier_wave[j % num_samples_per_bit]);
    for (int i = 0; i < num_frame; i++) {
        Modulation(frame_bit[i]);
        for (int j = 0; j < header_len; j++)
            output_buffer.setSample(0, len_warm_up + i * len_frame + j, header[j]);
        for (int j = 0; j < num_samples_per_bit * num_bits_per_frame; j++)
            output_buffer.setSample(0, len_warm_up + i * len_frame + header_len + j, frame_wave[j]);
        for (int j = 0; j < len_zeros; j++)
            output_buffer.setSample(0, len_warm_up + i * len_frame + header_len + num_samples_per_bit * num_bits_per_frame + j, 0);
    }
    //ofstream of;
    //of.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021-main\\Sender\\Builds\\VisualStudio2019\\out.out", ios::trunc);
    //for (int i = 0; i < output_buffer.getNumSamples(); i++) {
    //    if (of.is_open()) {
    //        of << output_buffer.getSample(0, i) << endl;
    //    }
    //}
    //of.close();
}

void Sender::BeginSend()
{

    const ScopedLock sl(lock);
    playingSampleNum = 0;
    isPlaying = true;
    send();
    startTimer(50);
}


void Sender::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels, float** outputChannelData, int numOutputChannels, int numSamples) {
    const ScopedLock sl(lock);

    auto* playBuffer = output_buffer.getReadPointer(0);
    /*
    // We need to clear the output buffers, in case they're full of junk..
    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear(outputChannelData[i], numSamples);
     */
    // Generate Sine Wave Data
   
    for (int i = 0; i < numSamples; i++)
    {
        for (auto j = numOutputChannels; --j >= 0;)
        {

            if (outputChannelData[j] != nullptr)
            {
                // Write the sample into the output channel
                outputChannelData[j][i] = (playingSampleNum < output_buffer.getNumSamples()) ? playBuffer[playingSampleNum] : 0.0f;
                ++playingSampleNum;
            }
        }
    }
}


int main(int argc, const char* argv[]) {
    
    MessageManager::getInstance();
    /* Initialize Player */
    AudioDeviceManager dev_manager;
    dev_manager.initialiseWithDefaultDevices(1, 1);
    AudioDeviceManager::AudioDeviceSetup dev_info;
    dev_info = dev_manager.getAudioDeviceSetup();
    dev_info.sampleRate = 48000; // Setup sample rate to 48000 Hz
    dev_manager.setAudioDeviceSetup(dev_info, false);

    unique_ptr<Sender> sender;
    sender.reset(new Sender());
    std::cout << "Press any ENTER to start transmitting.\n";
    getchar();
    getchar();
    dev_manager.addAudioCallback(sender.get());
    sender->BeginSend();
    while (sender->isPlaying)
    {

    }
    std::cout << "Transmitting stopped.\n";
    dev_manager.removeAudioCallback(sender.get());
    juce::DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();
}
