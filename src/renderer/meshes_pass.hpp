#pragma once
#include "renderer/render_pass.hpp"
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
    const std::vector<Model>& models = {}
    );

  virtual bool Init() override;
  virtual void Render() override;
  virtual void RenderSkybox();
  virtual bool SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding) override;

  void AddModel(const Model& model) { m_models.push_back(model); };
  void AddSkybox(const Skybox& skybox);

  Skybox& GetSkybox();

private:
  std::vector<Model> m_models;

  Shader* m_skybBoxShader;
  std::string m_vsFilePath;
  std::string m_fsFilePath;

  bool m_hasSkybox;
  Skybox m_skybox;
};
}
