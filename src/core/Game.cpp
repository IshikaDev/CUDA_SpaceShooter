
#include "core/Game.hpp"

#include <algorithm>
#include <cmath>

Game::Game()
    : window(
        sf::VideoMode({WIDTH, HEIGHT}),
        "CUDA Space Shooter | RTX 5060"
    ),
      player(static_cast<float>(WIDTH), static_cast<float>(HEIGHT)) {

    window.setFramerateLimit(60);
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();

        const float deltaTime =
            std::min(frameClock.restart().asSeconds(), 0.05f);

        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* key =
                event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }
    }
}

void Game::update(float deltaTime) {
    player.update(deltaTime);
    player.shoot(bullets);

    for (auto& bullet : bullets)
        bullet.update(deltaTime);

    bullets.erase(
    std::remove_if(
        bullets.begin(),
        bullets.end(),
        [](const Bullet& bullet) {
            return bullet.isOffscreen();
        }
    ),
    bullets.end()
);
}

void Game::render() {
    window.clear(sf::Color(5, 8, 22));

    // Temporary decorative stars.
    for (int i = 0; i < 180; ++i) {
        sf::CircleShape star(1.5f);
        star.setFillColor(sf::Color(185, 190, 215));
        star.setPosition({
            static_cast<float>((i * 73) % WIDTH),
            static_cast<float>((i * 137) % HEIGHT)
        });
        window.draw(star);
    }

    player.draw(window);

    for (const auto& bullet : bullets)
        bullet.draw(window);

    window.display();
}
