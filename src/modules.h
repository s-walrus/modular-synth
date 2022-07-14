#pragma once
#include <cinttypes>
#include <cmath>

#include "unit.h"

static const std::size_t kSampleRate = 48000;

template <SynthData val>
SynthData modConst(SynthData, SynthData, SynthData, SynthData) {
    return val;
}

template <SynthData val>
constexpr SynthUnitNode NodeConst{
    .unit = modConst<val>,
    .inputs = {kSynthNoInput, kSynthNoInput, kSynthNoInput, kSynthNoInput},
};

template <std::uint32_t>
SynthData ModSineWave(SynthData freq, SynthData amp, SynthData, SynthData) {
    static double phase = 0;
    phase += (double)freq / 1000 / kSampleRate;
    if (phase >= 100) {
        phase = 0;
    }
    return sin(phase * 2 * M_PI) * amp;
}

template <SynthData denominator>
SynthData modAmp(SynthData src, SynthData mod, SynthData, SynthData) {
    return src * mod;
}
