#include <cmath>
#include <iostream>

#include "../src/player.h"
#include "../src/synth.h"
#include "../src/unit.h"

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

SynthData ModMixer(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    return i1 + i2 + i3 + i4;
}

constexpr SynthUnit e_min_part1 =
    RunSynthUnitTree<5,
                     MakeSynthUnitTreeTraversal<5>(std::array<SynthUnitNode, 5>{
                         SynthUnitNode{
                             .unit = ModSine<196000>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<246940>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<329630>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModMixer,
                             .inputs = {0, 1, 2, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModGentleStart<kSampleRate / 2>,
                             .inputs = {3, kSynthUnitInputs, kSynthUnitInputs,
                                        kSynthUnitNoInput},
                         },
                     })>;

constexpr SynthUnit e_min_part2 =
    RunSynthUnitTree<5,
                     MakeSynthUnitTreeTraversal<5>(std::array<SynthUnitNode, 5>{
                         SynthUnitNode{
                             .unit = ModSine<130810>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<82410>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<164010>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModMixer,
                             .inputs = {0, 1, 2, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModGentleStart<kSampleRate * 2>,
                             .inputs = {3, kSynthUnitInputs, kSynthUnitInputs,
                                        kSynthUnitNoInput},
                         },
                     })>;

int main(int argc, char** argv) {
    Synth synth{
        {SynthUnitNode{e_min_part1,
                       {kSynthUnitNoInput, kSynthUnitNoInput, kSynthUnitNoInput,
                        kSynthUnitNoInput}},
         SynthUnitNode{e_min_part2,
                       {kSynthUnitInputs, kSynthUnitNoInput, kSynthUnitNoInput,
                        kSynthUnitNoInput}},
         SynthUnitNode{ModMixer, {0, 1, kSynthUnitNoInput, kSynthUnitNoInput}}},
        2};
    MiniaudioOutput player{synth.GetOutputBuffer(), kSampleRate};
    synth.Start();
    player.Start();

    std::cout << "Press ENTER to quit..." << std::endl;
    getchar();

    player.Stop();
    synth.Stop();
}
