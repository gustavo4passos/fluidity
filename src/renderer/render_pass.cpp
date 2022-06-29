#include "render_pass.hpp"
#include "../utils/glcall.h"
#include <cassert>

namespace fluidity
{
RenderPass::RenderPass(
  int bufferWidth,
  int bufferHeight,
  int numberOfParticles,
  const float pointRadius,
  GLuint particlesVAO)
  : m_bufferWidth(bufferWidth),
  m_bufferHeight(bufferHeight),
  m_numberOfParticles(numberOfParticles),
  m_pointRadius(pointRadius),
  m_particlesVAO(particlesVAO),
  m_framebuffer({ {}, bufferWidth, bufferHeight }) 
{ /* */ } 

bool RenderPass::Init()
{
    if (!m_framebuffer.Init()) return false;
    if (!SetUniforms())       return false;
    return true;
}

bool RenderPass::SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding)
{
  GLuint index = glGetUniformBlockIndex(m_shader->programID(), name.c_str());
  if (index == GL_INVALID_INDEX) return false;

  GLCall(glUniformBlockBinding(m_shader->programID(), index, 
        uniformBlockBinding));
  
  return true;
}

Shader& RenderPass::GetShader()
{
  assert(m_shader != nullptr);
  return *m_shader;
}

}
