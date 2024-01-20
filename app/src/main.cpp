#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <shader.hpp>
#include <model.hpp>
#include <textures.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Keep this header, stb_image should be implemented here.
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
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GLContext Context = SDL_GL_CreateContext(Window);

  if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  Shader tex_shader("shaders/ShadowedNormal.vertexshader",
                    NULL,
                    "shaders/ShadowedNormal.fragmentshader");

  Shader cube_shadow_shader("shaders/CubeShadowMap.vertexshader",
                            "shaders/CubeShadowMap.geometryshader",
                            "shaders/CubeShadowMap.fragmentshader");

  std::shared_ptr<Model> CrateModel = Model::FromOBJ("models/crate.obj");

  std::vector<std::shared_ptr<Model>> walls = {
    Model::FlatModel(2, 2, {-10, -5, 10}, {-10, -5, -10}, {-10, 5, -10}),
    Model::FlatModel(2, 2, {-10, -5, -10}, {10, -5, -10}, {10, 5, -10}),
    Model::FlatModel(2, 2, {10, -5, -10}, {10, -5, 10}, {10, 5, 10}),
    Model::FlatModel(2, 2, {10, -5, 10}, {-10, -5, 10}, {-10, 5, 10})
  };

  std::vector<std::shared_ptr<Model>> floors = {
    Model::FlatModel(4, 4, {-10, -5, 10}, {10, -5, 10}, {10, -5, -10}),
    Model::FlatModel(4, 4, {10, 5, 10}, {-10, 5, 10}, {-10, 5, -10})
  };

  std::shared_ptr<Texture> CrateTexture =
    std::make_shared<Texture>(Texture::Format::PNG,
                              "textures/crate_albedo.png",
                              "textures/crate_normals.png");
  std::shared_ptr<Texture> WallTexture =
    std::make_shared<Texture>(Texture::Format::PNG,
                              "textures/wall_albedo.png",
                              "textures/wall_normal.png");
  std::shared_ptr<Texture> FloorTexture =
    std::make_shared<Texture>(Texture::Format::PNG,
                              "textures/floor_albedo.png",
                              "textures/floor_normal.png");

  tex_shader.use();
  tex_shader.set_int("DIffueTextureSampler", 0);
  tex_shader.set_int("NormalTextureSampler", 1);
  tex_shader.set_int("DepthSampler", 2);

  // Utilities for shadow mapping
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

  glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
	glm::vec3 initialCameraPos(0, 2, -5);

	glm::mat4 View = glm::lookAt(
				initialCameraPos,
				glm::vec3(0, -3, 0),
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
				glm::vec3(0, 0, 0),
				glm::vec3(0, 1, 0)
			  );
		MVP = Projection * View * Model;
    glm::mat3 ModelView = glm::mat3(View * Model);

    glm::mat4 rotateLightX =
		  glm::rotate(glm::mat4(1.0f), glm::radians(angle_light), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 lightPos = rotateLightX * glm::vec4(5, 2, 5, 1);

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

    cube_shadow_shader.use();

    cube_shadow_shader.set_vec3("LightPosition", glm::vec3(lightPos.x, lightPos.y, lightPos.z));
    cube_shadow_shader.set_float("far_plane", far);
    for(int i = 0; i < 6; i++)
      cube_shadow_shader.set_mat4(std::string("ShadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(4.0f, -3.5f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    cube_shadow_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(2.0f, 3.0f, 1.0));
    Model = glm::scale(Model, glm::vec3(0.75f));
    cube_shadow_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-3.0f, -1.0f, 0.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    cube_shadow_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1.5f, 2.0f, -3.0));
    Model = glm::rotate(Model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    Model = glm::scale(Model, glm::vec3(0.75f));
    cube_shadow_shader.set_mat4("M", Model);
    CrateModel->render();

    // Proper rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WinWidth, WinHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    tex_shader.use();

    tex_shader.set_mat4("P", Projection);
    tex_shader.set_mat4("V", View);
    tex_shader.set_vec3("LightPosition", glm::vec3(lightPos.x, lightPos.y, lightPos.z));
    tex_shader.set_vec3("CameraPosition", glm::vec3(newCameraPos.x, newCameraPos.y, newCameraPos.z));
    tex_shader.set_float("far_plane", far);

    CrateTexture->use();

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1.0f, -6.5f, -4.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    tex_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(2.0f, -2.0f, 3.0));
    Model = glm::scale(Model, glm::vec3(0.75f));
    tex_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-3.0f, -4.0f, 3.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    tex_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-4.5f, -2.0f, -2.0));
    Model = glm::rotate(Model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    Model = glm::scale(Model, glm::vec3(0.75f));
    tex_shader.set_mat4("M", Model);
    CrateModel->render();

    WallTexture->use();

    Model = glm::mat4(1.0f);
    tex_shader.set_mat4("M", Model);

    for(int i = 0; i < 4; i++)
      walls[i]->render();

    FloorTexture->use();

    for(int i = 0; i < 2; i++)
      floors[i]->render();

    SDL_GL_SwapWindow(Window);
  }

  glDeleteTextures(1, &depthCubemap);
  glDeleteFramebuffers(1, &depthMapFBO);
  
  SDL_DestroyWindow(Window);

  return 0;
}

