# src/CMakeLists.txt

file(GLOB SRC_FILES
    "*.cpp"
)


add_executable(world ${SRC_FILES})
add_library(glad
    ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

target_include_directories(world PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(world
    glfw
    glad
)

# Linux-specific libs
if(UNIX AND NOT APPLE)
    target_link_libraries(world
        dl
        pthread
        X11
        Xrandr
        Xi
        Xxf86vm
        Xcursor
    )
endif()