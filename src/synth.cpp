#include "synth.h"

#include <algorithm>
#include <array>
#include <cassert>

bool Synth::Node::CanProcessData() const {
    bool isInputDataAvailable = true;
    for (const Buffer* buf_p : inputs) {
        isInputDataAvailable &= buf_p == nullptr || buf_p->CanReceive();
    }
    return isInputDataAvailable && output.CanPost();
}

void Synth::Node::ProcessData() {
    assert(CanProcessData());
    std::array<SynthData, kBatchSize> input_bufs[kSynthUnitInputs];
    std::array<SynthData, kBatchSize> output_buf;

    for (std::size_t n_input = 0; n_input < kSynthUnitInputs; ++n_input) {
        if (inputs[n_input]) {
            inputs[n_input]->ReceiveTo(input_bufs[n_input]);
        } else {
            assert(std::all_of(input_bufs[n_input].begin(),
                               input_bufs[n_input].end(),
                               [](SynthData x) { return x == 0; }));
        }
    }

    for (std::size_t i = 0; i < kBatchSize; ++i) {
        // FIXME question still holds: can it be written more concisely?
        output_buf[i] = synth_unit(input_bufs[0][i], input_bufs[1][i],
                                   input_bufs[2][i], input_bufs[3][i]);
    }

    output.PostFrom(output_buf);
}
