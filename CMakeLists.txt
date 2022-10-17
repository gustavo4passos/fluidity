cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(fluidity)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# Set static linking
set(BUILD_SHARED_LIBS off)

# External submodule - cnpy
add_subdirectory(./external/cnpy)

set(ENABLE_SIMULATOR off)

# Enable simulator
if (ENABLE_SIMULATOR)
  enable_language(CUDA)
  find_package(CUDA 11.3 REQUIRED)
  include_directories(./external/cuda-fluid-simulator/cuda-common/inc)
  include_directories(./external/cuda-fluid-simulator/cuda-simulation/src)
  include_directories(${CUDA_INCLUDE_DIRS})
  file(GLOB SIMULATION_SOURCES ./src/simulation/*.cpp ./external/cuda-fluid-simulator/cuda-simulation/src/particleSystem_cuda.cu ./external/cuda-fluid-simulator/cuda-simulation/src/particleSystem.cpp)
endif()

# Windows specific settings
if (WIN32)
    # External submodule - imgui
    file(GLOB IMGUI_SOURCES ./external/imgui/imgui.cpp ./external/imgui/imgui_demo.cpp ./external/imgui/imgui_draw.cpp ./external/imgui/imgui_tables.cpp ./external/imgui/imgui_widgets.cpp ./external/imgui/backends/imgui_impl_sdl.cpp ./external/imgui/backends/imgui_impl_opengl3.cpp)
    
    # External submodule - assimp
    set(ASSIMP_BUILD_TESTS          off)
    set(ASSIMP_BUILD_ASSIMP_VIEW    off)
    set(ASSIMP_BUILD_ASSIMP_SAMPLES off)
    set(ASSIMP_INSTALL              off)
    add_subdirectory(./external/assimp)

    # External submodules includes - windows
    include_directories(./external/assimp/include)
    include_directories(./external/imgui/backends)
    include_directories(./external/imgui)

    # External submodule - stb
    find_path(STB_INCLUDE_DIRS "stb.h")
    
    # External submodule - tinyfiledialogs
    file(GLOB TINYFILEDIALOGS_SOURCES ./external/libtinyfiledialogs/tinyfiledialogs.c)

# Linux specific settings
elseif (UNIX)
    # External dependencies includes - linux
    include_directories(/usr/include/imgui/backends)
    include_directories(/usr/include/imgui)
    include_directories(/usr/include/stb)

    find_package(imgui QUIET COMPONENTS imgui-core imgui-sdl imgui-opengl3)
    find_package(assimp REQUIRED)
else()
    message(FATAL_ERROR "Only Windows and Linux are supported at the moment.")
endif()


# External submodules includes 
include_directories(./external/cnpy/)
include_directories(./external/libtinyfiledialogs)

find_package(OpenGL          REQUIRED)
find_package(GLEW            REQUIRED)
find_package(SDL2     CONFIG REQUIRED)
find_package(glm      CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

file(GLOB SOURCES ./src/utils/*.cpp ./src/*.cpp ./src/renderer/*.cpp ./src/input/*.cpp)

# Assets
if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS ${CMAKE_CURRENT_SOURCE_DIR}/assets/appicon.rc)
    if (ENABLE_SIMULATOR)
      add_executable(fluidity ${IMGUI_SOURCES} ${SOURCES} ${TINYFILEDIALOGS_SOURCES} ${APP_ICON_RESOURCE_WINDOWS}  ${SIMULATION_SOURCES})
      set_target_properties(fluidity PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
      set_target_properties(fluidity PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
    else()
      add_executable(fluidity ${IMGUI_SOURCES} ${SOURCES} ${TINYFILEDIALOGS_SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
    endif()
    add_custom_target(copy-shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders)
    add_custom_target(copy-assets COMMAND ${CMAKE_COMMAND}  -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}/assets)

elseif (UNIX)
    add_executable(fluidity ${SOURCES})
    add_custom_target(copy-shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders)
    add_custom_target(copy-assets  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)
endif ()
    
target_include_directories(fluidity PUBLIC ./src)
add_dependencies(fluidity copy-shaders copy-assets)

if (WIN32)
    if (ENABLE_SIMULATOR)
      target_link_libraries(fluidity PUBLIC ${CUDA_LIBRARIES} GLEW::GLEW SDL2::SDL2 SDL2::SDL2main cnpy assimp yaml-cpp)
    else()
      target_link_libraries(fluidity PUBLIC GLEW::GLEW SDL2::SDL2 SDL2::SDL2main cnpy assimp yaml-cpp)
    endif()
elseif (UNIX)
    target_link_libraries(fluidity PUBLIC GLEW::GLEW ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} cnpy assimp yaml-cpp imgui stb)
else()
    message(FATAL_ERROR "Only Windows and Linux are supported at the moment.")
endif ()
