#include "fluid_renderer.hpp"
#include "utils/glcall.h"
#include "utils/opengl_utils.hpp"
#include "renderer/skybox.hpp"
#include "utils/logger.h"
#include "vec.hpp"
#include <SDL2/SDL.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace fluidity
{
FluidRenderer::FluidRenderer(unsigned windowWidth, unsigned windowHeight, float pointRadius)
  :   Renderer(),
  m_textureRenderer(nullptr),
  m_particleRenderPass(nullptr),
  m_depthPass(nullptr),
  m_filterPass(nullptr),
  m_uniformBufferCameraData(0),
  m_uniformBufferLights(0),
  m_uniformBufferMaterial(0),
  m_windowWidth(windowWidth),
  m_windowHeight(windowHeight),
  m_aspectRatio((float) windowWidth / windowHeight),
  m_cameraController(Camera({ 17.f, 8.f, 0.5f }, 45.f)),
  m_currentFrame(0)
  { /* */ }

auto FluidRenderer::Init() -> bool 
{
  GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

  m_textureRenderer = new TextureRenderer();

  GLuint currentVao = m_scene.fluid.GetNumberOfFrames() > 0 ? 
    m_scene.fluid.GetFrameVao(m_currentFrame) : 0;

  // Init all passes
  m_particleRenderPass = new ParticleRenderPass(
      m_windowWidth,
      m_windowHeight,
      m_scene.fluid.GetNumberOfParticles(m_currentFrame),
      0.06250,
      currentVao
  );

  m_depthPass = new ParticlePass(
      m_windowWidth,
      m_windowHeight,
      m_scene.fluid.GetNumberOfParticles(m_currentFrame),
      currentVao,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/depth-pass.vert",
      "../../shaders/depth-pass.frag"
  );

  m_thicknessPass = new ParticlePass(
    m_windowWidth,
    m_windowHeight,
    m_scene.fluid.GetNumberOfParticles(m_currentFrame),
    currentVao,
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
    },
    &m_scene
  );

  m_meshesShadowPass = new MeshesPass(
    2048, // Shadow map resolution 
    2048,
    "../../shaders/mesh-shadow.vert",
    "../../shaders/mesh-shadow.frag",
    {
      { GL_R32F, GL_RED, GL_FLOAT }
    },
    &m_scene
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

  // Thickness pass -> Setup
  {
    auto renderState = m_thicknessPass->GetRenderState();
    renderState.useBlend                = true;
    renderState.useDepthTest            = false;
    renderState.blendSourceFactor       = GL_ONE;
    renderState.blendDestinationFactor  = GL_ONE;
    renderState.clearColor              = Vec4{ 0.f, 0.f, 0.f, 1.f };
    m_thicknessPass->SetRenderState(renderState);
  }
  // Meshes pass -> Setup
  {
    auto& meshesRenderState = m_meshesPass->GetRenderState();
    meshesRenderState.clearColor = { 206.f / 255.f, 96.f / 255.f, 44.f / 255.f, 1.f };
    auto& meshesShadowRenderState = m_meshesShadowPass->GetRenderState();
    meshesShadowRenderState.clearColor = { -1.f, -1.f, -1.f, 1.f };
  }

  if (!LoadScene()) return false;
  if (!InitUniformBuffers()) return false;

  return true;
}

void FluidRenderer::SetScene(const Scene& scene)
{
  for (auto& model : m_scene.models)
  {
    model.CleanUp();
  }
  m_scene.fluid.CleanUp();

  m_scene = scene;
}

bool FluidRenderer::LoadScene()
{
  // Sanity check: Guarantes the fluid renderer has been init
  assert(m_meshesPass != nullptr);

  m_meshesPass->RemoveSkybox();
  
  if (m_scene.skyboxPath != "")
  {
    Skybox skybox(m_scene.skyboxPath);
    if (skybox.Init())
    {
      m_meshesPass->RemoveSkybox();
      m_meshesPass->AddSkybox(skybox);
    }
    else
    {
      LOG_ERROR("Unable to load skybox " + m_scene.skyboxPath);
      return false;
    }
  }

  // If there are no lights in the scene, create a default one
  if (m_scene.lights.empty())
  {
    PointLight light;
    light.ambient  = { .2f, .2f, .2f, .2f };
    light.diffuse  = { 1.f, 1.f, 1.f, 1.f };
    light.specular = { 1.f, 1.f, 1.f, 1.f };
    light.position = { -16, 24.f, 7.5f, 1.f };

    m_scene.lights.push_back(light);
  }

  m_cameraController.SetCamera(m_scene.camera);

  SetUpStaticUniforms();

  return true;
}

void FluidRenderer::AdvanceFrame()
{
  int numFrames = m_scene.fluid.GetNumberOfFrames();
  if (m_scene.fluid.GetNumberOfFrames() > 0)
  {
    m_currentFrame = (m_currentFrame + 1) % m_scene.fluid.GetNumberOfFrames();
  }
  else m_currentFrame = 0;
}

auto FluidRenderer::SetVAOS() -> void
{
  assert(m_scene.fluid.GetNumberOfFrames() > 0);
  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetVAO(m_scene.fluid.GetFrameVao(m_currentFrame));
  }
}

