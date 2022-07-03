#include "synth.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
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

Synth::Buffer* Synth::GetOutputBuffer() { return output_buffer; }

void Synth::MakeAndAssignNodes(
    const std::vector<SynthUnitNode>& synth_unit_tree) {
    // Count receivers
    std::vector<std::size_t> receiver_cnt(synth_unit_tree.size(), 0);
    for (std::size_t i = 0; i < synth_unit_tree.size(); ++i) {
        for (auto input : synth_unit_tree[i].inputs) {
            if (input >= 0) {
                ++receiver_cnt[input];
            }
        }
    }
    if (receiver_cnt.back()) {
        throw "last node of SynthUnit tree must be the output unit, but it is a dependency of another SynthUnit";
    }
    receiver_cnt.back() = 1;

    // Make nodes
    struct MutableNode {
        SynthUnit synth_unit;
        std::unique_ptr<Buffer> output;
        std::array<Buffer*, kSynthUnitInputs> inputs{0};
    };
    std::vector<MutableNode> nodes(synth_unit_tree.size());
    for (std::size_t i = 0; i < synth_unit_tree.size(); ++i) {
        nodes[i].synth_unit = synth_unit_tree[i].unit;
        nodes[i].output.reset(new Buffer{receiver_cnt[i]});
        for (std::size_t n_input = 0; n_input < kSynthUnitInputs; ++n_input) {
            if (synth_unit_tree[i].inputs[n_input] >= 0) {
                nodes[i].inputs[n_input] =
                    nodes[synth_unit_tree[i].inputs[n_input]].output.get();
            } else {
                nodes[i].inputs[n_input] = nullptr;
            }
        }
    }
    output_buffer = nodes.back().output.get();

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
    bool have_executed_any_node;
    while (!workers_must_exit.load(std::memory_order_consume)) {
        have_executed_any_node = false;
        for (Node& node : assigned_nodes[worker_id]) {
            // TODO it may be better to do just a single node.ProcessData,
            // compare both approaches in performance
            while (node.CanProcessData()) {
                node.ProcessData();
                have_executed_any_node = true;
            }
        }
        if (!have_executed_any_node) {
            // TODO dynamically update sleep_time
            const auto sleep_time = std::chrono::milliseconds(20);
            std::this_thread::sleep_for(sleep_time);
        }
    }
}

bool Synth::Node::CanProcessData() const {
    bool isInputDataAvailable = true;
    for (const Buffer* input : inputs) {
        isInputDataAvailable &= input == nullptr || input->CanReceive(data_gen);
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

    data_gen = output->PostFrom(output_buf);
}
