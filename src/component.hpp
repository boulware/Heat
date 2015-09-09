#if !defined(COMPONENT_HPP)

#include <vector>

#include <SFML/System.hpp>

#include "constants.hpp"
#include "phase.hpp"
#include "cell_grid.hpp"
#include "material.hpp"

class component
{
private:
    float mTemperature;
    material mMaterial = materials::Materials["unspecified"];
public:
    float mLatentHeat;
    phase mPhase;

    component(material Material, float Temperature);
    component() : component(materials::Materials["unspecified"], 0.0) {}
    
    float GetTemperature() const;
    material GetMaterial() const;
    
    void SetTemperature(float Temperature);
    void SetMaterial(material Material);
    
    void Update();
    void AddEnergy(float EnergyAmount);
};

#define COMPONENT_HPP
#endif
