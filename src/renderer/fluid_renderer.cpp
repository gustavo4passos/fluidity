#include "fluid_renderer.h"
#include "utils/glcall.h"
#include "utils/opengl_utils.hpp"
#include "../utils/logger.h"
#include "../vec.hpp"
#include <SDL2/SDL.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
  m_cameraController(Camera({ 9.66f, 7.73f, 5 }, 45.f)),
  m_transparentFluid(true),
  m_renderShadows(true),
  m_nFilterIterations(3) //TODO: Should be configurable
  { /* */ }

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
      m_currentVAO,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/depth-pass.vert",
      "../../shaders/depth-pass.frag"
  );

  m_thicknessPass = new ParticlePass(
    m_windowWidth,
    m_windowHeight,
    m_currentNumberOfParticles,
    m_currentVAO,
    { GL_R32F, GL_RED, GL_FLOAT },
    "../../shaders/thickness-pass.vert",
    "../../shaders/thickness-pass.frag"
  );

  m_normalPass = new FilterPass(
      m_windowWidth,
      m_windowHeight,
      { GL_RGB32F, GL_RGB, GL_FLOAT },
      "../../shaders/normal-pass.frag"
  );

  m_filterPass = new FilterPass(
      m_windowWidth,
      m_windowHeight,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/filter-narrow-range.frag",
      true
  );

  m_compositionPass = new FilterPass(
    m_windowWidth,
    m_windowHeight,
    { GL_RGBA32F, GL_RGBA, GL_FLOAT },
    "../../shaders/composition-pass.frag"
  );

  m_meshesPass = new MeshesPass(
    m_windowWidth,
    m_windowHeight,
    "../../shaders/mesh.vert", 
    "../../shaders/mesh.frag",
    { 
      { GL_RGB32F, GL_RGB, GL_FLOAT },
      { GL_R32F,   GL_RED, GL_FLOAT }
    }
  );

  m_meshesShadowPass = new MeshesPass(
    m_windowWidth,
    m_windowHeight,
    "../../shaders/mesh-shadow.vert",
    "../../shaders/mesh-shadow.frag",
    {
      { GL_R32F, GL_RED, GL_FLOAT }
    }
  );

  m_renderPasses["ParticleRenderPass"] = m_particleRenderPass;
  m_renderPasses["DepthPass"]          = m_depthPass;
  m_renderPasses["FilterPass"]         = m_filterPass;
  m_renderPasses["NormalPass"]         = m_normalPass;
  m_renderPasses["CompositionPass"]    = m_compositionPass;
  m_renderPasses["ThicknessPass"]      = m_thicknessPass;
  m_renderPasses["MeshesPass"]         = m_meshesPass;
  m_renderPasses["MeshesShadowPass"]   = m_meshesShadowPass;

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

  SetUpStaticUniforms();

  // Meshes pass -> Setup
  {
    auto& meshesRenderState = m_meshesPass->GetRenderState();
    meshesRenderState.clearColor = { 234.f / 255.f, 221.f / 255.f, 202.f / 255.f, 1.f };
    Model m("C:\\dev\\FluidSimulationFiles\\Canyon\\canyon_boundary.obj");
    m.Load(true); // Smooth normals
    m_meshesPass->AddModel(m);
    m_meshesShadowPass->AddModel(m);

    auto& meshesShadowRenderState = m_meshesShadowPass->GetRenderState();
    meshesShadowRenderState.clearColor = { -1.f, -1.f, -1.f, 1.f };
  }

  if (!InitUniformBuffers()) return false;

  SetUpLights();
  SetUpMaterial();

  return true;
}

auto FluidRenderer::SetVAO(GLuint vao) -> void
{
  m_currentVAO = vao;

  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetVAO(vao);
  }
}

auto FluidRenderer::SetNumberOfParticles(unsigned n) -> void
{
  m_currentNumberOfParticles = n;

  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetNumVertices(n);
  }
}

auto FluidRenderer::SetFiltering(bool enabled) -> void
{
  m_filteringEnabled = enabled;
}

