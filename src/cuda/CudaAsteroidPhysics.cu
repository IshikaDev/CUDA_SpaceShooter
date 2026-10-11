
#include "cuda/CudaAsteroidPhysics.hpp"
#include "entities/Asteroid.hpp"

#include <cuda_runtime.h>

#include <algorithm>
#include <cstdio>
#include <vector>

namespace {

__global__ void updateAsteroidPositions(
    float* x,
    float* y,
    const float* speed,
    int count,
    float deltaTime
) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index < count) {
        // Each GPU thread updates one asteroid.
        y[index] += speed[index] * deltaTime;
    }
}

} // namespace

CudaAsteroidPhysics::CudaAsteroidPhysics() {
    int deviceCount = 0;

    if (cudaGetDeviceCount(&deviceCount) != cudaSuccess ||
        deviceCount == 0) {
        cudaGetLastError();
        std::fprintf(
            stderr,
            "[CUDA] No CUDA-capable device available.\n"
        );
        return;
    }

    available = true;

    std::printf(
        "[CUDA] GPU asteroid physics initialized.\n"
    );
}

CudaAsteroidPhysics::~CudaAsteroidPhysics() {
    release();
}

bool CudaAsteroidPhysics::isAvailable() const {
    return available;
}

bool CudaAsteroidPhysics::reserve(std::size_t count) {
    if (count <= capacity) {
        return true;
    }

    release();

    if (count == 0) {
        return true;
    }

    const std::size_t bytes = count * sizeof(float);

    if (cudaMalloc(&deviceX, bytes) != cudaSuccess ||
        cudaMalloc(&deviceY, bytes) != cudaSuccess ||
        cudaMalloc(&deviceSpeed, bytes) != cudaSuccess) {
        std::fprintf(
            stderr,
            "[CUDA] Device buffer allocation failed.\n"
        );

        release();
        available = false;
        return false;
    }

    capacity = count;
    return true;
}

void CudaAsteroidPhysics::release() {
    if (deviceX) {
        cudaFree(deviceX);
        deviceX = nullptr;
    }

    if (deviceY) {
        cudaFree(deviceY);
        deviceY = nullptr;
    }

    if (deviceSpeed) {
        cudaFree(deviceSpeed);
        deviceSpeed = nullptr;
    }

    capacity = 0;
}

bool CudaAsteroidPhysics::update(
    std::vector<Asteroid>& asteroids,
    float deltaTime
) {
    if (!available) {
        return false;
    }

    const std::size_t count = asteroids.size();

    if (count == 0) {
        return true;
    }

    if (count > static_cast<std::size_t>(2147483647)) {
        return false;
    }

    if (!reserve(count)) {
        return false;
    }

    std::vector<float> x(count);
    std::vector<float> y(count);
    std::vector<float> speed(count);

    for (std::size_t i = 0; i < count; ++i) {
        const auto position = asteroids[i].getPosition();

        x[i] = position.x;
        y[i] = position.y;
        speed[i] = asteroids[i].getSpeed();
    }

    const std::size_t bytes = count * sizeof(float);

    if (cudaMemcpy(
            deviceX, x.data(), bytes,
            cudaMemcpyHostToDevice) != cudaSuccess ||
        cudaMemcpy(
            deviceY, y.data(), bytes,
            cudaMemcpyHostToDevice) != cudaSuccess ||
        cudaMemcpy(
            deviceSpeed, speed.data(), bytes,
            cudaMemcpyHostToDevice) != cudaSuccess) {
        std::fprintf(stderr, "[CUDA] Upload failed.\n");
        available = false;
        release();
        return false;
    }

    constexpr int threadsPerBlock = 256;
    const int blocks = static_cast<int>(
        (count + threadsPerBlock - 1) / threadsPerBlock
    );

    updateAsteroidPositions<<<blocks, threadsPerBlock>>>(
        static_cast<float*>(deviceX),
        static_cast<float*>(deviceY),
        static_cast<float*>(deviceSpeed),
        static_cast<int>(count),
        deltaTime
    );

    if (cudaGetLastError() != cudaSuccess ||
        cudaDeviceSynchronize() != cudaSuccess) {
        std::fprintf(stderr, "[CUDA] Kernel execution failed.\n");
        available = false;
        release();
        return false;
    }

    if (cudaMemcpy(
            y.data(), deviceY, bytes,
            cudaMemcpyDeviceToHost) != cudaSuccess) {
        std::fprintf(stderr, "[CUDA] Download failed.\n");
        available = false;
        release();
        return false;
    }

    for (std::size_t i = 0; i < count; ++i) {
        asteroids[i].setPosition({x[i], y[i]});
    }

    return true;
}
