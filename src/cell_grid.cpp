#include "cell_grid.hpp"

#include "constants.hpp"

// TODO(tyler): This should be scaled by an outside factor so that you can see differences over small temperatures on systems with small temperature gradients (similar to temperature gun scaling)
void hsv(double Temperature, sf::Color& Result, double MinTemp = 0.0, double MaxTemp = 400.0)
{
    uint8_t Index = (Temperature - MinTemp) / ((MaxTemp - MinTemp) / 100);
    Result.r = constants::Colors[4 * (uint8_t)Index + 0];
    Result.g = constants::Colors[4 * (uint8_t)Index + 1];
    Result.b = constants::Colors[4 * (uint8_t)Index + 2];
    Result.a = 255;
}
    
cell_grid::cell_grid(int Width, int Height)
        :
        MinTemp(0.0), MaxTemp(500.0),
        mWidth(Width), mHeight(Height),
        mThisCollection(Width * Height, {0.8, 1.0}),
        mPrevCollection(Width * Height, {0.8, 1.0}),
        mVertices(sf::Quads, 4 * Width * Height)
{
    setScale(constants::Divider, constants::Divider);
        
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

void cell_grid::StepSubCollection()
{
    StepSubCollection(0, mWidth * mHeight);
}
    
void cell_grid::StepSubCollection(unsigned int MinIndex, unsigned int MaxIndex)
{
    mPrevCollection = mThisCollection;
        
    float dt = 0.1;

    for(unsigned i = MinIndex; i < MaxIndex; i++)
    {
        for(int yOffset = -1; yOffset <= 1; yOffset += 2)
        {
            for(int mult = mWidth; mult >= 1; mult -= (mWidth - 1))
            {
                for(int nOffset = -1; nOffset <= 1; nOffset += 2)
                {   
                    unsigned int iN = std::min(i + nOffset * mult, mWidth * mHeight - 1);
                    float dT = mPrevCollection[iN].Temperature - mPrevCollection[i].Temperature;
                    float R = mPrevCollection[iN].Resistance + mPrevCollection[i].Resistance;
                    float Q = (c * (dT / R)) * dt;

                    mThisCollection[iN].Temperature -= Q;
                    mThisCollection[i].Temperature += Q;
                }
            }
        }
    }
}

cell* cell_grid::GetCell(unsigned int CellX, unsigned int CellY)
{
    // TODO(tyler): This inappropriately returns a pointer to a cell even if you input invalid cell coords.
    CellX = std::min(CellX, mWidth - 1);
    CellY = std::min(CellY, mHeight - 1);

    return &mThisCollection[(CellY * mWidth) + CellX];
}

void cell_grid::SetCell(unsigned int CellX, unsigned int CellY, float NewTemperature, float NewResistance)
{
    *GetCell(CellX, CellY) = {NewTemperature, NewResistance};
}

void cell_grid::UpdateCellColors()
{
    for(int x = 0; x < mPrevCollection.size(); x++)
    {
        sf::Vertex* Quad = &mVertices[4 * x];

        float T = mPrevCollection[x].Temperature;
        sf::Color TemperatureColor;
        hsv(T, TemperatureColor, MinTemp, MaxTemp);

        Quad[0].color = TemperatureColor;
        Quad[1].color = TemperatureColor;
        Quad[2].color = TemperatureColor;
        Quad[3].color = TemperatureColor;
    }
}
    
    
void cell_grid::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
        
    target.draw(mVertices, states);
}
