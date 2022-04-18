cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(npz-rendering)

add_compile_definitions(DEBUG_BUILD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_subdirectory(./external/cnpy)
include_directories(./external/cnpy/)

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

# Compile Optick
# Optick only supports Vulkan and D3D12 for GPU profiling for, so it's disabled until
# the engine supports D3D12/Vulkan
# add_definitions(-DOPTICK_ENABLE_GPU_VULKAN=0) 
# add_definitions(-DOPTICK_ENABLE_GPU_D3D12=0)
# file(GLOB OPTICK_SOURCES ./Vendor/Optick/src/*.cpp)

file(GLOB SOURCES ./src/*.cpp ./src/renderer/*.cpp)

# Platform specific implementations
# if (WIN32)
#     file(GLOB PLATFORM_SPECIFIC_SOURCES ./Prisma/Platform/Windows/*.cpp)
# else()
#     message( FATAL_ERROR "Only Windows is supported at the moment")
# endif()

add_executable(npz-rendering ${SOURCES})
# target_precompile_headers(npz-rendering PUBLIC ./Prisma/Platform/Windows/WindowsAnsi.pch)
target_include_directories(npz-rendering PUBLIC ./src)
target_link_libraries(npz-rendering PUBLIC Shlwapi.lib GLEW::GLEW SDL2::SDL2 SDL2::SDL2main imgui::imgui glm yaml-cpp cnpy)