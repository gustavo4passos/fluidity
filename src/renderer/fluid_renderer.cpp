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
  m_currentFrame(0),
  m_lightModel("../../assets/cube.obj")
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
      0,
      0.06250,
      currentVao
  );

  m_depthPass = new ParticlePass(
      m_windowWidth,
      m_windowHeight,
      0,
      currentVao,
      { GL_R32F, GL_RED, GL_FLOAT },
      "../../shaders/depth-pass.vert",
      "../../shaders/depth-pass.frag"
  );

  m_thicknessPass = new ParticlePass(
    m_windowWidth,
    m_windowHeight,
    0,
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

  m_fluidShadowPass = new ParticlePass(
    2048,
    2048,
    0,
    currentVao,
    { GL_R32F, GL_RED, GL_FLOAT },
    "../../shaders/fluid-shadow.vs",
    "../../shaders/fluid-shadow.fs"
  );

  m_thicknessShadowPass = new ParticlePass(
    m_windowWidth,
    m_windowHeight,
    0,
    currentVao,
    { GL_R32F, GL_RED, GL_FLOAT },
    "../../shaders/thickness-shadow.vert",
    "../../shaders/thickness-shadow.frag"
  );

  m_renderPasses["ParticleRenderPass"] = m_particleRenderPass;
  m_renderPasses["DepthPass"]          = m_depthPass;
  m_renderPasses["FilterPass"]         = m_filterPass;
  m_renderPasses["NormalPass"]         = m_normalPass;
  m_renderPasses["CompositionPass"]    = m_compositionPass;
  m_renderPasses["ThicknessPass"]      = m_thicknessPass;
  m_renderPasses["MeshesPass"]         = m_meshesPass;
  m_renderPasses["MeshesShadowPass"]   = m_meshesShadowPass;
  m_renderPasses["FluidShadowPass"]    = m_fluidShadowPass;
  m_renderPasses["ThicknessShadow"]    = m_thicknessShadowPass;

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

  // Thickness shadow pass -> Setup
  {
    auto renderState = m_thicknessShadowPass->GetRenderState();
    renderState.useBlend                = true;
    renderState.useDepthTest            = false;
    renderState.blendSourceFactor       = GL_ONE;
    renderState.blendDestinationFactor  = GL_ONE;
    renderState.clearColor              = Vec4{ 0.f, 0.f, 0.f, 1.f };
    m_thicknessShadowPass->SetRenderState(renderState);
  }
  
  // Meshes pass -> Setup
  {
    auto& meshesRenderState = m_meshesPass->GetRenderState();
    meshesRenderState.clearColor = { 206.f / 255.f, 96.f / 255.f, 44.f / 255.f, 1.f };
    auto& meshesShadowRenderState = m_meshesShadowPass->GetRenderState();
    meshesShadowRenderState.clearColor = { 1.f, 1.f, 1.f, 1.f };
  }

  // Fluid shadow pass -> Setup
  {
    auto& m_fluidShadowRenderState = m_fluidShadowPass->GetRenderState();
    m_fluidShadowRenderState.useDepthTest = true;
    m_fluidShadowRenderState.clearColor = { 1.f, 1.f, 1.f, 1.f };
  }


  // Load light model - Represents the light in the scene
  m_lightModel.Load();
  m_lightModel.GetMaterial().emissive = true;
  m_lightModel.SetScale({ 0.2f, 0.2f, 0.2f });

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

  ResetPlayback();
  Play();

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

void FluidRenderer::SetCurrentFrame(int frame)
{
  m_currentFrame = frame < m_scene.fluid.GetNumberOfFrames() ? frame : m_currentFrame;
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

  constexpr int CAMERA_DATA_UB_INDEX    = 0;
  constexpr int LIGHTS_UB_INDEX         = 1;
  constexpr int LIGHT_MATRICES_UB_INDEX = 2;
  constexpr int MATERIAL_UB_INDEX       = 3;

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
  GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(UbMaterial), nullptr, GL_STATIC_DRAW));
  GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, MATERIAL_UB_INDEX, m_uniformBufferMaterial, 0, sizeof(UbMaterial)));

  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

  // Setup uniform buffers on render passes
  for (auto& renderPassPair : m_renderPasses)
  {
    if (!renderPassPair.second->SetUniformBuffer("CameraData", CAMERA_DATA_UB_INDEX))
    {
      LOG_WARNING("Unable to set CameraData uniform buffer on " + renderPassPair.first);
    }

    if (!renderPassPair.second->SetUniformBuffer("Lights", LIGHTS_UB_INDEX))
    {
      LOG_WARNING("Unable to set Lights uniform buffer on " + renderPassPair.first);
    }
    if (!renderPassPair.second->SetUniformBuffer("LightMatrices", LIGHT_MATRICES_UB_INDEX))
    {
      LOG_WARNING("Unable to set LightMatrices uniform buffer on " + renderPassPair.first);
    }
    if (!renderPassPair.second->SetUniformBuffer("Material", MATERIAL_UB_INDEX))
    {
      LOG_WARNING("Unable to set Material uniform buffer on " + renderPassPair.first);
    }
}

  return true;
}

