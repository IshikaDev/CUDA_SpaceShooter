
#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>

class Background {
public:
    Background(float width, float height);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

private:
    struct Star {
        sf::CircleShape shape;
        float speed;
        float phase;
        float baseAlpha;
    };

    float width;
    float height;
    float elapsedTime = 0.0f;

    std::vector<Star> stars;
    std::vector<sf::CircleShape> nebulaClouds;

    void createNebula();
    void createStars();
};
