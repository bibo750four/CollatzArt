# 1. Specify the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# 2. Project name and language (C++ in this case)
project(CollatzArt VERSION 1.0 LANGUAGES CXX)

# 3. Specify the C++ standard (e.g., C++17 or C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# 4. Define the source files AND header files explicitly
set(SOURCES
    # Source files
    src/main.cpp
    src/engine/CollatzEngine.cpp
    src/app/CLIView.cpp
    src/app/PlaybackController.cpp
    src/app/AppController.cpp
    src/render/FeatherTree.cpp
    src/render/FeatherRenderer.cpp
    
    # Header files (Adding these makes them visible in Xcode)
    src/engine/CollatzEngine.hpp
    src/render/Renderer.hpp
    src/render/FeatherRenderer.hpp
    src/render/FeatherTree.hpp
    src/render/RenderConfig.hpp
    src/app/CLIView.hpp
    src/app/Command.hpp
    src/app/CommandQueue.hpp
    src/app/AppController.hpp
    src/app/PlaybackController.hpp
)

# 5. Organize files into virtual folders matching your disk structure in Xcode
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCES})

# 6. Create the final executable
add_executable(CollatzArt ${SOURCES})

# 7. Include directories for the target (so compiler can find them)
target_include_directories(CollatzArt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
        ${CMAKE_CURRENT_SOURCE_DIR}/src/render
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app
)

target_link_libraries(CollatzArt PRIVATE SFML::Graphics SFML::Window SFML::System)

