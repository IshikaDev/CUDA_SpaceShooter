
#include "entities/Player.hpp"

#include <algorithm>

Player::Player(float width, float height)
    : worldWidth(width), worldHeight(height) {

    ship.setPointCount(3);
    ship.setPoint(0, {0.0f, 0.0f});
    ship.setPoint(1, {20.0f, 36.0f});
    ship.setPoint(2, {-20.0f, 36.0f});

    ship.setFillColor(sf::Color::Cyan);
    ship.setPosition({width / 2.0f, height - 85.0f});
}

void Player::update(float deltaTime) {
    shootCooldown = std::max(0.0f, shootCooldown - deltaTime);

    sf::Vector2f movement{0.0f, 0.0f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        movement.x -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        movement.x += 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        movement.y -= 1.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        movement.y += 1.0f;

    // Normalize diagonal movement.
    const float length = std::sqrt(
        movement.x * movement.x + movement.y * movement.y
    );

    if (length > 0.0f) {
        movement.x /= length;
        movement.y /= length;
    }

    ship.move({
        movement.x * movementSpeed * deltaTime,
        movement.y * movementSpeed * deltaTime
    });

    sf::Vector2f p = ship.getPosition();
    p.x = std::clamp(p.x, 20.0f, worldWidth - 20.0f);
    p.y = std::clamp(p.y, 0.0f, worldHeight - 36.0f);
    ship.setPosition(p);
}

void Player::shoot(std::vector<Bullet>& bullets) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
        && shootCooldown <= 0.0f) {

        const auto position = ship.getPosition();
        bullets.emplace_back(
            sf::Vector2f{position.x, position.y + 5.0f}
        );

        shootCooldown = SHOOT_INTERVAL;
    }
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(ship);
}

sf::FloatRect Player::getBounds() const {
    return ship.getGlobalBounds();
}

void Player::reset() {
    ship.setPosition({worldWidth / 2.0f, worldHeight - 85.0f});
    shootCooldown = 0.0f;
}
