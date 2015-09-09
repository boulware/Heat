#if !defined(TEMPERATURE_SCALE_HPP)

#include <SFML/Graphics.hpp>

class temperature_scale : public sf::Drawable, public sf::Transformable
{
public:
    sf::FloatRect mRect;
    float mMinTemp, mMaxTemp;
    sf::Text mMinText, mMaxText;
    sf::Texture mTexture;
    sf::Sprite mSprite;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
 
public:
    temperature_scale(float MinTemp, float MaxTemp, sf::FloatRect Rect, const sf::Font& Font);
};
 
#define TEMPERATURE_SCALE_HPP
#endif
