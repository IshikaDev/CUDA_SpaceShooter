
#include "entities/Asteroid.hpp"

Asteroid::Asteroid(
    sf::Vector2f position,
    float asteroidRadius,
    float asteroidSpeed
)
    : radius(asteroidRadius), speed(asteroidSpeed) {
    shape.setRadius(radius);
    shape.setOrigin({radius, radius});
    shape.setPosition(position);
    shape.setFillColor(sf::Color(155, 160, 180));
    shape.setOutlineThickness(1.5f);
    shape.setOutlineColor(sf::Color(95, 105, 135));
}

void Asteroid::update(
    float deltaTime,
    float worldWidth,
    float worldHeight
) {
    (void)worldWidth;
    (void)worldHeight;

    shape.move({0.0f, speed * deltaTime});
}

void Asteroid::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

sf::FloatRect Asteroid::getBounds() const {
    return shape.getGlobalBounds();
}

sf::Vector2f Asteroid::getPosition() const {
    return shape.getPosition();
}

float Asteroid::getRadius() const {
    return radius;
}

float Asteroid::getSpeed() const {
    return speed;
}

void Asteroid::setPosition(sf::Vector2f position) {
    shape.setPosition(position);
}

bool Asteroid::isOffscreen(float worldHeight) const {
    return shape.getPosition().y - radius > worldHeight;
}
