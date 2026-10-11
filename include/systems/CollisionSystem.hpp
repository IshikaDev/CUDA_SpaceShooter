
#pragma once

#include "entities/Asteroid.hpp"
#include "entities/Bullet.hpp"

#include <vector>

class CollisionSystem {
public:
    // Returns the number of asteroids destroyed.
    int resolveBulletAsteroidCollisions(
        std::vector<Bullet>& bullets,
        std::vector<Asteroid>& asteroids
    );
};
