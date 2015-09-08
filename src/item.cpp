#include "item.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

item::item(std::string ItemFilePath, environment& SpawnEnvironment)
{
    std::ifstream ItemFile(ItemFilePath);

    unsigned int ItemWidth = 0;
    unsigned int ItemHeight = 0;

    // TODO(tyler): Implement a check so that all grid cells must be connected? Maybe not?
    if(ItemFile.is_open())
    {
        std::string Line;

        component EnvironmentComponent;
        EnvironmentComponent.mTemperature = SpawnEnvironment.mTemperature;
        EnvironmentComponent.mMaterial = SpawnEnvironment.mMaterial;
        mComponents.insert({"environment", EnvironmentComponent});
                    
        std::map<char, component*> ComponentMap;
        ComponentMap.insert({'.', &mComponents["environment"]});

        int StartOfItemData = 0;
        
        while(ItemFile >> Line)
        {
            std::smatch m;
            if(std::regex_match(Line, m, std::regex("^/(.)=(.*),(.*)$")))
            {
                component NewComponent;
                NewComponent.mTemperature = SpawnEnvironment.mTemperature;
                if(constants::Materials.find(m.str(3)) != constants::Materials.end())
                {
                    NewComponent.mMaterial = constants::Materials[m.str(3)];
                }
                else std::cout << "\nInvalid material found in " << ItemFilePath;
                
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

        mHeatGrid = cell_grid(ItemWidth, ItemHeight);
        mHeatGrid.c = 1.0;
        mHeatGrid.MinTemp = 200.0;
        mHeatGrid.MaxTemp = 600.0;
    
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
                    mHeatGrid.SetCell(CellX, CellY, SpawnEnvironment.mTemperature, ComponentMap[CellID]->mMaterial.mResistance);
                    ComponentMap[CellID]->mPositions.push_back({CellX, CellY});

                    CellX++;
                } break;
            }
        }
    }
    else
    {
        std::cout << "\nFailed to open " << ItemFilePath;
    }


    std::cout << "\nItem size is (" << ItemWidth << ", " << ItemHeight << ")";
}
    
void item::Update()
{
//    ResetEnvironmentCells();
    mHeatGrid.StepSubCollection();
    for(auto& component_map : mComponents)
    {
        component& Component = component_map.second;

        Component.Update(mHeatGrid);
    }
    mHeatGrid.UpdateCellColors();
}
    
// TODO(tyler): ComponentID is intrinsically linked to a MaterialID, which is not a good thing.
float item::GetComponentTemperature(std::string ComponentID)
{
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
}

void item::MoveToEnvironment(environment& Environment)
{
    mComponents["environment"].mTemperature = Environment.mTemperature;
    mComponents["environment"].mMaterial = Environment.mMaterial;

    for(auto& cell_position : mComponents["environment"].mPositions)
    {
        mHeatGrid.SetCell(cell_position.x, cell_position.y, Environment.mTemperature, Environment.mMaterial.mResistance);
    }
}

phase item::GetComponentPhase(std::string ComponentID)
{
    if(mComponents.find(ComponentID) != mComponents.end())
    {
        // The component exists in this item
        return mComponents[ComponentID].GetPhase();
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

    target.draw(mHeatGrid, states);
}

std::string item::GetStateString()
{
    std::stringstream StateString;

    for(const auto& component_pair : mComponents)
    {
        StateString
            << component_pair.first
            << " [" << PhaseToString(component_pair.second.GetPhase()) << "]"
            << " T: " << component_pair.second.mTemperature
            << "\n";
    }

    return StateString.str();
}
