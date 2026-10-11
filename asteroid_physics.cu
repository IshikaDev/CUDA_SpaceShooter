
#include <cuda_runtime.h>
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstdlib>

#define CUDA_CHECK(call) do {                                  \
    cudaError_t err = (call);                                  \
    if (err != cudaSuccess) {                                  \
        std::fprintf(stderr, "CUDA error: %s\n",                \
                     cudaGetErrorString(err));                 \
        return 1;                                               \
    }                                                          \
} while (0)

__global__ void updateAsteroids(
    float* x, float* y,
    const float* vx, const float* vy,
    int count, float dt,
    float width, float height)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < count) {
        float nextX = x[i] + vx[i] * dt;
        float nextY = y[i] + vy[i] * dt;

        // Wrap asteroids around the screen.
        if (nextX < 0.0f) nextX += width;
        if (nextX >= width) nextX -= width;
        if (nextY < 0.0f) nextY += height;
        if (nextY >= height) nextY -= height;

        x[i] = nextX;
        y[i] = nextY;
    }
}

int main(int argc, char** argv)
{
    int count = 100000;
    if (argc > 1) count = std::atoi(argv[1]);

    if (count <= 0) {
        std::fprintf(stderr, "Count must be positive.\n");
        return 1;
    }

    const float dt = 0.016f;
    const float width = 1280.0f;
    const float height = 720.0f;

    std::vector<float> x(count), y(count), vx(count), vy(count);
    std::vector<float> cpuX, cpuY;

    for (int i = 0; i < count; ++i) {
        x[i] = std::fmod(i * 17.3f, width);
        y[i] = std::fmod(i * 11.7f, height);
        vx[i] = (i % 101 - 50) * 0.1f;
        vy[i] = (i % 79 - 39) * 0.1f;
    }

    cpuX = x;
    cpuY = y;

    // CPU reference implementation.
    for (int i = 0; i < count; ++i) {
        cpuX[i] += vx[i] * dt;
        cpuY[i] += vy[i] * dt;

        if (cpuX[i] < 0.0f) cpuX[i] += width;
        if (cpuX[i] >= width) cpuX[i] -= width;
        if (cpuY[i] < 0.0f) cpuY[i] += height;
        if (cpuY[i] >= height) cpuY[i] -= height;
    }

    float *dx, *dy, *dvx, *dvy;

    CUDA_CHECK(cudaMalloc(&dx, count * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&dy, count * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&dvx, count * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&dvy, count * sizeof(float)));

    CUDA_CHECK(cudaMemcpy(dx, x.data(), count * sizeof(float),
                          cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dy, y.data(), count * sizeof(float),
                          cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dvx, vx.data(), count * sizeof(float),
                          cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dvy, vy.data(), count * sizeof(float),
                          cudaMemcpyHostToDevice));

    int threads = 256;
    int blocks = (count + threads - 1) / threads;

    updateAsteroids<<<blocks, threads>>>(
        dx, dy, dvx, dvy, count, dt, width, height);

    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(x.data(), dx, count * sizeof(float),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(y.data(), dy, count * sizeof(float),
                          cudaMemcpyDeviceToHost));

    float maxError = 0.0f;
    for (int i = 0; i < count; ++i) {
        maxError = fmaxf(maxError, fabsf(x[i] - cpuX[i]));
        maxError = fmaxf(maxError, fabsf(y[i] - cpuY[i]));
    }

    std::printf("GPU: %s\n", cudaGetErrorString(cudaSuccess));
    std::printf("GPU: RTX 5060 target sm_120\n");
    std::printf("Asteroids: %d\n", count);
    std::printf("Threads per block: %d\n", threads);
    std::printf("Blocks: %d\n", blocks);
    std::printf("Maximum CPU/GPU difference: %.9g\n", maxError);
    std::printf("Validation: %s\n",
                maxError < 0.001f ? "PASS" : "FAIL");

    cudaFree(dx);
    cudaFree(dy);
    cudaFree(dvx);
    cudaFree(dvy);

    return maxError < 0.001f ? 0 : 1;
}
