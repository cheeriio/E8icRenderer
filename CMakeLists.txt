cmake_minimum_required(VERSION 3.6)
project(Crazy_lighting)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_subdirectory(external)

include_directories(
	external/glad/include/
	external/glm/
	external/stb)

add_subdirectory(common)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
        ${SDL2_LIBRARIES}
	glad
	shader
	model)

add_executable(crazy_lighting app/src/main.cpp)
target_link_libraries(crazy_lighting PUBLIC ${ALL_LIBS})

add_custom_command(
   TARGET crazy_lighting POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/crazy_lighting${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/app/src/"
)

