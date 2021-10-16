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
public :
    Receiver()
    {
        syncPower_localMax = 0;
        isRecording = false;
    }

    void GenerateCarrierWave()
    {
        int carrierAmp = 1;
        int carrierFreq = 10000;

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
        fp[headerLength/ 2] = end_freq;
        for (int i = 1; i < headerLength / 2; i++)
            fp[i] = fp[i - 1] + freq_step;
        for (int i = headerLength / 2 + 1; i < headerLength; i++)
            fp[i] = fp[i - 1] - freq_step;
        for (int i = 1; i < headerLength; i++)
            omega[i] = omega[i - 1] + (fp[i] + fp[i - 1]) / 2.0 * time_gap;
        for (int i = 0; i < headerLength; i++)
            header_stack[i] = sin(2 * PI * omega[i]);
        syncHeader = Array<float>(header_stack,headerLength);
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
        std::ofstream debugf("synpower.txt");
        std::ofstream recordeddebug("record.txt");
        std::ofstream datadebug("data.txt");
        for (int i = 0; i < recordedSampleNum; i++)
        {
            recordeddebug << s[i] << "\n";
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
                    debugf << syncPower << "\n";
                    if (syncPower > (power * power) && syncPower > syncPower_localMax && syncPower > 0.5)
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
                            processingHeader.clear();
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
                                decodeData.add(1);
                            else if (sum < 0)
                                decodeData.add(0);
                        }
                        for (int i = 0; i < processingData.size() + 100; i++)
                        {
                            if (i >= processingData.size())
                                datadebug << 0 << "\n";
                            else
                                datadebug << processingData[i] << "\n";
                        }
                        processingData.clear();
                        state = 0;
                        headerPos = 0;
                    }            
                }
            } 
        }
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
private :
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
    int headerLength = 960;
    int sampleRate;
    float syncPower_localMax;
};
//==============================================================================
int main (int argc, char* argv[])
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
    (*receiver).WritetoFile();
    dev_manager.removeAudioCallback(receiver.get());
    DeletedAtShutdown::deleteAll();
    juce::MessageManager::deleteInstance();
    // ..your code goes here!

    
    return 0;
}
