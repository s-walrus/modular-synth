#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <vector>

#include "buffer.h"
#include "unit.h"

class Synth {
public:
    Synth(std::vector<SynthUnitNode> synth_unit_tree, std::size_t n_threads);
    void Run();

private:
    static const std::size_t kBatchSize = 256;
    // TODO replace BasicBuffer with one that supports partial saturation
    using Buffer = BasicBuffer<SynthData, kBatchSize>;

    struct Node {
        SynthUnit synth_unit;
        const std::unique_ptr<Buffer> output;
        std::array<Buffer* const, kSynthUnitInputs> inputs{0};
        // WARNING it is assumed that data generation is synchronized between
        // all buffers, i.e. for calculating k-th batch of output, data of k-th
        // generation is used. This asumption should be reflected in source code
        Buffer::DataGeneration data_gen = 0;

        bool CanProcessData() const;
        void ProcessData();
    };

    std::size_t kThreadCount;
    std::vector<std::list<Node>> assigned_nodes;
    std::atomic_bool workers_must_exit = false;

    void MakeAndAssignNodes(const std::vector<SynthUnitNode>& synth_unit_tree);
    void StartWorker(std::size_t worker_id);
};
