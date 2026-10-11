
#pragma once

#include <SFML/Graphics.hpp>
#include "entities/Bullet.hpp"

#include <vector>

class Player {
public:
    Player(float worldWidth, float worldHeight);

    void update(float deltaTime);
    void shoot(std::vector<Bullet>& bullets);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getBounds() const;
    void reset();

private:
    sf::ConvexShape ship;
    float worldWidth;
    float worldHeight;
    float movementSpeed = 320.0f;
    float shootCooldown = 0.0f;
    static constexpr float SHOOT_INTERVAL = 0.22f;
};
