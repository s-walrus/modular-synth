#pragma once

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include "../lib/miniaudio.h"
#include "synth.h"

class MiniaudioOutput {
public:
    MiniaudioOutput(Synth::Buffer* output_buffer,
                    std::size_t sample_rate = 48000);
    void Start();

private:
    const std::size_t kSampleRate;
    Synth::Buffer* const output_buf;
    ma_device output_device;

    struct CallbackData {
        Synth::Buffer* buffer;
        Synth::Buffer::DataGeneration data_gen;
    };

    CallbackData callbackData;
    static void MiniaudioCallback(ma_device* device_p, void* output_p,
                                  const void* input_p, ma_uint32 n_frames);
};
