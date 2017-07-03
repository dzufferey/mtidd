cmake_minimum_required(VERSION 3.6.0)
project(MTIDD)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PACKAGE_NAME mtidd)
set(PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "https://github.com/dzufferey/mtidd")
set(BUG_REPORT_URL "${PACKAGE_BUGREPORT}" CACHE STRING "Default URL where bug reports are to be submitted.")

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_COLOR_MAKEFILE ON)

option(BUILD_STATIC_BIN "Build static binary"  OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_SOURCE_DIR}/config.h"
  )

#Bring the headers into the project
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

#Bring the sources into the project
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")

# Testing

include(ExternalProject)

ExternalProject_Add(CATCH
  URL https://github.com/philsquared/Catch/releases/download/v1.9.5/catch.hpp
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/catch
  DOWNLOAD_NO_EXTRACT 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND echo ""
  INSTALL_COMMAND ""
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include) # to have catch

enable_testing()

add_subdirectory(test)


# Static vs dynamic

if(BUILD_STATIC_BIN AND BUILD_SHARED_LIBS)
  message(FATAL_ERROR "Can't handle both of BUILD_STATIC_BIN and BUILD_SHARED_LIBS options." )
endif()

if(NOT (BUILD_STATIC_BIN OR BUILD_SHARED_LIBS))
  message(FATAL_ERROR "Need either BUILD_STATIC_BIN or BUILD_SHARED_LIBS options." )
endif()

if(BUILD_STATIC_BIN)
  #Generate the shared library from the sources
  add_library(mtidd STATIC ${SOURCES})
endif()

if(BUILD_SHARED_LIBS)
  #Generate the shared library from the sources
  add_library(mtidd SHARED ${SOURCES})
endif()



#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
install(TARGETS mtidd DESTINATION lib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/" DESTINATION include/mtidd
        PATTERN "${PROJECT_SOURCE_DIR}/*.h")

