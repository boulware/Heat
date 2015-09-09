#if !defined(PHASE_HPP)

#include <string>

enum class phase
{
    Invalid,
    Solid,
    Liquid,
    Gas,
};

struct phase_transition
{
    phase mInitialPhase;
    phase mFinalPhase;
    float mTemperature;
    float mLatentHeat;

    phase_transition(phase InitialPhase, phase FinalPhase, float Temperature, float LatentHeat)
        :
        mInitialPhase(InitialPhase), mFinalPhase(FinalPhase), mTemperature(Temperature), mLatentHeat(LatentHeat)
    {
    }

    phase_transition() : phase_transition(phase::Invalid, phase::Invalid, -1.f, -1.f) {}
};

// TODO(tyler): At some point, this can probably be templated for all ranges.
struct temperature_range
{
    float mLower;
    float mUpper;

    temperature_range(float Lower, float Upper)
            :
            mLower(Lower), mUpper(Upper)
    {
    }

    temperature_range() : temperature_range(0.f, 1.f) {}
};

static std::string PhaseToString(phase Phase)
{
    switch(Phase)
    {
        case(phase::Solid): return "Solid";
        case(phase::Liquid): return "Liquid";
        case(phase::Gas): return "Gas";
        default: return "Invalid";
    }
}

#define PHASE_HPP
#endif
