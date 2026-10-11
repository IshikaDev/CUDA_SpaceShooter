
#include "systems/CollisionSystem.hpp"

#include <algorithm>

int CollisionSystem::resolveBulletAsteroidCollisions(
    std::vector<Bullet>& bullets,
    std::vector<Asteroid>& asteroids
) {
    int destroyed = 0;

    for (auto bullet = bullets.begin(); bullet != bullets.end();) {
        bool hit = false;

        for (auto asteroid = asteroids.begin();
             asteroid != asteroids.end();) {

            if (bullet->getBounds().findIntersection(
                    asteroid->getBounds()).has_value()) {

                asteroid = asteroids.erase(asteroid);
                bullet = bullets.erase(bullet);

                ++destroyed;
                hit = true;
                break;
            } else {
                ++asteroid;
            }
        }

        if (!hit) {
            ++bullet;
        }
    }

    return destroyed;
}
