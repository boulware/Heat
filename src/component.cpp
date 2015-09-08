#include "component.hpp"

phase component::GetPhase() const
{
    return mPhase;
}
    
void component::Update(cell_grid& Grid)
{
    float TemperatureSum = 0;

    for(auto& cell_position : mPositions)
    {
        TemperatureSum += Grid.GetCell(cell_position.x, cell_position.y)->Temperature;
    }

    mTemperature = TemperatureSum / (float)mPositions.size();

    mPhase = GetMaterialPhase(mMaterial, mTemperature);
}
