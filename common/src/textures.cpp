#include <textures.hpp>
#include <stb_image.h>

#include <iostream>

Texture::Texture(Texture::Format format, const char* albedo, const char* normal) {
  unsigned char *albedo_data, *normal_data;
  switch(format) {
    case Texture::Format::PNG:
      int w_albedo, h_albedo, w_normal, h_normal, comp;
      if(albedo) {
        albedo_data = stbi_load(albedo, &w_albedo, &h_albedo, &comp, STBI_rgb_alpha);
        if(albedo_data == nullptr) {
          std::cout << "Failed to load the texture " << albedo << std::endl;
          return;
        }
      } else {
        albedo_texture_ = 0;
      }
      if(normal) {
        normal_data = stbi_load(normal, &w_normal, &h_normal, &comp, STBI_rgb_alpha);
        if(normal_data == nullptr) {
          std::cout << "Failed to load the texture " << normal << std::endl;
          return;
        }
      } else {
        normal_texture_ = 0;
      }

      glGenTextures(1, &albedo_texture_);
      glBindTexture(GL_TEXTURE_2D, albedo_texture_);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_albedo, h_albedo, 0, GL_RGBA, GL_UNSIGNED_BYTE, albedo_data);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      glGenTextures(1, &normal_texture_);
      glBindTexture(GL_TEXTURE_2D, normal_texture_);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_normal, h_normal, 0, GL_RGB, GL_UNSIGNED_BYTE, normal_data);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      stbi_image_free(albedo_data);
      stbi_image_free(normal_data);

      break;
  }
}

Texture::Texture(Texture && other) {
  albedo_texture_ = other.albedo_texture_;
  normal_texture_ = other.normal_texture_;

  other.albedo_texture_ = 0;
  other.normal_texture_ = 0;
}

Texture& Texture::operator=(Texture && other) {
  if(this == &other)
    return *this;

  albedo_texture_ = other.albedo_texture_;
  normal_texture_ = other.normal_texture_;

  other.albedo_texture_ = 0;
  other.normal_texture_ = 0;

  return *this;
}

Texture::~Texture() {
  glDeleteTextures(1, &albedo_texture_);
  glDeleteTextures(1, &normal_texture_);
}

void Texture::use() {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, albedo_texture_);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_texture_);
}
