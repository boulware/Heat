#if !defined(COMPONENT_HPP)

#include <vector>

#include <SFML/System.hpp>

#include "constants.hpp"
#include "phase.hpp"
#include "cell_grid.hpp"

class component
{
private:
    phase mPhase;
public:
//    std::vector<std::pair<unsigned int, component*>> mNeighbors;
    std::vector<sf::Vector2u> mPositions;
    float mTemperature = 273;
    material mMaterial = constants::Materials["unspecified"];

    phase GetPhase() const;

    void Update(cell_grid& Grid);
};

#define COMPONENT_HPP
#endif
