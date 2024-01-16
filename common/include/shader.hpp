#ifndef _SHADER_HPP_GP_
#define _SHADER_HPP_GP_

#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
 public:
  Shader(const char* vertex_shader_path, const char* geometry_shader_path, const char* fragment_shader_path);
  ~Shader();

  void use();

  void set_int(const std::string &, int ) const;
  void set_float(const std::string &, float ) const;
  void set_vec2(const std::string &, const glm::vec2 &) const;
  void set_vec3(const std::string &, const glm::vec3 &) const;
  void set_vec4(const std::string &, const glm::vec4 &) const;
  void set_mat2(const std::string &, const glm::mat2 &) const;
  void set_mat3(const std::string &, const glm::mat3 &) const;
  void set_mat4(const std::string &, const glm::mat4 &) const;

  bool is_valid();

 private:
  GLuint id_;
  bool is_valid_;
};

#endif // _SHADER_HPP_GP_