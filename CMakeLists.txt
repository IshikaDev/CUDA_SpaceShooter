
cmake_minimum_required(VERSION 3.24)

project(CUDASpaceShooter LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES 120)

find_package(SFML 3 CONFIG REQUIRED
    COMPONENTS Graphics Window System
)

add_executable(cuda_space_shooter
    src/main.cpp
    src/core/Game.cpp
    src/entities/Player.cpp
    src/entities/Bullet.cpp
    src/entities/Asteroid.cpp
    src/systems/LevelManager.cpp
    src/systems/CollisionSystem.cpp
    src/rendering/Background.cpp
    src/cuda/CudaAsteroidPhysics.cu
)

target_include_directories(cuda_space_shooter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cuda_space_shooter PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)

# Standalone CUDA benchmark; does not depend on SFML.
add_executable(asteroid_benchmark
    src/benchmark/asteroid_benchmark.cu
)

set_target_properties(asteroid_benchmark PROPERTIES
    CUDA_ARCHITECTURES 120
    CUDA_STANDARD 17
    CUDA_STANDARD_REQUIRED ON
)
