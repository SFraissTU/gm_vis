cmake_minimum_required(VERSION 3.1.0)

project(visualizer)

# if Qt cannot be found, try setting the prefix path to the qt dir:
set(CMAKE_PREFIX_PATH "D:/Programme/Qt/6.2.4/msvc2019_64")

set(PYBIND11_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pybind11)
set(USE_QT6 TRUE)


#set(ANACONDA3_PATH $ENV{CONDA_PREFIX})
#execute_process(COMMAND conda info --base OUTPUT_VARIABLE ANACONDA3_PATH) # alternative, maybe it works better for windows
#message(STATUS "ANACONDA3_PATH=${ANACONDA3_PATH}")

if(USE_QT6)
	find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
else()
	find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()
find_package(Threads)

#set (Python_ROOT_DIR "C:/Program Files/Python310")
#set (Python_FIND_STRATEGY LOCATION)
#find_package (Python REQUIRED COMPONENTS Interpreter Development)

#message("Python_LIBRARIES:${Python_LIBRARIES}")
#message("Python_LIBRARY_DIRS:${Python_LIBRARY_DIRS}")
#message("Python_INCLUDE_DIRS:${Python_INCLUDE_DIRS}")

set(PYTHON_LIB "C:/Program Files/Python310/libs/python310.lib")
set(PYTHON_EXECUTABLE "C:/Program Files/Python310/python.exe")
include_directories("C:/Program Files/Python310/include")
include_directories("${PYBIND11_PATH}/include")
message(STATUS "${PYBIND11_PATH}/include")
#include_directories("D:/Simon/Studium/S-11 (WS19-20)/Diplomarbeit/pybind11/pybind11-2.9.2/include")

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
    lodepng.h
    lodepng.cpp
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
    shaders.qrc
    res.qrc
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
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../include
    SYSTEM ${PYBIND11_PATH}/include
)

add_compile_definitions(DECIMAL_TYPE=float)

add_subdirectory(pybind11)
pybind11_add_module(pygmvis
    ${PY_FILES}
    ${CORE_FILES}
   )

pybind11_add_module(pytest
    gmvis/pylib/test.cpp
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

