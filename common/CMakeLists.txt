cmake_minimum_required(VERSION 3.6)
project(Common)

add_library(shader include/shader.hpp src/shader.cpp)
add_library(model include/model.hpp src/model.cpp)
add_library(textures include/textures.hpp src/textures.cpp)

target_include_directories(shader PUBLIC include/)
target_include_directories(model PUBLIC include/)
target_include_directories(textures PUBLIC include/)
