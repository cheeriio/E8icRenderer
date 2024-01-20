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

bool LoadShaders(GLuint* id, const char* vertex_shader_path, const char* geometry_shader_path, const char* fragment_shader_path) {
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  GLuint geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  bool vert_compiled = false, geom_compiled = false, frag_compiled = false;
  bool success = true;

  if(vertex_shader_path) {
    vert_compiled = LoadShader(vertex_shader_id, vertex_shader_path);
    success = success && vert_compiled;
  }
  if(geometry_shader_path) {
    geom_compiled = LoadShader(geometry_shader_id, geometry_shader_path);
    success = success && geom_compiled;
  }
  if(fragment_shader_path) {
    frag_compiled = LoadShader(fragment_shader_id, fragment_shader_path);
    success = success && frag_compiled;
  }

  if(!success) {
    if(vert_compiled)
      glDeleteShader(vertex_shader_id);
    if(geom_compiled)
      glDeleteShader(geometry_shader_id);
    if(frag_compiled)
      glDeleteShader(fragment_shader_id);
    return false;
  }

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
  if(geometry_shader_path) {
    glDetachShader(program_id, geometry_shader_id);
    glDeleteShader(geometry_shader_id);
  }
  if(fragment_shader_path) {
    glDetachShader(program_id, fragment_shader_id);
    glDeleteShader(fragment_shader_id);
  }

	*id = program_id;
  return true;
}

}

Shader::Shader(const char* vertex_shader_path, const char* geometry_shader_path, const char* fragment_shader_path) {
  is_valid_ = LoadShaders(&id_, vertex_shader_path, geometry_shader_path, fragment_shader_path);
}

Shader::~Shader() {
  glDeleteProgram(id_);
}

void Shader::use() {
  glUseProgram(id_);
}

void Shader::set_int(const std::string & name, int value) const {
  glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::set_float(const std::string & name, float value) const {
  glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
}

void Shader::set_vec2(const std::string & name, const glm::vec2 & value) const {
  glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}

void Shader::set_vec3(const std::string & name, const glm::vec3 & value) const {
  glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}

void Shader::set_vec4(const std::string & name, const glm::vec4 & value) const {
  glUniform4fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
}

void Shader::set_mat2(const std::string & name, const glm::mat2 & value) const {
  glUniformMatrix2fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::set_mat3(const std::string & name, const glm::mat3 & value) const {
  glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::set_mat4(const std::string & name, const glm::mat4 & value) const {
  glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

bool Shader::is_valid() {
  return is_valid_;
}
