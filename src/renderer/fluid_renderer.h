#pragma once

#include "renderer.h"
#include "shader.h"
#include "fluid_surfaces_renderer.h"
#include "surface_smoothing_pass.h"
#include "particle_render_pass.hpp"
#include "particle_pass.hpp"
#include "filter_pass.hpp"
#include "texture_renderer.h"
#include "../utils/export_directives.h"
#include "../utils/camera_controller.hpp"
#include <unordered_map>

namespace fluidity
{

class FluidRenderer : public Renderer
{
public:
  FluidRenderer(unsigned windowWidth, unsigned windowHeight, float pointRadius);
  FluidRenderer(const FluidRenderer&) = delete;

  auto Init()  -> bool;
  auto Clear() -> void;
  auto SetVAO(GLuint vao) -> void;
  auto SetNumberOfParticles(unsigned n) -> void;
  auto Update() -> void override;
  auto Render() -> void override;
  auto SetClearColor(float r, float g, float b, float a) -> void;

  auto SetFiltering(bool enabled) -> void;
  auto GetFiltering() -> bool { return m_filteringEnabled; }
  auto ProcessInput(const SDL_Event& event) -> void;

private:
  auto InitUniformBuffers() -> bool;
  auto SetUpLights()        -> void;
  auto SetUpMaterial()      -> void; 
  auto UploadCameraData()   -> void;

  // Render passes
  ParticleRenderPass* m_particleRenderPass;
  ParticlePass*       m_depthPass;
  TextureRenderer*    m_textureRenderer;
  FilterPass*         m_filterPass;
  FilterPass*         m_normalPass;
  FilterPass*         m_compositionPass;

  GLuint m_currentVAO;

  // Useful for performing operations that affect every render pass
  std::unordered_map<std::string, RenderPass*> m_renderPasses;
  
  GLuint m_uniformBufferCameraData;
  GLuint m_uniformBufferLights;
  GLuint m_uniformBufferMaterial;

  static constexpr int NUM_TOTAL_LIGHTS = 8;

  unsigned m_currentNumberOfParticles;
  unsigned m_windowWidth;
  unsigned m_windowHeight;

  float m_aspectRatio;
  float m_pointRadius;

  bool m_filteringEnabled;

  CameraController m_cameraController;
};
}
