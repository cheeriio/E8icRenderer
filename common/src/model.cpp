#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <model.hpp>

bool LoadOBJ(const char* path,
             std::vector<glm::vec3>& ret_vertices,
             std::vector<glm::vec2>& ret_uvs,
             std::vector<glm::vec3>& ret_normals) {
  std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;

  std::ifstream ifs(path, std::ios::in);
  if(!ifs.is_open()) {
    std::cout << "Could not open " << path << std::endl;
    return false;
  }

  std::string line;
  std::stringstream sstr;
  sstr << ifs.rdbuf();
  ifs.close();

  bool has_normals = false;
  bool uses_quads = false;
  bool first_face = true;

  while(std::getline(sstr, line)) {
    std::stringstream line_sstr;
    line_sstr << line;
    std::string header;
    line_sstr >> header;
    if(header == "v") {
      glm::vec3 vertex;
      line_sstr >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    } else if(header == "vt") {
      glm::vec2 uv;
      line_sstr >> uv.x >> uv.y;
      uvs.push_back(glm::vec2(uv.x, 1.0 - uv.y));
    } else if(header == "vn") {
      has_normals = true;
      glm::vec3 normal;
      line_sstr >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if(header == "f") {
      if(first_face) {
        first_face = false;
        std::string defs[4];
        int cnt = 0;
        while(line_sstr >> defs[cnt])
          cnt++;
        if(cnt == 4)
          uses_quads = true;
        std::stringstream swap_sstr;
        for(int i = 0; i < cnt; i++)
          swap_sstr << defs[i] << " ";
        line_sstr.swap(swap_sstr);
      }
      char ch;
      unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];

      int limit = uses_quads ? 4 : 3;
      for(int i = 0; i < limit; i++) {
        line_sstr >> vertexIndex[i] >> ch >> uvIndex[i];
        if(has_normals)
          line_sstr >> ch >> normalIndex[i];
      }

      vertexIndices.push_back(vertexIndex[0]);
      vertexIndices.push_back(vertexIndex[1]);
      vertexIndices.push_back(vertexIndex[2]);
      uvIndices.push_back(uvIndex[0]);
      uvIndices.push_back(uvIndex[1]);
      uvIndices.push_back(uvIndex[2]);
      if(has_normals) {
        normalIndices.push_back(normalIndex[0]);
        normalIndices.push_back(normalIndex[1]);
        normalIndices.push_back(normalIndex[2]);
      }

      if(uses_quads) {
        vertexIndices.push_back(vertexIndex[2]);
        vertexIndices.push_back(vertexIndex[3]);
        vertexIndices.push_back(vertexIndex[0]);
        uvIndices.push_back(uvIndex[2]);
        uvIndices.push_back(uvIndex[3]);
        uvIndices.push_back(uvIndex[0]);
        if(has_normals) {
          normalIndices.push_back(normalIndex[2]);
          normalIndices.push_back(normalIndex[3]);
          normalIndices.push_back(normalIndex[0]);
        }
      }
      
    }
  }

  for(int i=0; i < vertexIndices.size(); i++ ) {
    int vertexIndex = vertexIndices[i];
    glm::vec3 vertex = vertices[vertexIndex - 1];
    ret_vertices.push_back(vertex);

    int uvIndex = uvIndices[i];
    glm::vec2 uv = uvs[uvIndex - 1];
    ret_uvs.push_back(uv);

    if(has_normals) {
      int normalIndex = normalIndices[i];
      glm::vec3 normal = normals[normalIndex - 1];
      ret_normals.push_back(normal);
    }
  }
  return true;
}

void ComputeTangents(std::vector<glm::vec3>& vertices,
                     std::vector<glm::vec2>& uvs,
                     std::vector<glm::vec3>& normals,
                     std::vector<glm::vec3>& tangents,
                     std::vector<glm::vec3>& bitangents) {
  for (int i = 0; i < vertices.size(); i += 3) {
    glm::vec3& v0 = vertices[i];
    glm::vec3& v1 = vertices[i + 1];
    glm::vec3& v2 = vertices[i + 2];

    glm::vec2& uv0 = uvs[i];
    glm::vec2& uv1 = uvs[i + 1];
    glm::vec2& uv2 = uvs[i + 2];

    glm::vec3 deltaPos1 = v1 - v0;
    glm::vec3 deltaPos2 = v2 - v0;

    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

    tangents.push_back(tangent);
    tangents.push_back(tangent);
    tangents.push_back(tangent);

    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
  }

  for (int i = 0; i < vertices.size(); i++) {
		glm::vec3& n = normals[i];
		glm::vec3& t = tangents[i];
		glm::vec3& b = bitangents[i];
		
		t = glm::normalize(t - n * glm::dot(n, t));
		
		if (glm::dot(glm::cross(n, t), b) < 0.0f){
			t = t * -1.0f;
		}

	}
}