auto FluidRenderer::ProcessInput(const SDL_Event& e) -> void 
{
  m_cameraController.ProcessInput(e);

  if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_f)
  {
    SetFiltering(!m_filteringEnabled);
  }

  if (e.type == SDL_KEYUP)
  {
    switch (e.key.keysym.sym)
    {
      // Filter iterations
      case (SDLK_UP):
      {
        m_nFilterIterations++;
        break;
      } 

      case (SDLK_DOWN):
      {
        m_nFilterIterations--;
        break;
      }

      case (SDLK_t):
      {
        m_transparentFluid = !m_transparentFluid;
        auto& compositionPassShader = m_compositionPass->GetShader();
        compositionPassShader.Bind();
        compositionPassShader.SetUniform1i("u_TransparentFluid", m_transparentFluid ? 1 : 0);
        compositionPassShader.Unbind();
        break;
      }

      // Render shadows
      case (SDLK_r):
      {
        m_renderShadows = !m_renderShadows;
        break;
      }

      // Light control
      case SDLK_j:
      {
        m_lights[0].position.x -= 2.f;
        break;
      }
      case SDLK_l:
      {
        m_lights[0].position.x += 2.f;
        break;
      }
      case SDLK_i:
      {
        m_lights[0].position.y += 2.f;
        break;
      }
      case SDLK_k:
      {
        m_lights[0].position.y -= 2.f;
        break;
      }

      case SDLK_y:
      {
        m_lights[0].position.z += 2.f;
        break;
      }

      case SDLK_h:
      {
        m_lights[0].position.z -= 2.f;
        break;
      }

      default: break;
    }
  }

  if (m_nFilterIterations < 0) m_nFilterIterations = 0;
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

  light.position = { -10, 12.f, -4.f, 1.f };

  m_lights.push_back(light);

  PointLight light2;
  light2.ambient  = { .1f, .1f, .1f, 1.f };
  light2.diffuse  = { .3f, .3f, .3f, 1.f };
  light2.specular = { 1.f, 1.f, 1.f, 1.f };
  light2.position = { 10, -20.f, -10.f, 1.f };
}

auto FluidRenderer::SetUpMaterial() -> void
{
  Material material;
  material.ambient   = { 0.1f, 0.1f, 0.1f, 1.f };
  material.diffuse   = { 0.5f, 0.5f, 0.9f, 1.f };
  material.specular  = { .7f, .7f, .7f, 1.f };
  material.shininess = 250;

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &material));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

