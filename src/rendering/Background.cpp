
#include "rendering/Background.hpp"

#include <algorithm>
#include <cmath>
#include <random>

Background::Background(float width, float height)
    : width(width), height(height) {
    createNebula();
    createStars();
}

void Background::createNebula() {
    auto addCloud = [this](
        float x, float y, float radius, sf::Color color
    ) {
        sf::CircleShape cloud(radius, 80);
        cloud.setOrigin({radius, radius});
        cloud.setPosition({x, y});
        cloud.setFillColor(color);
        nebulaClouds.push_back(cloud);
    };

    addCloud(width * 0.20f, height * 0.30f, 250.f,
             sf::Color(75, 20, 135, 20));
    addCloud(width * 0.23f, height * 0.32f, 175.f,
             sf::Color(105, 30, 180, 20));
    addCloud(width * 0.27f, height * 0.34f, 100.f,
             sf::Color(75, 45, 210, 18));

    addCloud(width * 0.79f, height * 0.47f, 280.f,
             sf::Color(10, 65, 150, 22));
    addCloud(width * 0.80f, height * 0.44f, 195.f,
             sf::Color(10, 115, 175, 20));
    addCloud(width * 0.74f, height * 0.48f, 120.f,
             sf::Color(25, 75, 205, 18));

    addCloud(width * 0.48f, height * 0.95f, 230.f,
             sf::Color(100, 20, 135, 16));
}

void Background::createStars() {
    std::mt19937 rng(5060);

    std::uniform_real_distribution<float> xDist(0.f, width);
    std::uniform_real_distribution<float> yDist(0.f, height);
    std::uniform_real_distribution<float> sizeDist(0.7f, 2.1f);
    std::uniform_real_distribution<float> speedDist(8.f, 38.f);
    std::uniform_real_distribution<float> alphaDist(110.f, 235.f);
    std::uniform_real_distribution<float> phaseDist(0.f, 6.28318f);

    for (int i = 0; i < 190; ++i) {
        const float radius = sizeDist(rng);

        Star star{
            sf::CircleShape(radius),
            speedDist(rng),
            phaseDist(rng),
            alphaDist(rng)
        };

        star.shape.setPosition({xDist(rng), yDist(rng)});

        if (i % 9 == 0) {
            star.shape.setFillColor(sf::Color(100, 210, 255));
        } else if (i % 5 == 0) {
            star.shape.setFillColor(sf::Color(190, 150, 255));
        } else {
            star.shape.setFillColor(sf::Color(220, 230, 255));
        }

        stars.push_back(star);
    }
}

void Background::update(float deltaTime) {
    elapsedTime += deltaTime;

    for (auto& star : stars) {
        auto position = star.shape.getPosition();
        position.y += star.speed * deltaTime;

        if (position.y > height + 3.f) {
            position.y = -3.f;
        }

        star.shape.setPosition(position);

        const float pulse =
            0.72f + 0.28f *
            std::sin(elapsedTime * 1.8f + star.phase);

        const auto alpha = static_cast<std::uint8_t>(
            std::clamp(star.baseAlpha * pulse, 0.f, 255.f)
        );

        auto color = star.shape.getFillColor();
        color.a = alpha;
        star.shape.setFillColor(color);
    }
}

void Background::draw(sf::RenderWindow& window) const {
    for (const auto& cloud : nebulaClouds) {
        window.draw(cloud);
    }

    for (const auto& star : stars) {
        window.draw(star.shape);
    }
}
