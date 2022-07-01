#pragma once
#include <cstdint>
#include <list>
#include <vector>

#include "buffer.h"
#include "unit.h"

class Synth {
public:
    Synth(/* tree of SynthUnits, number of threads */);
    void Run();

private:
    static const std::size_t kBatchSize = 256;
    // TODO replace BasicBuffer with one that supports partial saturation
    using Buffer = BasicBuffer<SynthData, kBatchSize>;

    struct Node {
        SynthUnit synth_unit;
        Buffer output;
        Buffer* const inputs[kSynthUnitInputs]{0};

        bool CanProcessData() const;
        void ProcessData();
    };

    std::size_t kThreadCount;
    std::vector<std::list<Node>> tasks;

    std::vector<Node>& MakeTasks(/* tree of SynthUnits */);
    void ScheduleTasks(const std::vector<Node>&);
    void StartWorker(std::size_t worker_id);
};