auto FluidRenderer::SetUpStaticUniforms() -> void
{
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

  // Thickness pass -> Init uniforms, set up render state
  {
    auto renderState = m_thicknessPass->GetRenderState();
    renderState.useBlend                = true;
    renderState.useDepthTest            = false;
    renderState.blendSourceFactor       = GL_ONE;
    renderState.blendDestinationFactor  = GL_ONE;
    renderState.clearColor              = Vec4{ 0.f, 0.f, 0.f, 1.f };
    m_thicknessPass->SetRenderState(renderState);

    auto& thicknessPassShader = m_thicknessPass->GetShader();
    thicknessPassShader.Bind();
    thicknessPassShader.SetUniform1i("u_UseAnisotropyKernel", 0);
    thicknessPassShader.SetUniform1f("u_PointRadius", (float)m_pointRadius * 1.2f);
    thicknessPassShader.SetUniform1f("u_PointScale", 
      (float)m_windowHeight / std::tanf(55.0 * 0.5 * 3.14159265358979323846f / 180.0));
    thicknessPassShader.SetUniform1i("u_HasSolid", 0);
    thicknessPassShader.Unbind();
  }
  
  // Narrow filter pass -> Init uniforms
  {
    auto& narrowFilterShader = m_filterPass->GetShader();
    narrowFilterShader.Bind();
    narrowFilterShader.SetUniform1i("u_DoFilter1D", 0);
    narrowFilterShader.SetUniform1i("u_FilterSize", 7);
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

  // Composoition pass -> Init uniforms 
  {
    auto& compositionPassShader = m_compositionPass->GetShader();
    compositionPassShader.Bind();
    compositionPassShader.SetUniform1i("u_HasSolid", 1);
    compositionPassShader.SetUniform1i("u_HasShadow", 0);
    compositionPassShader.SetUniform1i("u_DepthTex", 0);
    compositionPassShader.SetUniform1i("u_ThicknessTex", 1);
    compositionPassShader.SetUniform1i("u_NormalTex", 2);
    compositionPassShader.SetUniform1i("u_BackgroundTex", 3);
    compositionPassShader.SetUniform1i("u_SolidDepthMap", 4);
    compositionPassShader.SetUniform1i("u_TransparentFluid", m_transparentFluid ? 1 : 0);
    compositionPassShader.SetUniform1f("u_AttennuationConstant", 0.25f);
    compositionPassShader.SetUniform1f("u_ReflectionConstant", 0.f);
    compositionPassShader.Unbind();
  }
}

void FluidRenderer::SetUpPerFrameUniforms()
{
  // Meshes shadow pass
  // TODO: This can be done in one pass using the messes pass? Possibly
  // using alpha blending to generate depth
  {
    // If there are no lights, there's nothing to do here
    if (m_lights.size() == 0) return;

    // TODO: For now only one light is being used. How to handle multiple lights?
    auto& light1 = m_lights[0];
    // Find light matrix
    float radius = 10.f;
    glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, 1.f, 100.f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(light1.position.x, light1.position.y, light1.position.z),
      glm::vec3(0), // directional light, pointing at scene origin
      glm::vec3(0, 1.0, 0));
    
    glm::mat4 lightMatrix = lightProjection * lightView;

    auto& meshesShadowShader = m_meshesShadowPass->GetShader();
    meshesShadowShader.Bind();
    meshesShadowShader.SetUniformMat4("uLightMatrix", glm::value_ptr(lightMatrix));
    meshesShadowShader.Unbind();

    auto& meshesShader = m_meshesPass->GetShader();
    meshesShader.Bind();
    meshesShader.SetUniformMat4("uLightMatrix", glm::value_ptr(lightMatrix));
    meshesShader.SetUniform1i("uHasShadows", m_renderShadows ? 1 : 0);
    meshesShader.Unbind();
  }
}


auto FluidRenderer::Update() -> void
{
  m_cameraController.Update();
}

auto FluidRenderer::Render() -> void
{
  UploadCameraData();
  UploadLights();
  SetUpPerFrameUniforms();

  // m_particleRenderPass->Render();
  m_depthPass->Render();
  m_thicknessPass->Render();

  if (m_renderShadows)
  {
    m_meshesShadowPass->Render();
    m_meshesPass->SetInputTexture(m_meshesShadowPass->GetBuffer());
  }

  m_meshesPass->Render();
  
  for (int i = 0; i < m_nFilterIterations; i++)
  {
    if (i == 0) m_filterPass->SetInputTexture(m_depthPass->GetBuffer());
    else m_filterPass->SwapBuffers();
    m_filterPass->Render();
  }

  const auto& depthTexture = m_nFilterIterations > 0 ? m_filterPass->GetBuffer() : 
    m_depthPass->GetBuffer();

  m_normalPass->SetInputTexture(depthTexture);
  m_normalPass->Render();

  m_compositionPass->SetInputTexture(depthTexture, 0);
  m_compositionPass->SetInputTexture(m_thicknessPass->GetBuffer(), 1);
  m_compositionPass->SetInputTexture(m_normalPass->GetBuffer(), 2);
  m_compositionPass->SetInputTexture(m_meshesPass->GetBuffer(), 3);
  m_compositionPass->SetInputTexture(m_meshesPass->GetBuffer(1), 4);
  m_compositionPass->Render();

  m_textureRenderer->SetTexture(
      m_filteringEnabled ? m_compositionPass->GetBuffer() : m_meshesShadowPass->GetBuffer()
  );

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

auto FluidRenderer::UploadLights() -> void
{
  int numLights = m_lights.size();
  constexpr int numLightsFieldOffset = sizeof(PointLight) * NUM_TOTAL_LIGHTS;

  for (int i = 0; i < numLights; i++)
  {
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLights));
    // Upload light 1
    GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(PointLight) * i, sizeof(PointLight), &m_lights[i]));
  }

  // Upload number of lights
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, numLightsFieldOffset, sizeof(int), &numLights));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

}
