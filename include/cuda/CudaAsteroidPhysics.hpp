
#pragma once

#include <cstddef>
#include <vector>

class Asteroid;

class CudaAsteroidPhysics {
public:
    CudaAsteroidPhysics();
    ~CudaAsteroidPhysics();

    CudaAsteroidPhysics(const CudaAsteroidPhysics&) = delete;
    CudaAsteroidPhysics& operator=(const CudaAsteroidPhysics&) = delete;

    bool isAvailable() const;

    bool update(
        std::vector<Asteroid>& asteroids,
        float deltaTime
    );

private:
    void* deviceX = nullptr;
    void* deviceY = nullptr;
    void* deviceSpeed = nullptr;

    std::size_t capacity = 0;
    bool available = false;

    bool reserve(std::size_t count);
    void release();
};
