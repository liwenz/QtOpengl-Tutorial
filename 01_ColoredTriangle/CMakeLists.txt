cmake_minimum_required(VERSION 3.16)

project(01_ColoredTriangle VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the required Qt 6 modules: Widgets, OpenGL, OpenGLWidgets
# QOpenGLWidget inherits from QWidget, so the Widgets module is still required.
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

# Define the executable and its source files (only main.cpp, openglwidget.h/cpp are kept)
qt_add_executable(colored_triangle
    main.cpp
    openglwidget.cpp
    openglwidget.h
)

# Link the required Qt libraries and system OpenGL libraries
target_link_libraries(colored_triangle PRIVATE
    Qt::Widgets
    Qt::OpenGL
    Qt::OpenGLWidgets
    # The system OpenGL library might need to be explicitly linked on Windows
    opengl32
)
