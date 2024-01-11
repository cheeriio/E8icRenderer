
cmake_minimum_required (VERSION 2.6)
project(Crazy_lighting_OMG)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(external)

include_directories(
	external/Glad/include/
	external/glm/
	.)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	${SDL2_LIBRARIES})

add_definitions(
        -DTW_STATIC
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D
        -DGLEW_STATIC
        -D_CRT_SECURE_NO_WARNINGS
)

add_executable(crazy_lighting src/main.cpp)
target_link_libraries(crazy_lighting ${ALL_LIBS})