auto FluidRenderer::UploadMaterial() -> void
{
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferMaterial));
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UbMaterial), &m_scene.fluidMaterial));
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

  // Meshes pass -> Init uniforms
  {
    auto& meshesPassSahder = m_meshesPass->GetShader();
    meshesPassSahder.Bind();
    meshesPassSahder.SetUniform1i("uShadowMap", 0);
    meshesPassSahder.SetUniform1i("uSkybox", 1);
    meshesPassSahder.SetUniform1i("uFluidShadowMap", 2);
    meshesPassSahder.SetUniform1i("uFluidShadowThicknessMap", 3);
    meshesPassSahder.Unbind(); 
  }

  // Thickness pass -> Init uniforms, set up render state
  {

    auto& thicknessPassShader = m_thicknessPass->GetShader();
    thicknessPassShader.Bind();
    thicknessPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius * 1.2f);
    thicknessPassShader.SetUniform1f("u_PointScale", 
      (float)m_windowHeight / tanf(55.0 * 0.5 * 3.14159265358979323846f / 180.0));
    thicknessPassShader.SetUniform1i("u_HasSolid", 0);
    thicknessPassShader.Unbind();
  }

  // Thickness shadow -> Init uniforms, set up render state
  {

    auto& thicknessShadowPassShader = m_thicknessShadowPass->GetShader();
    thicknessShadowPassShader.Bind();
    thicknessShadowPassShader.SetUniform1f("u_PointScale", 
      (float)m_windowHeight / tanf(55.0 * 0.5 * 3.14159265358979323846f / 180.0));
    thicknessShadowPassShader.SetUniform1i("u_LightID", 0);
    thicknessShadowPassShader.Unbind();
  }
  
  // Narrow filter pass -> Init uniforms
  {
    auto& narrowFilterShader = m_filterPass->GetShader();
    narrowFilterShader.Bind();
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
    compositionPassShader.SetUniform1i("u_FluidShadowMaps[0]", 7);
    compositionPassShader.SetUniform1i("u_FluidShadowThickness[0]", 8);
    compositionPassShader.Unbind();
  }

  // Fluid shadow pass -> Init uniforms 
  {
    auto& fluidShadowShader = m_fluidShadowPass->GetShader();
    fluidShadowShader.Bind();
    fluidShadowShader.SetUniform1i("u_ScreenWidth", m_windowWidth);
    fluidShadowShader.SetUniform1i("u_ScreenHeight", m_windowHeight);
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
  float zNear = 1.0;
  float zFar  = 100.f;
  for (int i = 0; i < m_scene.lights.size(); i++)
  {
    auto& l = m_scene.lights[i];
    float radius = 10.f;
    glm::mat4 lightProjection = glm::ortho(-radius, radius, -radius, radius, zNear, zFar);
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
  meshesShader.SetUniform1i("uRenderShadows", lightingParameters.renderShadows ? 1 : 0);
  meshesShader.SetUniform1i("uRenderFluidShadows", lightingParameters.renderFluidShadows ? 1 : 0);
  meshesShader.SetUniform1f("uMinShadowBias", lightingParameters.minShadowBias);
  meshesShader.SetUniform1f("uMaxShadowBias", lightingParameters.maxShadowBias);
  meshesShader.SetUniform1f("uShadowIntensity", lightingParameters.shadowIntensity);
  meshesShader.SetUniform1f("uFluidShadowIntensity", lightingParameters.fluidShadowIntensity);
  meshesShader.SetUniform1i("uUsePcf", lightingParameters.usePcf ? 1 : 0);
  meshesShader.Unbind();

  auto& compositionPassShader = m_compositionPass->GetShader();
  compositionPassShader.Bind();
  compositionPassShader.SetUniform1i("u_TransparentFluid", fluidParameters.transparentFluid ? 1 : 0);
  // Detail: Shadows require at least one mesh for now
  // TODO: This will change when fluid shadows are added
#ifdef ENABLE_COMPOSITION_SHADOWS
  compositionPassShader.SetUniform1i("u_HasShadow", lightingParameters.renderShadows && m_scene.models.size() > 0 ? 1 : 0);
  compositionPassShader.SetUniform1f("u_ShadowIntensity", lightingParameters.shadowIntensity);
  compositionPassShader.SetUniform1f("uMinShadowBias", lightingParameters.minShadowBias);
  compositionPassShader.SetUniform1f("uMaxShadowBias", lightingParameters.maxShadowBias);
#endif

  compositionPassShader.SetUniform1f("u_AttennuationConstant", fluidParameters.attenuation);
  compositionPassShader.SetUniform1f("uRefractionModifier", fluidParameters.refractionModifier);
  compositionPassShader.SetUniform1i("uUseRefractionMask", filteringParameters.useRefractionMask ? 1 : 0);
  compositionPassShader.Unbind();

  auto& narrowFilterShader = m_filterPass->GetShader();
  narrowFilterShader.Bind();
  narrowFilterShader.SetUniform1i("u_DoFilter1D", m_scene.filteringParameters.filter1D ? 1 : 0);
  narrowFilterShader.SetUniform1i("u_FilterSize", filteringParameters.filterSize);
  narrowFilterShader.SetUniform1i("u_MaxFilterSize", filteringParameters.maxFilterSize);
  narrowFilterShader.SetUniform1f("u_ParticleRadius", fluidParameters.pointRadius);
  narrowFilterShader.Unbind();

  auto& thicknessPassShader = m_thicknessPass->GetShader();
  thicknessPassShader.Bind();
  thicknessPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius * 1.2f);

  auto& thicknessShadowPassShader = m_thicknessShadowPass->GetShader();
  thicknessShadowPassShader.Bind();
  thicknessShadowPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius * 1.2f);

  auto& depthPassShader = m_depthPass->GetShader();
  depthPassShader.Bind();
  depthPassShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius);

  auto& fluidShadowShader = m_fluidShadowPass->GetShader();
  fluidShadowShader.Bind();
  fluidShadowShader.SetUniform1f("u_PointRadius", fluidParameters.pointRadius);
  fluidShadowShader.SetUniform1i("u_LightID", 0);

  m_textureRenderer->SetGammaCorrectionEnabled(filteringParameters.gammaCorrection);
}


