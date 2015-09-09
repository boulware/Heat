#include "grid_texture.hpp"

#include <iostream>

// TODO(tyler): This should be scaled by an outside factor so that you can see differences over small temperatures on systems with small temperature gradients (similar to temperature gun scaling)
// TODO(tyler): Need to do some bound checks (or implement some kind of color look-up table class, which is probably the better option)
static sf::Color hsv(double Temperature, double MinTemp = 0.0, double MaxTemp = 1000.0)
{
    sf::Color TempColor;
    
    uint8_t Index = (Temperature - MinTemp) / ((MaxTemp - MinTemp) / 100);
    TempColor.r = constants::Colors[4 * (uint8_t)Index + 0];
    TempColor.g = constants::Colors[4 * (uint8_t)Index + 1];
    TempColor.b = constants::Colors[4 * (uint8_t)Index + 2];
    TempColor.a = 255;
    
    return TempColor;
}

grid_texture::grid_texture(unsigned int Width, unsigned int Height)
        :
        mWidth(Width), mHeight(Height),
        mVertices(sf::Quads, 4 * Width * Height)
{        
    for(int y = 0; y < Height; y++)
    {
        for(int x = 0; x < Width; x++)
        {
            sf::Vertex* Quad = &mVertices[4 * (y * Width + x)];

            Quad[0].position = sf::Vector2f(x, y);
            Quad[1].position = sf::Vector2f((x + 1), y);
            Quad[2].position = sf::Vector2f((x + 1), (y + 1));
            Quad[3].position = sf::Vector2f(x, (y + 1));      
        }
    }
}

void grid_texture::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    target.draw(mVertices, states);
}

void grid_texture::UpdateCellColors(std::vector<component*>& CellComponents)
{
    if(mVertices.getVertexCount() == CellComponents.size() * 4)
    {
        for(int y = 0; y < mHeight; y++)
        {
            for(int x = 0; x < mWidth; x++)
            {
                unsigned int i = y * mWidth + x;
                
                sf::Vertex* Quad = &mVertices[4 * i];

                sf::Color QuadColor = hsv(CellComponents[i]->GetTemperature());
                                          
                Quad[0].color = QuadColor;
                Quad[1].color = QuadColor;
                Quad[2].color = QuadColor;
                Quad[3].color = QuadColor;
            }
        }
    }
    else
    {
        std::cout << "\nERROR: Grid texture fed invalid components!";
    }
}
