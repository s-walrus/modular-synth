#include <cmath>
#include <iostream>

#include "../src/modules.h"
#include "../src/player.h"
#include "../src/synth.h"
#include "../src/unit.h"

const auto NOINP = kSynthNoInput;

#include "../build/_generated-su.h"

int main(int argc, char** argv) {
    Synth synth{{SynthUnitNode{main_su, {NOINP, NOINP, NOINP, NOINP}}}, 1};
    MiniaudioOutput player{synth.GetOutputBuffer(), kSampleRate};
    synth.Start();
    player.Start();

    std::cout << "Press ENTER to quit..." << std::endl;
    getchar();

    player.Stop();
    synth.Stop();
}
