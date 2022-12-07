#pragma once
#include "renderer/particle_render_pass.hpp"
#include "renderer/particle_render_pass.hpp"
#include "renderer/particle_pass.hpp"
#include "renderer/filter_pass.hpp"
#include "renderer/meshes_pass.hpp"
#include "renderer/texture_renderer.h"
#include "renderer/rendering_parameters.hpp"
#include "renderer/scene.hpp"
#include "utils/export_directives.h"
#include "utils/camera_controller.hpp"
#include "simulation/simulated_fluid.hpp"
#include <unordered_map>

namespace fluidity
{

class FluidRenderer : public Renderer
{
public:
  FluidRenderer(unsigned windowWidth, unsigned windowHeight, float pointRadius);
  FluidRenderer(const FluidRenderer&) = delete;

  bool Init();
  bool LoadScene();

  // Playback methods
  void Play()            { m_playing = true;       }
  void Pause()           { m_playing = false;      }
  void TogglePlayPause() { m_playing = !m_playing; }
  bool IsPlaying()       { return m_playing;       }
  void ResetPlayback()   { m_currentFrame = 0;     }
  void AdvanceFrame();
  void SetCurrentFrame(int frame);
  
  void SetScene(const Scene& scene);

  void Update() override;
  void Render() override;

  void ProcessInput(const SDL_Event& event);
  int GetCurrentFrame() { return m_currentFrame; }

  friend class GuiLayer;
private:
  bool InitUniformBuffers();
  void UploadCameraData();
  void UploadLights();
  void UploadMaterial(); 
  void SetUpStaticUniforms();
  void SetUpPerFrameUniforms();
  void RenderMeshes();
  void DoFiltering();
  void PrepareForCaustics();
  void RenderCaustics();

  void SetVAOS();
  void SetNumberOfParticles();

  void SetUpPlanes();
  void ComputePlanesParameters(glm::mat4& viewMatrix, Shader& shader);

  Shader* m_skybBoxShader;
  // Render passes
  TextureRenderer*    m_textureRenderer;
  ParticleRenderPass* m_particleRenderPass;
  ParticlePass*       m_depthPass;
  ParticlePass*       m_thicknessPass;
  ParticlePass*       m_fluidShadowPass;
  ParticlePass*       m_thicknessShadowPass;
  FilterPass*         m_filterPass;
  FilterPass*         m_normalPass;
  FilterPass*         m_compositionPass;
  FilterPass*         m_inverterPass;
  MeshesPass*         m_meshesPass;
  MeshesPass*         m_meshesShadowPass;

  // Caustics
  FilterPass*         m_causticsPass;
  MeshesPass*         m_causticsPolygonPass;
  Scene               m_causticsScene; // TODO: Only a workaround to get working for now. Should not be necessary a scene for caustics

  CameraController m_cameraController;

  // Useful for performing operations that affect every render pass
  std::unordered_map<std::string, RenderPass*> m_renderPasses;
  std::vector<LightMatrix> m_lightMatrices;
  
  GLuint m_uniformBufferCameraData;
  GLuint m_uniformBufferLights;
  GLuint m_uniformBufferLightMatrices;
  GLuint m_uniformBufferMaterial;

  static constexpr int NUM_TOTAL_LIGHTS = 8;
  Scene m_scene;
  Model m_lightModel;
  Model m_colliderModel;
  Model m_causticsModel;

  unsigned m_windowWidth;
  unsigned m_windowHeight;

  unsigned m_currentFrame = 0;
  bool m_playing = false;
  float m_aspectRatio;

#if FLUIDITY_ENABLE_SIMULATOR
public:
  void SetFluidType(FluidType type);
private:
  SimulatedFluid m_ps;
  bool InitParticleSystem();
#endif
};

}
