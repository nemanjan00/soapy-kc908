########################################################################
# Project setup -- only needed if device support is a stand-alone build
# We recommend that the support module be built in-tree with the driver.
########################################################################
cmake_minimum_required(VERSION 2.6)
project(SoapySDRKC908 CXX)
enable_testing()

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Header and library resources needed to communicate with the device.
# These may be found within the build tree or in an external project.
########################################################################
set(KCSDR_INCLUDE_DIRS "/usr/local/include/kcsdr/")
set(KCSDR_LIBRARIES "/usr/local/lib/libkcsdr.so")

########################################################################
# build the module
########################################################################
find_package(SoapySDR CONFIG)

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()

include_directories(${KCSDR_INCLUDE_DIRS})
SOAPY_SDR_MODULE_UTIL(
    TARGET KC908
    SOURCES KC908Support.cpp
    LIBRARIES ${KCSDR_LIBRARIES}
)
