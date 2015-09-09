#include "surface.hpp"

#include "constants.hpp"

surface::surface(std::pair<component*, component*> ComponentPair, unsigned int SurfaceArea)
        :
        mComponentPair(ComponentPair), mSurfaceArea(SurfaceArea)
{
    
}

void surface::Update(float dt)
{
    float dT = mComponentPair.first->GetTemperature() - mComponentPair.second->GetTemperature();
    float R = mComponentPair.first->GetMaterial().mResistance + mComponentPair.second->GetMaterial().mResistance;
    float Q = (constants::c * (dT / R)) * mSurfaceArea * dt;

    mComponentPair.first->AddEnergy(-Q);
    mComponentPair.second->AddEnergy(Q);
}
