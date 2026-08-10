cmake_minimum_required(VERSION 3.20)

project(Lunaris LANGUAGES C)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

add_library(Lunaris STATIC
    src/Lunaris.c
    src/gl.c
)

target_include_directories(Lunaris PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(Lunaris PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

target_link_libraries(Lunaris PUBLIC
    glfw
)
