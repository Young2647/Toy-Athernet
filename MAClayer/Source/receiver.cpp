#include "receiver.h"

Receiver::Receiver() {
    syncPower_localMax = 0;
    isRecording = false;
    data_state = -1;
    state = SYNC;

    isRecording = false;
    recordedSampleNum = 0;

    sampleRate = 48000;

    GenerateCarrierWave();
    GenerateHeader();
}

Receiver::Receiver(int bitlen, int packlen)
{
    bitLen = bitlen;
    packLen = packlen;

    syncPower_localMax = 0;
    isRecording = false;
    data_state = -1;
    state = SYNC;

    isRecording = false;
    recordedSampleNum = 0;

    sampleRate = 48000;

    GenerateCarrierWave();
    GenerateHeader();
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

}

void 
Receiver::audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels, int numSamples) 
{
    const ScopedLock sl(lock);
    if (isRecording)
    {
        for (int i = 0; i < numSamples; i++)
        {
            auto inputSamp = 0.0f;
            for (auto j = numInputChannels; --j >= 0;)
                if (inputChannelData[j] != nullptr)
                    inputSamp += inputChannelData[j][i];
            
            data_state = Demodulate(inputSamp);
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
    recordedSampleNum = 0;
    isRecording = true;
    fout = std::ofstream("input.out");
}

void 
Receiver::stopRecording()
{
    if (isRecording)
    {
        isRecording = false;
        fout.close();
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

int
Receiver::Demodulate(float sample)
{
    fout << sample << "\n";
    //std::cout << sample << "\n";
    power_ = power_ * (1 - 1.0 / 64.0) + sample * sample / 64.0;
    if (state == SYNC)// sync process
    {
        if (processingHeader.size() < headerLength)
        {
            processingHeader.add(sample);
            return NO_HEADER;
        }
        else
        {
            processingHeader.add(sample);
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
                tempBuffer.clear();
                _ifheadercheck = true;
            }
            else if (_ifheadercheck)
            {
                tempBuffer.add(sample);

                //recordeddebug << s[i] << "\n";
                if (tempBuffer.size() >= 500)
                {
                    std::cout << "header found.\n";
                    syncPower_localMax = 0;
                    state = DATA_PROCESS;
                    processingData = tempBuffer;
                }
            }
            return NO_HEADER;
        }
    }
    else if (state == DATA_PROCESS) //data process
    {
        processingData.add(sample);
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
            processingData.clear();
            processingHeader.clear();
            state = SYNC;
            _ifheadercheck = false;
            frame_data = Int2Byte(int_data);
            int_data.clear();
            tempBuffer.clear();
            return DATA_RECEIVED;
        }
        else
        {
            return DATA_PROCESS;
        }
    }
}

Array<int8_t>
Receiver::getData()
{
    data_state = SYNC;
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
    while (data_state != DATA_RECEIVED)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();
        if (duration >= MAX_WAITING_TIME) break;
    }
    return frame_data;
}