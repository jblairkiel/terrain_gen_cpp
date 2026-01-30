add_library(terrain_module
    noise.cpp
    heightsampler.cpp
    terrainmeshbuilder.cpp
    terrain.cpp
)

target_include_directories(terrain_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)