cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(featureBgs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

IF(UNIX)
  # add some standard warnings
  ADD_DEFINITIONS(-Wno-variadic-macros -Wno-long-long -Wall -Wextra -Winit-self -Woverloaded-virtual -Wsign-promo -Wno-unused-parameter -pedantic -Woverloaded-virtual -Wno-unknown-pragmas)
  
  # -ansi does not compile with sjn module
  #ADD_DEFINITIONS(-ansi)

  # if you like to have warinings about conversions, e.g. double->int or double->float etc., or float compare
  #ADD_DEFINITIONS(-Wconversion -Wfloat-equal)
endif(UNIX)

#set(CMAKE_PREFIX_PATH "/home/yzbx/software/Qt/5.4/gcc_64")
find_package(OpenCV REQUIRED)
find_package(Qt5Widgets REQUIRED)
#INCLUDE(${QT_USE_FILE})

if(${OpenCV_VERSION} VERSION_LESS 2.3.1)
  message (FATAL_ERROR "OpenCV version is not compatible: ${OpenCV_VERSION}")
endif()

file(GLOB sources yzbx_tracking.cpp)
file(GLOB main main.cpp)

#include_directories(${CMAKE_SOURCE_DIR})
include_directories("/home/yzbx/git/bgslibrary/package_bgs")

set(Bgs_LIBS "/home/yzbx/git/bgslibrary/build/libbgs.so")
add_library(yzbx_tracking SHARED ${sources})
target_link_libraries(yzbx_tracking ${OpenCV_LIBS} ${Bgs_LIBS})
set_property(TARGET yzbx_tracking PROPERTY PUBLIC_HEADER yzbx_tracking.h)

add_executable(bgs_bin ${main})
target_link_libraries(bgs_bin ${OpenCV_LIBS} ${Bgs_LIBS} Qt5::Widgets yzbx_tracking)
set_target_properties(bgs_bin
  PROPERTIES OUTPUT_NAME bgs)

#INSTALL(TARGETS bgs
#	bgs_demo
#	bgs_bin
#  RUNTIME DESTINATION bin COMPONENT app
#  LIBRARY DESTINATION lib COMPONENT runtime
#  ARCHIVE DESTINATION lib COMPONENT runtime
#  PUBLIC_HEADER DESTINATION include/package_bgs COMPONENT dev
#  FRAMEWORK DESTINATION "/Library/Frameworks"
#)
