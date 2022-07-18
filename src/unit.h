#pragma once
#include <array>
#include <cassert>
#include <cstdint>

#include "constexpr-deque.h"

typedef int32_t SynthData;
typedef SynthData (*SynthUnit)(SynthData, SynthData, SynthData, SynthData);
const std::size_t kSynthUnitInputs = 4;
const std::int32_t kSynthNoInput = -5;

inline SynthData DummySynthUnit(SynthData, SynthData, SynthData, SynthData) {
    return 0;
}

// SynthUnitNode describes SynthUnit as a node of a tree
struct SynthUnitNode {
    SynthUnit unit;
    int32_t inputs[kSynthUnitInputs];
};

// UnitTraversalStep describes how to call SynthUnit function while
// traversing SynthUnit tree: where to get inputs and where to put output
struct UnitTraversalStep {
    SynthUnit func;
    std::size_t inputs[kSynthUnitInputs];
    std::size_t output;
};

// SynthUnitTreeTraversal describes how to execute a SynthUnit tree
template <std::size_t n_steps>
struct SynthUnitTreeTraversal {
    std::size_t buf_size;
    std::array<UnitTraversalStep, n_steps> steps;
};

// RunSynthUnit executes a tree of SynthUnits.
// For each SynthUnit, a separate RunSynthUnit function must be generated,
// unless local SynthUnit memory is not desired.
template <std::size_t n_nodes, SynthUnitTreeTraversal<n_nodes> traversal>
static constexpr SynthData RunSynthUnitTree(SynthData i1, SynthData i2,
                                            SynthData i3, SynthData i4) {
    [[maybe_unused]] SynthData buf[traversal.buf_size];
    buf[0] = 0;
    buf[1] = i1;
    buf[2] = i2;
    buf[3] = i3;
    buf[4] = i4;
#pragma clang loop unroll(full)
    for (auto step : traversal.steps) {
        // FIXME is it possible remove code duplication here?
        buf[step.output] = step.func(buf[step.inputs[0]], buf[step.inputs[1]],
                                     buf[step.inputs[2]], buf[step.inputs[3]]);
    }
    return buf[traversal.buf_size - 1];
}

// MakeSynthUnitTreeTraversal creates a traversal given a tree of SynthUnits
template <std::size_t n_nodes>
constexpr SynthUnitTreeTraversal<n_nodes> MakeSynthUnitTreeTraversal(
    const std::array<SynthUnitNode, n_nodes>& nodes) {
    // FIXME add input correctness check
    class TopsortSolver {
    public:
        constexpr TopsortSolver(const std::array<SynthUnitNode, n_nodes>& nodes)
            : nodes(nodes) {}

        constexpr Deque<std::size_t, n_nodes> MakeTopsort() {
            for (std::size_t i = 0; i < n_nodes; ++i) {
                if (!used[i]) {
                    dfs(i);
                }
            }
            return topsort;
        }

    private:
        const std::array<SynthUnitNode, n_nodes>& nodes;
        Deque<std::size_t, n_nodes> topsort;
        bool used[n_nodes]{0};

        constexpr void dfs(std::size_t v) {
            used[v] = true;
            for (std::int32_t input : nodes[v].inputs) {
                if (input >= 0 && !used[input]) {
                    dfs(input);
                }
            }
            topsort.PushBack(v);
        }
    };

    TopsortSolver ts(nodes);
    Deque<std::size_t, n_nodes> topsort = ts.MakeTopsort();

    if (topsort.Back() != n_nodes - 1) {
        throw "last node of SynthUnit tree must be the output unit, but it is a dependency of another SynthUnit";
    }

    SynthUnitTreeTraversal<n_nodes> traversal{
        .buf_size = 1 + kSynthUnitInputs + n_nodes,
    };
    const std::size_t buf_outputs_offset = 5;
    std::size_t i = 0;
    for (; !topsort.Empty(); ++i, topsort.PopFront()) {
        std::size_t node_i = topsort.Front();
        traversal.steps[i] = {
            .func = nodes[node_i].unit,
            .output = buf_outputs_offset + node_i,
        };
        for (int input_i = 0; input_i < kSynthUnitInputs; ++input_i) {
            std::int32_t input = nodes[node_i].inputs[input_i];
            std::size_t buf_input;
            if (input >= 0) {
                buf_input = buf_outputs_offset + input;
            } else if (input == kSynthNoInput) {
                buf_input = 0;
            } else {
                buf_input = -input;
            }
            traversal.steps[i].inputs[input_i] = buf_input;
        }
    }
    for (; i < traversal.steps.size(); ++i) {
        traversal.steps[i] = {
            .func = DummySynthUnit,
            .inputs = {0},
            .output = 0,
        };
    }
    return traversal;
}

template <SynthUnitNode... nodes>
SynthData CompileSynthUnit(SynthData i1, SynthData i2, SynthData i3,
                           SynthData i4) {
    return RunSynthUnitTree<sizeof...(nodes),
                            MakeSynthUnitTreeTraversal<sizeof...(nodes)>(
                                std::array<SynthUnitNode, sizeof...(nodes)>{
                                    nodes...})>(i1, i2, i3, i4);
}
