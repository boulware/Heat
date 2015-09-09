#if !defined(SURFACE_HPP)

#include <utility>

#include "component.hpp"

class surface
{
private:
    std::pair<component*, component*> mComponentPair;
    unsigned int mSurfaceArea = 0;
public:
    surface(std::pair<component*, component*> ComponentPair, unsigned int SurfaceArea);

    void Update(float dt = 1.0);
};

#define SURFACE_HPP
#endif