auto FluidRenderer::SetNumberOfParticles() -> void
{
  for (auto& renderPassPair : m_renderPasses)
  {
    renderPassPair.second->SetNumVertices(m_scene.fluid.GetNumberOfParticles(m_currentFrame));
  }
}

auto FluidRenderer::ProcessInput(const SDL_Event& e) -> void 
{
  m_cameraController.ProcessInput(e);
}

auto FluidRenderer::InitUniformBuffers() -> bool
{
  // Init uniform buffers data
  GLCall(glGenBuffers(1, &m_uniformBufferCameraData));
  GLCall(glGenBuffers(1, &m_uniformBufferLights));
  GLCall(glGenBuffers(1, &m_uniformBufferMaterial));
  GLCall(glGenBuffers(1, &m_uniformBufferLightMatrices));

  constexpr int CAMERA_DATA_UB_INDEX = 0;
  constexpr int LIGHTS_UB_INDEX = 1;
  constexpr int LIGHT_MATRICES_UB_INDEX = 2;
  constexpr int MATERIAL_UB_INDEX = 3;

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferCameraData));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr, GL_DYNAMIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_DATA_UB_INDEX, 
    m_uniformBufferCameraData, 0, sizeof(CameraData)));

  // Size of Lights is the total number of lights + an int that stores the number of lights in the scene
  constexpr int LIGHTS_UB_SIZE = sizeof(PointLight) * NUM_TOTAL_LIGHTS + sizeof(int);
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLights));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, LIGHTS_UB_SIZE, nullptr, GL_STATIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_UB_INDEX, m_uniformBufferLights, 0, 
    LIGHTS_UB_SIZE));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLightMatrices));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(LightMatrix) * NUM_TOTAL_LIGHTS, nullptr, GL_DYNAMIC_DRAW));
  glBindBufferRange(GL_UNIFORM_BUFFER, LIGHT_MATRICES_UB_INDEX, m_uniformBufferLightMatrices,
    0, sizeof(LightMatrix) * NUM_TOTAL_LIGHTS);

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial));
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), nullptr, GL_STATIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_UB_INDEX, m_uniformBufferMaterial, 0, sizeof(Material)));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

  // Setup uniform buffers on render passes
  for (auto& renderPassPair : m_renderPasses)
  {
    if (!renderPassPair.second->SetUniformBuffer("CameraData", CAMERA_DATA_UB_INDEX))
    {
      LOG_ERROR("Unable to set CameraData uniform buffer on " + renderPassPair.first);
    }

    if (!renderPassPair.second->SetUniformBuffer("Lights", LIGHTS_UB_INDEX))
    {
      LOG_ERROR("Unable to set Lights uniform buffer on " + renderPassPair.first);
    }
    if (!renderPassPair.second->SetUniformBuffer("LightMatrices", LIGHT_MATRICES_UB_INDEX))
    {
      LOG_ERROR("Unable to set LightMatrices uniform buffer on " + renderPassPair.first);
    }
    if (!renderPassPair.second->SetUniformBuffer("Material", MATERIAL_UB_INDEX))
    {
      LOG_ERROR("Unable to set Material uniform buffer on " + renderPassPair.first);
    }
}

  return true;
}

