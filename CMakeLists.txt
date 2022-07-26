cmake_minimum_required(VERSION 3.1.0)

project(visualizer)

#### CONFIGURATION

# -- Qt --

# > If you want to use Qt5 rather than Qt6, set this to FALSE
set(USE_QT6 TRUE)

# > if QT cannot be found, try setting the prefix path to the QT dir:
#set(CMAKE_PREFIX_PATH "D:/Programme/Qt/6.2.4/msvc2019_64")

# -- Python --

# > If you want to use your Anaconda-Python, uncomment the following three lines
#set(ANACONDA3_PATH $ENV{CONDA_PREFIX})
#message(STATUS "ANACONDA3_PATH=${ANACONDA3_PATH}")
#set (Python_ROOT_DIR ${ANACONDA3_PATH}/bin)

# > If Python cannot be found, try setting the Python directory manually:
#set(Python_ROOT_DIR "C:/Program Files/Python310")

# -- Eigen / Pybind11 --

# > If you want to use other eigen/pybind versions, set the paths here:
set(EIGEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/eigen)
set(PYBIND11_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/pybind11)

#### --------------


if(USE_QT6)
	find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
else()
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()
find_package(Threads)

set (Python_FIND_STRATEGY LOCATION)
find_package (Python REQUIRED COMPONENTS Interpreter Development)

#message("Python_LIBRARIES:${Python_LIBRARIES}")
#message("Python_LIBRARY_DIRS:${Python_LIBRARY_DIRS}")
#message("Python_INCLUDE_DIRS:${Python_INCLUDE_DIRS}")


set(UI_FILES
    gmvis/ui/VisualizerWindow.ui
    gmvis/ui/VisualizerWindow.qrc
    gmvis/ui/VisualizerWindow.h
    gmvis/ui/VisualizerWindow.cpp
    gmvis/ui/main.cpp
    gmvis/ui/DisplayWidget.h
    gmvis/ui/DisplayWidget.cpp
    gmvis/ui/GaussianListItem.h
    gmvis/ui/GaussianListItem.cpp)

set(PY_FILES
    gmvis/pylib/PythonInterface.h
    gmvis/pylib/PythonInterface.cpp
    gmvis/pylib/OffscreenRenderSurface.h
    gmvis/pylib/OffscreenRenderSurface.cpp
    gmvis/pylib/Image.h
    gmvis/pylib/Image.cpp
    gmvis/pylib/Visualizer.h
    gmvis/pylib/Visualizer.cpp
    gmvis/pylib/pyimport.h
)

set(CORE_FILES
    gmvis/core/DataLoader.h
    gmvis/core/DataLoader.cpp
    gmvis/core/Camera.h
    gmvis/core/Camera.cpp
    gmvis/core/RawGaussian.h
    gmvis/core/Gaussian.h
    gmvis/core/Gaussian.cpp
    gmvis/core/GaussianMixture.h
    gmvis/core/GaussianMixture.cpp
    gmvis/core/GMIsoellipsoidRenderer.h
    gmvis/core/GMIsoellipsoidRenderer.cpp
    gmvis/core/GMIsosurfaceRenderer.h
    gmvis/core/GMIsosurfaceRenderer.cpp
    gmvis/core/GMPositionsRenderer.h
    gmvis/core/GMPositionsRenderer.cpp
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
    gmvis/core/LineStrip.h
    gmvis/core/LineRenderer.h
    gmvis/core/LineRenderer.cpp
    gmvis/shaders.qrc
    gmvis/res.qrc
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    SYSTEM ${Python_INCLUDE_DIRS}
    SYSTEM ${EIGEN_PATH}
    SYSTEM ${PYBIND11_PATH}/include
)

add_compile_definitions(DECIMAL_TYPE=float)

add_subdirectory(${PYBIND11_PATH})
pybind11_add_module(pygmvis
    ${PY_FILES}
    ${CORE_FILES}
   )
   
add_executable(visualizer
    ${UI_FILES}
    ${CORE_FILES}
)

if(USE_QT6)
	target_link_libraries(visualizer Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(pygmvis PRIVATE Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets ${CMAKE_THREAD_LIBS_INIT})

    add_custom_command(TARGET visualizer POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:visualizer>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:visualizer>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:visualizer>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::OpenGL> $<TARGET_FILE_DIR:visualizer>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::OpenGLWidgets> $<TARGET_FILE_DIR:visualizer>)

else()
	target_link_libraries(visualizer Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(pygmvis PRIVATE Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})

    #ToDo: Add copying of DLLs to target dir
endif()

target_compile_definitions(pygmvis PUBLIC PY_LIB)
if(NOT MSVC)
 target_compile_options(pygmvis PUBLIC -fvisibility=hidden)
endif()
                      
install(TARGETS visualizer pygmvis
        DESTINATION ./)

