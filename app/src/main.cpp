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
  glEnable(GL_CULL_FACE);

  GLuint VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  GLuint tex_shader;
  // LoadShaders(&tex_shader, "shaders/Light.vertexshader", NULL, "shaders/Light.fragmentshader");
  LoadShaders(&tex_shader, "shaders/LightNormal.vertexshader", NULL, "shaders/LightNormal.fragmentshader");

  // Loading the crate .obj model
  std::vector<glm::vec3> crate_vertices;
  std::vector<glm::vec2> crate_uvs;
  std::vector<glm::vec3> crate_normals;
  bool res = LoadOBJ("models/crate.obj", crate_vertices, crate_uvs, crate_normals);
  if(!res) {
    std::cout << "Failed to load the model" << std::endl;
    return -1;
  }

  std::vector<glm::vec3> crate_tangents;
  std::vector<glm::vec3> crate_bitangents;

  ComputeTangents(crate_vertices, crate_uvs, crate_normals, crate_tangents, crate_bitangents);

  // Loading the crate .png textures
  int crate_w, crate_h, crate_comp;
  unsigned char* crate_image = stbi_load("textures/crate.png", &crate_w, &crate_h, &crate_comp, STBI_rgb_alpha);

  if(crate_image == nullptr) {
    std::cout << "Failed to load the texture" << std::endl;
    return -1;
  }

  int crate_n_w, crate_n_h, crate_n_comp;
  unsigned char* crate_normals_image = stbi_load("textures/crate_normals.png", &crate_n_w, &crate_n_h, &crate_n_comp, STBI_rgb);

  if(crate_normals_image == nullptr) {
    std::cout << "Failed to load the texture" << std::endl;
    return -1;
  }

  GLuint Texture;
  glGenTextures(1, &Texture);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, crate_w, crate_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, crate_image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint TextureNormals;
  glGenTextures(1, &TextureNormals);
  glBindTexture(GL_TEXTURE_2D, TextureNormals);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, crate_n_w, crate_n_h, 0, GL_RGB, GL_UNSIGNED_BYTE, crate_normals_image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  GLuint TextureID  = glGetUniformLocation(tex_shader, "DiffuseTextureSampler");
  GLuint NormalTextureID  = glGetUniformLocation(tex_shader, "NormalTextureSampler");

  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, crate_vertices.size() * sizeof(glm::vec3), crate_vertices.data(), GL_STATIC_DRAW);

  GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, crate_uvs.size() * sizeof(glm::vec2), crate_uvs.data(), GL_STATIC_DRAW);

  GLuint normalbuffer;
  glGenBuffers(1, &normalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  glBufferData(GL_ARRAY_BUFFER, crate_normals.size() * sizeof(glm::vec3), crate_normals.data(), GL_STATIC_DRAW);

  GLuint tangentbuffer;
  glGenBuffers(1, &tangentbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
  glBufferData(GL_ARRAY_BUFFER, crate_tangents.size() * sizeof(glm::vec3), crate_tangents.data(), GL_STATIC_DRAW);

  GLuint bitangentbuffer;
  glGenBuffers(1, &bitangentbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
  glBufferData(GL_ARRAY_BUFFER, crate_bitangents.size() * sizeof(glm::vec3), crate_bitangents.data(), GL_STATIC_DRAW);


  

  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glm::vec3 initialCameraPos(7, 7, -5);

	glm::mat4 View = glm::lookAt(
				initialCameraPos,
				glm::vec3(0, 1, 0),
				glm::vec3(0, 1, 0)
			  );
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

  GLuint matrix_id = glGetUniformLocation(tex_shader, "MVP");
  GLuint view_matrix_id = glGetUniformLocation(tex_shader, "V");
	GLuint model_matrix_id = glGetUniformLocation(tex_shader, "M");
  GLuint model__view_matrix_id = glGetUniformLocation(tex_shader, "MV");

  GLuint light_id = glGetUniformLocation(tex_shader, "LightPosition");

  float angle = 0.0;
  float angle_light = 0.0;
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
          case SDLK_UP:
            angle_light += 2.0f;
            break;
          case SDLK_DOWN:
            angle_light -= 2.0f;
            break;
          case SDLK_ESCAPE:
            Running = false;
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

    // Rendering the shadow map



    // Rendering the box
    glm::mat4 rotateX =
		  glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newCameraPos = rotateX * glm::vec4(initialCameraPos, 1);
		glm::mat4 View = glm::lookAt(
				glm::vec3(newCameraPos),
				glm::vec3(0,2.5,0),
				glm::vec3(0,1,0)
			  );
		MVP = Projection * View * Model;
    glm::mat3 ModelView = glm::mat3(View * Model);

    glClearColor(0.5f, 0.5f, 0.5f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(tex_shader);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, &View[0][0]);
    glUniformMatrix3fv(model__view_matrix_id, 1, GL_FALSE, &ModelView[0][0]);

    glm::mat4 rotateLightX =
		  glm::rotate(glm::mat4(1.0f), glm::radians(angle_light), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 lightPos = rotateLightX * glm::vec4(20, 0, 20, 1);
    glUniform3f(light_id, lightPos.x, lightPos.y, lightPos.z);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureNormals);
		glUniform1i(NormalTextureID, 1);

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

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
		glVertexAttribPointer(
			3,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
		glVertexAttribPointer(
			4,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    SDL_GL_SwapWindow(Window);
  }

  glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
  glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(tex_shader);
  glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VAO);

  SDL_DestroyWindow(Window);

  return 0;
}

