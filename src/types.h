#include <cstdint>

typedef std::int32_t SynthData;
typedef SynthData(*SynthUnit)(SynthData, SynthData, SynthData, SynthData);
const std::size_t kSynthUnitInputs = 4;
