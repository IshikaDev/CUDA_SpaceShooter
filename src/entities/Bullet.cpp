
#include "entities/Bullet.hpp"

Bullet::Bullet(sf::Vector2f position) {
    shape.setSize({4.0f, 14.0f});
    shape.setOrigin({2.0f, 7.0f});
    shape.setPosition(position);
    shape.setFillColor(sf::Color::Cyan);
}

void Bullet::update(float deltaTime) {
    shape.move({0.0f, -speed * deltaTime});
}

void Bullet::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

sf::FloatRect Bullet::getBounds() const {
    return shape.getGlobalBounds();
}

bool Bullet::isOffscreen() const {
    return shape.getPosition().y < -20.0f;
}
