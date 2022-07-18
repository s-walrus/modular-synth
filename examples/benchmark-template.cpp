#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
using namespace std::chrono;

#include "../src/modules.h"
#include "../src/player.h"
#include "../src/synth.h"
#include "../src/unit.h"

const auto NOINP = kSynthNoInput;
const std::size_t N_SAMPLES = 4000000;

#include "../build/_generated-su.h"

int main(int argc, char** argv) {
    Synth synth{{SynthUnitNode{main_su, {NOINP, NOINP, NOINP, NOINP}}}, 1};
    Synth::Buffer* synth_buf = synth.GetOutputBuffer();

    synth.Start();

    std::size_t sample_cnt = 0;
    std::size_t data_gen = 0;
    std::array<SynthData, Synth::kBatchSize> local_buf;

    auto start = high_resolution_clock::now();
    while (sample_cnt < N_SAMPLES) {
        while (!synth_buf->CanReceive(data_gen)) {
        }
        data_gen = synth_buf->ReceiveTo(local_buf);
        sample_cnt += Synth::kBatchSize;
    }
    auto stop = high_resolution_clock::now();

    synth.Stop();

    auto duration = duration_cast<microseconds>(stop - start);

    if (argc >= 2 && std::strcmp(argv[1], "-b") == 0) {
        std::cout << duration << "\n";
    } else {
        std::cout << "Processed " << N_SAMPLES << " samples in " << duration
                  << "\n";
    }
}
