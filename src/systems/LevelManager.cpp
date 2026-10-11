
#include "systems/LevelManager.hpp"
#include "cuda/CudaAsteroidPhysics.hpp"

#include <algorithm>
#include <random>

LevelManager::LevelManager(float width, float height)
    : worldWidth(width),
      worldHeight(height),
      rng(std::random_device{}()) {
}

void LevelManager::updateSpawning(float deltaTime) {
    spawnTimer += deltaTime;

    while (spawnTimer >= spawnInterval) {
        spawnTimer -= spawnInterval;
        spawnAsteroid();
    }
}

void LevelManager::update(float deltaTime) {
    updateSpawning(deltaTime);

    for (auto& asteroid : asteroids) {
        asteroid.update(deltaTime, worldWidth, worldHeight);
    }

    removeOffscreenAsteroids();
}

void LevelManager::update(
    float deltaTime,
    CudaAsteroidPhysics& cudaPhysics
) {
    updateSpawning(deltaTime);

    // If GPU physics fails, update the asteroids on the CPU
    // during this frame instead.
    if (!cudaPhysics.update(asteroids, deltaTime)) {
        for (auto& asteroid : asteroids) {
            asteroid.update(deltaTime, worldWidth, worldHeight);
        }
    }

    removeOffscreenAsteroids();
}

void LevelManager::removeOffscreenAsteroids() {
    asteroids.erase(
        std::remove_if(
            asteroids.begin(),
            asteroids.end(),
            [this](const Asteroid& asteroid) {
                return asteroid.isOffscreen(worldHeight);
            }
        ),
        asteroids.end()
    );
}

void LevelManager::draw(sf::RenderWindow& window) const {
    for (const auto& asteroid : asteroids) {
        asteroid.draw(window);
    }
}

std::vector<Asteroid>& LevelManager::getAsteroids() {
    return asteroids;
}

const std::vector<Asteroid>& LevelManager::getAsteroids() const {
    return asteroids;
}

void LevelManager::reset() {
    asteroids.clear();
    spawnTimer = 0.0f;
    level = 1;
    spawnInterval = 0.8f;
}

void LevelManager::spawnAsteroid() {
    std::uniform_real_distribution<float> xDistribution(
        15.0f, worldWidth - 15.0f
    );

    std::uniform_real_distribution<float> radiusDistribution(
        10.0f, 22.0f
    );

    std::uniform_real_distribution<float> speedDistribution(
        100.0f, 190.0f
    );

    const float radius = radiusDistribution(rng);

    asteroids.emplace_back(
        sf::Vector2f{
            xDistribution(rng),
            -radius
        },
        radius,
        speedDistribution(rng)
    );
}
