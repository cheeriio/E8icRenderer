#ifndef _TEXTURE_HPP_GP_
#define _TEXTURE_HPP_GP_

#include <glad/glad.h>

class Texture {
 public:
  enum class Format {
    PNG
  };
  
  Texture(Format, const char* albedo, const char* normal);

  Texture() = delete;
  Texture(const Texture &) = delete;
  Texture& operator=(const Texture&) = delete;

  Texture(Texture &&);
  Texture& operator=(Texture &&);

  ~Texture();

  void use();

 private:
  GLuint albedo_texture_;
  GLuint normal_texture_;
};

#endif // _TEXTURE_HPP_GP_