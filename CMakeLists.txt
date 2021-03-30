cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(terminus VERSION 1.0.1 DESCRIPTION "terminus library")

option(BUILD_TESTING "Build unit tests" OFF)
option(BUILD_APPS "Build main applications" ON)

add_subdirectory(lib)

if (BUILD_APPS)
  add_subdirectory(client)
  add_subdirectory(server)
endif ()

if (BUILD_TESTING)
  add_subdirectory(test)
endif ()
