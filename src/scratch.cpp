#include <cmath>
#include <cstdlib>
#include <iostream>

#include "../lib/RtAudio.h"
#include "unit.h"

SynthData ModSine(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    static std::uint32_t time = 0;
    ++time;
    return (std::sin((double)time / 10)) * (1 << 30);
}

constexpr SynthUnit su =
    RunSynthUnit<1, MakeSynthUnitTreeTraversal<1>(std::array<SynthUnitNode, 1>{
                        SynthUnitNode{
                            .unit = ModSine,
                            .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                       kSynthUnitNoInput, kSynthUnitNoInput},
                        },
                    })>;

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include "../lib/miniaudio.h"

const std::size_t kSampleRate = 48000;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput,
                   ma_uint32 frameCount) {
    SynthData* output = (SynthData*)pOutput;

    /* This callback is tied to the specific sample format and rate. */
    /* Populate 'pOutput' with 'frameCount' frames. */
    for (int curFrame = 0; curFrame < frameCount; ++curFrame) {
        SynthData val = su(0, 0, 0, 0);
        output[curFrame * 2 + 0] = val;
        output[curFrame * 2 + 1] = val;
    }

    (void)pInput; /* Unused. */
}

int main(int argc, char** argv) {
    ma_device_config deviceConfig;
    ma_device device;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_s32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = kSampleRate;
    deviceConfig.dataCallback = data_callback;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return -4;
    }

    printf("Device Name: %s\n", device.playback.name);

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop__em, 0, 1);
#else
    printf("Press Enter to quit...\n");
    getchar();
#endif

    ma_device_uninit(&device);

    return 0;
}
