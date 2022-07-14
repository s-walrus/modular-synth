#include <cmath>
#include <iostream>

#include "../src/modules.h"
#include "../src/player.h"
#include "../src/synth.h"
#include "../src/unit.h"

const auto NOINP = kSynthNoInput;

const std::size_t kSampleRate = 48000;

template <std::size_t freq>
SynthData ModSine(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    const static double time_mod = 2 * M_PI / kSampleRate * freq / 1000;
    static std::uint32_t time = 0;
    ++time;
    return (std::sin((double)time * time_mod)) * (1 << 28);
}

template <std::size_t start_length>
SynthData ModGentleStart(SynthData i1, SynthData i2, SynthData i3,
                         SynthData i4) {
    static std::uint32_t time = 0;
    if (time < start_length) {
        ++time;
    }
    return i1 * ((double)time / start_length);
}

SynthData ModReverseSawtooth(SynthData freq, SynthData base, SynthData peak,
                             SynthData) {
    static double phase = 0;
    phase += (double)freq / 1000 / kSampleRate;
    if (phase >= 1) {
        phase = 0;
    }
    return peak - (peak - base) * phase;
}

SynthData ModSaw(SynthData freq, SynthData base, SynthData peak, SynthData) {
    static double phase = 0;
    phase += (double)freq / 1000 / kSampleRate;
    if (phase >= 1) {
        phase = 0;
    }
    return std::abs(phase * 2 - 1) * (peak - base) + base;
}

template <std::uint32_t span>
SynthData ModSmoother(SynthData src, SynthData, SynthData, SynthData) {
    static SynthData buf[span] = {0};
    static std::size_t buf_i = 0;
    static std::uint64_t sum = 0;

    buf[buf_i] = src;
    sum += buf[buf_i];
    ++buf_i;
    if (buf_i == span) {
        buf_i = 0;
    }
    sum -= buf[buf_i];
    return sum / span;
}

template <std::uint32_t scale>
SynthData ModAmp(SynthData src, SynthData mod, SynthData, SynthData) {
    return src * ((double)mod / scale);
}

SynthData ModMixer(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    return i1 + i2 + i3 + i4;
}

constexpr SynthUnit e_min_part1 =
    CompileSynthUnit<SynthUnitNode{
                         .unit = ModSine<196000>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModSine<246940>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModSine<329630>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModMixer,
                         .inputs = {0, 1, 2, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModGentleStart<kSampleRate / 2>,
                         .inputs = {3, NOINP, NOINP, NOINP},
                     }>;

constexpr SynthUnit e_min_part2 =
    CompileSynthUnit<SynthUnitNode{
                         .unit = ModSine<130810>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModSine<82410>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModSine<164010>,
                         .inputs = {NOINP, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModMixer,
                         .inputs = {0, 1, 2, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModGentleStart<kSampleRate * 2>,
                         .inputs = {3, NOINP, NOINP, NOINP},
                     }>;

constexpr SynthUnit dirtifier =
    CompileSynthUnit<NodeConst<6000>,                    //
                     NodeConst<(int)((1 << 16) * 0.6)>,  //
                     NodeConst<(int)((1 << 16) * 1.2)>,  //
                     SynthUnitNode{
                         .unit = ModReverseSawtooth,
                         .inputs = {0, 1, 2, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModSmoother<100>,
                         .inputs = {3, NOINP, NOINP, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModAmp<1 << 16>,
                         .inputs = {-1, 4, NOINP, NOINP},
                     }>;

/* constexpr SynthUnit squarifier =
    CompileSynthUnit<NodeConst<6000 << 2>,               //
                     NodeConst<(int)((1 << 16) * 0.9)>,  //
                     NodeConst<(int)((1 << 16) * 1.1)>,  //
                     SynthUnitNode{
                         .unit = ModSaw,
                         .inputs = {0, 1, 2, NOINP},
                     },
                     SynthUnitNode{
                         .unit = ModAmp<1 << 16>,
                         .inputs = {-1, 4, NOINP, NOINP},
                     }>; */

int main(int argc, char** argv) {
    Synth synth{{
                    //
                    SynthUnitNode{e_min_part1, {NOINP, NOINP, NOINP, NOINP}},
                    SynthUnitNode{dirtifier, {0, NOINP, NOINP, NOINP}},
                    SynthUnitNode{e_min_part2, {NOINP, NOINP, NOINP, NOINP}},
                    SynthUnitNode{ModMixer, {1, 2, NOINP, NOINP}}
                    //
                },
                2};
    MiniaudioOutput player{synth.GetOutputBuffer(), kSampleRate};
    synth.Start();
    player.Start();

    std::cout << "Press ENTER to quit..." << std::endl;
    getchar();

    player.Stop();
    synth.Stop();
}
