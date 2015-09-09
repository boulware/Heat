#include "component.hpp"

component::component(material Material, float Temperature)
        :
        mMaterial(Material), mTemperature(Temperature), mLatentHeat(0.f)
{
    mPhase = mMaterial.GetPhase(mTemperature);
}

float component::GetTemperature() const
{
    return mTemperature;
}

material component::GetMaterial() const
{
    return mMaterial;
}

void component::SetTemperature(float Temperature)
{
    mTemperature = Temperature;
    mPhase = mMaterial.GetPhase(mTemperature);
}

void component::SetMaterial(material Material)
{
    mMaterial = Material;
    mPhase = mMaterial.GetPhase(mTemperature);
}

void component::Update()
{
//    mPhase = mMaterial.GetPhase(mTemperature);
}

void component::AddEnergy(float EnergyAmount)
{
    float dT  = (EnergyAmount / mMaterial.mSpecificHeat);

    phase_transition PhaseTransition = mMaterial.GetPhaseTransition(mTemperature, mTemperature + dT);

    if(PhaseTransition.mTemperature == -1.f)
    {
        mTemperature += dT;
    }
    else
    {   
        mTemperature = PhaseTransition.mTemperature;
        
        mLatentHeat += EnergyAmount;

        if(mLatentHeat >= PhaseTransition.mLatentHeat)
        {
            mPhase = PhaseTransition.mFinalPhase;
            mTemperature += (EnergyAmount / mMaterial.mSpecificHeat);
            mLatentHeat = 0.f;
        }
    }
}
