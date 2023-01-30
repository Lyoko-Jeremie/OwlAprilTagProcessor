cmake_minimum_required(VERSION 3.25)
set(ProjectMainName OwlAprilTagProcessor)
project(${ProjectMainName})

set(CMAKE_CXX_STANDARD 20)


set(DEBUG_BOOST_ASIO OFF CACHE BOOL "DEBUG_BOOST_ASIO")

set(USE_DYNAMIC_BOOST OFF CACHE BOOL "USE_DYNAMIC_BOOST")

set(BOOST_ROOT "" CACHE PATH "BOOST_ROOT")

set(AprilTag_INCLUDE_DIRS "" CACHE PATH "AprilTag_INCLUDE_DIRS")
set(AprilTag_LIB "" CACHE PATH "AprilTag_LIB")

if (DEFAULT_CONFIG)
    # ===================== vvvvvv config segment vvvvvv =================================

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

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")


add_executable(${ProjectMainName} src/main.cpp src/AprilTagData.cpp src/AprilTagData.h)

target_link_libraries(${ProjectMainName} ${CMAKE_THREAD_LIBS_INIT})

include_directories(${Boost_INCLUDE_DIRS} ${AprilTag_INCLUDE_DIRS})
target_link_libraries(${ProjectMainName} ${Boost_LIBRARIES} ${AprilTag_LIB})
link_directories(${Boost_LIBRARY_DIRS})


if (WIN32)
    target_link_libraries(${ProjectMainName} wsock32 ws2_32 crypt32)
else (WIN32)
endif (WIN32)
