#include "receiver.h"

Receiver::Receiver() {
    syncPower_localMax = 0;
    isRecording = false;
}

void 
Receiver::GenerateCarrierWave()
{
    int carrierAmp = 1;
    int carrierFreq = 5000;

    for (int j = 0; j < bitLen; j++)
    {
        carrierWave.add(carrierAmp * cos(2 * PI * carrierFreq * j / sampleRate));
    }

}

void 
Receiver::GenerateHeader() {
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

void 
Receiver::audioDeviceAboutToStart(AudioIODevice* device) 
{
    isRecording = false;
    recordedSampleNum = 0;

    sampleRate = 48000;

    recordedSound.clear();

    GenerateCarrierWave();
    GenerateHeader();
}

void 
Receiver::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels, int numSamples) 
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

void 
Receiver::startRecording()
{

    const ScopedLock sl(lock);
    recordedSound.clear();
    recordedSampleNum = 0;

    recordedSound.setSize(1, 15 * sampleRate);
    isRecording = true;
}

void 
Receiver::stopRecording()
{
    if (isRecording)
    {
        isRecording = false;

        Demodulate(recordedSound);
    }
}

Array<int8_t> 
Receiver::Int2Byte(Array<int>& int_data)
{
    if (int_data.size() / 8 * 8 != int_data.size())
    {
        std::cout << "data length wrong!\n";
    }
    Array<int8_t> byte_data;

    for (int i = 0; i < int_data.size(); i += 8)
    {
        int8_t temp = 0;
        for (int j = 0; j < 8; j++)
        {
            temp = temp << 1;
            if (int_data[i * 8 + j] == 1)
                temp = temp | 0x01;
        }
        byte_data.add(temp);
    }
    return byte_data;
}

Array<int8_t> 
Receiver::Demodulate(AudioBuffer<float>& buffer)
{
    int headerPos = 0;
    int state = 0; // 0 = sync, 1 = decode
    auto* s = buffer.getReadPointer(0);

    Array<float> tempBuffer;
    float power = 0;
    //std::ofstream debugf("synpower.txt");
    //std::ofstream recordeddebug("record.txt");
    //std::ofstream datadebug("data.txt");
    //std::ofstream fout("record.out");
    Array<int> int_data;
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
                            int_data.add(1);
                        else if (sum < 0)
                            int_data.add(0);
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
    return Int2Byte(int_data);
}