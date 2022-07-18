cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(fluidity)

add_compile_definitions(DEBUG_BUILD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# External submodule - cnpy
add_subdirectory(./external/cnpy)

# External submodule - assimp
SET(ASSIMP_BUILD_TESTS OFF)
SET(ASSIMP_BUILD_ASSIMP_VIEW OFF)
SET(ASSIMP_BUILD_ASSIMP_SAMPLES OFF)
SET(ASSIMP_INSTALL OFF)

add_subdirectory(./external/assimp)

include_directories(./external/cnpy/)
include_directories(./external/assimp/include)
include_directories(./external/imgui/backends)
include_directories(./external/imgui)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
#find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
#find_package(yaml-cpp CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

file(GLOB SOURCES ./src/utils/*.cpp ./src/*.cpp ./src/renderer/*.cpp ./src/input/*.cpp)
file(GLOB IMGUI_SOURCES ./external/imgui/imgui.cpp ./external/imgui/imgui_demo.cpp ./external/imgui/imgui_draw.cpp ./external/imgui/imgui_tables.cpp ./external/imgui/imgui_widgets.cpp ./external/imgui/backends/imgui_impl_sdl.cpp ./external/imgui/backends/imgui_impl_opengl3.cpp)

add_executable(fluidity ${IMGUI_SOURCES} ${SOURCES})
target_include_directories(fluidity PUBLIC ./src)

# Assets
if (WIN32)
    add_custom_target(copy-shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)
    add_custom_target(copy-assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}/assets)
elseif (UNIX)
    add_custom_target(copy-shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
    add_custom_target(copy-assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
endif ()


add_dependencies(fluidity copy-assets copy-shaders)

if (WIN32)
    target_link_libraries(fluidity PUBLIC GLEW::GLEW SDL2::SDL2 SDL2::SDL2main cnpy assimp)
elseif (UNIX)
    target_link_libraries(fluidity PUBLIC GLEW::GLEW ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} cnpy assimp)
else()
    message(FATAL_ERROR "Only Windows and Linux are supported at the moment.")
endif ()
