#if !defined(PHASE_HPP)

#include <string>

#include "material.hpp"

enum class phase
{
    Invalid,
    Solid,
    Liquid,
    Gas,
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

static phase GetMaterialPhase(material& Material, float Temperature, float Pressure = 0)
{
    if(Temperature < Material.mMeltingPoint) return phase::Solid;
    else if(Temperature < Material.mBoilingPoint) return phase::Liquid;
    else return phase::Gas;
}

#define PHASE_HPP
#endif
