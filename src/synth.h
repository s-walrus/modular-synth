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
    static const std::size_t kBufferSize = 256;
    // FIXME BasicBuffer is a temporary solution. A buffer should have multiple
    // fillable slots or support for partial saturation, otherwise BasicBuffer
    // must be cleverly used to make use of concurrency.
    using Buffer = BasicBuffer<SynthData, kBufferSize>;

    class Task {
    public:
        bool CanRun() const;
        void Run();
    };

    std::size_t kThreadCount;
    std::vector<std::list<Task>> tasks;

    std::vector<Task>& MakeTasks(/* tree of SynthUnits */);
    void ScheduleTasks(const std::vector<Task>&);
    void StartWorker(std::size_t worker_id);
};
