
#pragma once

#include <SFML/Graphics.hpp>

class Asteroid {
public:
    Asteroid(sf::Vector2f position, float radius, float speed);

    void update(float deltaTime, float worldWidth, float worldHeight);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    float getRadius() const;
    float getSpeed() const;

    void setPosition(sf::Vector2f position);

    bool isOffscreen(float worldHeight) const;

private:
    sf::CircleShape shape;
    float radius;
    float speed;
};
