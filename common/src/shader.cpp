#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <shader.hpp>

namespace {

bool LoadShader(GLuint shader_id, const char* path) {
  std::string code;
  std::ifstream ifs(path, std::ios::in);
  if(!ifs.is_open()) {
    std::cout << "Could not open " << path << std::endl;
    return false;
  }

  std::stringstream sstr;
  sstr << ifs.rdbuf();
  code = sstr.str();
  ifs.close();

	const char* shader_code_ptr = code.c_str();
	glShaderSource(shader_id, 1, &shader_code_ptr, NULL);
	glCompileShader(shader_id);

  GLint result = GL_FALSE;
	int log_length;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length){
		std::vector<char> message(log_length+1);
		glGetShaderInfoLog(shader_id, log_length, NULL, message.data());
		std::cout << message.data() << std::endl;
	}

  return result == GL_TRUE;
}

}

bool LoadShaders(GLuint* id, const char* vertex_shader_path, const char* geometry_shader_path, const char* fragment_shader_path) {
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  GLuint geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  bool success = true;

  if(vertex_shader_path)
    success = success && LoadShader(vertex_shader_id, vertex_shader_path);
  if(geometry_shader_path)
    success = success && LoadShader(geometry_shader_id, geometry_shader_path);
  if(fragment_shader_path)
    success = success && LoadShader(fragment_shader_id, fragment_shader_path);

  if(!success)
    return false;

	GLuint program_id = glCreateProgram();
  if(vertex_shader_path)
	  glAttachShader(program_id, vertex_shader_id);
  if(geometry_shader_path)
    glAttachShader(program_id, geometry_shader_id);
  if(fragment_shader_path)
	  glAttachShader(program_id, fragment_shader_id);
    
	glLinkProgram(program_id);

  GLint result = GL_FALSE;
	int log_length;
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

	if (log_length > 0){
		std::vector<char> message(log_length+1);
		glGetProgramInfoLog(program_id, log_length, NULL, message.data());
		std::cout << message.data() << std::endl;
	}
	
  if(vertex_shader_path) {
    glDetachShader(program_id, vertex_shader_id);
    glDeleteShader(vertex_shader_id);
  }
  if(vertex_shader_path) {
    glDetachShader(program_id, geometry_shader_id);
    glDeleteShader(geometry_shader_id);
  }
  if(vertex_shader_path) {
    glDetachShader(program_id, fragment_shader_id);
    glDeleteShader(fragment_shader_id);
  }

	*id = program_id;
  return true;
}
