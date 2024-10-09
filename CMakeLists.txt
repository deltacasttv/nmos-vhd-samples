cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED CMAKE_PROJECT_TOP_LEVEL_INCLUDES)
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/nmos-cpp/Development/third_party/cmake/conan_provider.cmake")
endif()

project(
   nmos-vhd-samples
   DESCRIPTION "NMOS samples for the VideoMasterHD"
   LANGUAGES CXX)

set(NMOS_CPP_BUILD_TESTS OFF)
set(NMOS_CPP_BUILD_EXAMPLES OFF)

find_package(VideoMasterHD REQUIRED)

if (APPLE)
    set(VideoMasterHD_INCLUDE_DIR "/Applications/Deltacast/Library/Frameworks/VideoMasterHD.framework/Headers")
endif()

add_subdirectory(nmos-cpp/Development/)
add_subdirectory(src)