auto FluidRenderer::UploadMaterial() -> void
{
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Material), &m_scene.fluidMaterial));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

auto FluidRenderer::SetUpStaticUniforms() -> void
{
  auto& fluidParameters     = m_scene.fluidParameters;
  auto& filteringParameters = m_scene.filteringParameters;

  // TODO: Maybe these passes should be serialized, or initialized in another class?
  // Depth pass -> Init uniforms
  {
    auto& depthPassShader = m_depthPass->GetShader();
    depthPassShader.Bind();
    depthPassShader.SetUniform1i("u_UseAnisotropyKernel", 0);
    depthPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius);
    depthPassShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    depthPassShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
    depthPassShader.Unbind(); 
  }

  // Thickness pass -> Init uniforms, set up render state
  {

    auto& thicknessPassShader = m_thicknessPass->GetShader();
    thicknessPassShader.Bind();
    thicknessPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius * 1.2f);
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
    narrowFilterShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    narrowFilterShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
    narrowFilterShader.SetUniform1i("u_FilterSize", filteringParameters.filterSize);
    narrowFilterShader.SetUniform1i("u_MaxFilterSize", filteringParameters.maxFilterSize);
    narrowFilterShader.SetUniform1f("u_ParticleRadius", fluidParameters.pointRadius);
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
    compositionPassShader.SetUniform1i("u_DepthTex",            0);
    compositionPassShader.SetUniform1i("u_ThicknessTex",        1);
    compositionPassShader.SetUniform1i("u_NormalTex",           2);
    compositionPassShader.SetUniform1i("u_BackgroundTex",       3);
    compositionPassShader.SetUniform1i("u_SolidDepthMap",       4);
    compositionPassShader.SetUniform1i("u_SolidShadowMaps[0]",  5);
    compositionPassShader.SetUniform1i("u_SkyBoxTex",           6);
    compositionPassShader.SetUniform1i("u_TransparentFluid", fluidParameters.transparentFluid ? 1 : 0);
    compositionPassShader.SetUniform1f("u_ReflectionConstant", 0.f);
    compositionPassShader.Unbind();
  }
}

void FluidRenderer::SetUpPerFrameUniforms()
{
  auto& fluidParameters     = m_scene.fluidParameters;
  auto& filteringParameters = m_scene.filteringParameters;
  auto& lightingParameters  = m_scene.lightingParameters;

  UploadCameraData();
  UploadLights();
  UploadMaterial();

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLightMatrices));
  for (int i = 0; i < m_scene.lights.size(); i++)
  {
    auto& l = m_scene.lights[i];
    float radius = 10.f;
    glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, 1.f, 100.f);
    // Find light matrix
    glm::mat4 lightView = glm::lookAt(glm::vec3(l.position.x, l.position.y, l.position.z),
      glm::vec3(0), // directional light, pointing at scene origin
      glm::vec3(0, 1.0, 0));

    GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Mat4), glm::value_ptr(lightView)));
    GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Mat4), sizeof(Mat4), glm::value_ptr(lightProjection)));
  }
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

  auto& meshesShader = m_meshesPass->GetShader();
  meshesShader.Bind();
  meshesShader.SetUniform1i("uHasShadows", lightingParameters.renderShadows ? 1 : 0);
  meshesShader.SetUniform1f("uMinShadowBias", lightingParameters.minShadowBias);
  meshesShader.SetUniform1f("uMaxShadowBias", lightingParameters.maxShadowBias);
  meshesShader.SetUniform1f("uShadowIntensity", lightingParameters.shadowIntensity);
  meshesShader.SetUniform1i("uUsePcf", lightingParameters.usePcf ? 1 : 0);
  meshesShader.Unbind();

  auto& compositionPassShader = m_compositionPass->GetShader();
  compositionPassShader.Bind();
  compositionPassShader.SetUniform1i("u_TransparentFluid", fluidParameters.transparentFluid ? 1 : 0);
  // Detail: Shadows require at least one mesh for now
  // TODO: This will change when fluid shadows are added
  compositionPassShader.SetUniform1i("u_HasShadow", lightingParameters.renderShadows && m_scene.models.size() > 0 ? 1 : 0);
  compositionPassShader.SetUniform1f("u_ShadowIntensity", lightingParameters.shadowIntensity);
  compositionPassShader.SetUniform1f("u_AttennuationConstant", fluidParameters.attenuation);
  compositionPassShader.SetUniform1f("uMinShadowBias", lightingParameters.minShadowBias);
  compositionPassShader.SetUniform1f("uMaxShadowBias", lightingParameters.maxShadowBias);
  compositionPassShader.SetUniform1f("uRefractionModifier", fluidParameters.refractionModifier);
  compositionPassShader.Unbind();

  auto& narrowFilterShader = m_filterPass->GetShader();
  narrowFilterShader.Bind();
  narrowFilterShader.SetUniform1i("u_FilterSize", filteringParameters.filterSize);
  narrowFilterShader.SetUniform1i("u_MaxFilterSize", filteringParameters.maxFilterSize);
  narrowFilterShader.SetUniform1f("u_ParticleRadius", fluidParameters.pointRadius);
  narrowFilterShader.Unbind();

  auto& thicknessPassShader = m_thicknessPass->GetShader();
  thicknessPassShader.Bind();
  thicknessPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius * 1.2f);

  auto& depthPassShader = m_depthPass->GetShader();
  depthPassShader.Bind();
  depthPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius);

  m_textureRenderer->SetGammaCorrectionEnabled(filteringParameters.gammaCorrection);
}


