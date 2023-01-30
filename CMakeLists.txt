cmake_minimum_required(VERSION 3.25)
set(ProjectMainName OwlAprilTagProcessor)
project(${ProjectMainName})

set(CMAKE_CXX_STANDARD 20)


set(OpenCV_DIR "" CACHE PATH "OpenCV_DIR")
set(OpenCV_STATIC ON CACHE BOOL "OpenCV_STATIC")
set(OpenCV_WORLD OFF CACHE BOOL "OpenCV_WORLD")

set(DEBUG_BOOST_ASIO OFF CACHE BOOL "DEBUG_BOOST_ASIO")

set(USE_DYNAMIC_BOOST OFF CACHE BOOL "USE_DYNAMIC_BOOST")

set(BOOST_ROOT "" CACHE PATH "BOOST_ROOT")

set(AprilTag_INCLUDE_DIRS "" CACHE PATH "AprilTag_INCLUDE_DIRS")
set(AprilTag_LIB "" CACHE PATH "AprilTag_LIB")

if (DEFAULT_CONFIG)
    # ===================== vvvvvv config segment vvvvvv =================================

    #set(OpenCV_DIR d:/CodeLib/opencv-4.5.5-build/)
    set(OpenCV_DIR h:/CodeLib/opencv-4.5.5-install/)
    set(OpenCV_STATIC OFF)
    set(OpenCV_WORLD ON)

    set(BOOST_ROOT h:/CodeLib/boost_1_81_0_install_x64_v11p3/)
    set(USE_DYNAMIC_BOOST OFF)
    #set(BOOST_ROOT d:/CodeLib/boost_1_80_0_install_x64_v11p3/)
    #set(Boost_INCLUDE_DIR d:/CodeLib/boost_1_74_0_install_x64_v11p3/include/boost-1_74)
    #set(Boost_LIBRARY_DIR d:/CodeLib/boost_1_74_0_install_x64_v11p3/lib)
    #set(USE_DYNAMIC_BOOST true)
    #set(BOOST_DEBUG ON)

    #add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)

    set(AprilTag_INCLUDE_DIRS h:/Code/apriltag-build-install/include/apriltag)
    set(AprilTag_LIB h:/Code/apriltag-build-install/lib/libapriltag.a)

    # ===================== ^^^^^^ config segment ^^^^^^ =================================
endif (DEFAULT_CONFIG)


if (DEBUG_BOOST_ASIO)
    add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
endif (DEBUG_BOOST_ASIO)

option(USE_DYNAMIC_BOOST "Config USE_DYNAMIC_BOOST")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "enable -O1 when debug to resolve 'string table overflow at offset' 'too big' error on mingw")
    # -O1 to resolve "string table overflow at offset" error on mingw
    add_definitions(-Wall -Wextra -O1 -g)
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(BOOST_THEAD_MODULE "thread")
add_definitions(-DUSE_BOOST_THEAD)

if (USE_DYNAMIC_BOOST)
    message(STATUS "USE DYNAMIC BOOST !!!")
    add_definitions(-DBOOST_ALL_DYN_LINK=1)
else (USE_DYNAMIC_BOOST)
    message(STATUS "USE STATIC BOOST !!!")
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif (USE_DYNAMIC_BOOST)

set(Boost_USE_MULTITHREADED ON)
#find_package(Boost REQUIRED COMPONENTS  REQUIRED)
find_package(Boost 1.81.0 REQUIRED COMPONENTS system filesystem program_options json log ${BOOST_THEAD_MODULE} REQUIRED)

if (OpenCV_WORLD)
    find_package(OpenCV REQUIRED world)
else (OpenCV_WORLD)
    find_package(OpenCV REQUIRED)
endif (OpenCV_WORLD)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBRARY_DIRS: ${OpenCV_LIBRARY_DIRS}")
message(STATUS "OpenCV_LIBRARIES: ${OpenCV_LIBRARIES}")
message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")
message(STATUS "OpenCV ARCH: ${OpenCV_ARCH}")
message(STATUS "OpenCV RUNTIME: ${OpenCV_RUNTIME}")
message(STATUS "OpenCV STATIC: ${OpenCV_STATIC}")

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")

message(STATUS "CMAKE_THREAD_LIBS_INIT: ${CMAKE_THREAD_LIBS_INIT}")

add_executable(${ProjectMainName} src/main.cpp src/AprilTagData.cpp src/AprilTagData.h)

target_link_libraries(${ProjectMainName} ${CMAKE_THREAD_LIBS_INIT})

include_directories(${Boost_INCLUDE_DIRS} ${AprilTag_INCLUDE_DIRS} ${OpenCV_LIBRARIES})
target_link_libraries(${ProjectMainName} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES} ${AprilTag_LIB})
link_directories(${Boost_LIBRARY_DIRS})


if (WIN32)
    target_link_libraries(${ProjectMainName} wsock32 ws2_32 crypt32)
else (WIN32)
endif (WIN32)
