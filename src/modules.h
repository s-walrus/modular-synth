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

template <SynthData divisor, SynthData h1, SynthData h2, SynthData h3,
          std::uint32_t>
SynthData ModTone(SynthData freq, SynthData amp, SynthData, SynthData) {
    static double phase = 0;

    phase += (double)freq / 1000 / kSampleRate;
    if (phase >= 100) {
        phase = 0;
    }
    return (sin(phase * 2 * M_PI) + sin(phase * 4 * M_PI) * h1 / divisor +
            sin(phase * 8 * M_PI) * h2 / divisor +
            sin(phase * 16 * M_PI) * h3 / divisor) *
           amp;
}

inline SynthData modAdd(SynthData i1, SynthData i2, SynthData i3,
                        SynthData i4) {
    return i1 + i2 + i3 + i4;
}

template <SynthData denominator>
SynthData modAmp(SynthData src, SynthData mod, SynthData, SynthData) {
    return src * ((double)mod / denominator);
}

template <SynthData divisor, SynthData time_scale, std::uint32_t>
SynthData ModGentleStart(SynthData, SynthData, SynthData, SynthData) {
    static std::size_t time = 0;
    if (time != 10000) {
        ++time;
    }
    return std::min(1., sqrt((double)time / time_scale)) * divisor;
}

template <std::size_t buf_size, std::uint32_t>
requires(buf_size % 2 == 0) SynthData
    ModToneUpshift(SynthData src, SynthData, SynthData, SynthData) {
    static SynthData in_buf[buf_size];
    static SynthData out_buf[buf_size];
    static std::size_t in_buf_end = 0;
    static std::size_t out_buf_begin = buf_size;

    in_buf[in_buf_end] = src;
    ++in_buf_end;
    if (in_buf_end == buf_size) {
        for (std::size_t i = 0; i * 2 < buf_size; ++i) {
            out_buf[i] = in_buf[i * 2];
            out_buf[i * 2] = in_buf[i * 2];
        }
        out_buf_begin = 0;
    }

    if (out_buf_begin == buf_size) {
        return 0;
    } else {
        return out_buf[out_buf_begin++];
    }
}

template <std::uint32_t span_width, std::uint32_t>
SynthData ModSmoother(SynthData src, SynthData, SynthData, SynthData) {
    static SynthData buf[span_width] = {0};
    static std::size_t buf_i = 0;
    static std::uint64_t sum = 0;

    buf[buf_i] = src;
    sum += buf[buf_i];
    ++buf_i;
    if (buf_i == span_width) {
        buf_i = 0;
    }
    sum -= buf[buf_i];
    return sum / span_width;
}
