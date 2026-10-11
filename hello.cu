#include <cstdio>
#include <cuda_runtime.h>

__global__ void helloGPU() {
    printf("Hello from CUDA thread %d!\n", threadIdx.x);
}

int main() {
    helloGPU<<<1, 5>>>();

    cudaError_t err = cudaDeviceSynchronize();

    if (err != cudaSuccess) {
        printf("CUDA error: %s\n", cudaGetErrorString(err));
        return 1;
    }

    printf("CUDA is working!\n");
    return 0;
}