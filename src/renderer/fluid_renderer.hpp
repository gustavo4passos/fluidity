#pragma once

#include "renderer.h"
#include "shader.h"
#include "fluid_surfaces_renderer.h"
#include "surface_smoothing_pass.h"
#include "renderer/particle_render_pass.hpp"
#include "renderer/particle_pass.hpp"
#include "renderer/filter_pass.hpp"
#include "renderer/meshes_pass.hpp"
#include "texture_renderer.h"
#include "utils/export_directives.h"
#include "utils/camera_controller.hpp"
#include "renderer/texture.hpp"
#include "renderer/model.hpp"
#include "renderer/rendering_parameters.hpp"
#include <unordered_map>

namespace fluidity
{

class FluidRenderer : public Renderer
{
public:
  FluidRenderer(unsigned windowWidth, unsigned windowHeight, float pointRadius);
  FluidRenderer(const FluidRenderer&) = delete;

  bool Init();
  void SetVAO(GLuint vao);
  void SetNumberOfParticles(unsigned n);

  void Update() override;
  void Render() override;

  void ProcessInput(const SDL_Event& event);

  friend class GuiLayer;
private:
  bool InitUniformBuffers();
  void UploadCameraData();
  void UploadLights();
  void SetUpLights();
  void UploadMaterial(); 
  void SetUpStaticUniforms();
  void SetUpPerFrameUniforms();

  GLuint m_currentVAO;

  Shader* m_skybBoxShader;
  // Render passes
  TextureRenderer*    m_textureRenderer;
  ParticleRenderPass* m_particleRenderPass;
  ParticlePass*       m_depthPass;
  ParticlePass*       m_thicknessPass;
  FilterPass*         m_filterPass;
  FilterPass*         m_normalPass;
  FilterPass*         m_compositionPass;
  MeshesPass*         m_meshesPass;
  MeshesPass*         m_meshesShadowPass;

  CameraController m_cameraController;

  // Useful for performing operations that affect every render pass
  std::unordered_map<std::string, RenderPass*> m_renderPasses;
  std::vector<PointLight>  m_lights;
  std::vector<LightMatrix> m_lightMatrices;
  
  GLuint m_uniformBufferCameraData;
  GLuint m_uniformBufferLights;
  GLuint m_uniformBufferLightMatrices;
  GLuint m_uniformBufferMaterial;

  static constexpr int NUM_TOTAL_LIGHTS = 8;
  FilteringParameters m_filteringParameters;
  ShadowMapParameters m_shadowMapParameters;
  FluidRenderingParameters m_fluidRenderingParameters;
  Material m_fluidMaterial;

  unsigned m_currentNumberOfParticles;
  unsigned m_windowWidth;
  unsigned m_windowHeight;

  float m_aspectRatio;
  float m_pointRadius;

  bool m_filteringEnabled;
  bool m_renderShadows;
  bool m_transparentFluid;
};
}
