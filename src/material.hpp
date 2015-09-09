#if !defined(MATERIAL_HPP)

#include <map>

#include "constants.hpp"
#include "phase.hpp"

struct material
{
    float mResistance;
    float mDensity;
    float mSpecificHeat;
    // TODO(tyler): At some point, I need to find a way to consolidate these two maps into one that can be used to effectively determine the phase for a given temperature.
    // Maybe consolidate into a phase_diagram object? That would be cleaner overall, but still be kind of weird
    std::map<phase, temperature_range> mPhaseRanges;
    std::map<std::pair<phase, phase>, float> mPhaseTransitionPoints;
    std::map<std::pair<phase, phase>, float> mLatentHeats;

    material(float Resistance, float MeltingPoint, float BoilingPoint, float Density, float SpecificHeat, float LatentHeatOfFusion, float LatentHeatOfVaporization)
            :
            mResistance(Resistance),
            mDensity(Density),
            mSpecificHeat(SpecificHeat)
    {
        mPhaseRanges[phase::Solid] = {0, MeltingPoint};
        mPhaseRanges[phase::Liquid] = {MeltingPoint, BoilingPoint};
        mPhaseRanges[phase::Gas] = {BoilingPoint, constants::Infinity};
        mPhaseTransitionPoints[std::pair<phase, phase>(phase::Solid, phase::Liquid)] = MeltingPoint;
        mPhaseTransitionPoints[std::pair<phase, phase>(phase::Liquid, phase::Gas)] = BoilingPoint;

        for(const auto& phase_transition_point : mPhaseTransitionPoints)
        {
            phase FirstPhase = phase_transition_point.first.first;
            phase SecondPhase = phase_transition_point.first.second;
            if(FirstPhase > SecondPhase)
            {
                mPhaseTransitionPoints.erase(phase_transition_point.first);
                mPhaseTransitionPoints[std::pair<phase, phase>(SecondPhase, FirstPhase)] = phase_transition_point.second;
            }
        }
        
        mLatentHeats[std::pair<phase, phase>(phase::Solid, phase::Liquid)] = LatentHeatOfFusion;
        mLatentHeats[std::pair<phase, phase>(phase::Liquid, phase::Gas)] = LatentHeatOfVaporization;

        for(const auto& latent_heat : mLatentHeats)
        {
            phase FirstPhase = latent_heat.first.first;
            phase SecondPhase = latent_heat.first.second;
            if(FirstPhase > SecondPhase)
            {
                mLatentHeats.erase(latent_heat.first);
                mLatentHeats[std::pair<phase, phase>(SecondPhase, FirstPhase)] = latent_heat.second;
            }
        }
    }

    material()
            :
            material(1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f)
    {
    }
    
    phase GetPhase(float Temperature)
    {
        for(auto& phase_range : mPhaseRanges)
        {
            if(Temperature >= phase_range.second.mLower &&  Temperature <= phase_range.second.mUpper)
            {
                return phase_range.first;
            }
        }

        return phase::Invalid;
    }
    
    phase_transition GetPhaseTransition(float InitialTemperature, float FinalTemperature)
    {
        phase_transition PhaseTransition;

        PhaseTransition.mInitialPhase = GetPhase(InitialTemperature);
        PhaseTransition.mFinalPhase = GetPhase(FinalTemperature);
        
        if(PhaseTransition.mInitialPhase == PhaseTransition.mFinalPhase)
        {
            PhaseTransition.mTemperature = -1.f;
            PhaseTransition.mLatentHeat = -1.f;
        }
        else if(PhaseTransition.mInitialPhase < PhaseTransition.mFinalPhase)
        {
            PhaseTransition.mTemperature = mPhaseTransitionPoints[std::pair<phase, phase>(PhaseTransition.mInitialPhase, PhaseTransition.mFinalPhase)];
            PhaseTransition.mLatentHeat = mLatentHeats[std::pair<phase, phase>(PhaseTransition.mInitialPhase, PhaseTransition.mFinalPhase)];
        }
        else
        {
            PhaseTransition.mTemperature = mPhaseTransitionPoints[std::pair<phase, phase>(PhaseTransition.mFinalPhase, PhaseTransition.mInitialPhase)];
            PhaseTransition.mLatentHeat = mLatentHeats[std::pair<phase, phase>(PhaseTransition.mFinalPhase, PhaseTransition.mInitialPhase)];
        }

        return PhaseTransition;
    }

};

namespace materials
{
    static std::map<std::string, material> Materials = {
        // {Resistance, MeltingPoint, BoilingPoint, Density, SpecificHeat, LatentHeatOfFusion, LatentHeatOfVaporization}
        {"unspecified", {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f}},
        {"water", {1.8f, 273.f, 373.f, 1000.f, 4186.f, 3.34e5f, 2.26e6f}},
        {"polystyrene", {1000.f, 513.f, 1000.f, 1000.f, 1400.f, 7.14e4f, 0.f}},
        {"air", {60.f, 63.f, 77.f, 1.2f, 1000.f, 2.57e4, 2.00e5f}},
        {"aluminum6061", {5.99e-3f, 873.f, 2800.f, 2700.f, 896.f, 3.98e5f, 1.05e7f}}
    };
}
#define MATERIAL_HPP
#endif
