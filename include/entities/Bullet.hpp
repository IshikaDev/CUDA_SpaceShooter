
#pragma once

#include <SFML/Graphics.hpp>

class Bullet {
public:
    explicit Bullet(sf::Vector2f position);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;
    bool isOffscreen() const;

private:
    sf::RectangleShape shape;
    float speed = 600.0f;
};

