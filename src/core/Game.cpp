
#include "core/Game.hpp"

#include <algorithm>
#include <string>

Game::Game()
    : window(
          sf::VideoMode({WIDTH, HEIGHT}),
          "CUDA Space Shooter | RTX 5060"
      ),
      background(
          static_cast<float>(WIDTH),
          static_cast<float>(HEIGHT)
      ),
      player(
          static_cast<float>(WIDTH),
          static_cast<float>(HEIGHT)
      ),
      levelManager(
          static_cast<float>(WIDTH),
          static_cast<float>(HEIGHT)
      ) {
    window.setFramerateLimit(60);

    // Windows system font for the HUD.
    fontLoaded = hudFont.openFromFile(
        "C:/Windows/Fonts/arial.ttf"
    );
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

            if (gameOver &&
                key->code == sf::Keyboard::Key::R) {
                resetGame();
            }
        }
    }
}

void Game::update(float deltaTime) {
    background.update(deltaTime);

    if (gameOver) {
        return;
    }

    if (invulnerabilityTimer > 0.0f) {
        invulnerabilityTimer -= deltaTime;
    }

    player.update(deltaTime);
    player.shoot(bullets);

    for (auto& bullet : bullets) {
        bullet.update(deltaTime);
    }

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

    levelManager.update(deltaTime, cudaPhysics);

    // Award points for asteroids destroyed by bullets.
    const int destroyed =
        collisionSystem.resolveBulletAsteroidCollisions(
            bullets,
            levelManager.getAsteroids()
        );

    score += destroyed * 10;

    // Detect ship-versus-asteroid collisions.
    if (invulnerabilityTimer <= 0.0f) {
        auto& asteroids = levelManager.getAsteroids();

        for (auto it = asteroids.begin();
             it != asteroids.end();) {
            if (player.getBounds()
                    .findIntersection(it->getBounds())
                    .has_value()) {
                it = asteroids.erase(it);

                --lives;
                invulnerabilityTimer = 1.2f;

                if (lives <= 0) {
                    lives = 0;
                    gameOver = true;
                }

                // One hit per invulnerability window.
                break;
            } else {
                ++it;
            }
        }
    }

    updateLevel();
}

void Game::updateLevel() {
    // Increase the level every 30 seconds.
    levelTimer += 1.0f / 60.0f;

    if (levelTimer >= 30.0f) {
        levelTimer = 0.0f;
        ++level;
    }
}

void Game::resetGame() {
    score = 0;
    lives = 3;
    level = 1;
    levelTimer = 0.0f;
    invulnerabilityTimer = 0.0f;
    gameOver = false;

    bullets.clear();
    player.reset();
    levelManager.reset();
    frameClock.restart();
}

void Game::render() {
    window.clear(sf::Color(3, 5, 20));

    background.draw(window);

    levelManager.draw(window);

    // Blink the player briefly after taking damage.
    if (invulnerabilityTimer <= 0.0f ||
        static_cast<int>(invulnerabilityTimer * 10) % 2 == 0) {
        player.draw(window);
    }

    for (const auto& bullet : bullets) {
        bullet.draw(window);
    }

    if (fontLoaded) {
        sf::Text hud(
            hudFont,
            "SCORE: " + std::to_string(score) +
            "     LIVES: " + std::to_string(lives) +
            "     LEVEL: " + std::to_string(level),
            20
        );

        hud.setPosition({20.f, 16.f});
        hud.setFillColor(sf::Color(180, 235, 255));
        window.draw(hud);

        if (gameOver) {
            sf::RectangleShape overlay(
                sf::Vector2f(
                    static_cast<float>(WIDTH),
                    static_cast<float>(HEIGHT)
                )
            );

            overlay.setFillColor(sf::Color(0, 0, 12, 190));
            window.draw(overlay);

            sf::Text title(hudFont, "GAME OVER", 48);
            title.setFillColor(sf::Color(255, 90, 120));

            const auto titleBounds = title.getLocalBounds();
            title.setOrigin({
                titleBounds.position.x +
                    titleBounds.size.x / 2.f,
                titleBounds.position.y +
                    titleBounds.size.y / 2.f
            });
            title.setPosition({
                WIDTH / 2.f,
                HEIGHT / 2.f - 45.f
            });

            sf::Text result(
                hudFont,
                "Final Score: " + std::to_string(score),
                24
            );
            result.setFillColor(sf::Color::White);

            const auto resultBounds = result.getLocalBounds();
            result.setOrigin({
                resultBounds.position.x +
                    resultBounds.size.x / 2.f,
                resultBounds.position.y +
                    resultBounds.size.y / 2.f
            });
            result.setPosition({
                WIDTH / 2.f,
                HEIGHT / 2.f + 15.f
            });

            sf::Text restart(hudFont, "Press R to Restart", 20);
            restart.setFillColor(sf::Color(100, 225, 255));

            const auto restartBounds = restart.getLocalBounds();
            restart.setOrigin({
                restartBounds.position.x +
                    restartBounds.size.x / 2.f,
                restartBounds.position.y +
                    restartBounds.size.y / 2.f
            });
            restart.setPosition({
                WIDTH / 2.f,
                HEIGHT / 2.f + 65.f
            });

            window.draw(title);
            window.draw(result);
            window.draw(restart);
        }
    }

    window.display();
}
