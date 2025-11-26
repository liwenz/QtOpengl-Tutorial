# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Define the project name and version
# Changed the project name for the new tutorial stage
project(IndexedQuadTutorial VERSION 0.1 LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt auto tools (MOC, UIC, RCC)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find the required Qt 6 modules: Widgets, OpenGL, OpenGLWidgets
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

# Define the executable and its source files
# Updated the executable name to 'indexed_quad'
qt_add_executable(indexed_quad
    main.cpp
    openglwidget.cpp
    openglwidget.h
)

# Link the required Qt libraries and system OpenGL libraries
target_link_libraries(indexed_quad PRIVATE
    Qt::Widgets
    Qt::OpenGL
    Qt::OpenGLWidgets
    # The system OpenGL library might need to be explicitly linked on Windows
    opengl32
)
