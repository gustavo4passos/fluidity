cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(npz-rendering)

add_compile_definitions(DEBUG_BUILD)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

add_subdirectory(./external/cnpy)
include_directories(./external/cnpy/)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
#find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
#find_package(yaml-cpp CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

file(GLOB SOURCES ./src/utils/*.cpp ./src/*.cpp ./src/renderer/*.cpp)

add_executable(npz-rendering ${SOURCES})
target_include_directories(npz-rendering PUBLIC ./src)

# Assets
if (WIN32)
    add_custom_target(copy-assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)
elseif (UNIX)
    add_custom_target(copy-assets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
endif ()


add_dependencies(npz-rendering copy-assets)

if (WIN32)
    target_link_libraries(npz-rendering PUBLIC GLEW::GLEW SDL2::SDL2 SDL2::SDL2main cnpy)
elseif (UNIX)
    target_link_libraries(npz-rendering PUBLIC GLEW::GLEW ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} cnpy)
else()
    message(FATAL_ERROR "Only Windows and Linux are supported at the moment.")
endif ()
