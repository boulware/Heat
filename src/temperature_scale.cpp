#include "temperature_scale.hpp"

#include <iostream>

#include "constants.hpp"

void temperature_scale::draw(sf::RenderTarget& target, sf::RenderStates states) const
{       
    states.transform *= getTransform();

    target.draw(mSprite, states);
    target.draw(mMinText, states);
    target.draw(mMaxText, states);
}
 

temperature_scale::temperature_scale(float MinTemp, float MaxTemp, sf::FloatRect Rect)
        :
        mMinTemp(MinTemp), mMaxTemp(MaxTemp),
        mMinText(std::to_string(static_cast<int>(MinTemp)), constants::fontCourierNew, 18),
        mMaxText(std::to_string(static_cast<int>(MaxTemp)), constants::fontCourierNew, 18),
        mRect(Rect)
{
    setPosition(mRect.left, mRect.top);
    mSprite.setPosition(0, 0);
    mSprite.setScale(mRect.width / static_cast<float>(constants::Colors.size() / 4), mRect.height);
        
    sf::Image Image;
    Image.create(constants::Colors.size() / 4, 1, constants::Colors.data());
        
    if(!mTexture.loadFromImage(Image))
    {
        std::cout << "\nFailed to load texture";
    }

    mSprite.setTexture(mTexture);

    float TextYOffset = 5.0;
    float TextXOffset = 5.0;
        
    mMinText.setOrigin(0.f, 18.f);
    mMinText.setPosition(TextXOffset, -TextYOffset);

    mMaxText.setOrigin(mMinText.getLocalBounds().width, 18.f);
    mMaxText.setPosition(mRect.width - TextXOffset, -TextYOffset);
}
   
