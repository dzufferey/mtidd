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

set(CMAKE_CXX_FLAGS         "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -g -fno-inline -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -O3 -DNDEBUG")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/mtidd_config.h.in"
  "${PROJECT_SOURCE_DIR}/mtidd_config.h"
  )

#Bring the headers into the project
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

#Bring the sources into the project
file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")

# Testing

include(ExternalProject)

ExternalProject_Add(CATCH
  URL https://github.com/catchorg/Catch2/releases/download/v2.2.3/catch.hpp
  DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/catch
  DOWNLOAD_NO_EXTRACT 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND echo ""
  INSTALL_COMMAND ""
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include) # to have catch

enable_testing()

add_subdirectory(test)


# genrate the libraries
add_library(mtidd_static STATIC ${SOURCES})
add_library(mtidd_shared SHARED ${SOURCES})

SET_TARGET_PROPERTIES(mtidd_static PROPERTIES OUTPUT_NAME mtidd CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(mtidd_shared PROPERTIES OUTPUT_NAME mtidd CLEAN_DIRECT_OUTPUT 1)


#Set the location for library installation -- i.e., /usr/lib in this case
# not really necessary in this example. Use "sudo make install" to apply
install(TARGETS mtidd_static DESTINATION lib)
install(TARGETS mtidd_shared DESTINATION lib)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/" DESTINATION include/mtidd
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "test" EXCLUDE)

