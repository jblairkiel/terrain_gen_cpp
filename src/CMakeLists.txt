# Gather all source files
file(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/camera/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/terrain/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/rendering/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/*.cpp"
)

# Build the main executable
add_executable(world ${SRC_FILES})

# -----------------------------
# GLAD
# -----------------------------
add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

# -----------------------------
# Include directories for world
# -----------------------------
target_include_directories(world PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/camera
    ${CMAKE_SOURCE_DIR}/src/terrain
    ${CMAKE_SOURCE_DIR}/src/shaders
    ${CMAKE_SOURCE_DIR}/src/rendering
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/external/glad/include
)

# -----------------------------
# Link libraries (ONE call only)
# -----------------------------
target_link_libraries(world PRIVATE
    glad
    glfw
    glm
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