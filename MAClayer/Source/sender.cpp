#include "sender.h"


Sender::Sender(int nbpf, int nspb) {
    num_bits_per_frame = nbpf;
    num_samples_per_bit = nspb;
    len_warm_up = 480;
    header_len = 480;
    sample_rate = 48000;
    carrier_freq = 5000;
    carrier_phase = 0;
    carrier_amp = 1;
    len_zeros = 20;
    num_frame = 100;
    output_buffer_idx = 0;
    len_frame = header_len + num_samples_per_bit * num_bits_per_frame + len_zeros;
    frame_wave.resize(num_bits_per_frame * num_samples_per_bit);
    GenerateCarrierWave();
    header_wave.resize(header_len);
    generateHeader();
    isPlaying = false;
    //fout = std::ofstream("output.out");
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

void Sender::generateHeader() {
    int start_freq = 2000;
    int end_freq = 10000;
    float freq_step = (end_freq - start_freq) / (header_len / 2);
    float time_gap = (float)1 / (float)sample_rate;
    std::vector<float> fp;
    std::vector<float> omega;
    for (int i = 0; i < header_len; i++) {
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
    header_wave = Array<float>(header_stack, header_len);
}


void Sender::Modulation(Array<int8_t> cur_frame_data, int frame_len) {
    frame_wave.fill(0);
    for (int i = 0; i < frame_len; i++) {
        for (int j = 0; j < num_samples_per_bit; j++) {
            /*if (cur_frame_data[i] > 1) {
                cout << cur_frame_data[i] << endl;
            }*/
            frame_wave.set(i * num_samples_per_bit + j, ((int)cur_frame_data[i] * 2 - 1) * carrier_wave[j]);
        }
    }
}

void Sender::sendOnePacket(int frame_len, Array<int8_t> cur_frame_data) {
    vector<int8_t> vec;
    for (int i = 0; i < 16; i++) {
        vec.push_back(cur_frame_data[i]);
    }
    Modulation(cur_frame_data, frame_len);
    for (int j = 0; j < header_len; j++, output_buffer_idx++)
        output_buffer.setSample(0, output_buffer_idx, header_wave[j]);
    for (int j = 0; j < frame_len * num_samples_per_bit; j++, output_buffer_idx++)
        output_buffer.setSample(0, output_buffer_idx, frame_wave[j]);
    for (int j = 0; j < len_zeros; j++, header_wave[j])
        output_buffer.setSample(0, output_buffer_idx, 0);
}

void Sender::printOutput_buffer() {
    ofstream of;
    of.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021\\out.out", ios::trunc);
    for (int i = 0; i < output_buffer.getNumSamples(); i++) {
        if (of.is_open() && i < 210000) {
            of << output_buffer.getSample(0, i) << endl;
        }
    }
    of.close();
}

int Sender::startSend()
{
    const ScopedLock sl(lock);
    playingSampleNum = 0;
    isPlaying = true;
    int len_buffer = num_frame * len_frame;
    output_buffer.setSize(1, len_buffer + 480 + len_warm_up);
    output_buffer.clear();
    for (int j = 0; j < 480; j++, output_buffer_idx++)
        output_buffer.setSample(0, j, carrier_wave[j % num_samples_per_bit]);
    //of.open("C:\\Users\\zhaoyb\\Desktop\\CS120-Shanghaitech-Fall2021\\out.out", ios::trunc);
    return 1;
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
                //outputChannelData[j][i] = (playingSampleNum < output_buffer.getNumSamples()) ? 1.0f : 0.0f;
                outputChannelData[j][i] = (playingSampleNum < output_buffer.getNumSamples()) ? playBuffer[playingSampleNum] : 0.0f;
                //fout << outputChannelData[j][i] << "\n";
                ++playingSampleNum;
            }
        }
    }
   
    
    //for (int i = 0; i < numSamples; i++)
    //{
    //    for (auto j = numOutputChannels; --j >= 0;)
    //    {

    //        if (outputChannelData[j] != nullptr)
    //        {
    //            // Write the sample into the output channel
    //            //outputChannelData[j][i] = (playingSampleNum < output_buffer.getNumSamples()) ? 1.0f : 0.0f;
    //            of << outputChannelData[j][i] << endl;
    //        }
    //    }
    //}
    //of.close();
}