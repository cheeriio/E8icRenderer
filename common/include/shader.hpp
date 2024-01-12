#ifndef _SHADER_HPP_GP_
#define _SHADER_HPP_GP_

#include <glad/glad.h>

bool LoadShaders(GLuint* id, const char* vertex_shader_path, const char* geometry_shader_path, const char* fragment_shader_path);

#endif // _SHADER_HPP_GP_