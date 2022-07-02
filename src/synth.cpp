#include "synth.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <thread>

Synth::Synth(std::vector<SynthUnitNode> synth_unit_tree, std::size_t n_threads)
    : kThreadCount(n_threads) {
    MakeAndAssignNodes(synth_unit_tree);
}

void Synth::Run() {
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < kThreadCount; ++i) {
        threads.emplace_back(
            [this](std::size_t worker_id) { StartWorker(worker_id); }, i);
    }

    std::cout << "Press Enter to quit..." << std::endl;
    getchar();

    workers_must_exit = true;
    for (std::size_t i = 0; i < kThreadCount; ++i) {
        threads[i].join();
    }
}

void Synth::MakeAndAssignNodes(
    const std::vector<SynthUnitNode>& synth_unit_tree) {
    // Make nodes
    struct MutableNode {
        SynthUnit synth_unit;
        std::unique_ptr<Buffer> output;
        std::array<Buffer*, kSynthUnitInputs> inputs{0};
    };
    std::vector<MutableNode> nodes(synth_unit_tree.size());
    for (std::size_t i = 0; i < synth_unit_tree.size(); ++i) {
        nodes[i].synth_unit = synth_unit_tree[i].unit;
        nodes[i].output.reset(new Buffer);
        for (std::size_t n_input = 0; n_input < kSynthUnitInputs; ++n_input) {
            if (synth_unit_tree[i].inputs[n_input] >= 0) {
                nodes[i].inputs[n_input] =
                    nodes[synth_unit_tree[i].inputs[n_input]].output.get();
            } else {
                nodes[i].inputs[n_input] = nullptr;
            }
        }
    }

    // Assign nodes
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        Node node = {
            .synth_unit = nodes[i].synth_unit,
            .output = std::move(nodes[i].output),
            // FIXME desperately looking for a better way to make these
            // enumerations
            .inputs =
                {
                    nodes[i].inputs[0],
                    nodes[i].inputs[1],
                    nodes[i].inputs[2],
                    nodes[i].inputs[3],
                },
        };
        assigned_nodes[i % assigned_nodes.size()].push_back(std::move(node));
    }
}

void Synth::StartWorker(std::size_t worker_id) {
    while (!workers_must_exit.load(std::memory_order_consume)) {
        for (Node& node : assigned_nodes[worker_id]) {
            // TODO it may be better to do just a single node.ProcessData,
            // compare both approaches in performance
            while (node.CanProcessData()) {
                node.ProcessData();
            }
            // TODO what if assigned_nodes are empty? what if no nodes can
            // process data? probably should add waiting mechanism
        }
    }
}

bool Synth::Node::CanProcessData() const {
    bool isInputDataAvailable = true;
    for (const Buffer* buf_p : inputs) {
        isInputDataAvailable &= buf_p == nullptr || buf_p->CanReceive();
    }
    return isInputDataAvailable && output->CanPost();
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

    output->PostFrom(output_buf);
}
