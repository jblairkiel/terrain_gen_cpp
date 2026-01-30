# src/CMakeLists.txt

add_subdirectory(camera)
add_subdirectory(terrain)
add_subdirectory(shaders)

file(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(world ${SRC_FILES})

target_include_directories(world PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(world PRIVATE
    camera_module
    terrain_module
    shader_module
    glad
    glfw
)

if(UNIX AND NOT APPLE)
    target_link_libraries(world PRIVATE
        dl pthread X11 Xrandr Xi Xxf86vm Xcursor
    )
endif()