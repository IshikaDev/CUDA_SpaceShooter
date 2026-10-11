
#include <SFML/Graphics.hpp>
#include <cuda_runtime.h>

#include <cmath>
#include <iostream>
#include <vector>

struct Asteroid {
    float x, y, vx, vy;
};

// Each CUDA thread updates one asteroid.
__global__ void updateAsteroids(
    float* x, float* y,
    const float* vx, const float* vy,
    int count, float dt,
    float width, float height)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= count) return;

    x[i] += vx[i] * dt;
    y[i] += vy[i] * dt;

    if (x[i] < 0) x[i] += width;
    if (x[i] >= width) x[i] -= width;
    if (y[i] < 0) y[i] += height;
    if (y[i] >= height) y[i] -= height;
}

int main()
{
    constexpr unsigned W = 1000;
    constexpr unsigned H = 700;
    constexpr int N = 2000;

    sf::RenderWindow window(
        sf::VideoMode({W, H}),
        "CUDA Space Shooter | RTX 5060"
    );
    window.setFramerateLimit(60);

    std::vector<Asteroid> asteroids(N);
    for (int i = 0; i < N; ++i) {
        asteroids[i] = {
            static_cast<float>((i * 73) % W),
            static_cast<float>((i * 137) % H),
            static_cast<float>((i % 7) - 3) * 18.0f,
            static_cast<float>((i % 9) - 4) * 15.0f
        };
    }

    std::vector<float> x(N), y(N), vx(N), vy(N);
    for (int i = 0; i < N; ++i) {
        x[i] = asteroids[i].x;
        y[i] = asteroids[i].y;
        vx[i] = asteroids[i].vx;
        vy[i] = asteroids[i].vy;
    }

    float *dx = nullptr, *dy = nullptr, *dvx = nullptr, *dvy = nullptr;
    auto checkCuda = [](cudaError_t err, const char* operation) {
        if (err != cudaSuccess) {
            std::cerr << operation << ": "
                      << cudaGetErrorString(err) << '\n';
            std::exit(EXIT_FAILURE);
        }
    };

    checkCuda(cudaMalloc(&dx, N * sizeof(float)), "cudaMalloc x");
    checkCuda(cudaMalloc(&dy, N * sizeof(float)), "cudaMalloc y");
    checkCuda(cudaMalloc(&dvx, N * sizeof(float)), "cudaMalloc vx");
    checkCuda(cudaMalloc(&dvy, N * sizeof(float)), "cudaMalloc vy");

    checkCuda(cudaMemcpy(dx, x.data(), N * sizeof(float),
                         cudaMemcpyHostToDevice), "copy x");
    checkCuda(cudaMemcpy(dy, y.data(), N * sizeof(float),
                         cudaMemcpyHostToDevice), "copy y");
    checkCuda(cudaMemcpy(dvx, vx.data(), N * sizeof(float),
                         cudaMemcpyHostToDevice), "copy vx");
    checkCuda(cudaMemcpy(dvy, vy.data(), N * sizeof(float),
                         cudaMemcpyHostToDevice), "copy vy");

    sf::CircleShape player(15.f, 3);
    player.setFillColor(sf::Color::Cyan);
    player.setOrigin({15.f, 15.f});
    player.setPosition({W / 2.f, H - 80.f});

    sf::CircleShape rock(3.f);
    rock.setFillColor(sf::Color(190, 190, 205));

    sf::Clock clock;
    float elapsed = 0.f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const float dt = std::min(clock.restart().asSeconds(), 0.05f);
        elapsed += dt;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            player.move({-300.f * dt, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            player.move({300.f * dt, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            player.move({0.f, -300.f * dt});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            player.move({0.f, 300.f * dt});

        auto p = player.getPosition();
        p.x = std::clamp(p.x, 15.f, W - 15.f);
        p.y = std::clamp(p.y, 15.f, H - 15.f);
        player.setPosition(p);

        updateAsteroids<<<(N + 255) / 256, 256>>>(
            dx, dy, dvx, dvy, N, dt, (float)W, (float)H);
        checkCuda(cudaGetLastError(), "CUDA kernel launch");
        checkCuda(cudaMemcpy(x.data(), dx, N * sizeof(float),
                             cudaMemcpyDeviceToHost), "copy positions x");
        checkCuda(cudaMemcpy(y.data(), dy, N * sizeof(float),
                             cudaMemcpyDeviceToHost), "copy positions y");

        window.clear(sf::Color(5, 8, 22));

        for (int i = 0; i < N; ++i) {
            rock.setPosition({x[i], y[i]});
            window.draw(rock);
        }

        // Simple rotating player ship.
        player.setRotation(sf::degrees(std::sin(elapsed) * 4.f));
        window.draw(player);
        window.display();
    }

    cudaFree(dx);
    cudaFree(dy);
    cudaFree(dvx);
    cudaFree(dvy);
    return 0;
}
