cmake_minimum_required(VERSION 3.16)

# Set the project name and version
project(QuestPC VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug symbols
set(CMAKE_BUILD_TYPE Debug)

# Add compile options for better debugging
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

# Create the executable
add_executable(${PROJECT_NAME} main.cpp)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
