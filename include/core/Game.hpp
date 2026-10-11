
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "core/GameState.hpp"
#include "entities/Player.hpp"
#include "entities/Bullet.hpp"

class Game {
public:
    Game();
    void run();

private:
    static constexpr unsigned int WIDTH = 1000;
    static constexpr unsigned int HEIGHT = 700;

    sf::RenderWindow window;
    sf::Clock frameClock;
    GameState state = GameState::Playing;

    Player player;
    std::vector<Bullet> bullets;

    void processEvents();
    void update(float deltaTime);
    void render();
};
