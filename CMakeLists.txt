cmake_minimum_required(VERSION 3.10)
project(ProceduralWorld)

enable_testing()

set(CMAKE_CXX_STANDARD 17)

# Add subdirectories
find_package(glfw3 REQUIRED)
add_subdirectory(external/glad)
add_subdirectory(src)
add_subdirectory(tests)
