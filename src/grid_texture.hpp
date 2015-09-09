#if !defined(GRID_TEXTURE_HPP)

#include <SFML/Graphics.hpp>

#include "component.hpp"

class grid_texture : public sf::Drawable, public sf::Transformable
{

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;
public:
    unsigned int mWidth, mHeight;
    sf::VertexArray mVertices;
    
    grid_texture(unsigned int Width, unsigned int Height);
    void UpdateCellColors(std::vector<component*>& CellComponents);
};

#define GRID_TEXTURE_HPP
#endif
