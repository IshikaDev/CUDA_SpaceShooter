\# CUDA Space Shooter



A 2D space shooter built with C++, SFML, and NVIDIA CUDA. The project explores GPU-accelerated asteroid physics, CUDA kernel execution, PTX inspection, and CPU-versus-GPU performance measurement.



The goal is to build a practical understanding of how CUDA executes parallel workloads, where GPU acceleration helps, and how data-transfer overhead affects real application performance.



\## Features



\- \*\*2D gameplay:\*\* Player movement, bullets, falling asteroids, collision detection, score, lives, game-over state, and restart functionality.

\- \*\*GPU-accelerated asteroid physics:\*\* A CUDA kernel updates asteroid positions in parallel.

\- \*\*CPU fallback:\*\* Asteroid movement can fall back to CPU updates when CUDA is unavailable or an update fails.

\- \*\*PTX generation and inspection:\*\* Compile the CUDA kernel to NVIDIA PTX and inspect its low-level instructions.

\- \*\*Standalone benchmark:\*\* Compare CPU execution, GPU kernel-only execution, and GPU end-to-end execution at different asteroid counts.



\## Technology Stack



| Component | Technology |

|---|---|

| Language | C++17, CUDA C++ |

| Graphics | SFML 3 |

| Build system | CMake |

| CUDA compiler | NVIDIA `nvcc` |

| GPU | NVIDIA GeForce RTX 5060 Laptop GPU |

| GPU compute capability | 12.0 |

| Platform | Windows |

| Build environment | Visual Studio Build Tools and MSVC |



\## Project Structure



```text

CUDA-Space-Shooter/

├── include/

│   ├── core/

│   │   └── Game.hpp

│   ├── cuda/

│   │   └── CudaAsteroidPhysics.hpp

│   ├── entities/

│   │   ├── Asteroid.hpp

│   │   ├── Bullet.hpp

│   │   └── Player.hpp

│   ├── rendering/

│   └── systems/

├── src/

│   ├── core/

│   ├── cuda/

│   │   └── CudaAsteroidPhysics.cu

│   ├── entities/

│   ├── rendering/

│   ├── systems/

│   ├── benchmark/

│   │   └── asteroid\_benchmark.cu

│   └── main.cpp

├── build/                   # Generated build files; not source

├── CMakeLists.txt

└── README.md

```



The exact contents of generated and optional directories may vary by checkout.



\## Architecture



The game separates gameplay, rendering, and GPU computation.



1\. \*\*Game layer:\*\* Coordinates the game loop and frame updates.

2\. \*\*Entity layer:\*\* Represents the player, bullets, and asteroids.

3\. \*\*Level manager:\*\* Manages asteroid updates, spawning, and removal.

4\. \*\*CUDA physics layer:\*\* Transfers asteroid data to the GPU, launches the update kernel, and returns updated positions.

5\. \*\*Rendering and collision systems:\*\* Handle visual rendering and collision-related gameplay on the CPU.



\### CUDA asteroid update



For each asteroid, the kernel applies the following update:



\\\[

y\_i^{\\,new}=y\_i^{\\,old}+v\_i\\Delta t

\\]



Each CUDA thread processes one asteroid index. Threads that correspond to valid asteroid indices update the associated position; excess threads return without accessing an invalid element.



Conceptually, the kernel performs:



```cpp

\_\_global\_\_ void updateAsteroidPositions(

&#x20;   float\* x,

&#x20;   float\* y,

&#x20;   const float\* speed,

&#x20;   int count,

&#x20;   float deltaTime

) {

&#x20;   int index = blockIdx.x \* blockDim.x + threadIdx.x;



&#x20;   if (index < count) {

&#x20;       y\[index] += speed\[index] \* deltaTime;

&#x20;   }

}

```



This illustrates the parallel update pattern; the integrated game implementation and standalone benchmark may use different parameter details.



The implementation uses GPU memory buffers and CUDA kernel launches, while the CPU remains responsible for the surrounding game logic.



\## Requirements



\- Windows with an NVIDIA CUDA-capable GPU

\- NVIDIA graphics driver

\- CUDA Toolkit 13.1 or a compatible configured toolkit

\- CMake 3.24 or newer

\- Visual Studio Build Tools with a compatible MSVC toolset and Windows SDK

\- SFML 3.x, available through the configured vcpkg installation



The original development environment used the MSVC 14.44 toolset with CUDA Toolkit 13.1. A different toolchain may require configuration changes.



\## Build and Run



Open \*\*Command Prompt (CMD)\*\* in the project root.



\### 1. Configure the compiler environment



The following setup uses the installed MSVC 14.44 toolset and CUDA Toolkit 13.1:



```cmd

set "PATH=C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\VC\\Tools\\MSVC\\14.44.35207\\bin\\HostX64\\x64;C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v13.1\\bin;C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;C:\\Windows\\System32;C:\\Windows;C:\\Windows\\System32\\Wbem"

```



This configures the command lookup path for the current CMD session. The required MSVC and Windows SDK `INCLUDE` and `LIB` environment variables must also be configured.



\### 2. Configure CMake



```cmd

cmake -S . -B build -G "NMake Makefiles" -DCMAKE\_TOOLCHAIN\_FILE="C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE\_BUILD\_TYPE=Release -DCMAKE\_CUDA\_COMPILER="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.1/bin/nvcc.exe" -DCMAKE\_CUDA\_ARCHITECTURES=120

```



