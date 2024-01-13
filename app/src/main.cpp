#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <shader.hpp>
#include <model.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#define WinWidth 1600
#define WinHeight 900

int main (int ArgCount, char **Args)
{
  int32_t WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
  SDL_Window *Window = SDL_CreateWindow("GP Project",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WinWidth,
                                        WinHeight,
                                        WindowFlags);
  if(!Window) {
    std::cout << "Failed to create a window";
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GLContext Context = SDL_GL_CreateContext(Window);

  if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); 

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  GLuint tex_shader;
  LoadShaders(&tex_shader, "shaders/TexturedVertexShader.vertexshader", NULL, "shaders/TexturedFragmentShader.fragmentshader");

  GLuint matrix_id = glGetUniformLocation(tex_shader, "MVP");

  // Loading the crate .obj model
  std::vector<glm::vec3> crate_vertices;
  std::vector<glm::vec2> crate_uvs;
  std::vector<glm::vec3> crate_normals;
  bool res = LoadOBJ("models/crate.obj", crate_vertices, crate_uvs, crate_normals);
  if(!res) {
    std::cout << "Failed to load the model" << std::endl;
    return -1;
  }

  std::cout << "Vertices:" << std::endl;
  for(int i = 0; i < crate_vertices.size(); i++) {
    std::cout << crate_vertices[i].x << " "
              << crate_vertices[i].y << " "
              << crate_vertices[i].z << std::endl;
  }

  std::cout << "UVs:" << std::endl;
  for(int i = 0; i < crate_uvs.size(); i++) {
    std::cout << crate_uvs[i].x << " "
              << crate_uvs[i].y << std::endl;
  }

  std::cout << "Normals:" << std::endl;
  for(int i = 0; i < crate_normals.size(); i++) {
    std::cout << crate_normals[i].x << " "
              << crate_normals[i].y << " "
              << crate_normals[i].z << std::endl;
  }

  // Loading the crate .png texture
  int crate_w, crate_h, crate_comp;
  unsigned char* crate_image = stbi_load("textures/crate.png", &crate_w, &crate_h, &crate_comp, STBI_rgb_alpha);

  if(crate_image == nullptr) {
    std::cout << "Failed to load the texture" << std::endl;
    return -1;
  }

  GLuint Texture;
  glGenTextures(1, &Texture);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, crate_w, crate_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, crate_image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint TextureID  = glGetUniformLocation(tex_shader, "myTextureSampler");

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, crate_vertices.size() * sizeof(glm::vec3), crate_vertices.data(), GL_STATIC_DRAW);

  GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, crate_uvs.size() * sizeof(glm::vec2), crate_uvs.data(), GL_STATIC_DRAW);

  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glm::vec3 initialCameraPos(7, 7, -5);

	glm::mat4 View = glm::lookAt(
				initialCameraPos,
				glm::vec3(0, 1, 0),
				glm::vec3(0, 1, 0)
			  );
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

  float angle = 0;
  int32_t Running = 1;

  while (Running)
  {
    SDL_Event Event;

    while (SDL_PollEvent(&Event))
    {
      if (Event.type == SDL_KEYDOWN)
      {
        switch (Event.key.keysym.sym)
        {
          case SDLK_RIGHT:
            angle += 2.0f;
            break;
          case SDLK_LEFT:
            angle -= 2.0f;
            break;
          default:
            break;
        }
      }
      else if (Event.type == SDL_QUIT)
      {
        Running = 0;
      } 
    }

    glm::mat4 rotateX =
		  glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newCameraPos = rotateX * glm::vec4(initialCameraPos, 1);
		glm::mat4 View = glm::lookAt(
				glm::vec3(newCameraPos),
				glm::vec3(0,2.5,0),
				glm::vec3(0,1,0)
			  );
		MVP = Projection * View * Model;

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(tex_shader);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

    glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
    glUniform1i(TextureID, 0);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
      0,
      3,
      GL_FLOAT,
      GL_FALSE,
      0,
      (void*)0
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
      1,
      2,
      GL_FLOAT,
      GL_FALSE,
      0,
      (void*)0
    );

    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    SDL_GL_SwapWindow(Window);
  }

  glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(tex_shader);
  glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VAO);

  SDL_DestroyWindow(Window);

  return 0;
}

