#include "fluid_renderer.h"
#include "../utils/glcall.h"
#include "../utils/logger.h"
#include "../Vec.hpp"
#include <SDL2/SDL.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fluidity
{
FluidRenderer::FluidRenderer(unsigned windowWidth, unsigned windowHeight, float pointRadius)
  :   Renderer(),
  m_textureRenderer(nullptr),
  m_particleRenderPass(nullptr),
  m_depthPass(nullptr),
  m_filterPass(nullptr),
  m_currentVAO(0),
  m_uniformBufferCameraData(0),
  m_uniformBufferLights(0),
  m_uniformBufferMaterial(0),
  m_currentNumberOfParticles(0),
  m_windowWidth(windowWidth),
  m_windowHeight(windowHeight),
  m_aspectRatio((float) windowWidth / windowHeight),
  m_pointRadius(pointRadius),
  m_filteringEnabled(true),
  m_cameraController(Camera({ 9.66f, 7.73f, 5}, 45.f))
  { /* */ }

auto FluidRenderer::Clear() -> void
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

auto FluidRenderer::SetClearColor(float r, float g, float b, float a) -> void
{
  glClearColor(r, g, b, a);
}

auto FluidRenderer::SetVAO(GLuint vao) -> void
{
  m_currentVAO = vao;

  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetParticlesVAO(vao);
  }
}

auto FluidRenderer::SetNumberOfParticles(unsigned n) -> void
{
  m_currentNumberOfParticles = n;

  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetNumberOfParticles(n);
  }
}

auto FluidRenderer::SetFiltering(bool enabled) -> void
{
  m_filteringEnabled = enabled;
}

auto FluidRenderer::Init() -> bool 
{
  GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

  m_textureRenderer = new TextureRenderer();

  m_particleRenderPass = new ParticleRenderPass(
      m_windowWidth,
      m_windowHeight,
      m_currentNumberOfParticles,
      m_pointRadius,
      m_currentVAO
  );

  m_depthPass = new ParticlePass(
      m_windowWidth,
      m_windowHeight,
      m_currentNumberOfParticles,
      m_pointRadius,
      m_currentVAO,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/depth-pass.vert",
      "../../shaders/depth-pass.frag"
  );

  m_normalPass = new FilterPass(
      m_windowWidth,
      m_windowHeight,
      m_pointRadius,
      { GL_RGB32F, GL_RGB, GL_FLOAT },
      "../../shaders/normal-pass.frag"
  );

  m_filterPass = new FilterPass(
      m_windowWidth,
      m_windowHeight,
      m_pointRadius,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/filter-narrow-range.frag",
      true
  );

  m_renderPasses["ParticleRenderPass"] = m_particleRenderPass;
  m_renderPasses["DepthPass"]          = m_depthPass;
  m_renderPasses["FilterPass"]         = m_filterPass;
  m_renderPasses["NormalPass"]         = m_normalPass;

  for (auto& renderPassPair : m_renderPasses)
  {
    if (!renderPassPair.second->Init())
    {
      LOG_ERROR("Unable to initialize " + renderPassPair.first);
      return false;
    }
  }

  if(!m_textureRenderer->Init()) 
  {
    LOG_ERROR("Unable to initialize texture renderer.");
    return false;  
  }

  // TODO: Maybe these passes should be serialized, or initialized in another class?
  // Depth pass -> Init uniforms
  {
    auto& depthPassShader = m_depthPass->GetShader();
    depthPassShader.Bind();
    depthPassShader.SetUniform1i("u_UseAnisotropyKernel", 0);
    depthPassShader.SetUniform1f("u_PointRadius", (float)m_pointRadius);
    depthPassShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    depthPassShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
    depthPassShader.Unbind(); 
  }

  // Narrow filter pass -> Init uniforms
  {
    auto& narrowFilterShader = m_filterPass->GetShader();
    std::cout << "m_windowWidth: " << m_windowWidth << "\n";
    std::cout << "m_windowHeight: " << m_windowHeight << "\n";
    narrowFilterShader.Bind();
    narrowFilterShader.SetUniform1i("u_DoFilter1D", 0);
    narrowFilterShader.SetUniform1i("u_FilterSize", 5);
    narrowFilterShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    narrowFilterShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
    narrowFilterShader.SetUniform1i("u_MaxFilterSize", 100);
    narrowFilterShader.SetUniform1f("u_ParticleRadius", m_pointRadius);
    narrowFilterShader.Unbind();
  }
  
  // Normal pass -> Init uniforms 
  {
    auto& normalPassShader = m_normalPass->GetShader();
    normalPassShader.Bind();
    normalPassShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    normalPassShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
    normalPassShader.Unbind();
  }

  if (!InitUniformBuffers()) return false;

  SetUpLights();
  SetUpMaterial();

  return true;
}

