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
  LoadShaders(&tex_shader,
              "shaders/ShadowedNormal.vertexshader",
              NULL,
              "shaders/ShadowedNormal.fragmentshader");

  GLuint cube_shadow_shader;
  LoadShaders(&cube_shadow_shader,
              "shaders/CubeShadowMap.vertexshader",
              "shaders/CubeShadowMap.geometryshader",
              "shaders/CubeShadowMap.fragmentshader");

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

  stbi_image_free(crate_image);

  GLuint TextureNormals;
  glGenTextures(1, &TextureNormals);
  glBindTexture(GL_TEXTURE_2D, TextureNormals);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, crate_n_w, crate_n_h, 0, GL_RGB, GL_UNSIGNED_BYTE, crate_normals_image);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(crate_normals_image);


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

  GLuint model_matrix_id = glGetUniformLocation(tex_shader, "M");
  GLuint view_matrix_id = glGetUniformLocation(tex_shader, "V");
  GLuint projection_matrix_id = glGetUniformLocation(tex_shader, "P");
  GLuint light_id = glGetUniformLocation(tex_shader, "LightPosition");
  GLuint position_id = glGetUniformLocation(tex_shader, "CameraPosition");
  GLuint far_plane_id = glGetUniformLocation(tex_shader, "far_plane");
  GLuint reverse_normals_id = glGetUniformLocation(tex_shader, "reverse_normal");


  // Utilities for shadow mapping
  GLuint DepthSampler = glGetUniformLocation(tex_shader, "DepthSampler");

  GLuint depthMapFBO = 0;
  glGenFramebuffers(1, &depthMapFBO);

  GLuint depthCubemap;
  glGenTextures(1, &depthCubemap);

  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
  for (int i = 0; i < 6; i++)
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  GLuint cs_model_matrix_id = glGetUniformLocation(cube_shadow_shader, "M");
  GLuint cs_matrices_id = glGetUniformLocation(cube_shadow_shader, "ShadowMatrices");
  GLuint cs_light_id = glGetUniformLocation(cube_shadow_shader, "LightPosition");
  GLuint cs_far_plane_id = glGetUniformLocation(cube_shadow_shader, "far_plane");

  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glm::vec3 initialCameraPos(7, 7, -5);

	glm::mat4 View = glm::lookAt(
				initialCameraPos,
				glm::vec3(0, 1, 0),
				glm::vec3(0, 1, 0)
			  );
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;


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

    glm::mat4 rotateLightX =
		  glm::rotate(glm::mat4(1.0f), glm::radians(angle_light), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 lightPos = rotateLightX * glm::vec4(5, 5, 5, 1);

    // Rendering to the depth buffer
    float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
    float near = 0.1f;
    float far = 100.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                    glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(cube_shadow_shader);

    glUniform3f(cs_light_id, lightPos.x, lightPos.y, lightPos.z);
    glUniform1f(cs_far_plane_id, far);
    for(int i = 0; i < 6; i++)
      glUniformMatrix4fv(
        glGetUniformLocation(
          cube_shadow_shader,
          std::string("ShadowMatrices[" + std::to_string(i) + "]").c_str()),
        1,
        GL_FALSE,
        &shadowTransforms[i][0][0]);

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

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(4.0f, -3.5f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    glUniformMatrix4fv(cs_model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(2.0f, 3.0f, 1.0));
    Model = glm::scale(Model, glm::vec3(0.75f));
    glUniformMatrix4fv(cs_model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-3.0f, -1.0f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    glUniformMatrix4fv(cs_model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1.5f, 2.0f, -3.0));
    Model = glm::rotate(Model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    Model = glm::scale(Model, glm::vec3(0.75f));
    glUniformMatrix4fv(cs_model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    glDisableVertexAttribArray(0);


    // Proper rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WinWidth, WinHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(tex_shader);

    glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, &Projection[0][0]);
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, &View[0][0]);
    glUniform3f(light_id, lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(position_id, newCameraPos.x, newCameraPos.y, newCameraPos.z);
    glUniform1f(far_plane_id, far);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureNormals);
		glUniform1i(NormalTextureID, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    glUniform1i(DepthSampler, 2);

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

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-2.5f, -4.0f, -2.5f));
    Model = glm::scale(Model, glm::vec3(8.0f));
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDisable(GL_CULL_FACE);
    glUniform1i(tex_shader, true);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);
    glUniform1i(tex_shader, false);
    glEnable(GL_CULL_FACE);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(4.0f, -3.5f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(2.0f, 3.0f, 1.0));
    Model = glm::scale(Model, glm::vec3(0.75f));
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-3.0f, -1.0f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, crate_vertices.size() * 3);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1.5f, 2.0f, -3.0));
    Model = glm::rotate(Model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    Model = glm::scale(Model, glm::vec3(0.75f));
    glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, &Model[0][0]);
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
  glDeleteBuffers(1, &tangentbuffer);
  glDeleteBuffers(1, &bitangentbuffer);

	glDeleteProgram(tex_shader);
  glDeleteProgram(cube_shadow_shader);

  glDeleteTextures(1, &Texture);
  glDeleteTextures(1, &TextureNormals);
  glDeleteTextures(1, &depthCubemap);
  
  glDeleteFramebuffers(1, &depthMapFBO);
  
	glDeleteVertexArrays(1, &VAO);

  SDL_DestroyWindow(Window);

  return 0;
}

