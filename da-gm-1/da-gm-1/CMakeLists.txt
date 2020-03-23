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
    gmvis/ui/VisualizerWindow.ui
    gmvis/ui/VisualizerWindow.qrc
    gmvis/ui/VisualizerWindow.h
    gmvis/ui/VisualizerWindow.cpp
    gmvis/ui/main.cpp
    gmvis/ui/DisplayWidget.h
    gmvis/ui/DisplayWidget.cpp
    gmvis/core/DataLoader.h
    gmvis/core/DataLoader.cpp
    gmvis/core/Camera.h
    gmvis/core/Camera.cpp
    gmvis/core/Gaussian.h
    gmvis/core/Gaussian.cpp
    gmvis/core/GaussianMixture.h
    gmvis/core/GaussianMixture.cpp
    gmvis/core/GMIsoellipsoidRenderer.h
    gmvis/core/GMIsoellipsoidRenderer.cpp
    gmvis/core/GMRenderModes.h
    gmvis/core/GMDensityRenderer.h
    gmvis/core/GMDensityRenderer.cpp
    gmvis/core/GMDensityRaycastRenderer.h
    gmvis/core/GMDensityRaycastRenderer.cpp
    gmvis/core/GMDensityRasterizeRenderer.h
    gmvis/core/GMDensityRasterizeRenderer.cpp
    gmvis/core/Helper.h
    gmvis/core/PointCloud.h
    gmvis/core/PointCloudRenderer.h
    gmvis/core/PointCloudRenderer.cpp
    gmvis/core/ScreenFBO.h
    gmvis/core/ScreenFBO.cpp
)

target_link_libraries(visualizer Qt5::Widgets)

add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:visualizer>/res/)
add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:visualizer>/shaders/)