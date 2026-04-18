cmake_minimum_required(VERSION 3.13)

project(solar-mgr)

set(PROJECT_VERSION "0.2")

message(STATUS "Running CMake for ${CMAKE_PROJECT_NAME} v${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/Install" CACHE PATH "Default install in main directory" FORCE)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include_directories("influxdb")
add_subdirectory(json)

include_directories("src")
add_subdirectory("src")

