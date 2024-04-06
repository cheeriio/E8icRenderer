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

void renderQuad();

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

  Shader shadowed_tex_normal_shader("shaders/ShadowedNormal.vert",
                                    NULL,
                                    "shaders/ShadowedNormal.frag");

  Shader cube_shadow_shader("shaders/CubeShadowMap.vert",
                            "shaders/CubeShadowMap.geom",
                            "shaders/CubeShadowMap.frag");
  
  Shader monocolor_shader("shaders/Monocolor.vert",
                          NULL,
                          "shaders/Monocolor.frag");
  
  Shader blur_shader("shaders/Blur.vert",
                     NULL,
                     "shaders/Blur.frag");
  
  Shader bloom_shader("shaders/Bloom.vert",
                      NULL,
                      "shaders/Bloom.frag");
  
  Shader shadowed_monocolor_shader("shaders/ShadowedMono.vert",
                                   NULL,
                                   "shaders/ShadowedMono.frag");

  std::shared_ptr<Model> CrateModel = Model::FromOBJ("models/crate.obj");
  std::shared_ptr<Model> BunnyModel = Model::FromOBJ("models/bunny.obj");
  std::shared_ptr<Model> LightbulbModel = Model::Sphere(3);

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

  shadowed_tex_normal_shader.use();
  shadowed_tex_normal_shader.set_int("DiffuseTextureSampler", 0);
  shadowed_tex_normal_shader.set_int("NormalTextureSampler", 1);
  shadowed_tex_normal_shader.set_int("DepthSampler", 2);

  blur_shader.use();
  blur_shader.set_int("Image", 0);

  bloom_shader.use();
  bloom_shader.set_int("Scene", 0);
  bloom_shader.set_int("BloomBlur", 1);

  shadowed_monocolor_shader.use();
  shadowed_monocolor_shader.set_int("DepthSampler", 2);

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

  // Utilities for HDR rendering
  GLuint hdrFBO;
  glGenFramebuffers(1, &hdrFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
  GLuint colorBuffers[2];
  glGenTextures(2, colorBuffers);
  for (unsigned int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, WinWidth, WinHeight, 0, GL_RGBA, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
    );
  }

  GLuint rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WinWidth, WinHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachments);


  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  GLuint pingpongFBO[2];
  GLuint pingpongColorbuffers[2];
  glGenFramebuffers(2, pingpongFBO);
  glGenTextures(2, pingpongColorbuffers);
  for (unsigned int i = 0; i < 2; i++) {
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F, WinWidth, WinHeight, 0, GL_RGBA, GL_FLOAT, NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0
    );
  }

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
    glm::vec4 lightPos = rotateLightX * glm::vec4(5, 0, 5, 1);

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

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1, -4.5, -3.5));
    Model = glm::scale(Model, glm::vec3(15));
    cube_shadow_shader.set_mat4("M", Model);
    BunnyModel->render();

    // Proper rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, WinWidth, WinHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shadowed_tex_normal_shader.use();

    shadowed_tex_normal_shader.set_mat4("P", Projection);
    shadowed_tex_normal_shader.set_mat4("V", View);
    shadowed_tex_normal_shader.set_vec3("LightPosition", glm::vec3(lightPos.x, lightPos.y, lightPos.z));
    shadowed_tex_normal_shader.set_vec3("CameraPosition", glm::vec3(newCameraPos.x, newCameraPos.y, newCameraPos.z));
    shadowed_tex_normal_shader.set_float("far_plane", far);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    CrateTexture->use();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1.0f, -6.5f, -4.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    shadowed_tex_normal_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(2.0f, -2.0f, 3.0));
    Model = glm::scale(Model, glm::vec3(0.75f));
    shadowed_tex_normal_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-3.0f, -4.0f, 3.0));
    Model = glm::scale(Model, glm::vec3(0.5f));
    shadowed_tex_normal_shader.set_mat4("M", Model);
    CrateModel->render();

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-4.5f, -2.0f, -2.0));
    Model = glm::rotate(Model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    Model = glm::scale(Model, glm::vec3(0.75f));
    shadowed_tex_normal_shader.set_mat4("M", Model);
    CrateModel->render();

    WallTexture->use();

    Model = glm::mat4(1.0f);
    shadowed_tex_normal_shader.set_mat4("M", Model);

    for(int i = 0; i < 4; i++)
      walls[i]->render();

    FloorTexture->use();

    for(int i = 0; i < 2; i++)
      floors[i]->render();

    monocolor_shader.use();
    monocolor_shader.set_mat4("V", View);
    monocolor_shader.set_mat4("P", Projection);

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(lightPos.x, lightPos.y, lightPos.z));
    Model = glm::scale(Model, glm::vec3(0.15));
    monocolor_shader.set_mat4("M", Model);
    monocolor_shader.set_vec3("Color", {1.5f, 1.5f, 1.5f});
    LightbulbModel->render();

    shadowed_monocolor_shader.use();
    shadowed_monocolor_shader.set_mat4("V", View);
    shadowed_monocolor_shader.set_mat4("P", Projection);
    shadowed_monocolor_shader.set_float("far_plane", far);
    shadowed_monocolor_shader.set_vec3("LightPosition", glm::vec3(lightPos.x, lightPos.y, lightPos.z));

    Model = glm::mat4(1.0f);
    Model = glm::translate(Model, glm::vec3(-1, -4.5, -3.5));
    Model = glm::scale(Model, glm::vec3(15));
    shadowed_monocolor_shader.set_mat4("M", Model);
    shadowed_monocolor_shader.set_vec3("Color", {0.5f, 0.2f, 0.9f});
    BunnyModel->render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    blur_shader.use();
    for (unsigned int i = 0; i < amount; i++) {
      glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
      blur_shader.set_int("horizontal", horizontal);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
      renderQuad();
      horizontal = !horizontal;
      if (first_iteration)
        first_iteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bloom_shader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
    bloom_shader.set_float("exposure", 1.0f);
    renderQuad();

    SDL_GL_SwapWindow(Window);
  }

  glDeleteTextures(1, &depthCubemap);
  glDeleteFramebuffers(1, &depthMapFBO);
  
  SDL_DestroyWindow(Window);

  return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
  if (quadVAO == 0) {
    float quadVertices[] = {
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

