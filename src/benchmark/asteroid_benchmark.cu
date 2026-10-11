
#include <cuda_runtime.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define CUDA_CHECK(call)                                                   \
    do {                                                                   \
        cudaError_t err = (call);                                          \
        if (err != cudaSuccess) {                                          \
            std::fprintf(stderr, "CUDA error: %s\n",                       \
                         cudaGetErrorString(err));                         \
            std::exit(EXIT_FAILURE);                                       \
        }                                                                  \
    } while (0)

__global__ void updateAsteroidPositions(
    const float* x,
    float* y,
    const float* speed,
    int count,
    float deltaTime)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < count) {
        // Same position-update calculation as the game.
        y[i] += speed[i] * deltaTime;
    }
}

using Clock = std::chrono::steady_clock;

static double elapsedMs(Clock::time_point start,
                        Clock::time_point end)
{
    return std::chrono::duration<double, std::milli>(
        end - start).count();
}

static void cpuUpdate(std::vector<float>& y,
                      const std::vector<float>& speed,
                      float dt)
{
    for (std::size_t i = 0; i < y.size(); ++i) {
        y[i] += speed[i] * dt;
    }
}

static float maxDifference(const std::vector<float>& a,
                           const std::vector<float>& b)
{
    float maximum = 0.0f;

    for (std::size_t i = 0; i < a.size(); ++i) {
        float difference = std::fabs(a[i] - b[i]);

        if (difference > maximum) {
            maximum = difference;
        }
    }

    return maximum;
}

