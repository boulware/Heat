#if !defined(ITEM_HPP)

#include <string>

#include "cell_grid.hpp"
#include "environment.hpp"
#include "component.hpp"

class item : public sf::Drawable, public sf::Transformable
{
private:
    cell_grid mHeatGrid;
    std::map<std::string, component> mComponents;
public:
    item(std::string ItemFile, environment& SpawnEnvironment);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    
    void Update();

    void MoveToEnvironment(environment& Environment);

    // TODO(tyler): ComponentID is intrinsically linked to a MaterialID, which is not a good thing.
    float GetComponentTemperature(std::string ComponentID);

    phase GetComponentPhase(std::string ComponentID);

    std::string GetStateString();
};

#define ITEM_HPP
#endif
