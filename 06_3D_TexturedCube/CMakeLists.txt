cmake_minimum_required(VERSION 3.16)
project(3D_TexturedCube VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGLWidgets)


qt_add_executable(3D_TexturedCube
    main.cpp
    openglwidget.h
    openglwidget.cpp
)

target_link_libraries(3D_TexturedCube PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
)

qt_finalize_executable(3D_TexturedCube)
