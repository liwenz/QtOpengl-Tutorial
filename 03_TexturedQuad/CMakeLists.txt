# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Define the project name and version
project(TexturedQuadTutorial VERSION 0.1 LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt auto tools (MOC, UIC, RCC)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find the required Qt 6 modules: Widgets, OpenGL, OpenGLWidgets, and Gui (for QImage/QPixmap/QTexture)
# QGui is needed for QImage loading if not using the Qt resource system directly.
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets Gui)

# Define the executable and its source files
qt_add_executable(textured_quad
    main.cpp
    openglwidget.cpp
    openglwidget.h
)

# Link the required Qt libraries and system OpenGL libraries
target_link_libraries(textured_quad PRIVATE
    Qt::Widgets
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Gui
    # The system OpenGL library might need to be explicitly linked on Windows
    opengl32
)
