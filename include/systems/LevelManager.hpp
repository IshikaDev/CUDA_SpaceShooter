
#pragma once

#include "entities/Asteroid.hpp"

#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

class CudaAsteroidPhysics;

class LevelManager {
public:
    LevelManager(float worldWidth, float worldHeight);

    void update(float deltaTime);
    void update(float deltaTime, CudaAsteroidPhysics& cudaPhysics);

    void draw(sf::RenderWindow& window) const;

    std::vector<Asteroid>& getAsteroids();
    const std::vector<Asteroid>& getAsteroids() const;

    void reset();

private:
    float worldWidth;
    float worldHeight;

    float spawnTimer = 0.0f;
    float spawnInterval = 0.8f;
    int level = 1;

    std::vector<Asteroid> asteroids;
    std::mt19937 rng;

    void spawnAsteroid();
    void updateSpawning(float deltaTime);
    void removeOffscreenAsteroids();
};
