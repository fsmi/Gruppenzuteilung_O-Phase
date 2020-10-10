cmake_minimum_required(VERSION 3.12)
project(OPhaseGroupAssignment)

add_executable(Main main.cpp)

execute_process(COMMAND cmake -P ${CMAKE_CURRENT_SOURCE_DIR}/download_boost.cmake)

include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/boost/)

# glob boost sources
file(GLOB MINI_BOOST_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/boost/libs/graph/src/*.cpp)

add_library(mini_boost STATIC ${MINI_BOOST_SOURCES})
set_target_properties(mini_boost PROPERTIES LINKER_LANGUAGE CXX)
set(Boost_LIBRARIES mini_boost)
set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/boost/boost/)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
