# tests/CMakeLists.txt

file(GLOB TEST_FILES "*.cpp")

add_executable(camera_tests ${TEST_FILES})

# Catch2 v3 from apt-get
find_package(Catch2 3 REQUIRED)

target_link_libraries(camera_tests PRIVATE
    Catch2::Catch2WithMain
)

target_include_directories(camera_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

include(Catch)
catch_discover_tests(camera_tests)