auto FluidRenderer::Update() -> void
{
  m_cameraController.Update();
  if (IsPlaying()) AdvanceFrame();
}

auto FluidRenderer::Render() -> void
{
  SetUpPerFrameUniforms(); 
  GLuint depthTexture = 0;
  if (m_scene.fluid.GetNumberOfFrames() > 0)
  {
    SetVAOS();
    SetNumberOfParticles();

    m_depthPass->Render();
    m_thicknessPass->Render();

    m_fluidShadowPass->Render();
    m_thicknessShadowPass->Render();
  
    RenderMeshes();
    DoFiltering();
    
    depthTexture = m_scene.filteringParameters.nIterations > 0 ? m_filterPass->GetBuffer() : 
      m_depthPass->GetBuffer();

    m_normalPass->SetInputTexture(depthTexture);
    m_normalPass->Render();
    m_compositionPass->SetInputTexture({ depthTexture,                        0 });
    m_compositionPass->SetInputTexture({ m_thicknessPass->GetBuffer(),        1 });
    m_compositionPass->SetInputTexture({ m_normalPass->GetBuffer(),           2 });
    m_compositionPass->SetInputTexture({ m_meshesPass->GetBuffer(),           3 });
    m_compositionPass->SetInputTexture({ m_meshesPass->GetBuffer(1),          4 });
    m_compositionPass->SetInputTexture({ m_meshesShadowPass->GetBuffer(0),    5 });
    m_compositionPass->SetInputTexture({ m_filterPass->GetBuffer(0),          7 });
    m_compositionPass->SetInputTexture({ m_thicknessShadowPass->GetBuffer(0), 8 });
    
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
    // Since there is no fluid, force disable fluid shadows
    if (m_scene.lightingParameters.renderFluidShadows)
    {
      auto& meshesPassShader = m_meshesPass->GetShader();
      meshesPassShader.Bind();
      meshesPassShader.SetUniform1i("uRenderFluidShadows", 0);
      RenderMeshes();
    }
    m_textureRenderer->SetTexture(m_meshesPass->GetBuffer());
  }

  m_textureRenderer->Render();
}