auto FluidRenderer::ProcessInput(const SDL_Event& e) -> void 
{
  m_cameraController.ProcessInput(e);

  if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f)
  {
    SetFiltering(!m_filteringEnabled);
  }
}

auto FluidRenderer::InitUniformBuffers() -> bool
{
  // Init uniform buffers data
  GLCall(glGenBuffers(1, &m_uniformBufferCameraData));
  GLCall(glGenBuffers(1, &m_uniformBufferLights));
  GLCall(glGenBuffers(1, &m_uniformBufferMaterial));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferCameraData));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr, GL_DYNAMIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uniformBufferCameraData, 0, sizeof(CameraData)));

  // Size of Lights is the total number of lights + an int that stores the number of lights in the scene
  constexpr int LIGHTS_UB_SIZE = sizeof(PointLight) * NUM_TOTAL_LIGHTS + sizeof(int);
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLights));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, LIGHTS_UB_SIZE, nullptr, GL_STATIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_uniformBufferLights, 0, LIGHTS_UB_SIZE));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 2, m_uniformBufferMaterial, 0, sizeof(Material)));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

  // Setup uniform buffers on render passes
  for (auto& renderPassPair : m_renderPasses)
  {
    if (!renderPassPair.second->SetUniformBuffer("CameraData", 0))
    {
      LOG_ERROR("Unable to set CameraData uniform buffer on " + renderPassPair.first);
    }

    if (!renderPassPair.second->SetUniformBuffer("Lights", 1))
    {
      LOG_ERROR("Unable to set Lights uniform buffer on " + renderPassPair.first);
    }
    if (!renderPassPair.second->SetUniformBuffer("Material", 2))
    {
      LOG_ERROR("Unable to set Material uniform buffer on " + renderPassPair.first);
    }
  }

  return true;
}

auto FluidRenderer::SetUpLights() -> void
{
  PointLight light;
  light.ambient  = { .2f, .2f, .2f, .2f };
  light.diffuse  = { 1.f, 1.f, 1.f, 1.f };
  light.specular = { 1.f, 1.f, 1.f, 1.f };

  light.position = { -10, 20.f, 10.f, 1.f };

  PointLight light2;
  light2.ambient  = { .1f, .1f, .1f, 1.f };
  light2.diffuse  = { .3f, .3f, .3f, 1.f };
  light2.specular = { 1.f, 1.f, 1.f, 1.f };
  light2.position = { 10, -20.f, -10.f, 1.f };

  int numLights = 1;
  constexpr int numLightsFieldOffset = sizeof(PointLight) * NUM_TOTAL_LIGHTS;

  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLights);
  // Upload light 1
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PointLight), &light);

  // Upload light 2
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(PointLight), sizeof(PointLight), &light2);

  // Upload number of lights
  glBufferSubData(GL_UNIFORM_BUFFER, numLightsFieldOffset, sizeof(int), &numLights);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto FluidRenderer::SetUpMaterial() -> void
{
  Material material;
  material.ambient   = { 0.1f, 0.1f, 0.1f, 1.f };
  material.diffuse   = { 0.f, 1.f, 0.f, 1.f };
  material.specular  = { 1.f, 1.f, 1.f, 1.f };
  material.shininess = 250;

  glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &material);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

auto FluidRenderer::Update() -> void
{
  m_cameraController.Update();
}

auto FluidRenderer::Render() -> void
{
  UploadCameraData();
  m_particleRenderPass->Render();
  m_depthPass->Render();

  constexpr int FILTER_ITERATIONS = 3;
  for (int i = 0; i < FILTER_ITERATIONS; i++)
  {
    if (i == 0) m_filterPass->SetInputTexture(m_depthPass->GetBuffer());
    else m_filterPass->SwapBuffers();
    m_filterPass->Render();
  }

  m_normalPass->SetInputTexture(m_filterPass->GetBuffer());
  m_normalPass->Render();

#if true
  m_textureRenderer->SetTexture(
      m_filteringEnabled ? m_filterPass->GetBuffer() : m_normalPass->GetBuffer()
      );
#else
  m_textureRenderer->SetTexture(
     m_particleRenderPass->GetBuffer() 
  );
#endif

  Clear();
  m_textureRenderer->Render();
}

auto FluidRenderer::UploadCameraData() -> void
{
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferCameraData));

  auto view = m_cameraController.GetCamera().GetViewMatrix();
  auto projection = m_cameraController.GetCamera().GetProjectionMatrix();

  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view)));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), 
        glm::value_ptr(projection)));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2,  sizeof(glm::mat4), 
        glm::value_ptr(glm::inverse(view))));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3,  sizeof(glm::mat4), 
        glm::value_ptr(glm::inverse(projection))));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
}
