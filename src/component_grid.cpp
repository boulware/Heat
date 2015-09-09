#include "component_grid.hpp"

component_grid::component_grid(unsigned int Width, unsigned int Height)
        :
        mWidth(Width), mHeight(Height),
        mCells(mWidth * mHeight, nullptr),
        mGridTexture(mWidth, mHeight)
{
}

component* component_grid::GetCell(unsigned int X, unsigned int Y) const
{
    return mCells[Y * mWidth + X];
}

void component_grid::SetCell(unsigned int X, unsigned int Y, component* Component)
{
    mCells[Y * mWidth + X] = Component;
}

std::vector<surface> component_grid::CalculateSurfaces()
{
    std::map<std::pair<component*, component*>, unsigned int> SurfaceData;

    for(unsigned int y = 0; y < mHeight; y++)
    {
        for(unsigned int x = 0; x < mWidth; x++)
        {
            if(x + 1 < mWidth)
            {
// Right neighbor exists
// NOTE(tyler): Silly little algorithm to ensure that each std::pair generated is symmetric (surface between A and B is the same as a surface between B and A)
                std::pair<component*, component*> OrderedPair = std::pair<component*, component*>(GetCell(x, y), GetCell(x + 1, y));
                if(OrderedPair.first > OrderedPair.second) OrderedPair = std::pair<component*, component*>(OrderedPair.second, OrderedPair.first);

// NOTE(tyler): We don't care about "surfaces" between elements of the same component (yet!)
                if(OrderedPair.first != OrderedPair.second)
                {
                    SurfaceData[OrderedPair] += 1;
                }
            }
            if(y + 1 < mHeight)
            {
// Down neighbor exists
                std::pair<component*, component*> OrderedPair = std::pair<component*, component*>(GetCell(x, y), GetCell(x, y + 1));
                if(OrderedPair.first > OrderedPair.second) OrderedPair = std::pair<component*, component*>(OrderedPair.second, OrderedPair.first);

                if(OrderedPair.first != OrderedPair.second)
                {
                    SurfaceData[OrderedPair] += 1;
                }
            }
        }
    }

    std::vector<surface> ComponentSurfaces;

    for(auto& surface_data : SurfaceData)
    {
//        surface TempSurface
//        TempSurface.mComponentPair = surface_data.first;
//        TempSurface.mSurfaceArea = surface_data.second;
        ComponentSurfaces.push_back(surface(surface_data.first, surface_data.second));
    }

    return ComponentSurfaces;
}

void component_grid::UpdateGridTexture()
{
    mGridTexture.UpdateCellColors(mCells);
}

void component_grid::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    target.draw(mGridTexture, states);
}
