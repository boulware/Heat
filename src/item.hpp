#if !defined(ITEM_HPP)

#include <string>

#include "environment.hpp"
#include "component.hpp"
#include "component_grid.hpp"
#include "surface.hpp"

class item : public sf::Drawable, public sf::Transformable
{
private:
    float mEnvironmentTemperature;
    std::vector<surface> mComponentSurfaces;
    std::map<std::string, component> mComponents;
    std::map<std::string, float> mNextComponentTemperature;;
    component_grid mComponentGrid;
public:
    item(std::string ItemFile, environment& SpawnEnvironment);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    
    void Update();

    void MoveToEnvironment(environment& Environment);

    float GetComponentTemperature(std::string ComponentID);

    phase GetComponentPhase(std::string ComponentID);

    std::string GetStateString();
};

#define ITEM_HPP
#endif
