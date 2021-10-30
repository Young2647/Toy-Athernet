/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include<fstream>

#define PI acos(-1)
using namespace juce;


class Receiver : public AudioIODeviceCallback
{
public:
    Receiver()
    {
        syncPower_localMax = 0;
        isRecording = false;
    }

    void GenerateCarrierWave()
    {
        int carrierAmp = 1;
        int carrierFreq = 5000;

        for (int j = 0; j < bitLen; j++)
        {
            carrierWave.add(carrierAmp * cos(2 * PI * carrierFreq * j / sampleRate));
        }

    }

    void GenerateHeader() {
        int start_freq = 2000;
        int end_freq = 10000;
        float freq_step = (end_freq - start_freq) / (headerLength / 2);
        float time_gap = (float)1 / (float)sampleRate;
        std::vector<float> fp;
        std::vector<float> omega;
        for (int i = 0; i < headerLength; i++)
        {
            fp.push_back(0);
            omega.push_back(0);
        }
        float* header_stack = new float[headerLength];
        fp[0] = start_freq;
        fp[headerLength / 2] = end_freq;
        for (int i = 1; i < headerLength / 2; i++)
            fp[i] = fp[i - 1] + freq_step;
        for (int i = headerLength / 2 + 1; i < headerLength; i++)
            fp[i] = fp[i - 1] - freq_step;
        for (int i = 1; i < headerLength; i++)
            omega[i] = omega[i - 1] + (fp[i] + fp[i - 1]) / 2.0 * time_gap;
        for (int i = 0; i < headerLength; i++)
            header_stack[i] = sin(2 * PI * omega[i]);
        syncHeader = Array<float>(header_stack, headerLength);
    }

    void audioDeviceAboutToStart(AudioIODevice* device) override
    {
        isRecording = false;
        recordedSampleNum = 0;

        sampleRate = 48000;

        recordedSound.clear();

        GenerateCarrierWave();
        GenerateHeader();
    }

    void audioDeviceStopped() override {}

    void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels, int numSamples) override
    {
        const ScopedLock sl(lock);
        if (isRecording)
        {
            auto* recordingBuffer = recordedSound.getWritePointer(0);
            for (int i = 0; i < numSamples; i++)
            {
                if (recordedSampleNum < recordedSound.getNumSamples())
                {
                    auto inputSamp = 0.0f;
                    for (auto j = numInputChannels; --j >= 0;)
                        if (inputChannelData[j] != nullptr)
                            inputSamp += inputChannelData[j][i];
                    recordingBuffer[recordedSampleNum] = inputSamp;
                    recordedSampleNum++;
                }
            }
        }

        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != nullptr)
                zeromem(outputChannelData[i], (size_t)numSamples * sizeof(float));
    }

    void startRecording()
    {

        const ScopedLock sl(lock);
        recordedSound.clear();
        recordedSampleNum = 0;

        recordedSound.setSize(1, 15 * sampleRate);
        isRecording = true;
    }

    void stopRecording()
    {
        if (isRecording)
        {
            isRecording = false;

            Demodulate(recordedSound);
        }
    }




    void Demodulate(AudioBuffer<float>& buffer)
    {
        int headerPos = 0;
        int state = 0; // 0 = sync, 1 = decode
        auto* s = buffer.getReadPointer(0);

        Array<float> tempBuffer;
        float power = 0;
        //std::ofstream debugf("synpower.txt");
        //std::ofstream recordeddebug("record.txt");
        //std::ofstream datadebug("data.txt");
        std::ofstream fout("record.out");
        for (int i = 0; i < recordedSampleNum; i++)
            //for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            //recordeddebug << s[i] << "\n";
            power = power * (1 - 1 / 64) + s[i] * s[i] / 64;
            if (processingHeader.size() < headerLength)
            {
                processingHeader.add(s[i]);
                continue;
            }
            else
            {
                if (state == 0) // sync process
                {
                    processingHeader.add(s[i]);
                    if (processingHeader.size() > headerLength)
                    {
                        processingHeader.removeRange(0, 1);
                    }
                    float syncPower = 0;
                    for (int j = 0; j < headerLength; j++)
                    {
                        syncPower += syncHeader[j] * processingHeader[j];
                    }
                    //debugf << syncPower << "\n";
                    if (syncPower > syncPower_localMax && syncPower > 0.5)
                    {
                        syncPower_localMax = syncPower;
                        headerPos = i;
                        tempBuffer.clear();
                    }
                    else if (headerPos != 0)
                    {
                        tempBuffer.add(s[i]);
                        //recordeddebug << s[i] << "\n";
                        if (i > headerPos + 500)
                        {
                            std::cout << "header found at " << headerPos << std::endl;
                            syncPower_localMax = 0;
                            state = 1;
                            processingData = tempBuffer;
                        }

                    }
                }
                else if (state == 1) //data process
                {
                    processingData.add(s[i]);
                    if (processingData.size() == bitLen * packLen)
                    {
                        for (int j = 0; j < packLen; j++)
                        {
                            float sum = 0;
                            for (int k = 0; k < bitLen; k++)
                            {
                                int temp = processingData[j * bitLen + k] * carrierWave[k];
                                sum += processingData[j * bitLen + k] * carrierWave[k];
                                //sum +=  carrierWave[j];
                            }
                            if (sum > 0)
                                fout << 1;
                            else if (sum < 0)
                                fout << 0;
                        }
                        /*
                        for (int i = 0; i < processingData.size() + 100; i++)
                        {
                            if (i >= processingData.size())
                                datadebug << 0 << "\n";
                            else
                                datadebug << processingData[i] << "\n";
                        }
                        */
                        processingData.clear();
                        processingHeader.clear();
                        state = 0;
                        headerPos = 0;
                    }
                }
            }
        }
        fout.close();
    }

    void WritetoFile()
    {
        std::ofstream fout("record.out");
        if (!fout)
        {
            std::cout << "File open Error!" << std::endl;
        }
        else
        {
            for (int i = 0; i < decodeData.size(); i++)
            {
                if (i < 10000)
                {
                    fout << decodeData[i];

                }
            }
        }
        fout.close();
    }

    bool isRecording;
private:
    AudioBuffer<float> recordedSound;

    Array<float> processingHeader;
    Array<float> processingData;
    Array<float> syncHeader;
    Array<float> carrierWave;
    Array<int> decodeData;
    
    CriticalSection lock;

    int recordedSampleNum = -1;
    int bitLen = 48; //the length of one bit
    int packLen = 100; // how many bits per frame
    int headerLength = 480;
    int sampleRate;
    float syncPower_localMax;
};

using namespace std;
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
    f.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021-main\\input.in");
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
    int len_buffer = num_frame * len_frame;
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


class MAClayer : public AudioIODeviceCallback  
{
public :
    MAClayer() {

    };
    void receive() {
        AudioBuffer<float> tempbuffer;
        Array<int> data = Mac_receiver.Demodulate(tempbuffer);
        MACframe receive_frame(data);
    }
private :
    Receiver Mac_receiver;
    Sender Mac_sender;

    unsigned int frame_size;
    
};

#define TYPE_ACK 0
#define TYPE_DATA 1

/// <summary>
/// A frame contains a header, a type, a frame_id, data
/// </summary>
class MACframe {
public :
    MACframe(Array<int8_t> all_data) {
        
    }
private :
    Array<float> header;
    int type;
    int frame_id;
    Array<float> data;
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
