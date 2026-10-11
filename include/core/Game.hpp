
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "core/GameState.hpp"
#include "entities/Player.hpp"
#include "entities/Bullet.hpp"
#include "systems/LevelManager.hpp"
#include "systems/CollisionSystem.hpp"
#include "cuda/CudaAsteroidPhysics.hpp"
#include "rendering/Background.hpp"

class Game {
public:
    Game();
    void run();

private:
    static constexpr unsigned int WIDTH = 1000;
    static constexpr unsigned int HEIGHT = 700;

    sf::RenderWindow window;
    sf::Clock frameClock;
    sf::Font hudFont;

    GameState state = GameState::Playing;

    Background background;
    Player player;
    LevelManager levelManager;
CudaAsteroidPhysics cudaPhysics;
    CollisionSystem collisionSystem;

    std::vector<Bullet> bullets;

    int score = 0;
    int lives = 3;
    int level = 1;

    float invulnerabilityTimer = 0.0f;
    float levelTimer = 0.0f;

    bool gameOver = false;
    bool fontLoaded = false;

    void processEvents();
    void update(float deltaTime);
    void render();
    void resetGame();
    void updateLevel();
};
