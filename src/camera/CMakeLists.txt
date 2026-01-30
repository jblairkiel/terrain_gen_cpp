add_library(camera_module
    thirdpersoncamera.cpp
)

target_include_directories(camera_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
