cmake_minimum_required(VERSION 3.1.0)

project(visualizer)


set(ANACONDA3_PATH $ENV{CONDA_PREFIX})
#execute_process(COMMAND conda info --base OUTPUT_VARIABLE ANACONDA3_PATH) # alternative, maybe it works better for windows
message(STATUS "ANACONDA3_PATH=${ANACONDA3_PATH}")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Threads)

set (Python_ROOT_DIR ${ANACONDA3_PATH}/bin)
set (Python_FIND_STRATEGY LOCATION)
find_package (Python REQUIRED COMPONENTS Interpreter Development)

message("Python_LIBRARIES:${Python_LIBRARIES}")
message("Python_LIBRARY_DIRS:${Python_LIBRARY_DIRS}")
message("Python_INCLUDE_DIRS:${Python_INCLUDE_DIRS}")

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
    SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/pybind11/include
)

add_compile_definitions(DECIMAL_TYPE=float)

add_subdirectory(pybind11)
pybind11_add_module(pygmvis
    ${PY_FILES}
    ${CORE_FILES}
   )

add_executable(visualizer
    ${UI_FILES}
    ${CORE_FILES}
)

#add_executable(pythontest
#    ${PY_FILES}
#    ${CORE_FILES}
#)

target_link_libraries(visualizer Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(pythontest Qt5::Widgets ${Python_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pygmvis PRIVATE Qt5::Widgets ${CMAKE_THREAD_LIBS_INIT})
#target_compile_definitions(pythontest PUBLIC PY_LIB)
target_compile_definitions(pygmvis PUBLIC PY_LIB)
if(NOT MSVC)
  #target_compile_options(pythontest PUBLIC -fvisibility=hidden)
  target_compile_options(pygmvis PUBLIC -fvisibility=hidden)
endif()

add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/res/ $<TARGET_FILE_DIR:visualizer>/res/)
add_custom_command(TARGET visualizer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:visualizer>/shaders/)
                       
install(TARGETS visualizer pygmvis
        DESTINATION ./)

