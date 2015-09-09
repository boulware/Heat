#include "item.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include "component_grid.hpp"

item::item(std::string ItemFilePath, environment& SpawnEnvironment)
{
    mEnvironmentTemperature = SpawnEnvironment.mTemperature;
    
    std::ifstream ItemFile(ItemFilePath);

    unsigned int ItemWidth = 0;
    unsigned int ItemHeight = 0;

    // TODO(tyler): Implement a check so that all grid cells must be connected? Maybe not?
    if(ItemFile.is_open())
    {
        std::string Line;

        mComponents.insert(std::pair<std::string, component>("environment", {SpawnEnvironment.mMaterial, mEnvironmentTemperature}));
                    
        std::map<char, component*> ComponentMap;
        ComponentMap.insert({'.', &mComponents["environment"]});

        int StartOfItemData = 0;
        
        while(ItemFile >> Line)
        {
            std::smatch m;
            if(std::regex_match(Line, m, std::regex("^/(.)=(.*),(.*)$")))
            {
                component NewComponent(materials::Materials["unspecified"], mEnvironmentTemperature);
                if(materials::Materials.find(m.str(3)) != materials::Materials.end())
                {
                    NewComponent.SetMaterial(materials::Materials[m.str(3)]);
                }
                
                mComponents.insert({m.str(2), NewComponent});
                ComponentMap.insert({const_cast<char*>(m.str(1).c_str())[0], &mComponents[m.str(2)]});
                
                StartOfItemData = ItemFile.tellg();
            }
            else
            {
                ItemWidth = std::max(ItemWidth, static_cast<unsigned int>(Line.length()));
                ItemHeight++;
            }
        }

        for(const auto& component : mComponents)
        {
            mNextComponentTemperature[component.first] = component.second.GetTemperature();
        }
        
        mComponentGrid = component_grid(ItemWidth, ItemHeight);
        
        ItemFile.clear();
        // TODO(tyler): Ew. Please fix.
        ItemFile.seekg(StartOfItemData + 2);
        char CellID;
        unsigned int CellX = 0;
        unsigned int CellY = 0;
        
        while(ItemFile >> std::noskipws >> CellID)
        {
            switch(CellID)
            {
                case('\n'):
                {
                    CellY++;
                    CellX = 0;
                } break;
                default:
                {
                    mComponentGrid.SetCell(CellX, CellY, ComponentMap[CellID]);

                    CellX++;
                } break;
            }
        }

        mComponentSurfaces = mComponentGrid.CalculateSurfaces();
    }
    else
    {
        std::cout << "\nFailed to open " << ItemFilePath;
    }

    mComponentGrid.setScale(5.0, 5.0);
}
    
void item::Update()
{
    for(auto& component_surface : mComponentSurfaces)
    {
        component_surface.Update();
    }

    for(auto& component : mComponents)
    {
        component.second.Update();
    }

    mComponents["environment"].SetTemperature(mEnvironmentTemperature);
    mComponentGrid.UpdateGridTexture();
}
    
float item::GetComponentTemperature(std::string ComponentID)
{
    /*
    if(mComponents.find(ComponentID) != mComponents.end())
    {
        // The component exists in this item
        float TemperatureSum = 0;

        for(auto& cell_position : mComponents[ComponentID].mPositions)
        {
            TemperatureSum += mHeatGrid.GetCell(cell_position.x, cell_position.y)->Temperature;
        }

        return TemperatureSum / (float)mComponents[ComponentID].mPositions.size();
    }
    else
    {
        // The component is not a component of this item
        std::cout << "\nTried to get an item's component's temperature, but the component was not a part of that item!";

        return -1.0;
    }
    */

    return 0.f;
}

void item::MoveToEnvironment(environment& Environment)
{
    mEnvironmentTemperature = Environment.mTemperature;
    
    mComponents["environment"].SetTemperature(mEnvironmentTemperature);
    mComponents["environment"].SetMaterial(Environment.mMaterial);
}

phase item::GetComponentPhase(std::string ComponentID)
{
    if(mComponents.find(ComponentID) != mComponents.end())
    {
        // The component exists in this item
//        return mPrevComponents[ComponentID].GetPhase();
        return mComponents[ComponentID].mPhase;
    }
    else
    {
        // The component is not a component of this item
        std::cout << "\nTried to get an item's component's phase, but the component was not a part of that item!";

        return phase::Invalid;
    }        
}

void item::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    target.draw(mComponentGrid, states);
}

std::string item::GetStateString()
{
    std::stringstream StateString;

    for(const auto& component : mComponents)
    {
        StateString
            << component.first
            << " [" << PhaseToString(component.second.mPhase) << "]"
            << " T: " << component.second.GetTemperature()
            << ", L: " << component.second.mLatentHeat
            << "\n";
    }

    return StateString.str();
}
