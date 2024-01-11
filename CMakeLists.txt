cmake_minimum_required(VERSION 3.6)
project(Crazy_lighting)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
# include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(external)

include_directories(
	external/glad/include/
	external/glm/
	.)
        
set(ALL_LIBS
	${OPENGL_LIBRARY}
	SDL2::SDL2-static
        glad)

add_executable(crazy_lighting src/main.cpp)
target_link_libraries(crazy_lighting PRIVATE ${ALL_LIBS})
