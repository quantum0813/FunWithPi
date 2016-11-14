cmake_minimum_required(VERSION 3.2)
project(FunWithPi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lgmp -fopenmp")

set(SOURCE_FILES pi.cpp)

add_executable(FunWithPi ${SOURCE_FILES})
