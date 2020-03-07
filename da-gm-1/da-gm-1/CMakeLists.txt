cmake_minimum_required(VERSION 3.1.0)

project(visualizer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(.)
include_directories(../include)

add_executable(visualizer
    VisualizerWindow.ui
    VisualizerWindow.qrc
    VisualizerWindow.h
    VisualizerWindow.cpp
    main.cpp
    DataLoader.h
    DataLoader.cpp
    DisplayWidget.h
    DisplayWidget.cpp
    Camera.h
    Camera.cpp
    Gaussian.h
    Gaussian.cpp
    GaussianMixture.h
    GaussianMixture.cpp
    GMIsoellipsoidRenderer.h
    GMIsoellipsoidRenderer.cpp
    GMRenderModes.h
    GMDensityRenderer.h
    GMDensityRenderer.cpp
    GMDensityRaycastRenderer.h
    GMDensityRaycastRenderer.cpp
    GMDensityRasterizeRenderer.h
    GMDensityRasterizeRenderer.cpp
    Helper.h
    PointCloud.h
    PointCloudRenderer.h
    PointCloudRenderer.cpp
    ScreenFBO.h
    ScreenFBO.cpp
)

target_link_libraries(visualizer Qt5::Widgets)