auto FluidRenderer::Update() -> void
{
  m_cameraController.Update();
}

auto FluidRenderer::Render() -> void
{
  SetUpPerFrameUniforms();
  
  if (m_scene.lightingParameters.renderShadows)
  {
    m_meshesShadowPass->Render();
    m_meshesPass->SetInputTexture(m_meshesShadowPass->GetBuffer());
  }

  m_meshesPass->Render();

  GLuint depthTexture = 0;
  if (m_scene.fluid.GetNumberOfFrames() > 0)
  {
    SetVAOS();
    SetNumberOfParticles();

    // m_particleRenderPass->Render();
    m_depthPass->Render();
    m_thicknessPass->Render();

    for (int i = 0; i < m_scene.filteringParameters.nIterations; i++)
    {
      if (i == 0) m_filterPass->SetInputTexture(m_depthPass->GetBuffer());
      else m_filterPass->SwapBuffers();
      m_filterPass->Render();
    }

    depthTexture = m_scene.filteringParameters.nIterations > 0 ? m_filterPass->GetBuffer() : 
      m_depthPass->GetBuffer();

    m_normalPass->SetInputTexture(depthTexture);
    m_normalPass->Render();
    m_compositionPass->SetInputTexture(depthTexture, 0);
    m_compositionPass->SetInputTexture(m_thicknessPass->GetBuffer(),     1);
    m_compositionPass->SetInputTexture(m_normalPass->GetBuffer(),        2);
    m_compositionPass->SetInputTexture(m_meshesPass->GetBuffer(),        3);
    m_compositionPass->SetInputTexture(m_meshesPass->GetBuffer(1),       4);
    m_compositionPass->SetInputTexture(m_meshesShadowPass->GetBuffer(0), 5);
    
    // TODO: This needs to be done at the render pass level
    if (m_meshesPass->HasSkybox())
    {
      GLuint skyboxTextureID = m_meshesPass->GetSkybox().GetTextureID();
      glActiveTexture(GL_TEXTURE0 + 6);
      glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
    }
    m_compositionPass->Render();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    m_textureRenderer->SetTexture(m_compositionPass->GetBuffer());
  }
  else
  {
    m_textureRenderer->SetTexture(m_meshesPass->GetBuffer());
  }

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
  int numLights = m_scene.lights.size();
  constexpr int numLightsFieldOffset = sizeof(PointLight) * NUM_TOTAL_LIGHTS;

  for (int i = 0; i < numLights; i++)
  {
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferLights));
    GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(PointLight) * i, sizeof(PointLight), &m_scene.lights[i]));
  }

  // Upload number of lights
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, numLightsFieldOffset, sizeof(int), &numLights));
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

}
