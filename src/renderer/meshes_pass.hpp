#pragma once
#include "renderer/render_pass.hpp"
#include "renderer/scene.hpp"
#include "renderer/model.hpp"
#include "renderer/skybox.hpp"

namespace fluidity
{

class MeshesPass : public RenderPass
{
public:
  MeshesPass(int bufferWidth,
    int bufferHeight,
    const std::string& vsFilePath,
    const std::string& fsFilePath,
    const std::vector<FramebufferAttachment> attachments,
    Scene* scene
    );

  virtual bool Init() override;
  virtual void Render() override;
  virtual void RenderSkybox(const RenderState& previousRenderState);
  virtual bool SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding) override;

  // void AddModel(const Model& model) { m_scene->models.push_back(model); };
  void AddSkybox(const Skybox& skybox);
  // std::vector<Model>& GetModels() { return m_scene->models; }
  // void RemoveModels();
  bool HasSkybox() { return m_hasSkybox; }
  void RemoveSkybox();

  Skybox& GetSkybox();

private:
  Shader* m_skybBoxShader;
  // TODO: This needs to be made const. Changes in the scene from here are not obvious
  Scene* m_scene;
  std::string m_vsFilePath;
  std::string m_fsFilePath;

  bool m_hasSkybox;
  Skybox m_skybox;
};
}
