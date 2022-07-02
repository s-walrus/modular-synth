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
        Buffer* const inputs[kSynthUnitInputs]{0};

        bool CanProcessData() const;
        void ProcessData();
    };

    std::size_t kThreadCount;
    std::vector<std::list<Node>> assigned_nodes;
    std::atomic_bool workers_must_exit = false;

    std::vector<Node> MakeNodes(
        const std::vector<SynthUnitNode>& synth_unit_tree);
    void AssignNodes(std::vector<Node>);
    void StartWorker(std::size_t worker_id);
};
