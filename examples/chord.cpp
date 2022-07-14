#include <cmath>
#include <iostream>

#include "../src/modules.h"
#include "../src/player.h"
#include "../src/synth.h"
#include "../src/unit.h"

const auto NOINP = kSynthNoInput;

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

SynthData ModMixer(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    return i1 + i2 + i3 + i4;
}

SynthUnit e_min_part1 =
    CompileSynthUnit<SynthUnitNode{
                         .unit = ModSineWave<196000>,
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

SynthUnit e_min_part2 =
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

int main(int argc, char** argv) {
    Synth synth{
        {SynthUnitNode{e_min_part1, {NOINP, NOINP, NOINP, NOINP}},
         SynthUnitNode{e_min_part2, {kSynthUnitInputs, NOINP, NOINP, NOINP}},
         SynthUnitNode{ModMixer, {0, 1, NOINP, NOINP}}},
        2};
    MiniaudioOutput player{synth.GetOutputBuffer(), kSampleRate};
    synth.Start();
    player.Start();

    std::cout << "Press ENTER to quit..." << std::endl;
    getchar();

    player.Stop();
    synth.Stop();
}
