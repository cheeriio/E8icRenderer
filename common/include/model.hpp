#ifndef _MODEL_HPP_GP_
#define _MODEL_HPP_GP_

#include <glad/glad.h>
#include <glm/glm.hpp>

bool LoadOBJ(const char* path,
             std::vector<glm::vec3>& vertices,
             std::vector<glm::vec2>& uvs,
             std::vector<glm::vec3>& normals);

void ComputeTangents(std::vector<glm::vec3>& vertices,
                     std::vector<glm::vec2>& uvs,
                     std::vector<glm::vec3>& normals,
                     std::vector<glm::vec3>& tangents,
                     std::vector<glm::vec3>& bitangents);

class Model {
 public:
  Model(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs);
  Model(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals);

  ~Model();

  static Model FromOBJ(const char * path);
  static Model FlatModel(float base_x, float base_y, glm::vec3 lower_left, glm::vec3 lower_right, glm::vec3 upper_right);
  void render();
  bool is_valid();
  
//  private:
  GLuint VAO_;
  GLuint vertexbuffer_;
  GLuint uvbuffer_;
  GLuint normalbuffer_;
  GLuint tangentbuffer_;
  GLuint bitangentbuffer_;

  unsigned int size_;
};

#endif // _MODEL_HPP_GP_