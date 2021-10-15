/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include<fstream>

using namespace juce;
class Receiver : public AudioIODeviceCallback
{
public :
    Receiver()
    {
        syncPower_localMax = 0;
        isRecording = false;
    }

    void audioDeviceAboutToStart(AudioIODevice* device) override
    {
        isRecording = false;
        recordedSampleNum = 0;

        sampleRate = 48000;
        
        recordedSound.clear();
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
                }
            }
            recordedSampleNum++;
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
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            if (i < headerLength)
            {
                processingHeader.add(s[i]);
                continue;
            }
            else
            {
                if (state == 0) // sync process
                {
                    if (processingHeader.size() > headerLength)
                    {
                        processingHeader.removeRange(0, 1);
                    }
                    processingHeader.add(s[i]);
                    float syncPower = 0;
                    for (int i = 0; i < headerLength; i++)
                    {
                        syncPower += syncHeader[i] * processingHeader[i];
                    }
                    if (syncPower > syncPower_localMax && syncPower > 0.05)
                    {
                        syncPower_localMax = syncPower;
                        headerPos = i;
                        tempBuffer.clear();
                    }
                    else if (headerPos != 0)
                    {
                        tempBuffer.add(s[i]);
                        if (i > headerPos + 200)
                        {
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
                        for (int i = 0; i < packLen; i++)
                        {
                            float sum = 0;
                            for (int j = 0; j < bitLen; j++)
                            {
                                sum += processingData[j] * carrierWave[j];
                            }
                            decodeData.add(sum > 0);
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
        std::ofstream fout("record.txt");
        if (!fout)
        {
            std::cout << "File open Error!" << std::endl;
        }
        else
        {
            for (int i = 0; i < decodeData.size(); i++)
                fout << decodeData[i];
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
    Array<bool> decodeData;

    CriticalSection lock;

    int recordedSampleNum = -1;
    int bitLen = 48; //the length of one bit
    int packLen = 100; // how many bits per frame
    int headerLength = 480;
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
