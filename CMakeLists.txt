cmake_minimum_required(VERSION 3.22...4.1)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(CraftMine LANGUAGES C CXX)
include(FetchContent)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_BUILD_PARALLEL_LEVEL 4)

if (WIN32)
    add_compile_options(-static-libgcc -static-libstdc++)
    add_link_options(-static)
endif()

FetchContent_Declare(
        zlib
        URL https://zlib.net/zlib-1.3.1.tar.gz
)

FetchContent_MakeAvailable(zlib)

add_subdirectory(include/glfw)

file(GLOB_RECURSE CRAFTMINE_SRC
        src/*.cpp
        include/FastNoise/FastNoise.cpp
        include/glad/glad.c
)

add_executable(CraftMine ${CRAFTMINE_SRC})

target_include_directories(CraftMine PRIVATE
        include
        include/glad
        include/glfw/include
        include/glm
        include/stb
        include/FastNoise
        src
)

target_link_libraries(CraftMine PRIVATE
        glfw
        zlibstatic
)

target_compile_definitions(CraftMine PRIVATE
        SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
