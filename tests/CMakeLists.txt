# tests/CMakeLists.txt

file(GLOB TEST_FILES "*.cpp")

add_executable(tests ${TEST_FILES})

# Catch2 v3 from apt-get
find_package(Catch2 3 REQUIRED)


target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(tests PRIVATE
    Catch2::Catch2WithMain
    glad
    shader_module
    camera_module
    terrain_module
    glfw

)

include(Catch)
catch_discover_tests(tests)
