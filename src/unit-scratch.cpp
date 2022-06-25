#include <iostream>

#include "unit.h"

SynthData f1(SynthData i1, SynthData i2, SynthData i3, SynthData i4) {
    return i1 + 2;
}

constexpr SynthUnit su =
    RunSynthUnit<2,
                 MakeSynthUnitTreeTraversal<2>(std::array<SynthUnitNode, 2>{
                     SynthUnitNode{
                         .unit = nullptr,
                         .inputs = {-1, -2, -3, -4},
                     },
                     SynthUnitNode{
                         .unit = nullptr,
                         .inputs = {0, kSynthUnitNoInput, 0, kSynthUnitNoInput},
                     }})>;

int main() {
    auto x = MakeSynthUnitTreeTraversal<2>(std::array<SynthUnitNode, 2>{
        SynthUnitNode{
            .unit = f1,
            .inputs = {-1, -2, -3, -4},
        },
        SynthUnitNode{
            .unit = f1,
            .inputs = {0, kSynthUnitNoInput, 0, kSynthUnitNoInput},
        }});
    std::cout << x.buf_size << std::endl;
}
