#include "player.h"

#include <iostream>

void MiniaudioOutput::MiniaudioCallback(ma_device* device_p, void* output_p,
                                        [[maybe_unused]] const void* input_p,
                                        ma_uint32 n_frames) {
    CallbackData* data_p = (MiniaudioOutput::CallbackData*)device_p->pUserData;
    SynthData* output_buf = (SynthData*)output_p;
    std::array<SynthData, Synth::kBatchSize> mid_buf;
    if (data_p->buffer->CanReceive(data_p->data_gen)) {
        data_p->data_gen = data_p->buffer->ReceiveTo(mid_buf);
    } else {
        mid_buf = {0};
    }

    for (int frame_i = 0; frame_i < n_frames; ++frame_i) {
        output_buf[frame_i * 2 + 0] = mid_buf[frame_i];
        output_buf[frame_i * 2 + 1] = mid_buf[frame_i];
    }
}

MiniaudioOutput::MiniaudioOutput(Synth::Buffer* output_buffer,
                                 std::size_t sample_rate)
    : output_buf(output_buffer), kSampleRate(sample_rate) {
    ma_device_config deviceConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_s32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = kSampleRate;
    deviceConfig.dataCallback = MiniaudioCallback;
    deviceConfig.pUserData = &callbackData;
    // deviceConfig.stopCallback = ???

    if (ma_device_init(NULL, &deviceConfig, &output_device) != MA_SUCCESS) {
        throw "Failed to open playback device.";
    }
}

void MiniaudioOutput::Start() {
    if (ma_device_start(&output_device) != MA_SUCCESS) {
        ma_device_uninit(&output_device);
        throw "Failed to start playback device.";
    }

    printf("Press Enter to quit...\n");
    getchar();

    ma_device_uninit(&output_device);
}