auto FluidRenderer::UploadCameraData() -> void
{
  GLCall(glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBufferCameraData));

  auto view = m_cameraController.GetCamera().GetViewMatrix();
  auto projection = m_cameraController.GetCamera().GetProjectionMatrix();
  glm::vec3 position = m_cameraController.GetCamera().GetPosition();
  glm::vec4 positionV4 = glm::vec4(position, 1.0);

  // View matrix - Offset 0
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view)));
  // Projection matrix - Offset 1
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), 
        glm::value_ptr(projection)));
  // Inverse view - Offset 2
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2,  sizeof(glm::mat4), 
        glm::value_ptr(glm::inverse(view))));
  // Inverse projection - Offset 3
  GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3,  sizeof(glm::mat4), 
        glm::value_ptr(glm::inverse(projection))));
  // Shadow matrix - Offset 4
  // ... Skip ...
  // Camera position - Offset 5
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 5, sizeof(glm::vec4), glm::value_ptr(
    positionV4));
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

void FluidRenderer::RenderMeshes()
{
    if (m_scene.lightingParameters.renderShadows)
    {
      m_meshesShadowPass->Render();
      m_meshesPass->SetInputTexture(m_meshesShadowPass->GetBuffer());
    }
    // Places light model in the scene
    if (m_scene.lightingParameters.showLightsOnScene)
    {
      // Positions light model in the same position as the light, and set color accordingly
      auto lightPos = m_scene.lights[0].position;
      auto lightColor = m_scene.lights[0].diffuse;
      m_lightModel.SetTranslation({ lightPos.x, lightPos.y, lightPos.z });
      m_lightModel.GetMaterial().diffuse = { lightColor.x, lightColor.y, lightColor.z };
      m_scene.models.push_back(m_lightModel);
    }

    if (m_meshesPass->HasSkybox()) m_meshesPass->SetInputTexture({ m_meshesPass->GetSkybox()
      .GetTextureID(), 1, TextureType::Cubemap });
    else m_meshesPass->SetInputTexture({ 0, 1, TextureType::Cubemap });
    m_meshesPass->SetInputTexture({ m_fluidShadowPass->GetBuffer(), 2 });
    m_meshesPass->SetInputTexture({ m_thicknessShadowPass->GetBuffer(), 3 });

    // Set background clear color (comes from meshes rendering, for now)
    m_meshesPass->GetRenderState().clearColor = m_scene.clearColor;
    m_meshesPass->Render();

    // Remove light model from scene so it does not affect other passes
    if (m_scene.lightingParameters.showLightsOnScene) m_scene.models.pop_back();
}
void FluidRenderer::DoFiltering()
{
    if (m_scene.filteringParameters.filter1D)
    {
      auto& narrowRangeFilterShader = m_filterPass->GetShader();
      for (int i = 0; i < m_scene.filteringParameters.nIterations; i++)
      {
        if (i == 0) m_filterPass->SetInputTexture(m_depthPass->GetBuffer());
        else m_filterPass->SwapBuffers();

        narrowRangeFilterShader.Bind();
        narrowRangeFilterShader.SetUniform1i("u_FilterDirection", 0);
        m_filterPass->Render();
        m_filterPass->SwapBuffers();
        
        narrowRangeFilterShader.Bind();
        narrowRangeFilterShader.SetUniform1i("u_FilterDirection", 1);
        m_filterPass->Render();
      }
    }
    else
    {
      for (int i = 0; i < m_scene.filteringParameters.nIterations; i++)
      {
        if (i == 0) m_filterPass->SetInputTexture(m_depthPass->GetBuffer());
        else m_filterPass->SwapBuffers();
        m_filterPass->Render();
      }
    }
}

}
