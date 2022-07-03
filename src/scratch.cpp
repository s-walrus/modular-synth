#include <cmath>
#include <iostream>

#include "player.h"
#include "synth.h"
#include "unit.h"

const std::size_t kSampleRate = 48000;

template <std::size_t freq>
SynthData ModSine(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    const static double time_mod = 2 * M_PI / kSampleRate * freq;
    static std::uint32_t time = 0;
    ++time;
    return (std::sin((double)time / 40)) * (1 << 28);
}

SynthData ModGentleStart(SynthData i1, SynthData i2, SynthData i3,
                         SynthData i4) {
    static std::uint32_t time = 0;
    const std::uint32_t kStartLength = 44000;
    if (time < kStartLength) {
        ++time;
    }
    return i1 * ((double)time / kStartLength);
}

SynthData ModMixer(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    return i1 + i2 + i3 + i4;
}

constexpr SynthUnit e_min_su =
    RunSynthUnitTree<8,
                     MakeSynthUnitTreeTraversal<8>(std::array<SynthUnitNode, 8>{
                         SynthUnitNode{
                             .unit = ModSine<82410>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<130810>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
                         SynthUnitNode{
                             .unit = ModSine<164010>,
                             .inputs = {kSynthUnitNoInput, kSynthUnitNoInput,
                                        kSynthUnitNoInput, kSynthUnitNoInput},
                         },
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
                             .inputs = {0, 1, 2, 3},
                         },
                         SynthUnitNode{
                             .unit = ModMixer,
                             .inputs = {6, 4, 5, kSynthUnitNoInput},
                         },
                     })>;

int main(int argc, char** argv) {
    Synth synth{{SynthUnitNode{e_min_su,
                               {kSynthUnitNoInput, kSynthUnitNoInput,
                                kSynthUnitNoInput, kSynthUnitNoInput}},
                 SynthUnitNode{ModGentleStart,
                               {0, kSynthUnitNoInput, kSynthUnitNoInput,
                                kSynthUnitNoInput}}},
                2};
    MiniaudioOutput player{synth.GetOutputBuffer(), kSampleRate};
    synth.Start();
    player.Start();

    std::cout << "Press ENTER to quit..." << std::endl;
    getchar();

    player.Stop();
    synth.Stop();
}
