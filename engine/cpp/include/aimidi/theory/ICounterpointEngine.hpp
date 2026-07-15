#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

enum class CounterpointSpecies { first, second };

struct CounterpointRequest {
    std::vector<int>  cantus_firmus;
    std::vector<int>  bass_notes;
    int               bars;
    Seed              seed = 0;
    CounterpointSpecies species = CounterpointSpecies::first;
    int               ppq = 480;
};

class ICounterpointEngine {
public:
    virtual ~ICounterpointEngine() = default;
    virtual std::vector<MidiEvent> generate(const CounterpointRequest& req) const = 0;
};

std::unique_ptr<ICounterpointEngine> make_counterpoint_engine(
    std::shared_ptr<IScaleProvider> scales);

} // namespace aimidi::theory
