cmake_minimum_required(VERSION 2.6)
project(Jarvis)

set(GPP14 -std=c++14)
set(WALL -Wall)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GPP14} ${WALL} ${CURL}")
set(BOOST_ROOT /usr/include/boost)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(BOOST_COMPONENTS system)

find_package(Boost 1.62.0 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

if (Boost_FOUND)
	message(STATUS “Boost found”)
	include_directories(${Boost_INCLUDE_DIRS})

	add_executable(test ${TESTS_SOURCE} AsyncServer.cpp)
  	target_link_libraries(test ${Boost_LIBRARIES})
endif()
