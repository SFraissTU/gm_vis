cmake_minimum_required(VERSION 3.1.0)

project(visualizer)


if (WIN32)
    set(PYTHON_H_PATH "D:/Programme/Anaconda3/include")
    #PYTHON_LIB_PATH is only needed for test app for python lib
    set(PYTHON_LIB_PATH "D:/Programme/Anaconda3/pkgs/python-3.6.9-h5500b2f_0/libs/python36.lib")
else()
    #set(Python_ADDITIONAL_VERSIONS 3.7)
    find_package(PythonLibs 3 COMPONENTS Interpreter Development REQUIRED)
    message("PYTHON_INCLUDE_DIRS:${PYTHON_INCLUDE_DIRS}")
endif()

set(UI_FILES
    gmvis/ui/VisualizerWindow.ui
    gmvis/ui/VisualizerWindow.qrc
    gmvis/ui/VisualizerWindow.h
    gmvis/ui/VisualizerWindow.cpp
    gmvis/ui/main.cpp
    gmvis/ui/DisplayWidget.h
    gmvis/ui/DisplayWidget.cpp
)

set(PY_FILES
    gmvis/pylib/Image.h
    gmvis/pylib/Image.cpp
    gmvis/pylib/pyimport.h
    gmvis/pylib/pyimport.cpp
)

set(PYSTANDARD_FILES
    gmvis/pylib/standard/PythonInterface.h
    gmvis/pylib/standard/PythonInterface.cpp
    gmvis/pylib/standard/OffscreenRenderSurface.h
    gmvis/pylib/standard/OffscreenRenderSurface.cpp
    gmvis/pylib/standard/Visualizer.h
    gmvis/pylib/standard/Visualizer.cpp
    lodepng.h
    lodepng.cpp
)

set(PYLIVE_FILES
    gmvis/pylib/live/LiveVisWindow.h
    gmvis/pylib/live/LiveVisWindow.cpp
    gmvis/pylib/live/LiveVisWindow.ui
    gmvis/pylib/live/LiveVisWindow.qrc
    gmvis/pylib/live/PythonLiveInterface.h
    gmvis/pylib/live/PythonLiveInterface.cpp
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

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Threads)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    SYSTEM ${PYTHON_INCLUDE_DIRS}
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../include
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include
    SYSTEM ${PYTHON_H_PATH}
)

add_subdirectory(pybind11)
pybind11_add_module(pygmvis
    ${PY_FILES}
    ${PYSTANDARD_FILES}
    ${CORE_FILES}
)

pybind11_add_module(pygmvislive
    ${PY_FILES}
    ${PYLIVE_FILES}
    ${UI_FILES}
    ${CORE_FILES}
)

add_executable(visualizer
    ${UI_FILES}
    ${CORE_FILES}
)

add_executable(pythontest
    ${PY_FILES}
    ${PYSTANDARD_FILES}
    ${CORE_FILES}
)

target_link_libraries(visualizer Qt5::Widgets ${PYTHON_LIB_PATH} ${PYTHON_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pythontest Qt5::Widgets ${PYTHON_LIB_PATH} ${PYTHON_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pygmvis PRIVATE Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pygmvislive PRIVATE Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
target_compile_definitions(pythontest PUBLIC PY_LIB)
target_compile_definitions(pygmvis PUBLIC PY_LIB)
target_compile_definitions(pygmvislive PUBLIC PY_LIB)
if(NOT MSVC)
  target_compile_options(pythontest PUBLIC -fvisibility=hidden)
  target_compile_options(pygmvis PUBLIC -fvisibility=hidden)
  target_compile_options(pygmvislive PUBLIC -fvisibility=hidden)
endif()

add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:visualizer>/res/)
add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:visualizer>/shaders/)
