#if !defined(COMPONENT_GRID_HPP)

#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "constants.hpp"
#include "component.hpp"
#include "surface.hpp"
#include "grid_texture.hpp"

class component_grid : public sf::Drawable, public sf::Transformable
{
private:
    unsigned int mWidth, mHeight;
    std::vector<component*> mCells;
    grid_texture mGridTexture;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
public:
    component_grid(unsigned int Width, unsigned int Height);
    component_grid() : component_grid(0, 0) {}

    component* GetCell(unsigned int X, unsigned int Y) const;
    void SetCell(unsigned int X, unsigned int Y, component* Component);
    std::vector<surface> CalculateSurfaces();
    void UpdateGridTexture();
};

#define COMPONENT_GRID_HPP
#endif
