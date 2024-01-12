cmake_minimum_required(VERSION 3.6)
project(Common)

add_library(shader include/shader.hpp src/shader.cpp)
target_include_directories(shader PUBLIC include/)