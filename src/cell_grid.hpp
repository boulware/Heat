#if !defined(CELL_GRID_HPP)

#include <vector>

#include <SFML/Graphics.hpp>

#include "cell.hpp"

// TODO(tyler): A cell_grid and its graphical representation need to be separated because of both OOP and performance (hsv() can be expensive).
class cell_grid : public sf::Drawable, public sf::Transformable
{
public:
    float MinTemp, MaxTemp;
    float c;
    unsigned int mWidth, mHeight;
    std::vector<cell> mThisCollection;
    std::vector<cell> mPrevCollection;
    
    cell_grid(int Width, int Height);
    cell_grid() : cell_grid(0, 0) {}

    void StepSubCollection();
    
    void StepSubCollection(unsigned int MinIndex, unsigned int MaxIndex);

    cell* GetCell(unsigned int CellX, unsigned int CellY);
    void SetCell(unsigned int CellX, unsigned int CellY, float NewTemperature, float NewResistance);
    void UpdateCellColors();
    
private:
    sf::VertexArray mVertices;
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#define CELL_GRID_HPP
#endif
