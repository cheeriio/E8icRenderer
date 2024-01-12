cmake_minimum_required(VERSION 3.6)
project(glad)

add_library(glad STATIC include/glad/glad.h src/glad.c)
target_link_libraries(glad ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})
target_include_directories(glad PUBLIC include/)
