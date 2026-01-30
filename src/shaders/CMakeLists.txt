add_library(shader_module
    shader.cpp
)

target_include_directories(shader_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/glad/include
)
target_link_libraries(shader_module PUBLIC
    glad
)