Adjust the toolchain path if vcpkg is installed elsewhere.



\### 3. Build the game



```cmd

cmake --build build --target cuda\_space\_shooter

```



\### 4. Run the game



```cmd

build\\cuda\_space\_shooter.exe

```



\## Generate and Inspect PTX



PTX (Parallel Thread Execution) is NVIDIA's virtual instruction-set representation for GPU code. It is useful for understanding how CUDA source code is lowered into GPU-oriented instructions before machine-specific execution.



Generate PTX from the integrated CUDA source:



```cmd

"C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v13.1\\bin\\nvcc.exe" -ccbin "C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\VC\\Tools\\MSVC\\14.44.35207\\bin\\HostX64\\x64" -ptx -arch=sm\_120 -std=c++17 -Iinclude -Ibuild\\vcpkg\_installed\\x64-windows\\include src\\cuda\\CudaAsteroidPhysics.cu -o build\\CudaAsteroidPhysics.ptx

```



Inspect the generated file:



```cmd

notepad build\\CudaAsteroidPhysics.ptx

```



Search for relevant instructions:



```cmd

findstr /C:"ld.global" /C:"st.global" /C:"mul.f32" /C:"add.f32" /C:"fma.rn.f32" build\\CudaAsteroidPhysics.ptx

```



The generated PTX contains global-memory loads and stores and a fused multiply-add instruction for the position update. Exact instructions can vary with compiler versions and compilation options.



PTX is not the same as SASS, the machine-level instruction representation used by NVIDIA GPUs. PTX inspection alone does not establish the final hardware instruction sequence.



\## Benchmark



The standalone benchmark compares three measurements:



\- \*\*CPU time:\*\* Time for the CPU position-update loop.

\- \*\*GPU kernel time:\*\* GPU kernel execution measured using CUDA events, excluding explicit host-to-device and device-to-host transfers.

\- \*\*GPU end-to-end time:\*\* Measured GPU workflow including data transfers, kernel launch, synchronization, and result retrieval.



The benchmark uses these asteroid counts:



\- 1,000

\- 10,000

\- 100,000

\- 1,000,000



\### Build and run the benchmark



After configuring CMake with the benchmark target:



```cmd

cmake --build build --target asteroid\_benchmark

build\\asteroid\_benchmark.exe

```



\### Initial benchmark results



Environment: NVIDIA GeForce RTX 5060 Laptop GPU, compute capability 12.0. Results are from one recorded run and are specific to this machine and benchmark implementation.



| Asteroids | CPU (ms) | GPU kernel (ms) | GPU end-to-end (ms) | Kernel speedup |

|---:|---:|---:|---:|---:|

| 1,000 | 0.000059 | 0.006104 | 0.065561 | 0.010× |

| 10,000 | 0.000755 | 0.009842 | 0.080145 | 0.077× |

| 100,000 | 0.009346 | 0.007220 | 0.312856 | 1.294× |

| 1,000,000 | 0.172699 | 0.011688 | 1.828437 | 14.776× |



The kernel speedup is calculated as:



\\\[

S\_{\\text{kernel}}=

\\frac{T\_{\\text{CPU}}}{T\_{\\text{GPU kernel}}}

\\]



A speedup greater than 1 means the measured GPU kernel time is lower than the CPU time for the corresponding operation.



\### Interpretation



\- Small workloads favor the CPU because GPU launch overhead is significant relative to the amount of work.

\- At 100,000 and 1,000,000 asteroids, the measured kernel-only times are lower than the CPU times.

\- Host-device transfers and synchronization substantially increase the end-to-end GPU time.

\- In the recorded run, the end-to-end GPU path was slower than the CPU baseline at every tested workload size.



\*\*Important:\*\* These measurements demonstrate kernel-level parallel execution, not an overall game performance improvement. The end-to-end timings and numerical validation should be rechecked after benchmark methodology improvements and repeated runs before making stronger performance claims.



\## Current Limitations



\- Asteroid movement is a simple arithmetic workload, so it may not be computationally intensive enough to justify GPU offloading at small and medium scales.

\- Moving data between CPU and GPU introduces overhead.

\- Collision detection, rendering, spawning, and other gameplay operations remain outside the CUDA kernel.

\- Benchmark measurements are sensitive to hardware, drivers, compiler settings, workload size, and measurement methodology.

\- A single benchmark run is not sufficient to establish a general performance claim.



\## Future Work



\- Improve benchmark repeatability and report median measurements across multiple runs.

\- Validate numerical correctness independently for CPU, kernel-only, and end-to-end execution.

\- Inspect generated SASS in addition to PTX.

\- Evaluate the effects of memory-transfer reduction and GPU-resident data.

\- Investigate workloads where parallel execution provides a meaningful end-to-end benefit.



\## Learning Objectives



This project is intended to explore:



\- CUDA thread and block indexing

\- GPU memory allocation and host-device transfers

\- Kernel launch and synchronization behavior

\- PTX instruction inspection

\- CPU-versus-GPU benchmarking

\- The distinction between kernel speedup and end-to-end acceleration



\## License



No license is specified yet. Add a license file before distributing the project for reuse.



