#ifndef _MODEL_HPP_GP_
#define _MODEL_HPP_GP_

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

#endif // _MODEL_HPP_GP_