int main()
{
    int device = 0;
    cudaDeviceProp properties{};

    CUDA_CHECK(cudaGetDevice(&device));
    CUDA_CHECK(cudaGetDeviceProperties(&properties, device));

    std::printf("CUDA Asteroid Physics Benchmark\n");
    std::printf("GPU: %s\n", properties.name);
    std::printf("CUDA capability: %d.%d\n",
                properties.major, properties.minor);
    std::printf("Iterations per measurement: 100\n\n");

    const int sizes[] = {
        1000, 10000, 100000, 1000000
    };

    constexpr int repetitions = 100;
    constexpr float dt = 0.016f;
    constexpr int threadsPerBlock = 256;

    std::printf(
        "%-12s %14s %16s %18s %14s %12s\n",
        "Asteroids",
        "CPU ms",
        "GPU kernel ms",
        "GPU end-to-end ms",
        "Kernel speedup",
        "Max error");

    std::printf(
        "-----------------------------------------------------------------------------------------------\n");

    for (int count : sizes) {
        std::vector<float> x(count);
        std::vector<float> initialY(count);
        std::vector<float> speed(count);

        for (int i = 0; i < count; ++i) {
            x[i] = static_cast<float>(i % 1000);
            initialY[i] = static_cast<float>(i % 700);
            speed[i] = 10.0f + static_cast<float>(i % 90);
        }

        // CPU baseline: 100 updates over the same input.
        std::vector<float> cpuY = initialY;

        auto cpuStart = Clock::now();

        for (int iteration = 0;
             iteration < repetitions; ++iteration) {
            cpuUpdate(cpuY, speed, dt);
        }

        auto cpuEnd = Clock::now();

        double cpuTotalMs = elapsedMs(cpuStart, cpuEnd);
        double cpuAverageMs = cpuTotalMs / repetitions;

        // Allocate GPU buffers.
        float* deviceX = nullptr;
        float* deviceY = nullptr;
        float* deviceSpeed = nullptr;

        std::size_t bytes =
            static_cast<std::size_t>(count) * sizeof(float);

        CUDA_CHECK(cudaMalloc(&deviceX, bytes));
        CUDA_CHECK(cudaMalloc(&deviceY, bytes));
        CUDA_CHECK(cudaMalloc(&deviceSpeed, bytes));

        int blocks =
            (count + threadsPerBlock - 1) / threadsPerBlock;

        // Warm-up launch and initial upload.
        CUDA_CHECK(cudaMemcpy(
            deviceX, x.data(), bytes, cudaMemcpyHostToDevice));

        CUDA_CHECK(cudaMemcpy(
            deviceY, initialY.data(), bytes,
            cudaMemcpyHostToDevice));

        CUDA_CHECK(cudaMemcpy(
            deviceSpeed, speed.data(), bytes,
            cudaMemcpyHostToDevice));

        updateAsteroidPositions<<<blocks, threadsPerBlock>>>(
            deviceX, deviceY, deviceSpeed, count, dt);

        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaDeviceSynchronize());

        // Kernel-only timing: data stays resident on the GPU.
        CUDA_CHECK(cudaMemcpy(
            deviceY, initialY.data(), bytes,
            cudaMemcpyHostToDevice));

        cudaEvent_t kernelStart, kernelStop;

        CUDA_CHECK(cudaEventCreate(&kernelStart));
        CUDA_CHECK(cudaEventCreate(&kernelStop));

        CUDA_CHECK(cudaEventRecord(kernelStart));

        for (int iteration = 0;
             iteration < repetitions; ++iteration) {
            updateAsteroidPositions<<<blocks, threadsPerBlock>>>(
                deviceX, deviceY, deviceSpeed, count, dt);
        }

        CUDA_CHECK(cudaGetLastError());
        CUDA_CHECK(cudaEventRecord(kernelStop));
        CUDA_CHECK(cudaEventSynchronize(kernelStop));

        float kernelTotalMs = 0.0f;

        CUDA_CHECK(cudaEventElapsedTime(
            &kernelTotalMs, kernelStart, kernelStop));

        double kernelAverageMs =
            kernelTotalMs / repetitions;

        std::vector<float> kernelY(count);

        CUDA_CHECK(cudaMemcpy(
            kernelY.data(), deviceY, bytes,
            cudaMemcpyDeviceToHost));

        float kernelError = maxDifference(cpuY, kernelY);

        // End-to-end timing: upload all inputs, launch, synchronize,
        // and copy updated positions back on every iteration.
        std::vector<float> endToEndY = initialY;

        auto endToEndStart = Clock::now();

        for (int iteration = 0;
             iteration < repetitions; ++iteration) {
            CUDA_CHECK(cudaMemcpy(
                deviceX, x.data(), bytes,
                cudaMemcpyHostToDevice));

            CUDA_CHECK(cudaMemcpy(
                deviceY, endToEndY.data(), bytes,
                cudaMemcpyHostToDevice));

            CUDA_CHECK(cudaMemcpy(
                deviceSpeed, speed.data(), bytes,
                cudaMemcpyHostToDevice));

            updateAsteroidPositions<<<blocks, threadsPerBlock>>>(
                deviceX, deviceY, deviceSpeed, count, dt);

            CUDA_CHECK(cudaGetLastError());
            CUDA_CHECK(cudaDeviceSynchronize());

            CUDA_CHECK(cudaMemcpy(
                endToEndY.data(), deviceY, bytes,
                cudaMemcpyDeviceToHost));
        }

        auto endToEndEnd = Clock::now();

        double endToEndAverageMs =
            elapsedMs(endToEndStart, endToEndEnd)
            / repetitions;

        float endToEndError =
            maxDifference(cpuY, endToEndY);

        float maximumError =
            (kernelError > endToEndError)
                ? kernelError
                : endToEndError;

        double kernelSpeedup =
            kernelAverageMs > 0.0
                ? cpuAverageMs / kernelAverageMs
                : 0.0;

        std::printf(
            "%-12d %14.6f %16.6f %18.6f %13.3fx %12.8f\n",
            count,
            cpuAverageMs,
            kernelAverageMs,
            endToEndAverageMs,
            kernelSpeedup,
            maximumError);

        if (maximumError > 1e-3f) {
            std::fprintf(
                stderr,
                "WARNING: numerical difference exceeds tolerance "
                "for %d asteroids.\n",
                count);
        }

        CUDA_CHECK(cudaEventDestroy(kernelStart));
        CUDA_CHECK(cudaEventDestroy(kernelStop));

        CUDA_CHECK(cudaFree(deviceX));
        CUDA_CHECK(cudaFree(deviceY));
        CUDA_CHECK(cudaFree(deviceSpeed));
    }

    CUDA_CHECK(cudaDeviceReset());

    return 0;
}
